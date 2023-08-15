#include "detector.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <thread>

#include <fmt/chrono.h>

#include <bgp_platform/common/types.hpp>
#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/ip.hpp>
#include <bgp_platform/utils/logger.hpp>
#include <bgp_platform/utils/strconv.hpp>
#include <bgp_platform/utils/types.hpp>

#include "file_watcher.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

namespace {
[[maybe_unused]] constexpr auto PREFIX_OUTAGE_THRESHOLD  = 0.8;
[[maybe_unused]] constexpr auto PREFIX_RESTORE_THRESHOLD = 0.8;
[[maybe_unused]] constexpr auto AS_OUTAGE_THRESHOLD      = 0.8;
[[maybe_unused]] constexpr auto AS_RESTORE_THRESHOLD     = 0.8;
}  // namespace

void Detector::Detect(fs::path route_data_path) {
  if (!fs::exists(route_data_path)) {
    throw std::invalid_argument("Route data path does not exist");
  }

  if (!fs::is_directory(route_data_path)) {
    throw std::invalid_argument("Route data path is not a directory");
  }

  do {
    std::vector<fs::path> files = ListAllFiles(route_data_path);
    std::vector<fs::path> rib_files;
    std::copy_if(begin(files), end(files), back_inserter(rib_files),
                 [](const fs::path& path) {
                   return StartsWith(path.filename().string(), "bview"sv);
                 });
    if (rib_files.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(3600));
      continue;
    }
    auto latest_rib_file = std::max_element(
        rib_files.begin(), rib_files.end(),
        [](const fs::path& path1, const fs::path& path2) {
          return path1.filename().string() < path2.filename().string();
        });
    logger.Info("Found bview file: ", latest_rib_file->c_str());
    this->route_data_.ReadRibFile(*latest_rib_file);
    break;
  } while (true);

  FileWatcher watcher(route_data_path);
  while (true) {
    fs::path new_file = watcher.WaitForNewFile();
    logger.Info("New file: ", new_file.c_str());
    std::optional<CalendarTime> time =
        GetTimeFromUpdateFileName(new_file.filename().string());
    BGP_PLATFORM_IF_UNLIKELY(!time.has_value()) {
      logger.Warn("Fail to parse update file name: ",
                  new_file.filename().string());
      continue;
    }
    logger.Info("New update file: ", new_file.c_str());

    try {
      this->database_.SetTableTime(ToTimePoint(*time));
      this->route_data_.ReadUpdateFile(new_file);
    } catch (std::exception& e) {
      logger.Errorf("Failed to process update file {}! Exception: {}",
                    new_file.c_str(), e.what());
    }
  }
}

std::optional<CalendarTime> Detector::GetTimeFromUpdateFileName(
    std::string file_name) {
  std::regex  rgx(R"(updates\.(\d{8})\.(\d{4})\.gz)");
  std::smatch res;
  if (!std::regex_match(file_name, res, rgx)) {
    return std::nullopt;
  }

  auto date_str = res[1].str();
  auto time_str = res[2].str();
  return CalendarTime {
      StringToNumber<int>({date_str.data(), 4}),
      StringToNumber<int>({date_str.data() + 4, 2}),
      StringToNumber<int>({date_str.data() + 6, 2}),
      StringToNumber<int>({time_str.data(), 2}),
      StringToNumber<int>({time_str.data() + 2, 2}),
      {}  // Second default to 0
  };
}

void Detector::OnRouteRemoved(AsNum owner_as, IPPrefix prefix,
                              TimeStamp timestamp) {
  logger.Debug("Checking outages...");

  // this->CheckPrefixOutage(owner_as, prefix, timestamp);
  this->CheckASOutage(owner_as, prefix, timestamp);
  auto country = this->route_data_.GetAsCountry(owner_as);
  if (!country.empty()) {
    // TODO: Check country outage
  }
}

void Detector::OnRouteAdded(AsNum owner_as, IPPrefix prefix,
                            TimeStamp timestamp) {
  // TODO: Not implemented

  BGP_PLATFORM_UNUSED_PARAMETER(owner_as);
  BGP_PLATFORM_UNUSED_PARAMETER(prefix);
  BGP_PLATFORM_UNUSED_PARAMETER(timestamp);

  throw std::logic_error("Not implemented: Detector::OnRouteAdded!");
}

void Detector::CheckPrefixOutage(AsNum owner_as, IPPrefix prefix,
                                 TimeStamp timestamp) {
  logger.Debug("Checking prefix outage...");
  if (!this->InBlackList(prefix)) {
    logger.Debug("Not in black list.");
    if (auto owner_as_route_info_ptr =
            this->route_data_.GetASRouteInfo(owner_as);
        owner_as_route_info_ptr != nullptr) {
      auto& owner_as_route_info = *owner_as_route_info_ptr;
      logger.Debug("Found AS route info");
      if (auto prefix_info_itr = owner_as_route_info.prefixes.find(prefix);
          prefix_info_itr != end(owner_as_route_info.prefixes)) {
        logger.Debug("Found prefix route info");
        auto  timepoint          = TimeStampToTimePoint(timestamp);
        auto& prefix_info        = prefix_info_itr->second;
        auto  unreachable_vp_num = prefix_info.unreachable_vps.size();
        auto  reachable_vp_num   = prefix_info.reachable_vps.size();
        if (!prefix_info.is_outage) {
          logger.Debug("Prefix is not outage, checking outage.");
          // Check if the prefix is outaged
          if (unreachable_vp_num > (unreachable_vp_num + reachable_vp_num) *
                                       PREFIX_OUTAGE_THRESHOLD) {
            logger.Debug("Prefix begins to outage.");
            // The prefix is outaged
            prefix_info.is_outage = true;
            // Check if need to reset outage id
            // If the month of the last outage end time is not equal to the
            // current month, reset the outage id
            if (prefix_info.last_outage_start_time != TimePoint {} &&
                ToUTCTime(prefix_info.last_outage_start_time).month !=
                    ToUTCTime(timepoint).month) {
              prefix_info.outage_id = {};
            }
            owner_as_route_info.normal_prefixes.erase(prefix);
            owner_as_route_info.outage_prefixes.insert(prefix);

            // Record the outage start information
            logger.Debug("Recording prefix outage information.");
            ID   prefix_outage_id    = ++prefix_info.outage_id;
            auto prefix_outage_event = database::models::PrefixOutageEvent {
                {
                    owner_as,
                    prefix,
                    prefix_outage_id,
                },
                {
                    this->route_data_.GetAsCountry(owner_as),
                    this->route_data_.GetAsAutName(owner_as),
                    this->route_data_.GetAsOrgName(owner_as),
                    this->route_data_.GetAsType(owner_as),
                    timepoint,
                    std::nullopt,  // NULL
                    std::nullopt,  // NULL
                    {},            // TODO: Record pre_vp_paths
                    {},            // TODO: Record eve_vp_paths
                    "",            // TODO: Record outage_level
                    "",            // TODO: Record outage_level_description
                }};
            logger.Debug("Upload prefix outage to database.");
            auto table_name = this->database_.InsertPrefixOutageEvent(
                prefix_outage_event);  // Write to database
            this->outage_events_.prefix[std::move(prefix_outage_event.key)] = {
                std::move(table_name), std::move(prefix_outage_event.value)};
            logger.Debug("Recorded prefix outage.");
          }
        } else {
          logger.Debug("Prefix is outage. Check restoration.");
          if (reachable_vp_num > (unreachable_vp_num + reachable_vp_num) *
                                     PREFIX_RESTORE_THRESHOLD) {
            logger.Debug("Prefix outage restores.");
            prefix_info.is_outage = false;

            // TODO: set_pre_pre_vp_paths

            owner_as_route_info.outage_prefixes.erase(prefix);
            owner_as_route_info.normal_prefixes.insert(prefix);

            // Record outage ending info
            logger.Debug("Record prefix outage ending info.");
            auto prefix_outage_id = prefix_info.outage_id;
            auto prefix_outage_event_key =
                database::models::PrefixOutageEvent::Key {owner_as, prefix,
                                                          prefix_outage_id};
            auto prefix_outage_event_itr =
                this->outage_events_.prefix.find(prefix_outage_event_key);
            BGP_PLATFORM_IF_UNLIKELY(prefix_outage_event_itr ==
                                     this->outage_events_.prefix.end()) {
              logger.Errorf(
                  "A prefix outage event has NOT been recorded when the outage "
                  "started: Owner AS: {}, Prefix: {}, Outage ID: {}, At: "
                  "{:%Y-%m-%d %H:%M:%S}\n",
                  ToUnderlying(owner_as), IPPrefixToString(prefix),
                  ToUnderlying(prefix_outage_id),
                  fmt::gmtime(std::chrono::system_clock::to_time_t(timepoint)));
              return;
            }
            auto& [table_name, prefix_outage_event_value] =
                prefix_outage_event_itr->second;
            auto start_time = prefix_outage_event_value.start_time;
            prefix_outage_event_value.end_time = timepoint;
            prefix_outage_event_value.duration = timepoint - start_time;

            logger.Debug("Check whether to upload to database.");
            if (this->database_.TableExists(table_name)) {
              logger.Debug("Upload to database.");
              this->database_.PrefixOutageEnd(table_name,
                                              prefix_outage_event_key,
                                              prefix_outage_event_value);
              logger.Debug("Uploaded.");
            }

            prefix_info.last_outage_start_time = start_time;

            // TODO: Set pre_vp_paths

            this->outage_events_.prefix.erase(prefix_outage_event_itr);
            logger.Debug("Recorded prefix outage ending event.");
          }
        }
      }
    }
  }
  logger.Debug("Finish checking prefix outage.");
  return;
}

void Detector::CheckASOutage(AsNum owner_as, IPPrefix prefix,
                             TimeStamp timestamp) {
  BGP_PLATFORM_UNUSED_PARAMETER(prefix);
  try {
    logger.Debug("Checking AS outage...");

    auto as_route_info = this->route_data_.GetASRouteInfo(owner_as);
    if (as_route_info != nullptr) {
      logger.Debug("AS {} has route info.", ToUnderlying(owner_as));

      std::size_t as_withdrawed_path_count = 0, as_normal_path_count = 0;
      std::size_t collector_unreachable_vps = 0, collector_reachable_vps = 0;
      std::size_t unreachable_prefix = 0, reachable_prefix = 0;
      for (auto& prefix_route_pair : as_route_info->prefixes) {
        auto& prefix_route_info = prefix_route_pair.second;
        as_withdrawed_path_count +=
            std::accumulate(begin(prefix_route_info.withdrawed_vp_paths),
                            end(prefix_route_info.withdrawed_vp_paths), 0,
                            [](auto count, auto& vp_path_pair) {
                              return count + vp_path_pair.second.size();
                            });
        as_normal_path_count += std::accumulate(
            begin(prefix_route_info.vp_paths), end(prefix_route_info.vp_paths),
            0, [](auto count, auto& vp_path_pair) {
              return count + vp_path_pair.second.size();
            });
        collector_unreachable_vps += prefix_route_info.unreachable_vps.size();
        collector_reachable_vps += prefix_route_info.reachable_vps.size();
        if (prefix_route_info.reachable_vps.empty()) {
          ++unreachable_prefix;
        } else {
          ++reachable_prefix;
        }
      }
      double as_withdrawed_path_ratio =
          as_normal_path_count + as_withdrawed_path_count == 0
              ? 0.0
              : static_cast<double>(as_withdrawed_path_count) /
                    (as_normal_path_count + as_withdrawed_path_count);
      double collector_unreachable_vp_ratio =
          collector_reachable_vps + collector_unreachable_vps == 0
              ? 0.0
              : static_cast<double>(collector_unreachable_vps) /
                    (collector_reachable_vps + collector_unreachable_vps);
      double unreachable_prefix_ratio =
          unreachable_prefix + reachable_prefix == 0
              ? 0.0
              : static_cast<double>(unreachable_prefix) /
                    (unreachable_prefix + reachable_prefix);

      if (auto owner_as_route_info_ptr =
              this->route_data_.GetASRouteInfo(owner_as);
          owner_as_route_info_ptr != nullptr) {
        auto& owner_as_route_info = *owner_as_route_info_ptr;
        auto  outage_prefix_num   = owner_as_route_info.outage_prefixes.size();
        auto  normal_prefix_num   = owner_as_route_info.normal_prefixes.size();
        auto  timepoint           = TimeStampToTimePoint(timestamp);

        {
          std::ofstream input_data(this->detector_config_.input_cache_path);
          if (!input_data) {
            throw std::runtime_error("Failed to open input cache file!");
          }
          input_data << fmt::format(
                            "{},{},{},{},{},{}", as_withdrawed_path_ratio,
                            as_normal_path_count + as_withdrawed_path_count,
                            collector_unreachable_vp_ratio,
                            collector_reachable_vps + collector_unreachable_vps,
                            unreachable_prefix_ratio,
                            unreachable_prefix + reachable_prefix)
                     << std::endl;
        }

        {
          using namespace fmt::literals;
          int exit_code = std::system(
              fmt::format(
                  "python3 ./src/detect_model/logistic_regression.py "
                  "--eval "
                  "--eval_data_path={eval_data_path} "
                  "--output_path={output_path} "
                  "--model_path={model_path}",
                  "eval_data_path"_a =
                      this->detector_config_.input_cache_path.string(),
                  "output_path"_a =
                      this->detector_config_.output_cache_path.string(),
                  "model_path"_a = this->detector_config_.model_path.string())
                  .c_str());
          if (exit_code != 0) {
            throw std::runtime_error(
                "Failed to run logistic regression model!");
          }

          std::ifstream output_data(this->detector_config_.output_cache_path);
          if (!output_data) {
            throw std::runtime_error("Failed to open output cache file!");
          }
          int is_outage;
          output_data >> is_outage;
          if (!output_data) {
            throw std::runtime_error("Failed to read output cache file!");
          }

          if (!owner_as_route_info.is_outage) {
            logger.Debug("AS is not outage. Check outage.");
            if (is_outage != 0) {
              // Outage
              logger.Debug("A new AS outaged.");
              owner_as_route_info.is_outage = true;

              // Check if need to reset outage id
              // If the month of the last outage end time is not equal to the
              // current month, reset the outage id
              if (owner_as_route_info.last_outage_start_time != TimePoint {} &&
                  ToUTCTime(owner_as_route_info.last_outage_start_time).month !=
                      ToUTCTime(timepoint).month) {
                owner_as_route_info.outage_id = {};
              }

              // Update country info
              logger.Debug("Check updating country information.");
              auto country = this->route_data_.GetAsCountry(owner_as);
              if (!country.empty()) {
                auto& country_info =
                    this->route_data_.GetOrInsertCountryRouteInfo(country);
                if (auto owner_as_in_normal_ass_itr =
                        country_info.normal_ass.find(owner_as);
                    owner_as_in_normal_ass_itr !=
                    end(country_info.normal_ass)) {
                  country_info.normal_ass.erase(owner_as_in_normal_ass_itr);
                  country_info.outage_ass.insert(owner_as);
                }
              }

              // Record AS outage info
              logger.Debug("Recording AS outage information...");
              ID   as_outage_id    = ++owner_as_route_info.outage_id;
              auto as_outage_event = database::models::ASOutageEvent {
                  {
                      owner_as,
                      as_outage_id,
                  },
                  {
                      country,  // TODO: change to Chinese name
                      this->route_data_.GetAsAutName(owner_as),
                      this->route_data_.GetAsOrgName(owner_as),
                      this->route_data_.GetAsType(owner_as),
                      timepoint,
                      std::nullopt,
                      std::nullopt,
                      outage_prefix_num + normal_prefix_num,
                      outage_prefix_num,
                      (double)outage_prefix_num /
                          (outage_prefix_num + normal_prefix_num),
                      {},  // TODO: Record pre_vp_paths
                      {},  // TODO: Record eve_vp_paths
                      {begin(owner_as_route_info.outage_prefixes),
                       end(owner_as_route_info.outage_prefixes)},
                      "",  // TODO: Record outage_level
                      "",  // TODO: Record outage_level_description
                  }};
              logger.Debug("Uploading AS outage event to database.");
              auto table_name =
                  this->database_.InsertASOutageEvent(as_outage_event);
              this->outage_events_.as[std::move(as_outage_event.key)] = {
                  std::move(table_name), false,
                  std::move(as_outage_event.value)};
              logger.Debug("Recorded AS outage event.");
            }
          } else {
            logger.Debug(
                "AS is outage. Check restoration (Not implemented yet).");
            if (is_outage == 0) {
              // TODO: Restoration in database
            }
          }
        }
      }
      logger.Debug("Finish checking AS outage.");
      return;
    }
  } catch (std::exception& e) {
    logger.Error("Failed to check AS outage: ", e.what());
  }
}

BGP_PLATFORM_NAMESPACE_END

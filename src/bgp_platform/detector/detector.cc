#include "detector.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
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
constexpr auto PREFIX_OUTAGE_THRESHOLD  = 0.8;
constexpr auto PREFIX_RESTORE_THRESHOLD = 0.8;
constexpr auto AS_OUTAGE_THRESHOLD      = 0.8;
constexpr auto AS_RESTORE_THRESHOLD     = 0.8;
}  // namespace

void Detector::Detect(fs::path route_data_path, fs::path rib_data_name,
                      int month, int start_day, int end_day) {
  if (!fs::exists(route_data_path)) {
    throw std::invalid_argument("Route data path does not exist");
  }

  if (!fs::is_directory(route_data_path)) {
    throw std::invalid_argument("Route data path is not a directory");
  }

  do {
    // std::vector<fs::path> files = ListAllFiles(route_data_path);
    // std::vector<fs::path> rib_files;
    // std::copy_if(begin(files), end(files), back_inserter(rib_files),
    //              [](const fs::path& path) {
    //                return StartsWith(path.filename().string(), "bview"sv);
    //              });
    // if (rib_files.empty()) {
    //   std::this_thread::sleep_for(std::chrono::milliseconds(3600));
    //   continue;
    // }
    // auto latest_rib_file = std::max_element(
    //     rib_files.begin(), rib_files.end(),
    //     [](const fs::path& path1, const fs::path& path2) {
    //       return path1.filename().string() < path2.filename().string();
    //     });
    // logger.Info("Found bview file: ", latest_rib_file->c_str());

    this->ReadRibFile(rib_data_name);
    break;
  } while (true);

  // FileWatcher watcher(route_data_path);
  // for (int i = 1; i <= 31; ++i) {
  //   for (int j = 0; j < 24; ++j) {
  //     for (int k = 0; k < 60; k += 5) {
  for (int i = start_day; i <= end_day; ++i) {
    for (int j = 0; j < 24; ++j) {
      for (int k = 0; k < 60; k += 5) {
        try {
          char buf[64]       = {0};
          char month_buf[16] = {0};
          std::sprintf(buf, "updates.2022%02d%02d.%02d%02d.gz", month, i, j, k);
          std::sprintf(month_buf, "2022.%02d", month);
          std::string url = std::string("https://data.ris.ripe.net/rrc00/") +
                            month_buf + '/' + buf;
          // fs::path new_file = watcher.WaitForNewFile();
          fs::path new_file = fs::path(route_data_path).append(buf);
          std::cout << buf << std::endl
                    << url << std::endl
                    << new_file << std::endl;
          logger.Info("Downloading ew file: ", new_file.c_str());
          int ret = std::system(
              fmt::format("curl \"{}\" > \"{}\"", url, new_file.c_str())
                  .c_str());
          if (ret != 0) {
            logger.Warn("Fail to download!");
            continue;
          }
          logger.Info("Downloaded.");
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
            this->ReadUpdateFile(new_file);
            if (!fs::remove(new_file)) {
              logger.Warnf("Fail to remove update file: {}!", new_file.c_str());
            }
          } catch (std::exception& e) {
            logger.Errorf("Failed to process update file {}! Exception: {}",
                          new_file.c_str(), e.what());
          }
        } catch (std::exception& e) {
          logger.Errorf(
              "Fail to process i = {}, j = {}, k = {}.\nException: {}", i, j, k,
              e.what());
        } catch (...) {
          logger.Errorf(
              "Fail to process i = {}, j = {}, k = {}.\nUnknown error.", i, j,
              k);
        }
      }
    }
  }
}

void Detector::ReadRibFile(fs::path file_path) {
  DumpedFile dumped_file = DumpBGPFile(file_path);
  logger.Info("Dumped file: ", dumped_file.path().c_str());
  std::ifstream file(dumped_file.path(), std::ios::in);
  if (!file) {
    throw std::runtime_error("Failed to open dumped file!");
  }

  bool        is_first_success_line = true;
  std::string time_string           = "";
  for (Line line; std::getline(file, line.buf); ++line.num) {
    try {
      auto fields = SplitString(line.buf, '|');
      if (fields.size() < 7) {
        logger.Warn() << "Failed to parse line: " << line.num
                      << "\n- Content: " << line.buf;
        continue;
      }

      auto               timestamp   = StringToNumber<TimeStamp>(fields[1]);
      auto               prefix      = StringToIPPrefix(fields[5]);
      auto               as_path_str = SplitString(fields[6], ' ');
      std::vector<AsNum> as_path;
      as_path.reserve(as_path_str.size());

      // Aggregated AS: {as1, as2}
      BGP_PLATFORM_IF_UNLIKELY(as_path_str.back().front() == '{') {
        BGP_PLATFORM_IF_UNLIKELY(as_path_str.back().back() != '}') {
          throw std::runtime_error("Invalid AS path!");
        }
        std::transform(
            begin(as_path_str), end(as_path_str) - 1, back_inserter(as_path),
            [](const std::string_view& as_num_str) {
              return AsNum(
                  StringToNumber<std::underlying_type_t<AsNum>>(as_num_str));
            });

        auto aggregated_as_str = SplitString(
            as_path_str.back().substr(1, as_path_str.back().size() - 2), ',');
        if (aggregated_as_str.size() > 1) {
          throw std::logic_error(
              "Multiple aggregated AS has not been implemented!");
        }
        as_path.push_back(AsNum(StringToNumber<std::underlying_type_t<AsNum>>(
            aggregated_as_str[0])));
      }
      else {
        std::transform(
            begin(as_path_str), end(as_path_str), back_inserter(as_path),
            [](const std::string_view& as_num_str) {
              return AsNum(
                  StringToNumber<std::underlying_type_t<AsNum>>(as_num_str));
            });
      }

      auto vp_num = as_path[0];
      auto as_num = as_path.back();

      BGP_PLATFORM_IF_UNLIKELY(is_first_success_line) {
        time_string =
            fmt::format("{:%Y-%m-%d %H:%M:%S}",
                        fmt::gmtime(std::chrono::system_clock::to_time_t(
                            TimeStampToTimePoint(timestamp))));
        is_first_success_line = false;
      }

      // Record reachability information of prefix
      auto& as_route_info  = this->route_info_.as_route_info_[as_num];
      auto& as_prefix_info = as_route_info.prefixes[prefix];
      as_prefix_info.reachable_vps.insert(vp_num);
      auto& vp_path_info = as_prefix_info.vp_paths[vp_num];
      if (vp_path_info.empty()) {
        vp_path_info = std::move(as_path);
      } else {
        // TODO: Question: Whether to insert back?
        vp_path_info.insert(end(vp_path_info), begin(as_path), end(as_path));
      }

      // Record country information
      auto coutry = this->init_info_.GetAsCountry(as_num);
      if (!coutry.empty()) {
        this->route_info_.country_route_info_[coutry].normal_ass.insert(as_num);
      }

      // Record the AS to which the prefix belongs
      this->route_info_.prefix_route_info_[prefix].owner_ass_.insert(as_num);

      // TODO: Build IP Prefix Tree
    } catch (std::exception& e) {
      logger.Warn() << "Failed to parse line: " << line.num
                    << "\n- Content: " << line.buf
                    << "\n- Exception: " << e.what();
      continue;
    }
  }

  if (is_first_success_line) {
    throw std::runtime_error("Failed to parse any line of rib file!");
  }

  logger.Info("Successfully parsed rib file!");
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

void Detector::ReadUpdateFile(fs::path file_path) {
  logger.Info() << "Start to read update file: " << file_path.c_str();
  DumpedFile dumped_file = DumpBGPFile(file_path);
  logger.Info() << "Dumped file: " << dumped_file.path().c_str();
  this->DetectOutage(std::move(dumped_file));
  logger.Info() << "Finished to detect outage.";
}

void Detector::DetectOutage(DumpedFile update_file) {
  std::ifstream file(update_file.path(), std::ios::in);
  if (!file) {
    throw std::runtime_error("Failed to open dumped file!");
  }

  logger.Info("Detecting outage...");

  for (Line line; std::getline(file, line.buf); ++line.num) {
    try {
      logger.Debug("Parsing line: ", line.num, " of the updating file.");

      auto fields    = SplitString(line.buf, '|');
      auto timestamp = StringToNumber<TimeStamp>(fields[1]);
      auto flag      = fields[2];
      auto vp_num =
          AsNum(StringToNumber<std::underlying_type_t<AsNum>>(fields[4]));

      if (flag == "W"sv) {
        auto prefix = StringToIPPrefix(fields[5]);
        auto time_string =
            fmt::format("{:%Y-%m-%d %H:%M:%S}",
                        fmt::gmtime(std::chrono::system_clock::to_time_t(
                            TimeStampToTimePoint(timestamp))));
        logger.Debug("Found a withdraw message.");

        if (auto prefix_info_itr =
                this->route_info_.prefix_route_info_.find(prefix);
            prefix_info_itr != end(this->route_info_.prefix_route_info_)) {
          auto& owner_ass = prefix_info_itr->second.owner_ass_;
          logger.Debug("Found owner ASs info.");
          for (auto owner_as : owner_ass) {
            if (auto owner_as_route_info_itr =
                    this->route_info_.as_route_info_.find(owner_as);
                owner_as_route_info_itr !=
                end(this->route_info_.as_route_info_)) {
              auto& owner_as_route_info = owner_as_route_info_itr->second;
              if (auto prefix_info_itr =
                      owner_as_route_info.prefixes.find(prefix);
                  prefix_info_itr != end(owner_as_route_info.prefixes)) {
                auto& prefix_info = prefix_info_itr->second;
                if (auto rechable_vps_itr =
                        prefix_info.reachable_vps.find(vp_num);
                    rechable_vps_itr != end(prefix_info.reachable_vps)) {
                  prefix_info.reachable_vps.erase(rechable_vps_itr);
                  prefix_info.unreachable_vps.insert(vp_num);
                }
                if (auto vp_path_itr = prefix_info.vp_paths.find(vp_num);
                    vp_path_itr != end(prefix_info.vp_paths)) {
                  prefix_info.vp_paths.erase(vp_path_itr);
                }
              }
            }

            logger.Debug("Checking outages...");

            this->CheckPrefixOutage(owner_as, prefix, timestamp);
            // this->CheckASOutage(owner_as, prefix, timestamp);
            auto country = this->init_info_.GetAsCountry(owner_as);
            if (!country.empty()) {
              // TODO: Check country outage
            }
          }
        }
      }
    } catch (std::exception& e) {
      logger.Warn() << "Failed to parse line: " << line.num
                    << "\n- Content: " << line.buf
                    << "\n- Exception: " << e.what();
      continue;
    }
  }
}

void Detector::CheckPrefixOutage(AsNum owner_as, IPPrefix prefix,
                                 TimeStamp timestamp) {
  logger.Debug("Checking prefix outage...");
  if (!this->InBlackList(prefix)) {
    logger.Debug("Not in black list.");
    if (auto owner_as_route_info_itr =
            this->route_info_.as_route_info_.find(owner_as);
        owner_as_route_info_itr != end(this->route_info_.as_route_info_)) {
      auto& owner_as_route_info = owner_as_route_info_itr->second;
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
                    this->init_info_.GetAsCountry(owner_as),
                    this->init_info_.GetAsAutName(owner_as),
                    this->init_info_.GetAsOrgName(owner_as),
                    this->init_info_.GetAsType(owner_as),
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
  logger.Debug("Checking AS outage...");

  if (auto owner_as_route_info_itr =
          this->route_info_.as_route_info_.find(owner_as);
      owner_as_route_info_itr != end(this->route_info_.as_route_info_)) {
    auto& owner_as_route_info = owner_as_route_info_itr->second;
    auto  outage_prefix_num   = owner_as_route_info.outage_prefixes.size();
    auto  normal_prefix_num   = owner_as_route_info.normal_prefixes.size();
    auto  timepoint           = TimeStampToTimePoint(timestamp);
    if (!owner_as_route_info.is_outage) {
      logger.Debug("AS is not outage. Check outage.");
      if (outage_prefix_num >
          (outage_prefix_num + normal_prefix_num) * AS_OUTAGE_THRESHOLD) {
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
        auto country = this->init_info_.GetAsCountry(owner_as);
        if (!country.empty()) {
          auto& country_info = this->route_info_.country_route_info_[country];
          if (auto owner_as_in_normal_ass_itr =
                  country_info.normal_ass.find(owner_as);
              owner_as_in_normal_ass_itr != end(country_info.normal_ass)) {
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
                this->init_info_.GetAsAutName(owner_as),
                this->init_info_.GetAsOrgName(owner_as),
                this->init_info_.GetAsType(owner_as),
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
        auto table_name = this->database_.InsertASOutageEvent(as_outage_event);
        this->outage_events_.as[std::move(as_outage_event.key)] = {
            std::move(table_name), false, std::move(as_outage_event.value)};
        logger.Debug("Recorded AS outage event.");
      }
    } else {
      // TODO
      logger.Debug("AS is outage. Check restoration (Not implemented yet).");
    }
  }
  logger.Debug("Finish checking AS outage.");
  return;
}

BGP_PLATFORM_NAMESPACE_END

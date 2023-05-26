#include "outage_data_generator.hpp"

#include <algorithm>
#include <numeric>
#include <thread>
#include <tuple>
#include <vector>

#include <jsoncons_ext/csv/csv.hpp>

#include <bgp_platform/utils/logger.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

OutageDataGenerator::OutageDataGenerator(const InitConfig& config)
    : route_data_(*this, config.init_info_file_path),
      output_file_(config.output_file_path, std::ios::out),
      start_monitor_time_(config.start_monitor_time),
      end_monitor_time_(config.end_monitor_time) {
  if (!output_file_) {
    throw std::runtime_error("Cannot open output file: " +
                             config.output_file_path.string());
  }

  logger.Info("Reading outage data...");

  std::ifstream proto_data_file(config.proto_data_file_path, std::ios::in);
  if (!proto_data_file) {
    throw std::runtime_error("Cannot open outage proto data file: " +
                             config.proto_data_file_path.string());
  }

  proto_data_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  auto outage_proto_data = jsoncons::csv::decode_csv<std::vector<
      std::tuple<std::underlying_type_t<AsNum>, TimeStamp, TimeStamp>>>(
      proto_data_file);
  for (const auto& outage_event : outage_proto_data) {
    auto [as_num, start_time, end_time] = outage_event;
    this->outage_data_[AsNum(as_num)].emplace_back(start_time, end_time);
  }

  this->output_file_ << "Outage,"
                        "AS Withdrawed Path Ratio,"
                        "AS Path Count,"
                        "Collector Unreachable VP Ratio,"
                        "Collector VP Count,"
                        "Unreachable Prefix Ratio,"
                        "Prefix Count"
                     << std::endl;
  logger.Info("Done.");
}

void OutageDataGenerator::Generate(fs::path route_data_path) {
  if (!fs::exists(route_data_path)) {
    throw std::invalid_argument("Route data path does not exist");
  }

  if (!fs::is_directory(route_data_path)) {
    throw std::invalid_argument("Route data path is not a directory");
  }

  logger.Info("Finding Rib File...");
  do {
    std::vector<fs::path> files = ListAllFiles(route_data_path);
    std::vector<fs::path> rib_files;
    std::copy_if(begin(files), end(files), back_inserter(rib_files),
                 [](const fs::path& path) {
                   return StartsWith(path.filename().string(), "bview"sv);
                 });
    if (rib_files.empty()) {
      logger.Info("No bview file found, waiting...");
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

  {
    logger.Info("Finding Update Files...");
    std::vector<fs::path> files = ListAllFiles(route_data_path);
    std::vector<fs::path> update_files;
    std::copy_if(begin(files), end(files), back_inserter(update_files),
                 [](const fs::path& path) {
                   return StartsWith(path.filename().string(), "update"sv);
                 });
    std::sort(begin(update_files), end(update_files),
              [](const fs::path& path1, const fs::path& path2) {
                return path1.filename().string() < path2.filename().string();
              });
    for (const auto& update_file : update_files) {
      logger.Info("Found update file: ", update_file.c_str());
      try {
        this->route_data_.ReadUpdateFile(update_file);
      } catch (const std::exception& e) {
        logger.Errorf("Failed to process update file {}! Exception: {}",
                      update_file.c_str(), e.what());
      }
    }
  }

  logger.Info("Done.");
}

void OutageDataGenerator::OnRouteRemoved(AsNum owner_as, IPPrefix prefix,
                                         TimeStamp timestamp) {
  BGP_PLATFORM_UNUSED_PARAMETER(prefix);

  logger.Debug("One route removed.");

  if (timestamp < this->start_monitor_time_ ||
      timestamp > this->end_monitor_time_) {
    return;
  }

  logger.Debug("Checking outage...");

  if (auto as_outage_info_itr = this->outage_data_.find(owner_as);
      as_outage_info_itr != end(this->outage_data_)) {
    const auto& outage_times  = as_outage_info_itr->second;
    auto        as_route_info = this->route_data_.GetASRouteInfo(owner_as);
    if (as_route_info != nullptr) {
      if (!outage_times.empty()) {
        auto time_range = std::upper_bound(
            begin(outage_times), end(outage_times), timestamp,
            [](TimeStamp                              timestamp,
               const std::pair<TimeStamp, TimeStamp>& outage_time) {
              return timestamp < outage_time.first;
            });

        bool is_outage = false;
        if (time_range != begin(outage_times)) {
          --time_range;
          if (timestamp >= time_range->first &&
              timestamp <= time_range->second) {
            is_outage = true;
          }
        } else {
          is_outage = false;
        }

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
          as_normal_path_count +=
              std::accumulate(begin(prefix_route_info.vp_paths),
                              end(prefix_route_info.vp_paths), 0,
                              [](auto count, auto& vp_path_pair) {
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

        this->output_file_
            << fmt::format("{},{},{},{},{},{},{}", is_outage ? 1 : 0,
                           as_withdrawed_path_ratio,
                           as_normal_path_count + as_withdrawed_path_count,
                           collector_unreachable_vp_ratio,
                           collector_reachable_vps + collector_unreachable_vps,
                           unreachable_prefix_ratio,
                           unreachable_prefix + reachable_prefix)
            << std::endl;
      }
    }
  }
}

void OutageDataGenerator::OnRouteAdded(AsNum owner_as, IPPrefix prefix,
                                       TimeStamp timestamp) {
  BGP_PLATFORM_UNUSED_PARAMETER(owner_as);
  BGP_PLATFORM_UNUSED_PARAMETER(prefix);
  BGP_PLATFORM_UNUSED_PARAMETER(timestamp);
}

BGP_PLATFORM_NAMESPACE_END

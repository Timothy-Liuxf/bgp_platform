#include "detector.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <thread>

#include <fmt/chrono.h>

#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/ip.hpp>
#include <bgp_platform/utils/strconv.hpp>

#include "file_watcher.hpp"
#include "outage_event.hpp"
#include "types.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

Detector::Detector(fs::path as_info_path, fs::path top_nx_path,
                   fs::path top_ip_path)
    : init_info_(as_info_path, top_nx_path, top_ip_path) {}

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
    std::cout << "Found bview file: " << latest_rib_file->c_str() << std::endl;
    this->ReadRibFile(*latest_rib_file);
    break;
  } while (true);

  FileWatcher watcher(route_data_path);
  while (true) {
    fs::path new_file = watcher.WaitForNewFile();
    std::cout << "New file: " << new_file.c_str() << std::endl;
    if (!StartsWith(new_file.filename().string(), "update"sv)) {
      continue;
    }
    std::cout << "New update file: " << new_file.c_str() << std::endl;

    try {
      // TODO: Update table name
      this->ReadUpdateFile(new_file);
    } catch (std::exception& e) {
      std::cerr << "[ERROR] Failed to process update file: " << e.what()
                << std::endl;
    }
  }
}

void Detector::ReadRibFile(fs::path file_path) {
  DumpedFile dumped_file = DumpBGPFile(file_path);
  std::cout << "Dumped file: " << dumped_file.path().c_str() << std::endl;
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
        std::cout << "[WARNING] Failed to parse line: " << line.num
                  << std::endl;
        continue;
      }

      auto               timestamp   = StringToNumber<TimeStamp>(fields[1]);
      auto               prefix      = StringToIPPrefix(fields[5]);
      auto               as_path_str = SplitString(fields[6], ' ');
      std::vector<AsNum> as_path;
      as_path.reserve(as_path_str.size());
      std::transform(
          begin(as_path_str), end(as_path_str), back_inserter(as_path),
          [](const std::string_view& as_num_str) {
            return AsNum(
                StringToNumber<std::underlying_type_t<AsNum>>(as_num_str));
          });
      auto vp_num = as_path[0];
      auto as_num = as_path.back();

      BGP_PLATFORM_IF_UNLIKELY(is_first_success_line) {
        time_string =
            fmt::format("{:%Y-%m-%d %H:%M:%S}",
                        fmt::gmtime(std::chrono::system_clock::to_time_t(
                            TimpStampToTimePoint(timestamp))));
        is_first_success_line = false;
      }

      // Record reachability information of prefix
      auto& as_route_info  = this->route_info_.as_route_info_[as_num];
      auto& as_prefix_info = as_route_info.prefixes[prefix];
      as_prefix_info.reachable_vps.insert(vp_num);
      as_prefix_info.vp_paths[vp_num] = std::move(as_path);

      // Record country information
      auto coutry                     = this->init_info_.GetAsCountry(as_num);
      if (!coutry.empty()) {
        this->route_info_.country_route_info_[coutry].normal_ass.insert(as_num);
      }

      // Record the AS to which the prefix belongs
      this->route_info_.prefix_route_info_[prefix].owner_ass_.insert(as_num);

      // TODO: Build IP Prefix Tree
    } catch (std::exception& e) {
      std::cout << "[WARNING] Failed to parse line: " << line.num << '\n';
      std::cout << "- Exception: " << e.what() << std::endl;
      continue;
    }
  }

  if (is_first_success_line) {
    throw std::runtime_error("Failed to parse any line of rib file");
  }
}

void Detector::ReadUpdateFile(fs::path file_path) {
  std::cout << "Start to read update file: " << file_path.c_str() << std::endl;
  DumpedFile dumped_file = DumpBGPFile(file_path);
  std::cout << "Dumped file: " << dumped_file.path().c_str() << std::endl;
  this->DetectOutage(std::move(dumped_file));
  std::cout << "Finished to detect outage" << std::endl;
}

void Detector::DetectOutage(DumpedFile update_file) {
  std::ifstream file(update_file.path(), std::ios::in);
  if (!file) {
    throw std::runtime_error("Failed to open dumped file!");
  }

  for (Line line; std::getline(file, line.buf); ++line.num) {
    try {
      auto fields    = SplitString(line.buf, '|');
      auto timestamp = StringToNumber<TimeStamp>(fields[1]);
      auto flag      = fields[2];
      auto vp_num =
          AsNum(StringToNumber<std::underlying_type_t<AsNum>>(fields[4]));
      auto prefix = StringToIPPrefix(fields[5]);
      auto time_string =
          fmt::format("{:%Y-%m-%d %H:%M:%S}",
                      fmt::gmtime(std::chrono::system_clock::to_time_t(
                          TimpStampToTimePoint(timestamp))));

      if (flag == "W"sv) {
        if (auto prefix_info_itr =
                this->route_info_.prefix_route_info_.find(prefix);
            prefix_info_itr != end(this->route_info_.prefix_route_info_)) {
          auto& owner_ass = prefix_info_itr->second.owner_ass_;
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

            this->CheckPrefixOutage(owner_as, prefix, timestamp);
            // TODO: Check AS outages
            auto country = this->init_info_.GetAsCountry(owner_as);
            if (!country.empty()) {
              // TODO: Check country outage
            }
          }
        }
      }
    } catch (std::exception& e) {
      std::cout << "[WARNING] Failed to parse line: " << line.num << '\n';
      std::cout << "- Exception: " << e.what() << std::endl;
      continue;
    }
  }
}

void Detector::CheckPrefixOutage(AsNum owner_as, IPPrefix prefix,
                                 TimeStamp timestamp) {
  if (!this->InBlackList(prefix)) {
    if (auto owner_as_route_info_itr =
            this->route_info_.as_route_info_.find(owner_as);
        owner_as_route_info_itr != end(this->route_info_.as_route_info_)) {
      auto& owner_as_route_info = owner_as_route_info_itr->second;
      if (auto prefix_info_itr = owner_as_route_info.prefixes.find(prefix);
          prefix_info_itr != end(owner_as_route_info.prefixes)) {
        auto& prefix_info        = prefix_info_itr->second;
        auto  unreachable_vp_num = prefix_info.unreachable_vps.size();
        auto  reachable_vp_num   = prefix_info.reachable_vps.size();
        if (!prefix_info.is_outage) {
          // Check if the prefix is outaged
          if (unreachable_vp_num >
              (unreachable_vp_num + reachable_vp_num) * 0.8) {
            // The prefix is outaged
            prefix_info.is_outage = true;
            // Check if need to reset outage id
            // If the month of the last outage end time is not equal to the
            // current month, reset the outage id
            if (prefix_info.last_outage_start_time != TimePoint {} &&
                ToUTCTime(prefix_info.last_outage_start_time).month !=
                    ToUTCTime(timestamp).month) {
              prefix_info.outage_id = {};
            }
            owner_as_route_info.normal_prefixes.erase(prefix);
            owner_as_route_info.outage_prefixes.insert(prefix);

            // Record the outage start information
            ID          prefix_outage_id = ++prefix_info.outage_id;
            OutageEvent outage_event {
                {
                    owner_as,
                    prefix,
                    prefix_outage_id,
                },
                {
                    "",  // TODO: Get the name of the table
                    this->init_info_.GetAsCountry(owner_as),
                    this->init_info_.GetAsAutName(owner_as),
                    this->init_info_.GetAsOrgName(owner_as),
                    this->init_info_.GetAsType(owner_as),
                    TimePoint(Duration(timestamp)),
                    TimePoint {},
                    Duration {},
                    {},  // TODO: Record pre_vp_paths
                    "",  // TODO: Record outage_level
                    "",  // TODO: Record outage_level_description
                }};
            (void)outage_event;
            // TODO: Write to database
          }
        } else {
          // TODO: Check if the prefix is recovered
        }
      }
    }
  }
  return;
}

BGP_PLATFORM_NAMESPACE_END

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

#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/ip.hpp>
#include <bgp_platform/utils/strconv.hpp>

#include "file_watcher.hpp"
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
    using namespace std::literals;
    std::vector<fs::path> files = ListAllFiles(route_data_path);
    std::vector<fs::path> rib_files;
    std::copy_if(
        begin(files), end(files), back_inserter(rib_files),
        [](const fs::path& path) {
          return std::string_view(path.filename().string()).substr(0, 5) ==
                 "bview"sv;
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
  }
}

void Detector::ReadRibFile(fs::path rib_path) {
  DumpedFile dumped_file = DumpBGPFile(rib_path);
  std::cout << "Dumped file: " << dumped_file.path().c_str() << std::endl;
  std::ifstream file(dumped_file.path(), std::ios::in);
  if (!file) {
    throw std::runtime_error("Failed to open dumped file");
  }

  bool        is_first_success_line = true;
  std::string time_string           = "";
  for (Line line; std::getline(file, line.buf); ++line.num) {
    auto fields = SplitString(line.buf, '|');
    if (fields.size() < 7) {
      std::cout << "[WARNING] Failed to parse line: " << line.num << std::endl;
      continue;
    }
    try {
      auto               timestamp   = StringViewToNumber<TimeStamp>(fields[1]);
      auto               prefix      = StringToIPPrefix(fields[5]);
      auto               as_path_str = SplitString(fields[6], ' ');
      std::vector<AsNum> as_path;
      as_path.reserve(as_path_str.size());
      std::transform(
          begin(as_path_str), end(as_path_str), back_inserter(as_path),
          [](const std::string_view& as_num_str) {
            return AsNum(
                StringViewToNumber<std::underlying_type_t<AsNum>>(as_num_str));
          });
      auto vp_num = as_path[0];
      auto as_num = as_path.back();

      BGP_PLATFORM_IF_UNLIKELY(is_first_success_line) {
        auto               utc_time = ToUTCTime(timestamp);
        std::ostringstream string_builder;
        string_builder << utc_time.year << "-" << utc_time.month << "-"
                       << utc_time.day << " " << utc_time.hour << ":"
                       << utc_time.minute << ":" << utc_time.second;
        time_string           = string_builder.str();
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
      std::cout << "[WARNING] Failed to parse line: " << line.num << std::endl;
      std::cout << "- Exception: " << e.what() << std::endl;
      continue;
    }
  }

  if (is_first_success_line) {
    throw std::runtime_error("Failed to parse any line of rib file");
  }
}

BGP_PLATFORM_NAMESPACE_END

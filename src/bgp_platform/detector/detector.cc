#include "detector.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <thread>

#include "file_watcher.hpp"

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
    std::vector<fs::path> files = list_all_files(route_data_path);
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
    std::cout << "Found bview file: " << latest_rib_file->string() << std::endl;
    break;
  } while (true);

  FileWatcher watcher(route_data_path);
  while (true) {
    fs::path new_file = watcher.WaitForNewFile();
    std::cout << "New file: " << new_file.string() << std::endl;
  }
}

BGP_PLATFORM_NAMESPACE_END

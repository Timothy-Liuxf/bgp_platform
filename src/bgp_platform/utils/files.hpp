#ifndef BGP_PLATFORM_UTILS_FILES_HPP_
#define BGP_PLATFORM_UTILS_FILES_HPP_

#include <filesystem>
#include <vector>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

namespace fs = std::filesystem;

inline std::vector<fs::path> list_all_files(fs::path path) {
  std::vector<fs::path> files;
  for (const auto& entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file()) {
      files.emplace_back(entry.path());
    }
  }
  return files;
}

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_UTILS_FILES_HPP_

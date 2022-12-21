#ifndef BGP_PLATFORM_UTILS_FILES_HPP_
#define BGP_PLATFORM_UTILS_FILES_HPP_

#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

namespace fs = std::filesystem;

[[nodiscard]] inline std::vector<fs::path> ListAllFiles(fs::path path) {
  std::vector<fs::path> files;
  for (const auto& entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file()) {
      files.emplace_back(entry.path());
    }
  }
  return files;
}

class DumpedFile {
 public:
  DumpedFile(const fs::path& path) : path_(path) {}
  DumpedFile(const DumpedFile&) = delete;
  DumpedFile(DumpedFile&&)      = default;
  ~DumpedFile() { fs::remove(this->path_); }

  const fs::path& path() const { return this->path_; }

 private:
  fs::path path_;
};

[[nodiscard]] inline DumpedFile DumpBGPFile(fs::path path) {
  fs::path temp_path =
      fs::temp_directory_path() / path.filename().concat(".dmp");
  std::cout << "Dumping file: " << path.c_str() << " to " << temp_path.c_str()
            << std::endl;
  std::ostringstream cmd;
  cmd << "bgpdump -m " << path << " > " << temp_path;
  int ret = std::system(cmd.str().c_str());
  if (ret != 0) {
    throw std::runtime_error("Failed to dump file");
  }
  return DumpedFile(temp_path);
}

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_UTILS_FILES_HPP_

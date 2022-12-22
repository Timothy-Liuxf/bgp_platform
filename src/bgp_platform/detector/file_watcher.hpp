#ifndef BGP_PLATFORM_DETECTOR_FILE_WATCHER_HPP_
#define BGP_PLATFORM_DETECTOR_FILE_WATCHER_HPP_

#include <atomic>
#include <memory>
#include <thread>

#include <bgp_platform/utils/concurrent_queue.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/files.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

class FileWatcher {
 public:
  explicit FileWatcher(fs::path watch_path);

  [[nodiscard]] fs::path WaitForNewFile();

 private:
  concurrent_queue<fs::path>   new_files_queue_;
  std::unique_ptr<std::thread> watch_thread_;
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_DETECTOR_FILE_WATCHER_HPP_

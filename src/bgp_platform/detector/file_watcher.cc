#include "file_watcher.hpp"

#include <inotify-cpp/NotifierBuilder.h>

BGP_PLATFORM_NAMESPACE_BEGIN

FileWatcher::FileWatcher(fs::path watch_path) {
  this->watch_thread_ = std::make_unique<std::thread>([&, this]() {
    auto events             = {inotify::Event::close_write};
    auto handleNotification = [this](inotify::Notification notification) {
      this->new_files_queue_.emplace(notification.path);
    };
    inotify::BuildNotifier()
        .watchPathRecursively(watch_path)
        .onEvents(events, handleNotification)
        .onUnexpectedEvent([](auto) noexcept {})
        .run();
  });
}

fs::path FileWatcher::WaitForNewFile() { return this->new_files_queue_.pop(); }

BGP_PLATFORM_NAMESPACE_END

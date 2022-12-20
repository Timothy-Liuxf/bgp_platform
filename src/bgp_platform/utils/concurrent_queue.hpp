#ifndef BGP_PLATFORM_UTILS_CONCURRENT_QUEUE_HPP_
#define BGP_PLATFORM_UTILS_CONCURRENT_QUEUE_HPP_

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

template <typename Elem>
class concurrent_queue {
 private:
  using queue_type = ::std::queue<Elem>;

 public:
  using size_type       = typename queue_type::size_type;
  using value_type      = typename queue_type::value_type;
  using reference       = typename queue_type::reference;
  using const_reference = typename queue_type::const_reference;
  using container_type  = typename queue_type::container_type;

 public:
  concurrent_queue()                                   = default;
  concurrent_queue(const concurrent_queue&)            = delete;
  concurrent_queue& operator=(const concurrent_queue&) = delete;
  ~concurrent_queue()                                  = default;

 public:
  template <typename... Args>
  void emplace(Args&&... args) {
    std::unique_lock lock(this->mtx_);
    this->queue_.emplace(std::forward<Args>(args)...);
    this->cond_.notify_one();
  }

  void push(const value_type& value) {
    std::unique_lock lock(this->mtx_);
    this->queue_.push(value);
    this->cond_.notify_one();
  }

  [[nodiscard]] size_type size() const {
    std::unique_lock lock(this->mtx_);
    return this->queue_.size();
  }

  [[nodiscard]] bool empty() const {
    std::unique_lock lock(this->mtx_);
    return this->queue_.empty();
  }

  void clear() {
    std::unique_lock lock(this->mtx_);
    while (!this->queue_.empty()) {
      this->queue_.pop();
    }
  }

  [[nodiscard]] value_type pop() {
    std::unique_lock lock(this->mtx_);
    this->cond_.wait(lock, [this] { return !this->queue_.empty(); });
    auto value = std::move(this->queue_.front());
    this->queue_.pop();
    return value;
  }

  [[nodiscard]] std::optional<value_type> try_pop() {
    std::unique_lock lock(this->mtx_);
    if (this->queue_.empty()) {
      return std::nullopt;
    }
    auto value =
        std::make_optional<value_type>(std::move(this->queue_.front()));
    this->queue_.pop();
    return value;
  }

 private:
  mutable std::mutex      mtx_;
  std::queue<Elem>        queue_;
  std::condition_variable cond_;
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_UTILS_CONCURRENT_QUEUE_HPP_

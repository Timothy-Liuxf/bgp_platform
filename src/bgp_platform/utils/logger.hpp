#ifndef BGP_PLATFORM_UTILS_LOGGER_HPP_
#define BGP_PLATFORM_UTILS_LOGGER_HPP_

#include <iostream>
#include <string_view>
#include <utility>

#include <fmt/chrono.h>

#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/defs.hpp>

#define BGP_PLATFORM_LOG_NONE  0
#define BGP_PLATFORM_LOG_ERROR 1
#define BGP_PLATFORM_LOG_WARN  2
#define BGP_PLATFORM_LOG_INFO  3
#define BGP_PLATFORM_LOG_DEBUG 4

#if defined(BGP_PLATFORM_DISABLE_LOG)
#define BGP_PLATFORM_LOG_LEVEL BGP_PLATFORM_LOG_NONE
#elif defined(BGP_PLATFORM_ENABLE_LOG_DEBUG)
#define BGP_PLATFORM_LOG_LEVEL BGP_PLATFORM_LOG_DEBUG
#elif defined(BGP_PLATFORM_ENABLE_LOG_INFO)
#define BGP_PLATFORM_LOG_LEVEL BGP_PLATFORM_LOG_INFO
#elif defined(BGP_PLATFORM_ENABLE_LOG_WARN)
#define BGP_PLATFORM_LOG_LEVEL BGP_PLATFORM_LOG_WARN
#elif defined(BGP_PLATFORM_ENABLE_LOG_ERROR)
#define BGP_PLATFORM_LOG_LEVEL BGP_PLATFORM_LOG_ERROR
#else
#define BGP_PLATFORM_LOG_LEVEL BGP_PLATFORM_LOG_INFO
#endif

BGP_PLATFORM_NAMESPACE_BEGIN

namespace details {

using namespace std::literals::string_view_literals;

static constexpr inline std::string_view kBeginRed    = "\033[31m"sv;
static constexpr inline std::string_view kEndRed      = "\033[0m"sv;
static constexpr inline std::string_view kBeginGreen  = "\033[32m"sv;
static constexpr inline std::string_view kEndGreen    = "\033[0m"sv;
static constexpr inline std::string_view kBeginYellow = "\033[33m"sv;
static constexpr inline std::string_view kEndYellow   = "\033[0m"sv;
static constexpr inline std::string_view kBeginBlue   = "\033[34m"sv;
static constexpr inline std::string_view kEndBlue     = "\033[0m"sv;

}  // namespace details

class Logger {
 public:
  class LogHelper {
   private:
    LogHelper(std::ostream& os, std::string_view postfix)
        : os_(os), postfix_(postfix) {}
    LogHelper(const LogHelper&)            = delete;
    LogHelper(LogHelper&&)                 = delete;
    LogHelper& operator=(const LogHelper&) = delete;
    LogHelper& operator=(LogHelper&&)      = delete;

   public:
    ~LogHelper() noexcept(false) { this->os_ << this->postfix_ << std::endl; }

   public:
    template <typename Val>
    LogHelper& operator<<(Val&& val) {
      this->os_ << std::forward<Val>(val);
      return *this;
    }

    LogHelper& operator<<(std::ostream& (*func)(std::ostream&)) {
      this->os_ << func;
      return *this;
    }

   private:
    std::ostream&    os_;
    std::string_view postfix_;

    friend class Logger;
  };

  class EmptyLogHelper {
   public:
    template <typename Val>
    EmptyLogHelper& operator<<(Val&&) {
      return *this;
    }

    EmptyLogHelper& operator<<(std::ostream& (*)(std::ostream&)) {
      return *this;
    }
  };

 public:
#if BGP_PLATFORM_LOG_LEVEL >= BGP_PLATFORM_LOG_DEBUG
  LogHelper Debug() {
    std::cout << details::kBeginGreen;
    Logger::PrintTime(std::cout);
    std::cout << "[debug] ";
    return LogHelper(std::cout, details::kEndGreen);
  }
#else
  EmptyLogHelper Debug() { return EmptyLogHelper(); }
#endif  // BGP_PLATFORM_LOG_LEVEL >= BGP_PLATFORM_LOG_DEBUG

  template <typename... Vals>
  void Debug(Vals&&... vals) {
    (this->Debug() << ... << std::forward<Vals>(vals));
  }

  template <typename Format, typename... Args>
  void Debugf(Format&& fmt, Args&&... args) {
    this->Debug() << fmt::format(std::forward<Format>(fmt),
                                 std::forward<Args>(args)...);
  }

#if BGP_PLATFORM_LOG_LEVEL >= BGP_PLATFORM_LOG_INFO
  LogHelper Info() {
    Logger::PrintTime(std::cout);
    std::cout << "[info] ";
    return LogHelper(std::cout, {});
  }
#else
  EmptyLogHelper Info() { return EmptyLogHelper(); }
#endif  // BGP_PLATFORM_LOG_LEVEL >= BGP_PLATFORM_LOG_INFO

  template <typename... Vals>
  void Info(Vals&&... vals) {
    (this->Info() << ... << std::forward<Vals>(vals));
  }

  template <typename Format, typename... Args>
  void Infof(Format&& fmt, Args&&... args) {
    this->Info() << fmt::format(std::forward<Format>(fmt),
                                std::forward<Args>(args)...);
  }

#if BGP_PLATFORM_LOG_LEVEL >= BGP_PLATFORM_LOG_WARN
  LogHelper Warn() {
    std::cerr << details::kBeginYellow;
    Logger::PrintTime(std::cerr);
    std::cerr << "[warn] ";
    return LogHelper(std::cerr, details::kEndYellow);
  }
#else
  EmptyLogHelper Warn() { return EmptyLogHelper(); }
#endif  // BGP_PLATFORM_LOG_LEVEL >= BGP_PLATFORM_LOG_WARN

  template <typename... Vals>
  void Warn(Vals&&... vals) {
    (this->Warn() << ... << std::forward<Vals>(vals));
  }

  template <typename Format, typename... Args>
  void Warnf(Format&& fmt, Args&&... args) {
    this->Warn() << fmt::format(std::forward<Format>(fmt),
                                std::forward<Args>(args)...);
  }

#if BGP_PLATFORM_LOG_LEVEL >= BGP_PLATFORM_LOG_ERROR
  LogHelper Error() {
    std::cerr << details::kBeginRed;
    Logger::PrintTime(std::cerr);
    std::cerr << "[error] ";
    return LogHelper(std::cerr, details::kEndRed);
  }
#else
  EmptyLogHelper Error() { return EmptyLogHelper(); }
#endif  // BGP_PLATFORM_LOG_LEVEL >= BGP_PLATFORM_LOG_ERROR

  template <typename... Vals>
  void Error(Vals&&... vals) {
    (this->Error() << ... << std::forward<Vals>(vals));
  }

  template <typename Format, typename... Args>
  void Errorf(Format&& fmt, Args&&... args) {
    this->Error() << fmt::format(std::forward<Format>(fmt),
                                 std::forward<Args>(args)...);
  }

 private:
  static void PrintTime(std::ostream& os) {
    os << fmt::format("{:%Y-%m-%d %H:%M:%S} ",
                      fmt::localtime(std::chrono::system_clock::to_time_t(
                          std::chrono::system_clock::now())));
  }
};

inline Logger logger;

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_UTILS_LOGGER_HPP_

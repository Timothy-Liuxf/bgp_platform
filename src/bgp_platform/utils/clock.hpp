#ifndef BGP_PLATFORM_UTILS_CLOCK_HPP_
#define BGP_PLATFORM_UTILS_CLOCK_HPP_

#include <chrono>
#include <ctime>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

using TimeStamp =
    std::chrono::time_point<std::chrono::system_clock>::duration::rep;

[[nodiscard]] inline TimeStamp GetTimeStamp() {
  return std::chrono::system_clock::now().time_since_epoch().count();
}

[[nodiscard]] inline std::chrono::time_point<std::chrono::system_clock>
TimpStampToTimePoint(TimeStamp timestamp) {
  return std::chrono::system_clock::time_point(
      std::chrono::system_clock::duration(timestamp));
}

struct CalendarTime {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
};

[[nodiscard]] inline CalendarTime ToUTCTime(
    std::chrono::time_point<std::chrono::system_clock> tp) {
  auto c_time     = std::chrono::system_clock::to_time_t(tp);
  auto c_utc_time = *std::gmtime(&c_time);
  return CalendarTime {c_utc_time.tm_year + 1900, c_utc_time.tm_mon + 1,
                       c_utc_time.tm_mday,        c_utc_time.tm_hour,
                       c_utc_time.tm_min,         c_utc_time.tm_sec};
}

[[nodiscard]] inline CalendarTime ToUTCTime(TimeStamp timestamp) {
  return ToUTCTime(TimpStampToTimePoint(timestamp));
}

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_UTILS_CLOCK_HPP_

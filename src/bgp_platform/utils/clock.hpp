#ifndef BGP_PLATFORM_UTILS_CLOCK_HPP_
#define BGP_PLATFORM_UTILS_CLOCK_HPP_

#include <chrono>
#include <ctime>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

using TimeStamp = std::chrono::seconds::rep;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using Duration  = std::chrono::system_clock::duration;

[[nodiscard]] inline TimeStamp GetTimeStamp() {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

[[nodiscard]] inline TimePoint TimeStampToTimePoint(TimeStamp timestamp) {
  return std::chrono::system_clock::time_point(std::chrono::seconds(timestamp));
}

[[nodiscard]] inline TimeStamp TimePointToTimeStamp(TimePoint time_point) {
  return std::chrono::duration_cast<std::chrono::seconds>(
             time_point.time_since_epoch())
      .count();
}

struct CalendarTime {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
};

[[nodiscard]] inline CalendarTime ToUTCTime(TimePoint tp) {
  auto c_time     = std::chrono::system_clock::to_time_t(tp);
  auto c_utc_time = *std::gmtime(&c_time);
  return CalendarTime {c_utc_time.tm_year + 1900, c_utc_time.tm_mon + 1,
                       c_utc_time.tm_mday,        c_utc_time.tm_hour,
                       c_utc_time.tm_min,         c_utc_time.tm_sec};
}

[[nodiscard]] inline CalendarTime ToUTCTime(TimeStamp timestamp) {
  return ToUTCTime(TimeStampToTimePoint(timestamp));
}

[[nodiscard]] inline TimePoint ToTimePoint(CalendarTime time) {
  std::tm tm_time {};
  tm_time.tm_year = time.year - 1900;
  tm_time.tm_mon  = time.month - 1;
  tm_time.tm_mday = time.day;
  tm_time.tm_hour = time.hour;
  tm_time.tm_min  = time.minute;
  tm_time.tm_sec  = time.second;
  return std::chrono::system_clock::from_time_t(std::mktime(&tm_time));
}

struct CalendarDuration {
  int days;
  int hours;
  int minutes;
  int seconds;
};

[[nodiscard]] inline CalendarDuration ToCalendarDuration(Duration duration) {
  auto seconds =
      std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  auto minutes = seconds / 60;
  seconds %= 60;
  auto hours = minutes / 60;
  minutes %= 60;
  auto days = hours / 24;
  hours %= 24;
  return {static_cast<int>(days), static_cast<int>(hours),
          static_cast<int>(minutes), static_cast<int>(seconds)};
}

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_UTILS_CLOCK_HPP_

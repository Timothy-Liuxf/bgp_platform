#ifndef BGP_PLATFORM_UTILS_STRCONV_HPP_
#define BGP_PLATFORM_UTILS_STRCONV_HPP_

#include <charconv>
#include <string_view>
#include <vector>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

using std::literals::string_view_literals::        operator""sv;

[[nodiscard]] inline std::vector<std::string_view> SplitString(
    std::string_view str, char delim) {
  std::vector<std::string_view> elems;
  std::size_t                   pos = 0;
  std::size_t                   len = str.length();
  while (pos < len) {
    std::size_t find_pos = str.find_first_of(delim, pos);
    if (find_pos == std::string_view::npos) {
      find_pos = len;
    }
    elems.emplace_back(str.substr(pos, find_pos - pos));
    pos = find_pos + 1;
  }
  return elems;
}

template <typename Number>
[[nodiscard]] inline Number StringToNumber(std::string_view str,
                                           int              base = 10) {
  Number result;
  auto [ptr, ec] =
      std::from_chars(str.data(), str.data() + str.size(), result, base);
  (void)ptr;
  if (ec != std::errc() || ptr != str.data() + str.size()) {
    throw std::invalid_argument("File to convert string to number!");
  }
  return result;
}

[[nodiscard]] inline bool StartsWith(std::string_view str,
                                     std::string_view prefix) {
  return str.substr(0, prefix.size()) == prefix;
}

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_UTILS_STRCONV_HPP_

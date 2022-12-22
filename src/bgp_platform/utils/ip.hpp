#ifndef BGP_PLATFORM_UTILS_IP_HPP_
#define BGP_PLATFORM_UTILS_IP_HPP_

#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

#include <arpa/inet.h>

#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/strconv.hpp>

constexpr inline bool operator==(const in_addr& addr1, const in_addr& addr2) {
  return addr1.s_addr == addr2.s_addr;
}

constexpr inline bool operator==(const in6_addr& addr1, const in6_addr& addr2) {
  return addr1.s6_addr32[0] == addr2.s6_addr32[0] &&
         addr1.s6_addr32[1] == addr2.s6_addr32[1] &&
         addr1.s6_addr32[2] == addr2.s6_addr32[2] &&
         addr1.s6_addr32[3] == addr2.s6_addr32[3];
}

BGP_PLATFORM_NAMESPACE_BEGIN

using IPAddr = std::variant<::in_addr, ::in6_addr>;

struct IPPrefix {
  IPAddr  addr;
  uint8_t prefix_len;
};

constexpr inline bool operator==(const IPPrefix& prefix1,
                                 const IPPrefix& prefix2) {
  return prefix1.addr == prefix2.addr &&
         prefix1.prefix_len == prefix2.prefix_len;
}

[[nodiscard]] inline IPAddr StringToIPAddr(std::string_view str) {
  if (str.find(':') == std::string::npos) {
    // IPv4
    in_addr addr;
    int     ret = ::inet_pton(AF_INET, std::string(str).c_str(), &addr);
    if (ret != 1) {
      throw std::invalid_argument("Invalid IP address!");
    }
    return addr;
  } else {
    // IPv6
    in6_addr addr;
    int      ret = ::inet_pton(AF_INET6, std::string(str).c_str(), &addr);
    if (ret != 1) {
      throw std::invalid_argument("Invalid IP address!");
    }
    return addr;
  }
}

[[nodiscard]] inline IPPrefix StringToIPPrefix(std::string_view str) {
  auto pos = str.find('/');
  if (pos == std::string::npos) {
    throw std::invalid_argument("Invalid IP prefix!");
  }
  return IPPrefix {StringToIPAddr(str.substr(0, pos)),
                   StringToNumber<uint8_t>(str.substr(pos + 1))};
}

BGP_PLATFORM_NAMESPACE_END

namespace std {

template <>
struct hash<::in_addr> {
  std::size_t operator()(const ::in_addr& addr) const {
    return std::hash<uint32_t> {}(addr.s_addr);
  }
};

template <>
struct hash<::in6_addr> {
  std::size_t operator()(const ::in6_addr& addr) const {
    std::size_t result = 0;
    for (int i = 0; i < 4; ++i) {
      result ^= std::hash<uint32_t> {}(addr.s6_addr32[i]);
    }
    return result;
  }
};

template <>
struct hash<BGP_PLATFORM_NAMESPACE::IPPrefix> {
  std::size_t operator()(const BGP_PLATFORM_NAMESPACE::IPPrefix& prefix) const {
    std::size_t result = 0;
    if (std::holds_alternative<::in_addr>(prefix.addr)) {
      result = std::hash<::in_addr> {}(std::get<::in_addr>(prefix.addr));
    } else {
      result = std::hash<::in6_addr> {}(std::get<::in6_addr>(prefix.addr));
    }
    return result ^ std::hash<uint8_t> {}(prefix.prefix_len);
  }
};

}  // namespace std

#endif  // BGP_PLATFORM_UTILS_IP_HPP_

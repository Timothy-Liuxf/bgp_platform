#ifndef BGP_PLATFORM_COMMON_TYPES_HPP_
#define BGP_PLATFORM_COMMON_TYPES_HPP_

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

struct Line {
  std::string buf {};
  std::size_t num = 1;
};

enum class AsNum : std::uint16_t {};
enum class ID : std::uint64_t {};
using Country = std::string;

constexpr inline ID& operator++(ID& id) {
  return id = ID(static_cast<std::underlying_type_t<ID>>(id) + 1);
}

constexpr inline ID operator++(ID& id, int) {
  auto old_id = id;
  ++id;
  return old_id;
}

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_COMMON_TYPES_HPP_

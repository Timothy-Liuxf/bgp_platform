#ifndef BGP_PLATFORM_UTILS_TYPES_HPP_
#define BGP_PLATFORM_UTILS_TYPES_HPP_

#include <type_traits>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

template <typename T>
[[nodiscard]] constexpr inline std::underlying_type_t<T> ToUnderlying(
    const T& value) noexcept {
  return static_cast<std::underlying_type_t<T>>(value);
}

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_UTILS_TYPES_HPP_

#ifndef BGP_PLATFORM_UTILS_TYPES_HPP_
#define BGP_PLATFORM_UTILS_TYPES_HPP_

#include <functional>
#include <type_traits>
#include <vector>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

template <typename T>
[[nodiscard]] constexpr inline std::underlying_type_t<T> ToUnderlying(
    const T& value) noexcept {
  return static_cast<std::underlying_type_t<T>>(value);
}

namespace details {
template <typename, typename = void>
struct vector_hash_helper;

template <typename T>
struct vector_hash_helper<
    std::vector<T>, typename std::enable_if<std::is_scalar<T>::value &&
                                            !std::is_enum<T>::value>::type> {
  std::size_t operator()(const std::vector<T>& v) const noexcept {
    std::size_t seed = v.size();
    for (const auto& i : v) {
      seed ^= std::hash<T> {}(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

template <typename T>
struct vector_hash_helper<
    std::vector<T>, typename std::enable_if<std::is_enum<T>::value>::type> {
  std::size_t operator()(const std::vector<T>& v) const noexcept {
    std::size_t seed = v.size();
    for (const auto& i : v) {
      seed ^= std::hash<std::underlying_type_t<T>> {}(ToUnderlying(i)) +
              0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};
}  // namespace details

BGP_PLATFORM_NAMESPACE_END

namespace std {
template <typename T>
struct hash<std::vector<T>>
    : BGP_PLATFORM_NAMESPACE::details::vector_hash_helper<std::vector<T>> {};
}  // namespace std

#endif  // BGP_PLATFORM_UTILS_TYPES_HPP_

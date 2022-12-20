#ifndef BGP_PLATFORM_DETECTOR_TYPES_HPP_
#define BGP_PLATFORM_DETECTOR_TYPES_HPP_

#include <cstddef>
#include <cstdint>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

enum class VpNum : std::uint16_t {};
enum class AsNum : std::uint16_t {};
enum class ID : std::uint64_t {};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_DETECTOR_TYPES_HPP_

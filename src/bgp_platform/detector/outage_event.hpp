#ifndef BGP_PLATFORM_DETECTOR_OUTAGE_EVENT_HPP_
#define BGP_PLATFORM_DETECTOR_OUTAGE_EVENT_HPP_

#include <functional>
#include <vector>

#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/ip.hpp>

#include "types.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

struct OutageEvent {
  struct Key {
    AsNum    owner_as;
    IPPrefix prefix;
    ID       prefix_outage_id;
  } key;

  struct Value {
    std::string        table_name;
    Country            country;
    std::string        as_name;
    std::string        org_name;
    std::string        as_type;
    TimePoint          start_time;
    TimePoint          end_time;
    Duration           duration;
    std::vector<AsNum> pre_vp_path;
    std::string        outage_level;
    std::string        outage_level_description;
  } value;
};

BGP_PLATFORM_NAMESPACE_END

namespace std {
template <>
struct hash<BGP_PLATFORM_NAMESPACE::OutageEvent::Key> {
  std::size_t operator()(
      const BGP_PLATFORM_NAMESPACE::OutageEvent::Key& key) const {
    return std::hash<BGP_PLATFORM_NAMESPACE::AsNum> {}(key.owner_as) ^
           std::hash<BGP_PLATFORM_NAMESPACE::IPPrefix> {}(key.prefix) ^
           std::hash<BGP_PLATFORM_NAMESPACE::ID> {}(key.prefix_outage_id);
  }
};
}  // namespace std

#endif  // BGP_PLATFORM_DETECTOR_OUTAGE_EVENT_HPP_

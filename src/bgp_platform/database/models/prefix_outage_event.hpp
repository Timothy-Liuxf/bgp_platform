#ifndef BGP_PLATFORM_DETECTOR_OUTAGE_EVENT_HPP_
#define BGP_PLATFORM_DETECTOR_OUTAGE_EVENT_HPP_

#include <functional>
#include <vector>

#include <bgp_platform/common/types.hpp>
#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/ip.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

namespace database::models {

struct PrefixOutageEvent {
  struct Key {
    AsNum    owner_as;
    IPPrefix prefix;
    ID       outage_id;
  } key;

  struct Value {
    Country            country;
    std::string        as_name;
    std::string        org_name;
    std::string        as_type;
    TimePoint          start_time;
    TimePoint          end_time;
    Duration           duration;
    std::vector<AsNum> pre_vp_path;
    std::vector<AsNum> eve_vp_path;
    std::string        outage_level;
    std::string        outage_level_description;
  } value;
};

[[nodiscard]] inline bool operator==(PrefixOutageEvent::Key key1,
                                     PrefixOutageEvent::Key key2) {
  return key1.owner_as == key2.owner_as && key1.outage_id == key2.outage_id &&
         key1.prefix == key2.prefix;
}

}  // namespace database::models

BGP_PLATFORM_NAMESPACE_END

namespace std {
template <>
struct hash<BGP_PLATFORM_NAMESPACE::database::models::PrefixOutageEvent::Key> {
  std::size_t operator()(
      const BGP_PLATFORM_NAMESPACE::database::models::PrefixOutageEvent::Key&
          key) const {
    return std::hash<BGP_PLATFORM_NAMESPACE::AsNum> {}(key.owner_as) ^
           std::hash<BGP_PLATFORM_NAMESPACE::IPPrefix> {}(key.prefix) ^
           std::hash<BGP_PLATFORM_NAMESPACE::ID> {}(key.outage_id);
  }
};
}  // namespace std

#endif  // BGP_PLATFORM_DETECTOR_OUTAGE_EVENT_HPP_

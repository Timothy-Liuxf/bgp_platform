#ifndef BGP_PLATFORM_DATABASE_MODELS_AS_OUTAGE_EVENT_HPP_
#define BGP_PLATFORM_DATABASE_MODELS_AS_OUTAGE_EVENT_HPP_

#include <cstddef>
#include <functional>
#include <vector>

#include <bgp_platform/common/types.hpp>
#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/ip.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

namespace database::models {

struct ASOutageEvent {
  struct Key {
    AsNum owner_as;
    ID    outage_id;
  } key;

  struct Value {
    Country               country;
    std::string           as_name;
    std::string           org_name;
    std::string           as_type;
    TimePoint             start_time;
    TimePoint             end_time;
    Duration              duration;
    std::size_t           total_prefix_num;
    std::size_t           max_outage_prefix_num;
    double                max_outage_prefix_ratio;
    std::vector<AsNum>    pre_vp_path;
    std::vector<AsNum>    eve_vp_path;
    std::vector<IPPrefix> outage_prefixes;
    std::string           outage_level;
    std::string           outage_level_description;
  } value;
};

[[nodiscard]] inline bool operator==(ASOutageEvent::Key key1,
                                     ASOutageEvent::Key key2) {
  return key1.owner_as == key2.owner_as && key1.outage_id == key2.outage_id;
}

}  // namespace database::models

BGP_PLATFORM_NAMESPACE_END

namespace std {
template <>
struct hash<BGP_PLATFORM_NAMESPACE::database::models::ASOutageEvent::Key> {
  std::size_t operator()(
      const BGP_PLATFORM_NAMESPACE::database::models::ASOutageEvent::Key& key)
      const {
    return std::hash<BGP_PLATFORM_NAMESPACE::AsNum> {}(key.owner_as) ^
           std::hash<BGP_PLATFORM_NAMESPACE::ID> {}(key.outage_id);
  }
};
}  // namespace std

#endif  // BGP_PLATFORM_DATABASE_MODELS_AS_OUTAGE_EVENT_HPP_

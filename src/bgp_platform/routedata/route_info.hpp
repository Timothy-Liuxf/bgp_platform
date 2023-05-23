#ifndef BGP_PLATFORM_ROUTE_INFO_HPP_
#define BGP_PLATFORM_ROUTE_INFO_HPP_

#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <bgp_platform/common/types.hpp>
#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/ip.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

struct CountryRouteInfo {
  std::unordered_set<AsNum> normal_ass;
  std::unordered_set<AsNum> outage_ass;
  ID                        outage_id              = {};
  bool                      is_outage              = false;
  TimePoint                 last_outage_start_time = {};
};

struct AsRouteInfo {
  struct PrefixRouteInfo {
    std::unordered_set<AsNum>                     reachable_vps;
    std::unordered_set<AsNum>                     unreachable_vps;
    std::unordered_map<AsNum, std::vector<AsNum>> vp_paths;
    ID                                            outage_id = {};
    bool                                          is_outage = false;
    TimePoint                                     last_outage_start_time = {};
  };

  std::unordered_set<IPPrefix>                  normal_prefixes;
  std::unordered_set<IPPrefix>                  outage_prefixes;
  ID                                            outage_id = {};
  bool                                          is_outage = false;
  std::unordered_map<IPPrefix, PrefixRouteInfo> prefixes;
  TimePoint                                     last_outage_start_time = {};
};

struct PrefixRouteInfo {
  std::unordered_set<AsNum> owner_ass_;
};

struct RouteInfo {
  std::unordered_map<AsNum, AsRouteInfo>        as_route_info_;
  std::unordered_map<Country, CountryRouteInfo> country_route_info_;
  std::unordered_map<IPPrefix, PrefixRouteInfo> prefix_route_info_;
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_ROUTE_INFO_HPP_

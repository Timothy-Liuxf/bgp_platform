#ifndef BGP_PLATFORM_ROUTE_INFO_H_
#define BGP_PLATFORM_ROUTE_INFO_H_

#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <bgp_platform/utils/defs.hpp>

#include "types.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

struct PrefixRouteInfo {
  std::unordered_set<VpNum>                     reachable_vps;
  std::unordered_set<VpNum>                     unreachable_vps;
  std::unordered_map<VpNum, std::vector<VpNum>> vp_paths;
  ID                                            outage_id         = {};
  bool                                          is_outage         = false;
  std::chrono::system_clock::time_point         outage_start_time = {};
};

struct CountryRouteInfo {
  std::unordered_set<AsNum>             normal_ass;
  std::unordered_set<AsNum>             outage_ass;
  ID                                    outage_id         = {};
  bool                                  is_outage         = false;
  std::chrono::system_clock::time_point outage_start_time = {};
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_ROUTE_INFO_H_

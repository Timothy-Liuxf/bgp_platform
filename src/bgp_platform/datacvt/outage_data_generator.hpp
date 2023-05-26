#ifndef BGP_PLATFORM_DATACVT_OUTAGE_DATA_GENERATOR_HPP_
#define BGP_PLATFORM_DATACVT_OUTAGE_DATA_GENERATOR_HPP_

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include <bgp_platform/common/types.hpp>
#include <bgp_platform/routedata/routedata.hpp>
#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/files.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

class OutageDataGenerator : RouteDataListener {
 public:
  struct InitConfig {
    RouteData::InitInfoFilePath init_info_file_path;
    fs::path                    proto_data_file_path;
    fs::path                    output_file_path;
    TimeStamp                   start_monitor_time;
    TimeStamp                   end_monitor_time;
  };

  OutageDataGenerator(const InitConfig& config);
  void Generate(fs::path route_data_path);
  void OnRouteRemoved(AsNum owner_as, IPPrefix prefix,
                      TimeStamp timestamp) override;
  void OnRouteAdded(AsNum owner_as, IPPrefix prefix,
                    TimeStamp timestamp) override;

 private:
  RouteData route_data_;
  std::unordered_map<AsNum, std::vector<std::pair<TimeStamp, TimeStamp>>>
                outage_data_;
  std::ofstream output_file_;
  TimeStamp     start_monitor_time_;
  TimeStamp     end_monitor_time_;
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_DATACVT_OUTAGE_DATA_GENERATOR_HPP_

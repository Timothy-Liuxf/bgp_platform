#ifndef BGP_PLATFORM_DETECTOR_DETECTOR_HPP_
#define BGP_PLATFORM_DETECTOR_DETECTOR_HPP_

#include <optional>
#include <unordered_map>
#include <utility>

#include <bgp_platform/database/database.hpp>
#include <bgp_platform/routedata/routedata.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/files.hpp>

#include "outage_event.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

class Detector : public RouteDataListener {
 public:
  using InitInfoFilePath = RouteData::InitInfoFilePath;

  struct DatabaseConfig {
    std::string_view host;
    std::string_view port;
    std::string_view user;
    std::string_view password;
    std::string_view database;
  };

  Detector(const InitInfoFilePath& init_info_file_path,
           const DatabaseConfig&   database_config)
      : route_data_(*this, init_info_file_path),
        database_(database_config.host, database_config.port,
                  database_config.user, database_config.password,
                  database_config.database) {}
  Detector(const Detector&) = delete;

  void         Detect(fs::path route_data_path);
  virtual void OnRouteRemoved(AsNum owner_as, IPPrefix prefix,
                              TimeStamp timestamp) override;
  virtual void OnRouteAdded(AsNum owner_as, IPPrefix prefix,
                            TimeStamp timestamp) override;

 private:
  RouteData          route_data_;
  database::Database database_;
  OutageEvent        outage_events_;

 private:
  [[nodiscard]] bool InBlackList(const IPPrefix&) const { return false; }

 private:
  void CheckPrefixOutage(AsNum owner_as, IPPrefix prefix, TimeStamp timestamp);
  void CheckASOutage(AsNum owner_as, IPPrefix prefix, TimeStamp timestamp);

 private:
  static std::optional<CalendarTime> GetTimeFromUpdateFileName(
      std::string file_name);
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_DETECTOR_DETECTOR_HPP_

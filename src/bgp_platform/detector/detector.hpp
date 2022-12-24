#ifndef BGP_PLATFORM_DETECTOR_DETECTOR_HPP_
#define BGP_PLATFORM_DETECTOR_DETECTOR_HPP_

#include <bgp_platform/database/database.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/files.hpp>

#include "init_info.hpp"
#include "route_info.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

class Detector {
 public:
  struct InitInfoFilePath {
    fs::path as_info;
    fs::path top_nx;
    fs::path top_ip;
  };

  struct DatabaseConfig {
    std::string_view host;
    std::string_view port;
    std::string_view user;
    std::string_view password;
    std::string_view database;
  };

  Detector(const InitInfoFilePath& init_info_file_path,
           const DatabaseConfig&   database_config)
      : init_info_(init_info_file_path.as_info, init_info_file_path.top_nx,
                   init_info_file_path.top_ip),
        database_(database_config.host, database_config.port,
                  database_config.user, database_config.password,
                  database_config.database) {}
  Detector(const Detector&) = delete;

  void Detect(fs::path route_data_path);

 private:
  InitInfo           init_info_;
  RouteInfo          route_info_;
  database::Database database_;

 private:
  [[nodiscard]] bool InBlackList(const IPPrefix&) const { return false; }

 private:
  void ReadRibFile(fs::path file_path);
  void ReadUpdateFile(fs::path file_path);
  void DetectOutage(DumpedFile update_file);
  void CheckPrefixOutage(AsNum owner_as, IPPrefix prefix, TimeStamp timestamp);
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_DETECTOR_DETECTOR_HPP_

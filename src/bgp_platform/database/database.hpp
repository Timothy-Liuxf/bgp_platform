#ifndef BGP_PLATFORM_DATABASE_DATABASE_HPP_
#define BGP_PLATFORM_DATABASE_DATABASE_HPP_

#include <string_view>

#include <pqxx/pqxx>

#include <bgp_platform/common/types.hpp>
#include <bgp_platform/database/connector.hpp>
#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/defs.hpp>

#include "models/prefix_outage_event.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

namespace database {

class Database {
 public:
  Database(std::string_view host, std::string_view port, std::string_view user,
           std::string_view password, std::string_view database);

  void      SetTableTime(TimePoint table_time);
  TimePoint GetTableTime() const { return this->table_time_; }
  void      InsertPrefixOutageEvent(const models::PrefixOutageEvent& event);

 private:
  Connector   connector_;
  TimePoint   table_time_;
  std::string prefix_outage_table_name_;
  std::string as_outage_table_name_;
  std::string country_outage_table_name_;

 private:
  void CreateTable();
};

}  // namespace database

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_DATABASE_DATABASE_HPP_

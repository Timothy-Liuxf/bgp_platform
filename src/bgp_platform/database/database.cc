#include "database.hpp"

#include <type_traits>

#include <fmt/chrono.h>
#include <fmt/std.h>

#include <bgp_platform/utils/ip.hpp>
#include <bgp_platform/utils/logger.hpp>
#include <bgp_platform/utils/types.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

namespace database {

using fmt::literals::operator""_a;

Database::Database(std::string_view host, std::string_view port,
                   std::string_view user, std::string_view password,
                   std::string_view database)
    : connector_(host, port, user, password, database) {
  this->SetTableTime(std::chrono::system_clock::now());
}

void Database::SetTableTime(TimePoint table_time) {
  this->table_time_ = table_time;
  this->prefix_outage_table_name_ =
      fmt::format("prefix_outage_{:%Y%m}", table_time);
  this->as_outage_table_name_ = fmt::format("as_outage_{:%Y%m}", table_time);
  this->country_outage_table_name_ =
      fmt::format("country_outage_{:%Y%m}", table_time);

  this->CreateTable();
}

void Database::CreateTable() {
  auto work = this->connector_.GetWork();

  work.exec(fmt::format(std::string({
#include "sql/create_prefix_outage_table.sql.inc"
                        }),
                        this->prefix_outage_table_name_));
  logger.Info() << "Created prefix outage table: "
                << this->prefix_outage_table_name_;

  work.exec(fmt::format(std::string({
#include "sql/create_as_outage_table.sql.inc"
                        }),
                        this->as_outage_table_name_));
  logger.Info() << "Created AS outage table: " << this->as_outage_table_name_;

  work.exec(fmt::format(std::string({
#include "sql/create_country_outage_table.sql.inc"
                        }),
                        this->country_outage_table_name_));
  logger.Info() << "Created country outage table: "
                << this->country_outage_table_name_;
  work.commit();
}

std::string Database::InsertPrefixOutageEvent(
    const models::PrefixOutageEvent& event) {
  auto work       = this->connector_.GetWork();
  auto table_name = this->prefix_outage_table_name_;
  work.exec(fmt::format(
      std::string({
#include "sql/insert_prefix_outage_event.sql.inc"
      }),
      "table_name"_a = table_name, "asn"_a = ToUnderlying(event.key.owner_as),
      "country"_a = event.value.country, "as_name"_a = event.value.as_name,
      "org_name"_a = event.value.org_name, "as_type"_a = event.value.as_type,
      "s_time"_a = fmt::gmtime(
          std::chrono::system_clock::to_time_t(event.value.start_time)),
      "pre_vp_paths"_a       = "{}",  // TODO: Set pre_vp_path
      "eve_vp_paths"_a       = "{}",  // TODO: Set eve_vp_path
      "outage_level"_a       = "",    // TODO: Set outage_level
      "outage_level_descr"_a = "",    // TODO: Set outage_level_descr
      "prefix"_a             = IPPrefixToString(event.key.prefix),
      "outage_id"_a          = ToUnderlying(event.key.outage_id)));
  work.commit();
  logger.Info() << "Inserted prefix outage event: "
                << this->prefix_outage_table_name_ << " "
                << ToUnderlying(event.key.outage_id);
  return table_name;
}

std::string Database::InsertASOutageEvent(const models::ASOutageEvent& event) {
  auto work       = this->connector_.GetWork();
  auto table_name = this->as_outage_table_name_;
  work.exec(fmt::format(
      std::string({
#include "sql/insert_as_outage_event.sql.inc"
      }),
      "table_name"_a = table_name, "country"_a = event.value.country,
      "as_name"_a = event.value.as_name, "org_name"_a = event.value.org_name,
      "as_type"_a = event.value.as_type,
      "s_time"_a  = fmt::gmtime(
           std::chrono::system_clock::to_time_t(event.value.start_time)),
      "total_prefix_num"_a        = event.value.total_prefix_num,
      "max_outage_prefix_num"_a   = event.value.max_outage_prefix_num,
      "max_outage_prefix_ratio"_a = event.value.max_outage_prefix_ratio,
      "pre_vp_paths"_a            = "{}",  // TODO: Set pre_vp_path
      "eve_vp_paths"_a            = "{}",  // TODO: Set eve_vp_path
      "outage_prefixes"_a         = "",    // TODO: Set outage prefixes
      "outage_level"_a            = "",    // TODO: Set outage_level
      "outage_level_descr"_a      = "",    // TODO: Set outage_level_descr
      "asn"_a                     = ToUnderlying(event.key.owner_as),
      "outage_id"_a               = ToUnderlying(event.key.outage_id)));
  work.commit();
  return table_name;
}

void Database::PrefixOutageEnd(
    std::string_view                        table_name,
    const models::PrefixOutageEvent::Key&   event_key,
    const models::PrefixOutageEvent::Value& event_value) {
  auto        work = this->connector_.GetWork();
  std::string e_time_arg =
      event_value.end_time == std::nullopt
          ? std::string("NULL")
          : fmt::format("\'{:%Y-%m-%d %H:%M:%S}\'",
                        fmt::gmtime(std::chrono::system_clock::to_time_t(
                            event_value.end_time.value())));
  std::string duration_arg = "";
  if (event_value.duration == std::nullopt) {
    duration_arg = "NULL";
  } else {
    auto duration = ToCalendarDuration(event_value.duration.value());
    duration_arg =
        fmt::format("\'{} days {} hours {} minutes {} seconds\'", duration.days,
                    duration.hours, duration.minutes, duration.seconds);
  }
  work.exec(fmt::format(
      std::string {
#include "sql/update_prefix_outage_end.sql.inc"
      },
      "table_name"_a = table_name, "e_time"_a = e_time_arg,
      "duration"_a  = duration_arg,
      "prefix"_a    = IPPrefixToString(event_key.prefix),
      "outage_id"_a = ToUnderlying(event_key.outage_id),
      "asn"_a       = ToUnderlying(event_key.owner_as)));
  work.commit();
}

bool Database::TableExists(std::string_view table_name) {
  auto work = this->connector_.GetWork();
  auto res  = work.exec(fmt::format(std::string({
#include "sql/count_tables.sql.inc"
                                   }),
                                   "table_name"_a = table_name));
  return res[0][0].as<int>() == 1;
}

}  // namespace database

BGP_PLATFORM_NAMESPACE_END

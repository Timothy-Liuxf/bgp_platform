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
  auto duration   = ToCalendarDuration(event.value.duration);
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
      "e_time"_a = fmt::gmtime(
          std::chrono::system_clock::to_time_t(event.value.end_time)),
      "duration"_a =
          fmt::format("{} days {} hours {} minutes {} seconds", duration.days,
                      duration.hours, duration.minutes, duration.seconds),
      "pre_vp_paths"_a       = "{}",  // TODO: Set pre_vp_path
      "eve_vp_paths"_a       = "{}",  // TODO: Set eve_vp_path
      "outage_level"_a       = "",    // TODO: Set outage_level
      "outage_level_descr"_a = "",    // TODO: Set outage_level_descr
      "prefix"_a             = IPPrefixToString(event.key.prefix),
      "outage_id"_a          = ToUnderlying(event.key.outage_id)));
  work.commit();
  return table_name;
}

void Database::PrefixOutageEnd(
    std::string_view                        table_name,
    const models::PrefixOutageEvent::Key&   event_key,
    const models::PrefixOutageEvent::Value& event_value) {
  auto work     = this->connector_.GetWork();
  auto duration = ToCalendarDuration(event_value.duration);
  work.exec(fmt::format(
      std::string {
#include "sql/update_prefix_outage_end.sql.inc"
      },
      "table_name"_a = table_name,
      "e_time"_a     = fmt::gmtime(
              std::chrono::system_clock::to_time_t(event_value.end_time)),
      "duration"_a =
          fmt::format("{} days {} hours {} minutes {} seconds", duration.days,
                      duration.hours, duration.minutes, duration.seconds),
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

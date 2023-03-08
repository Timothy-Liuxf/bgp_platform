#ifndef BGP_PLATFORM_OUTAGE_EVENT_HPP_
#define BGP_PLATFORM_OUTAGE_EVENT_HPP_

#include <string>
#include <unordered_map>

#include <bgp_platform/database/models/prefix_outage_event.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

struct OutageEvent {
  struct PrefixOutageEventInfo {
    std::string                                table_name;
    database::models::PrefixOutageEvent::Value outage_event_value;
  };

  struct ASOutageEventInfo {
    std::string                            table_name;
    bool                                   if_changed;
    database::models::ASOutageEvent::Value outage_event_value;
  };

  std::unordered_map<database::models::PrefixOutageEvent::Key,
                     PrefixOutageEventInfo>
      prefix;

  std::unordered_map<database::models::ASOutageEvent::Key, ASOutageEventInfo>
      as;
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_OUTAGE_EVENT_HPP_

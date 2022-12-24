#include "connector.hpp"

#include <fmt/format.h>

BGP_PLATFORM_NAMESPACE_BEGIN

namespace database {

using fmt::literals::operator""_a;

Connector::Connector(std::string_view host, std::string_view port,
                     std::string_view user, std::string_view password,
                     std::string_view database)
    : connection_(
          fmt::format("postgresql://{user}:{password}@{host}:{port}/{database}",
                      "host"_a = host, "port"_a = port, "user"_a = user,
                      "password"_a = password, "database"_a = database)) {}

}  // namespace database

BGP_PLATFORM_NAMESPACE_END

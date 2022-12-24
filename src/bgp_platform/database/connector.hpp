#ifndef BGP_PLATFORM_DATABASE_CONNECTOR_HPP_
#define BGP_PLATFORM_DATABASE_CONNECTOR_HPP_

#include <string_view>

#include <pqxx/pqxx>

#include <bgp_platform/utils/defs.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

namespace database {

class Connector {
 public:
  Connector(std::string_view host, std::string_view port, std::string_view user,
            std::string_view password, std::string_view database);

  pqxx::work GetWork() { return pqxx::work(this->connection_); }

 private:
  pqxx::connection connection_;
};

}  // namespace database

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_DATABASE_CONNECTOR_HPP_

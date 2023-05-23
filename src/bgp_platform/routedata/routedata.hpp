#ifndef BGP_PLATFORM_ROUTEINFO_ROUTEINFO_HPP_
#define BGP_PLATFORM_ROUTEINFO_ROUTEINFO_HPP_

#include <bgp_platform/utils/defs.hpp>

#include "init_info.hpp"
#include "route_info.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

class RouteDataListener {
 public:
  virtual void OnRouteRemoved(AsNum owner_as, IPPrefix prefix,
                              TimeStamp timestamp) = 0;
  virtual void OnRouteAdded(AsNum owner_as, IPPrefix prefix,
                            TimeStamp timestamp)   = 0;
  virtual ~RouteDataListener() {}
};

class RouteData {
 public:
  struct InitInfoFilePath {
    fs::path as_info;
    fs::path top_nx;
    fs::path top_ip;
  };

  RouteData(RouteDataListener&      listener,
            const InitInfoFilePath& init_info_file_path)
      : listener_(listener),
        init_info_(init_info_file_path.as_info, init_info_file_path.top_nx,
                   init_info_file_path.top_ip) {}

  void                       ReadRibFile(fs::path file_path);
  void                       ReadUpdateFile(fs::path file_path);

  [[nodiscard]] AsRouteInfo* GetASRouteInfo(const AsNum& asn) {
    if (auto as_route_info_itr = this->route_info_.as_route_info_.find(asn);
        as_route_info_itr != this->route_info_.as_route_info_.end()) {
      return &as_route_info_itr->second;
    } else {
      return nullptr;
    }
  }

  [[nodiscard]] AsRouteInfo& GetOrInsertASRouteInfo(const AsNum& asn) {
    return this->route_info_.as_route_info_[asn];
  }

  [[nodiscard]] CountryRouteInfo* GetCountryRouteInfo(const Country& country) {
    if (auto country_route_info_itr =
            this->route_info_.country_route_info_.find(country);
        country_route_info_itr != this->route_info_.country_route_info_.end()) {
      return &country_route_info_itr->second;
    } else {
      return nullptr;
    }
  }

  [[nodiscard]] CountryRouteInfo& GetOrInsertCountryRouteInfo(
      const Country& country) {
    return this->route_info_.country_route_info_[country];
  }

  [[nodiscard]] PrefixRouteInfo* GetPrefixRouteInfo(const IPPrefix& country) {
    if (auto prefix_route_info_itr =
            this->route_info_.prefix_route_info_.find(country);
        prefix_route_info_itr != this->route_info_.prefix_route_info_.end()) {
      return &prefix_route_info_itr->second;
    } else {
      return nullptr;
    }
  }

  [[nodiscard]] PrefixRouteInfo& GetOrInsertPrefixRouteInfo(
      const IPPrefix& country) {
    return this->route_info_.prefix_route_info_[country];
  }

  [[nodiscard]] Country GetAsCountry(AsNum as_num) const {
    return this->init_info_.GetAsCountry(as_num);
  }

  [[nodiscard]] std::string GetAsAutName(AsNum as_num) const {
    return this->init_info_.GetAsAutName(as_num);
  }

  [[nodiscard]] std::string GetAsOrgName(AsNum as_num) const {
    return this->init_info_.GetAsOrgName(as_num);
  }

  [[nodiscard]] std::string GetAsType(AsNum as_num) const {
    return this->init_info_.GetAsType(as_num);
  }

 private:
  RouteDataListener& listener_;
  InitInfo           init_info_;
  RouteInfo          route_info_;
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_ROUTEINFO_ROUTEINFO_HPP_

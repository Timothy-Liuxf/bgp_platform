#ifndef BGP_PLATFORM_ROUTEINFO_INIT_INFO_HPP_
#define BGP_PLATFORM_ROUTEINFO_INIT_INFO_HPP_

#include <unordered_map>

#include <bgp_platform/common/types.hpp>
#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/files.hpp>

BGP_PLATFORM_NAMESPACE_BEGIN

struct AsInitInfo {
  Country     country;
  std::string aut_name;
  std::string org_name;
  std::string as_type;
};

class InitInfo {
 public:
  InitInfo(fs::path as_info_path, fs::path top_nx_path, fs::path top_ip_path);

  [[nodiscard]] Country     GetAsCountry(AsNum as_num) const;
  [[nodiscard]] std::string GetAsAutName(AsNum as_num) const;
  [[nodiscard]] std::string GetAsOrgName(AsNum as_num) const;
  [[nodiscard]] std::string GetAsType(AsNum as_num) const;

 private:
  std::unordered_map<AsNum, AsInitInfo> as_info_;
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_ROUTEINFO_INIT_INFO_HPP_

#ifndef BGP_PLATFORM_DETECTOR_INIT_INFO_HPP
#define BGP_PLATFORM_DETECTOR_INIT_INFO_HPP

#include <unordered_map>

#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/files.hpp>

#include "types.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

struct AsInfo {
  std::string country;
  std::string aut_name;
  std::string org_name;
  std::string as_type;
};

class InitInfo {
 public:
  InitInfo(fs::path as_info_path, fs::path top_nx_path, fs::path top_ip_path);

 private:
  std::unordered_map<AsNum, AsInfo> as_info_;
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_DETECTOR_INIT_INFO_HPP

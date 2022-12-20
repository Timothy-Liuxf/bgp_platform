#include "init_info.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include <jsoncons/json.hpp>

using namespace jsoncons;

JSONCONS_N_MEMBER_TRAITS(BGP_PLATFORM_NAMESPACE::AsInfo, 0, country, aut_name,
                         org_name, as_type)

BGP_PLATFORM_NAMESPACE_BEGIN

InitInfo::InitInfo(fs::path as_info_path, fs::path top_nx_path,
                   fs::path top_ip_path) {
  {
    std::ifstream as_info_file(as_info_path);
    auto          as_info_json = jsoncons::json::parse(as_info_file);
    for (const auto& as_info : as_info_json.object_range()) {
      auto as_num       = static_cast<AsNum>(std::stoi(as_info.key()));
      auto as_info_data = as_info.value().as<AsInfo>();
      this->as_info_.emplace(as_num, as_info_data);
    }
  }
  {
    std::ifstream top_ip_file(top_ip_path);
    std::string   buf;
    while (std::getline(top_ip_file, buf)) {
      // TODO: Read top ip
    }
  }
  {
    // TODO: Read top nx
    (void)top_nx_path;
  }
  return;
}

BGP_PLATFORM_NAMESPACE_END

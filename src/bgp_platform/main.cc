#include <fstream>
#include <iostream>
#include <string>

#include <jsoncons/json.hpp>

#include <bgp_platform/detector/detector.hpp>

using namespace bgp_platform;
using namespace jsoncons;
using namespace std::literals::string_literals;

int main() {
  const auto    config_path = "config/config.json"s;
  std::ifstream config_file(config_path, std::ios::in);
  if (!config_file) {
    std::cerr << "Cannot open config file: " << config_path << std::endl;
    return 1;
  }

  auto config = json::parse(config_file);
  config_file.close();

  const json& data_path = config["data-path"];
  const json& db_config = config["database"];

  Detector    detector(
         {data_path["as-dict"].as_string_view(),
          data_path["top-nx"].as_string_view(),
          data_path["top-ip"].as_string_view()},
         {db_config["host"].as_string_view(), db_config["port"].as_string_view(),
          db_config["user"].as_string_view(),
          db_config["password"].as_string_view(),
          db_config["database"].as_string_view()});
  detector.Detect("build");
  return 0;
}

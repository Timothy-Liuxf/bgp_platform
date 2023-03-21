#include <fstream>
#include <iostream>
#include <string>

#include <jsoncons/json.hpp>

#include <bgp_platform/detector/detector.hpp>
#include <bgp_platform/utils/logger.hpp>

using namespace bgp_platform;
using namespace jsoncons;
using namespace std::literals::string_literals;

int main() {
  try {
    logger.Debug("Debug mode.");

    const auto    config_path = "config/config.json"s;
    std::ifstream config_file(config_path, std::ios::in);
    if (!config_file) {
      logger.Error("Cannot open config file: ", config_path);
      return 1;
    }

    auto config = json::parse(config_file);
    config_file.close();

    const json& init_data_path = config["init-data-path"];
    const json& db_config      = config["database"];

    Detector    detector(
           {init_data_path["as-dict"].as_string_view(),
            init_data_path["top-nx"].as_string_view(),
            init_data_path["top-ip"].as_string_view()},
           {db_config["host"].as_string_view(), db_config["port"].as_string_view(),
            db_config["user"].as_string_view(),
            db_config["password"].as_string_view(),
            db_config["database"].as_string_view()});
    detector.Detect(config["route-data-path"].as_string_view(),
                    config["rib_data_name"].as_string_view());
  } catch (std::exception& e) {
    logger.Error(e.what());
    return 1;
  }
  return 0;
}

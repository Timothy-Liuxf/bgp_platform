#include <fstream>
#include <iostream>
#include <string>

#include <config.h>
#include <tclap/CmdLine.h>
#include <jsoncons/json.hpp>

#include <bgp_platform/datacvt/outage_data_generator.hpp>
#include <bgp_platform/detector/detector.hpp>
#include <bgp_platform/utils/clock.hpp>
#include <bgp_platform/utils/logger.hpp>

using namespace bgp_platform;
using namespace jsoncons;
using namespace std::literals::string_literals;
namespace tclap = TCLAP;

int main(int argc, char* argv[]) {
  try {
    logger.Debug("Debug mode.");

    tclap::CmdLine               cmd(PACKAGE_NAME, ' ', PACKAGE_VERSION);

    tclap::ValueArg<std::string> config_arg("c", "config", "Config file path",
                                            false, "config/config.json",
                                            "string");
    cmd.add(config_arg);

#define MODE_DETECT   "detect"
#define MODE_GENERATE "generate"
    tclap::ValueArg<std::string> mode_arg("m", "mode", "Mode", false, "detect",
                                          MODE_DETECT "|" MODE_GENERATE);
    cmd.add(mode_arg);

    cmd.parse(argc, argv);

    const auto    config_path = config_arg.getValue();
    std::ifstream config_file(config_path, std::ios::in);
    if (!config_file) {
      logger.Error("Cannot open config file: ", config_path);
      return 1;
    }

    auto config = json::parse(config_file);
    config_file.close();

    if (mode_arg.getValue() == MODE_GENERATE) {
      const json& outage_data_generator_config =
          config["outage_data_generator_config"];
      const json& init_data_path = config["init_data_path"];

      const json& start_monitor_time =
          outage_data_generator_config["start_monitor_time"];
      const json& end_monitor_time =
          outage_data_generator_config["end_monitor_time"];
      CalendarTime start_monitor_calendar {};
      CalendarTime end_monitor_calendar {};
      start_monitor_calendar.year   = start_monitor_time["year"].as<int>();
      start_monitor_calendar.month  = start_monitor_time["month"].as<int>();
      start_monitor_calendar.day    = start_monitor_time["day"].as<int>();
      start_monitor_calendar.hour   = start_monitor_time["hour"].as<int>();
      start_monitor_calendar.minute = start_monitor_time["minute"].as<int>();
      start_monitor_calendar.second = start_monitor_time["second"].as<int>();

      end_monitor_calendar.year     = end_monitor_time["year"].as<int>();
      end_monitor_calendar.month    = end_monitor_time["month"].as<int>();
      end_monitor_calendar.day      = end_monitor_time["day"].as<int>();
      end_monitor_calendar.hour     = end_monitor_time["hour"].as<int>();
      end_monitor_calendar.minute   = end_monitor_time["minute"].as<int>();
      end_monitor_calendar.second   = end_monitor_time["second"].as<int>();

      OutageDataGenerator generator(OutageDataGenerator::InitConfig {
          {init_data_path["as_dict"].as_string_view(),
           init_data_path["top_nx"].as_string_view(),
           init_data_path["top_ip"].as_string_view()},
          outage_data_generator_config["proto_data_path"].as_string_view(),
          outage_data_generator_config["output_file_path"].as_string_view(),
          TimePointToTimeStamp(ToTimePoint(start_monitor_calendar)),
          TimePointToTimeStamp(ToTimePoint(end_monitor_calendar))});
      generator.Generate(config["route_data_path"].as_string_view());
    } else if (mode_arg.getValue() == MODE_DETECT) {
      const json& init_data_path = config["init_data_path"];
      const json& db_config      = config["database"];

      Detector    detector({init_data_path["as_dict"].as_string_view(),
                            init_data_path["top_nx"].as_string_view(),
                            init_data_path["top_ip"].as_string_view()},
                           {db_config["host"].as_string_view(),
                            db_config["port"].as_string_view(),
                            db_config["user"].as_string_view(),
                            db_config["password"].as_string_view(),
                            db_config["database"].as_string_view()});
      detector.Detect(config["route_data_path"].as_string_view());
    } else {
      logger.Error("Invalid mode: ", mode_arg.getValue());
      return 1;
    }

  } catch (std::exception& e) {
    logger.Error(e.what());
    return 1;
  }
  return 0;
}

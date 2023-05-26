#include "routedata.hpp"

#include <fstream>
#include <iostream>

BGP_PLATFORM_NAMESPACE_BEGIN

AsPath RouteData::ParseAsPath(
    const std::vector<std::string_view>& as_path_str) {
  AsPath as_path;
  as_path.reserve(as_path_str.size());

  // Aggregated AS: {as1, as2}
  BGP_PLATFORM_IF_UNLIKELY(as_path_str.back().front() == '{') {
    BGP_PLATFORM_IF_UNLIKELY(as_path_str.back().back() != '}') {
      throw std::runtime_error("Invalid AS path!");
    }
    std::transform(
        begin(as_path_str), end(as_path_str) - 1, back_inserter(as_path),
        [](const std::string_view& as_num_str) {
          return AsNum(
              StringToNumber<std::underlying_type_t<AsNum>>(as_num_str));
        });

    auto aggregated_as_str = SplitString(
        as_path_str.back().substr(1, as_path_str.back().size() - 2), ',');
    if (aggregated_as_str.size() > 1) {
      throw std::logic_error(
          "Multiple aggregated AS has not been implemented!");
    }
    as_path.push_back(AsNum(
        StringToNumber<std::underlying_type_t<AsNum>>(aggregated_as_str[0])));
  }
  else {
    std::transform(begin(as_path_str), end(as_path_str), back_inserter(as_path),
                   [](const std::string_view& as_num_str) {
                     return AsNum(StringToNumber<std::underlying_type_t<AsNum>>(
                         as_num_str));
                   });
  }

  return as_path;
}

void RouteData::InsertNewAsPath(AsNum as_num, AsNum vp_num,
                                const AsPath& as_path, const IPPrefix& prefix,
                                TimeStamp timestamp) {
  // Record reachability information of prefix
  auto& as_route_info  = this->route_info_.as_route_info_[as_num];
  auto& as_prefix_info = as_route_info.prefixes[prefix];
  as_prefix_info.reachable_vps.insert(vp_num);
  as_prefix_info.vp_paths[vp_num].emplace(std::move(as_path));

  if (auto withdrawed_vp_paths_itr =
          as_prefix_info.withdrawed_vp_paths.find(vp_num);
      withdrawed_vp_paths_itr != end(as_prefix_info.withdrawed_vp_paths)) {
    auto& withdrawed_vp_paths = withdrawed_vp_paths_itr->second;
    withdrawed_vp_paths.erase(as_path);
  }

  // Record country information
  auto coutry = this->init_info_.GetAsCountry(as_num);
  if (!coutry.empty()) {
    this->route_info_.country_route_info_[coutry].normal_ass.insert(as_num);
  }

  // Record the AS to which the prefix belongs
  this->route_info_.prefix_route_info_[prefix].owner_ass_.insert(as_num);

  this->listener_.OnRouteAdded(as_num, prefix, timestamp);
}

void RouteData::ReadRibFile(fs::path file_path) {
  DumpedFile dumped_file = DumpBGPFile(file_path);
  logger.Info("Dumped file: ", dumped_file.path().c_str());
  std::ifstream file(dumped_file.path(), std::ios::in);
  if (!file) {
    throw std::runtime_error("Failed to open dumped file!");
  }

  bool        is_first_success_line = true;
  std::string time_string           = "";
  for (Line line; std::getline(file, line.buf); ++line.num) {
    try {
      auto fields = SplitString(line.buf, '|');
      if (fields.size() < 7) {
        logger.Warn() << "Failed to parse line: " << line.num
                      << "\n- Content: " << line.buf;
        continue;
      }

      auto timestamp   = StringToNumber<TimeStamp>(fields[1]);
      auto prefix      = StringToIPPrefix(fields[5]);
      auto as_path_str = SplitString(fields[6], ' ');
      auto as_path     = ParseAsPath(as_path_str);

      auto vp_num      = as_path[0];
      auto as_num      = as_path.back();

      BGP_PLATFORM_IF_UNLIKELY(is_first_success_line) {
        time_string =
            fmt::format("{:%Y-%m-%d %H:%M:%S}",
                        fmt::gmtime(std::chrono::system_clock::to_time_t(
                            TimeStampToTimePoint(timestamp))));
        is_first_success_line = false;
      }

      this->InsertNewAsPath(as_num, vp_num, as_path, prefix, timestamp);

      // TODO: Build IP Prefix Tree
    } catch (std::exception& e) {
      logger.Warn() << "Failed to parse line: " << line.num
                    << "\n- Content: " << line.buf
                    << "\n- Exception: " << e.what();
      continue;
    }
  }

  if (is_first_success_line) {
    throw std::runtime_error("Failed to parse any line of rib file!");
  }

  logger.Info("Successfully parsed rib file!");
}

void RouteData::ReadUpdateFile(fs::path file_path) {
  logger.Info() << "Start to read update file: " << file_path.c_str();
  DumpedFile dumped_file = DumpBGPFile(file_path);
  logger.Info() << "Dumped file: " << dumped_file.path().c_str();

  std::ifstream file(dumped_file.path(), std::ios::in);
  if (!file) {
    throw std::runtime_error("Failed to open dumped file!");
  }

  logger.Info("Reading update file...");

  for (Line line; std::getline(file, line.buf); ++line.num) {
    try {
      logger.Debug("Parsing line: ", line.num, " of the updating file.");

      auto fields    = SplitString(line.buf, '|');
      auto timestamp = StringToNumber<TimeStamp>(fields[1]);
      auto flag      = fields[2];
      auto vp_num =
          AsNum(StringToNumber<std::underlying_type_t<AsNum>>(fields[4]));
      auto prefix = StringToIPPrefix(fields[5]);

      if (flag == "W"sv) {
        auto time_string =
            fmt::format("{:%Y-%m-%d %H:%M:%S}",
                        fmt::gmtime(std::chrono::system_clock::to_time_t(
                            TimeStampToTimePoint(timestamp))));
        logger.Debug("Found a withdraw message.");

        if (auto prefix_info_itr =
                this->route_info_.prefix_route_info_.find(prefix);
            prefix_info_itr != end(this->route_info_.prefix_route_info_)) {
          auto& owner_ass = prefix_info_itr->second.owner_ass_;
          logger.Debug("Found owner ASs info.");
          for (auto owner_as : owner_ass) {
            if (auto owner_as_route_info_itr =
                    this->route_info_.as_route_info_.find(owner_as);
                owner_as_route_info_itr !=
                end(this->route_info_.as_route_info_)) {
              auto& owner_as_route_info = owner_as_route_info_itr->second;
              if (auto prefix_info_itr =
                      owner_as_route_info.prefixes.find(prefix);
                  prefix_info_itr != end(owner_as_route_info.prefixes)) {
                auto& prefix_info = prefix_info_itr->second;
                if (auto rechable_vps_itr =
                        prefix_info.reachable_vps.find(vp_num);
                    rechable_vps_itr != end(prefix_info.reachable_vps)) {
                  prefix_info.reachable_vps.erase(rechable_vps_itr);
                  prefix_info.unreachable_vps.insert(vp_num);
                }
                if (auto vp_path_itr = prefix_info.vp_paths.find(vp_num);
                    vp_path_itr != end(prefix_info.vp_paths)) {
                  auto  withdrawed_paths = std::move(vp_path_itr->second);
                  auto& withdrawed_paths_info =
                      prefix_info.withdrawed_vp_paths[vp_num];
                  for (auto& withdrawed_path : withdrawed_paths) {
                    withdrawed_paths_info.emplace(std::move(withdrawed_path));
                  }
                  prefix_info.vp_paths.erase(vp_path_itr);
                }
              }
            }

            this->listener_.OnRouteRemoved(owner_as, prefix, timestamp);
          }
        }
      } else if (flag == "A"sv) {
        auto as_path_str = SplitString(fields[6], ' ');
        auto as_path     = ParseAsPath(as_path_str);
        auto as_num      = as_path.back();

        this->InsertNewAsPath(as_num, vp_num, as_path, prefix, timestamp);
      }
    } catch (std::exception& e) {
      logger.Warn() << "Failed to parse line: " << line.num
                    << "\n- Content: " << line.buf
                    << "\n- Exception: " << e.what();
      continue;
    }
  }

  logger.Info() << "Finished reading update file.";
}

BGP_PLATFORM_NAMESPACE_END

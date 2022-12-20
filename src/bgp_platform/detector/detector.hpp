#ifndef BGP_PLATFORM_DETECTOR_DETECTOR_HPP_
#define BGP_PLATFORM_DETECTOR_DETECTOR_HPP_

#include <bgp_platform/utils/defs.hpp>
#include <bgp_platform/utils/files.hpp>

#include "init_info.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

class Detector {
 public:
  Detector(fs::path as_info_path, fs::path top_nx_path, fs::path top_ip_path);
  Detector(const Detector&) = delete;

  void Detect(fs::path route_data_path);

 private:
  InitInfo init_info_;
};

BGP_PLATFORM_NAMESPACE_END

#endif  // BGP_PLATFORM_DETECTOR_DETECTOR_HPP_

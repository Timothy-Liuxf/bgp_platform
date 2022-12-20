#include "detector.hpp"

BGP_PLATFORM_NAMESPACE_BEGIN

Detector::Detector(fs::path as_info_path, fs::path top_nx_path,
                   fs::path top_ip_path)
    : init_info_(as_info_path, top_nx_path, top_ip_path) {}

BGP_PLATFORM_NAMESPACE_END

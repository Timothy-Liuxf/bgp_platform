#include <bgp_platform/detector/detector.hpp>

using namespace bgp_platform;

int main() {
  Detector detector(
      {"info/new_as_dict.json", "info/top_nx.csv", "info/top_ip.txt"},
      {"localhost", "5432", "testuser", "testpasswd", "testdb"});
  detector.Detect("build");
  return 0;
}

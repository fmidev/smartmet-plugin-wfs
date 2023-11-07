#include <iostream>
#include <macgyver/DateTime.h>
#include "GeoServerDataIndex.h"

namespace pt = boost::posix_time;
using namespace SmartMet::Plugin::WFS;

int main(void)
{
  const char* conn_str = "dbname=DATABASE user=USERNAME password=PASSWORD host=HOSTNAME";
  GeoServerDB db(conn_str);

  const Fmi::DateTime end = Fmi::SecondClock::universal_time();
  const Fmi::DateTime begin = end - Fmi::Minutes(180);

  std::vector<std::string> layers;
  layers.push_back("korpo_hclass_alin_eureffin");
  layers.push_back("vantaa_hclass_alin_eureffin");
  layers.push_back("anjalankoski_hclass_alin_eureffin");

  double bbox[4] = {20, 60, 21, 61};
  const int crs = 4326;

  GeoServerDataIndex gs_index(db);
  gs_index.query(begin, end, layers, bbox, crs);

  for (auto it = gs_index.begin(); it != gs_index.end(); ++it)
  {
    std::cout << std::endl;
    std::cout << pt::to_simple_string(it->second.epoch) << std::endl;

    for (std::size_t i = 0; i < it->second.layers.size(); i++)
    {
      const auto& rec = it->second.layers[i];
      const auto orig_coords = rec.get_orig_coords();
      std::cout << rec.name << "[" << rec.orig_srs << "]: ";
      std::copy(
          orig_coords.begin(), orig_coords.end(), std::ostream_iterator<double>(std::cout, " "));
      std::cout << "\n";
      const auto dest_coords = rec.get_dest_coords();
      std::cout << rec.name << "[" << rec.dest_srs << "]: ";
      std::copy(
          dest_coords.begin(), dest_coords.end(), std::ostream_iterator<double>(std::cout, " "));
      std::cout << "\n";
    }
  }

  return 0;
}

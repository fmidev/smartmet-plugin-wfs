#define BOOST_TEST_MODULE TAreaUtils
#define BOOST_TEST_DYN_LINK 1
#include <iostream>
#include <memory>
#include <macgyver/DateTime.h>
#include <macgyver/Exception.h>
#include <newbase/NFmiLatLonArea.h>
#include <newbase/NFmiMercatorArea.h>
#include <newbase/NFmiRotatedLatLonArea.h>
#include <boost/test/unit_test.hpp>
#include "AreaUtils.h"
#include <boost/algorithm/hex.hpp>

using namespace boost::unit_test;

test_suite *init_unit_test_suite(int argc, char *argv[])
{
  const char *name = "AreaTools tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return NULL;
}

namespace wfs = SmartMet::Plugin::WFS;

BOOST_AUTO_TEST_CASE(latlon_area_1)
{
    OGRPolygon poly;
    NFmiLatLonArea area(NFmiPoint(180.0, -90.0), NFmiPoint(-180, 90.0));
    area.Init();
    try {
        wfs::get_latlon_boundary(&area, &poly);
    } catch (const Fmi::Exception& exc) {
        std::cout << exc << std::endl;
        throw;
    }
    BOOST_CHECK(!poly.IsEmpty());
    BOOST_CHECK(poly.IsValid());
}

BOOST_AUTO_TEST_CASE(mercator_area_1)
{
    OGRPolygon poly;
    NFmiMercatorArea area(NFmiPoint(-180, -90.0), NFmiPoint(180, 90.0));
    try {
        wfs::get_latlon_boundary(&area, &poly);
    } catch (const Fmi::Exception& exc) {
        std::cout << exc << std::endl;
        throw;
    }
    BOOST_CHECK(!poly.IsEmpty());
    BOOST_CHECK(poly.IsValid());
}

BOOST_AUTO_TEST_CASE(rotated_latlon_area_1)
{
    OGRPolygon poly;
    OGRPoint np(0.0, 90.0);
    NFmiRotatedLatLonArea area(NFmiPoint(40.0, 70.0), NFmiPoint(-120, 80.0));
    area.Init();
    wfs::get_latlon_boundary(&area, &poly);
    BOOST_CHECK(!poly.IsEmpty());
    BOOST_CHECK(poly.IsValid());
    BOOST_CHECK(!poly.Contains(&np));
    //std::cout << poly.exportToWkt() << std::endl;
}

#if 1

#pragma message "Broken for now. Must load Geonames engine to reenable test"
int main() { return 0; }

#else

#define BOOST_TEST_MODULE TSupportsTimeZone
#define BOOST_TEST_DYN_LINK 1
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <newbase/NFmiPoint.h>
#include "SupportsTimeZone.h"

using namespace boost::unit_test;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "SupportsTimeZone tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return NULL;
}

using namespace SmartMet::Plugin::WFS;

BOOST_AUTO_TEST_CASE(test_utc_selection)
{
  Fmi::TimeZonePtr tzp;

  BOOST_CHECK_NO_THROW(tzp = SupportsTimeZone::get_tz_for_site(25.0, 60.5, "UTC"));

  std::string test;
  Fmi::DateTime t1 = Fmi::DateTime::fromd_string("2013-05-16 14:38:55");
  BOOST_CHECK_NO_THROW(test = SupportsTimeZone::format_local_time(t1, tzp));
  BOOST_CHECK_EQUAL(test, std::string("2013-05-16T14:38:55Z"));
}

BOOST_AUTO_TEST_CASE(test_local_time_formatting)
{
  Fmi::TimeZonePtr tzp;

  BOOST_CHECK_NO_THROW(tzp = SupportsTimeZone::get_tz_for_site(25.0, 60.5, "Europe/Helsinki"));

  std::string test;
  Fmi::DateTime t1 = Fmi::DateTime::fromd_string("2013-05-16 14:38:55");
  BOOST_CHECK_NO_THROW(test = SupportsTimeZone::format_local_time(t1, tzp));
  BOOST_CHECK_EQUAL(test, std::string("2013-05-16T17:38:55+03:00"));

  t1 = Fmi::DateTime::fromd_string("2013-01-16 14:38:55");
  BOOST_CHECK_NO_THROW(test = SupportsTimeZone::format_local_time(t1, tzp));
  BOOST_CHECK_EQUAL(test, std::string("2013-01-16T16:38:55+02:00"));

  t1 = Fmi::DateTime::fromd_string("2013-03-31 00:59:59");
  BOOST_CHECK_NO_THROW(test = SupportsTimeZone::format_local_time(t1, tzp));
  BOOST_CHECK_EQUAL(test, std::string("2013-03-31T02:59:59+02:00"));

  t1 = Fmi::DateTime::fromd_string("2013-03-31 01:00:00");
  BOOST_CHECK_NO_THROW(test = SupportsTimeZone::format_local_time(t1, tzp));
  BOOST_CHECK_EQUAL(test, std::string("2013-03-31T04:00:00+03:00"));

  t1 = Fmi::DateTime::fromd_string("2013-03-31 01:00:01");
  BOOST_CHECK_NO_THROW(test = SupportsTimeZone::format_local_time(t1, tzp));
  BOOST_CHECK_EQUAL(test, std::string("2013-03-31T04:00:01+03:00"));

  t1 = Fmi::DateTime::fromd_string("2013-10-27 00:59:59");
  BOOST_CHECK_NO_THROW(test = SupportsTimeZone::format_local_time(t1, tzp));
  BOOST_CHECK_EQUAL(test, std::string("2013-10-27T03:59:59+03:00"));

  t1 = Fmi::DateTime::fromd_string("2013-10-27 01:00:00");
  BOOST_CHECK_NO_THROW(test = SupportsTimeZone::format_local_time(t1, tzp));
  BOOST_CHECK_EQUAL(test, std::string("2013-10-27T03:00:00+02:00"));

  t1 = Fmi::DateTime::fromd_string("2013-10-27 01:00:01");
  BOOST_CHECK_NO_THROW(test = SupportsTimeZone::format_local_time(t1, tzp));
  BOOST_CHECK_EQUAL(test, std::string("2013-10-27T03:00:01+02:00"));
}

#endif

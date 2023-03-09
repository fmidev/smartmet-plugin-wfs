#define BOOST_TEST_MODULE TXmlUtils
#define BOOST_TEST_DYN_LINK 1
#include <cstring>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/scoped_array.hpp>
#include <xercesc/util/Janitor.hpp>
#include "XmlEnvInit.h"
#include "XmlUtils.h"

using namespace boost::unit_test;
using namespace SmartMet::Plugin::WFS;

test_suite *init_unit_test_suite(int argc, char *argv[])
{
  const char *name = "XXmlUtils tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return NULL;
}

using namespace SmartMet::Plugin::WFS::Xml;


BOOST_AUTO_TEST_CASE(test_XMLCh_to_string)
{
    EnvInit init;
    const char* src = "Salacgrīva";
    boost::scoped_array<XMLCh> tmp(new XMLCh[std::strlen(src) + 1]);
    xercesc::XMLString::transcode(src, tmp.get(), std::strlen(src));
    std::string back = Xml::to_string(tmp.get());
    BOOST_CHECK_EQUAL(std::string(src), back);
}

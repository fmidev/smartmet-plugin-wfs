#define BOOST_TEST_MODULE TXmlUtils
#define BOOST_TEST_DYN_LINK 1
#include <cstring>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <xercesc/util/Janitor.hpp>
#include "XmlEnvInit.h"
#include "XmlUtils.h"

using namespace boost::unit_test;
using namespace SmartMet::Plugin::WFS;
using namespace xercesc;

test_suite *init_unit_test_suite(int argc, char *argv[])
{
  const char *name = "TXmlUtils tester";
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
    const char* src = "Džūkste";

    XMLCh* tmp = XMLString::transcode(src);
    std::string back = Xml::to_string(tmp);
    XMLString::release(&tmp);
    BOOST_CHECK_EQUAL(std::string(src), back);
}

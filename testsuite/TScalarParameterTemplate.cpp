#define BOOST_TEST_MODULE TScalarParameterTemplate
#define BOOST_TEST_DYN_LINK 1
#include <boost/test/unit_test.hpp>

#include <cstdio>
#include <string>
#include <macgyver/DateTime.h>
#include <macgyver/TypeName.h>
#include "RequestParameterMap.h"
#include "ScalarParameterTemplate.h"
#include "StoredQueryConfig.h"

using namespace boost::unit_test;
using SmartMet::Plugin::WFS::RequestParameterMap;
using SmartMet::Spine::Point;
using SmartMet::Spine::BoundingBox;

test_suite* init_unit_test_suite(int argc, char* argv[])
{
  const char* name = "ScalarParameterTemplate tester";
  unit_test_log.set_threshold_level(log_messages);
  framework::master_test_suite().p_name.value = name;
  BOOST_TEST_MESSAGE("");
  BOOST_TEST_MESSAGE(name);
  BOOST_TEST_MESSAGE(std::string(std::strlen(name), '='));
  return NULL;
}

using SmartMet::Spine::Value;

namespace
{
std::string create_config(const std::string& param_name,
                          const std::string& param_type,
                          const std::string& template_name,
                          const std::string& template_def,
                          const std::string& base_path = "")
{
  char fn_buf[80];
  strncpy(fn_buf, "/tmp/param_tmpl.XXXXXX", sizeof(fn_buf));
  fn_buf[sizeof(fn_buf) - 1] = 0;
  int fd = mkstemp(fn_buf);
  FILE* output = fdopen(fd, "w");

  fputs("disable = false;\n", output);
  fputs("id = \"foo\"; \n", output);
  fputs("constructor_name = \"create_foo\";\n", output);
  fputs("title = {en = \"the title\";};\n", output);
  fputs("abstract = {en = \"the abstract\";};\n", output);
  fputs("template = \"foo.c2t\";\n", output);
  fputs("defaultLanguage = \"en\";\n", output);
  fputs("returnTypeNames = [\"fooRetVal\"];\n", output);
  fputs("parameters:\n", output);
  fputs("(\n", output);
  fputs("  {\n", output);
  fprintf(output, "    name = \"%s\";\n", param_name.c_str());
  fputs("    title = {en=\"the title\";};\n", output);
  fputs("    abstract = {en=\"the abstract\";};\n", output);
  fputs("    xmlType = \"testType\";\n", output);
  fprintf(output, "    type = \"%s\";\n", param_type.c_str());
  fputs("  }\n", output);
  fputs(");\n", output);
  fputs("\n", output);
  fprintf(output, "%s:{\n", base_path == "" ? "handler_params" : base_path.c_str());
  fprintf(output, "%s = %s;\n", template_name.c_str(), template_def.c_str());
  fprintf(output, "};\n");

  fclose(output);
  close(fd);

  return fn_buf;
}

template <typename ValueType>
void add(RequestParameterMap& param_map, const std::string& name, ValueType value)
{
  Value tmp(value);
  param_map.insert_value(name, tmp);
}
}

BOOST_AUTO_TEST_CASE(simple_param_no_default)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing simple scalar parameter without default value]");

  const std::string fn = create_config("test", "string", "foo", "\"${test}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(not item.plain_text);
  BOOST_REQUIRE(item.param_ref);
  BOOST_CHECK_EQUAL(*item.param_ref, std::string("test"));
  BOOST_CHECK(not item.param_ind);
  BOOST_CHECK(not item.default_value);
}

BOOST_AUTO_TEST_CASE(simple_point_param)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing simple SmartMet::Spine::Point parameter without default value]");

  const std::string fn = create_config("test", "point", "foo", "\"${test}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  Point value;
  RequestParameterMap param_map(false);

  add(param_map, "test", "24.32,61.12,EPSG:4258");
  BOOST_CHECK_NO_THROW(value = pt->get<SmartMet::Spine::Point>(param_map));
  BOOST_CHECK_CLOSE(value.x, 24.32, 1e-10);
  BOOST_CHECK_CLOSE(value.y, 61.12, 1e-10);
  BOOST_CHECK_EQUAL(value.crs, std::string("EPSG:4258"));

  param_map.clear();
  add(param_map, "test", "24.32,61.12");
  BOOST_CHECK_NO_THROW(value = pt->get<SmartMet::Spine::Point>(param_map));
  BOOST_CHECK_CLOSE(value.x, 24.32, 1e-10);
  BOOST_CHECK_CLOSE(value.y, 61.12, 1e-10);
  BOOST_CHECK_EQUAL(value.crs, std::string(""));
}

BOOST_AUTO_TEST_CASE(simple_bbox_param)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing simple SmartMet::Spine::BoundingBox parameter without default value]");

  const std::string fn = create_config("test", "bbox", "foo", "\"${test}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  BoundingBox value;
  RequestParameterMap param_map(false);

  add(param_map, "test", "24.32,61.12,25.99,62.17,EPSG:4258");
  BOOST_CHECK_NO_THROW(value = pt->get<SmartMet::Spine::BoundingBox>(param_map));
  BOOST_CHECK_CLOSE(value.xMin, 24.32, 1e-10);
  BOOST_CHECK_CLOSE(value.yMin, 61.12, 1e-10);
  BOOST_CHECK_CLOSE(value.xMax, 25.99, 1e-10);
  BOOST_CHECK_CLOSE(value.yMax, 62.17, 1e-10);
  BOOST_CHECK_EQUAL(value.crs, std::string("EPSG:4258"));

  param_map.clear();
  add(param_map, "test", "24.32,61.12,25.99,62.17");
  BOOST_CHECK_NO_THROW(value = pt->get<SmartMet::Spine::BoundingBox>(param_map));
  BOOST_CHECK_CLOSE(value.xMin, 24.32, 1e-10);
  BOOST_CHECK_CLOSE(value.yMin, 61.12, 1e-10);
  BOOST_CHECK_CLOSE(value.xMax, 25.99, 1e-10);
  BOOST_CHECK_CLOSE(value.yMax, 62.17, 1e-10);
  BOOST_CHECK_EQUAL(value.crs, std::string(""));
}

BOOST_AUTO_TEST_CASE(simple_param_weak_ref_no_default)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing simple scalar parameter (weak reference) without default value]");

  const std::string fn = create_config("test", "string", "foo", "\"%{test2}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(item.weak);
  BOOST_CHECK(not item.plain_text);
  BOOST_REQUIRE(item.param_ref);
  BOOST_CHECK_EQUAL(*item.param_ref, std::string("test2"));
  BOOST_CHECK(not item.param_ind);
  BOOST_CHECK(not item.default_value);
}

BOOST_AUTO_TEST_CASE(simple_param_no_default_non_root)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing simple scalar parameter without default value (not root)]");

  const std::string fn = create_config("test", "string", "bar", "\"${test}\"", "foo");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_THROW(pt.reset(new ScalarParameterTemplate(*config, "fo", "bar", false)),
                      Fmi::Exception);
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", "bar", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(not item.plain_text);
  BOOST_REQUIRE(item.param_ref);
  BOOST_CHECK_EQUAL(*item.param_ref, std::string("test"));
  BOOST_CHECK(not item.param_ind);
  BOOST_CHECK(not item.default_value);
}

BOOST_AUTO_TEST_CASE(simple_param_with_default)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing simple scalar parameter with default value]");

  const std::string fn = create_config("test", "string", "foo", "\"${test:bar}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(not item.plain_text);
  BOOST_REQUIRE(item.param_ref);
  BOOST_CHECK_EQUAL(*item.param_ref, std::string("test"));
  BOOST_CHECK(not item.param_ind);
  BOOST_REQUIRE(item.default_value);
  BOOST_CHECK_EQUAL(*item.default_value, std::string("bar"));

  RequestParameterMap param_map(false);
  add(param_map, "test2", "baz");

  std::string value;
  BOOST_CHECK_NO_THROW(value = pt->get<std::string>(param_map));
  BOOST_CHECK_EQUAL(value, std::string("bar"));

  add(param_map, "test", "baz");
  BOOST_CHECK_NO_THROW(value = pt->get<std::string>(param_map));
  BOOST_CHECK_EQUAL(value, std::string("baz"));

  // Source array size > 1 : must throw exception
  add(param_map, "test", "foo");
  BOOST_CHECK_THROW(value = pt->get<std::string>(param_map), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(simple_param_weak_ref_with_default)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing simple scalar parameter (weak reference) with default value]");

  const std::string fn = create_config("test", "string", "foo", "\"%{test2:bar}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(not item.plain_text);
  BOOST_CHECK(item.weak);
  BOOST_REQUIRE(item.param_ref);
  BOOST_CHECK_EQUAL(*item.param_ref, std::string("test2"));
  BOOST_CHECK(not item.param_ind);
  BOOST_REQUIRE(item.default_value);
  BOOST_CHECK_EQUAL(*item.default_value, std::string("bar"));

  RequestParameterMap param_map(false);
  add(param_map, "test3", "baz");

  std::string value;
  BOOST_CHECK_NO_THROW(value = pt->get<std::string>(param_map));
  BOOST_CHECK_EQUAL(value, std::string("bar"));

  add(param_map, "test2", "baz");
  BOOST_CHECK_NO_THROW(value = pt->get<std::string>(param_map));
  BOOST_CHECK_EQUAL(value, std::string("baz"));
}

BOOST_AUTO_TEST_CASE(simple_param_with_default_containing_space)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE(
      "+ [Testing simple scalar parameter without default value which contains space]");

  const std::string fn = create_config("test", "string", "foo", "\"${test:bar baz 2}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(not item.plain_text);
  BOOST_REQUIRE(item.param_ref);
  BOOST_CHECK_EQUAL(*item.param_ref, std::string("test"));
  BOOST_CHECK(not item.param_ind);
  BOOST_REQUIRE(item.default_value);
  BOOST_CHECK_EQUAL(*item.default_value, std::string("bar baz 2"));

  RequestParameterMap param_map(false);
  add(param_map, "test2", "baz");

  std::string value;
  BOOST_CHECK_NO_THROW(value = pt->get<std::string>(param_map));
  BOOST_CHECK_EQUAL(value, std::string("bar baz 2"));

  add(param_map, "test", "baz");
  BOOST_CHECK_NO_THROW(value = pt->get<std::string>(param_map));
  BOOST_CHECK_EQUAL(value, std::string("baz"));

  // Source array size > 1 : must throw exception
  add(param_map, "test", "foo");
  BOOST_CHECK_THROW(value = pt->get<std::string>(param_map), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(simple_param_from_array_no_default)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing reading scalar parameter an array element]");

  const std::string fn = create_config("test", "int[4]", "foo", "\"${test[3]}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(not item.plain_text);
  BOOST_REQUIRE(item.param_ref);
  BOOST_CHECK_EQUAL(*item.param_ref, std::string("test"));
  BOOST_CHECK(item.param_ind);
  BOOST_CHECK_EQUAL(*item.param_ind, (std::size_t)3);
  BOOST_CHECK(not item.default_value);

  RequestParameterMap param_map(false);
  add(param_map, "test", (int64_t)1);
  add(param_map, "test", (int64_t)2);
  add(param_map, "test", (int64_t)3);
  add(param_map, "test", (int64_t)4);

  int64_t ivalue;
  uint64_t uvalue;
  double dvalue;
  BOOST_CHECK_NO_THROW(ivalue = pt->get<int64_t>(param_map));
  BOOST_CHECK_EQUAL(ivalue, (int64_t)4);
  BOOST_CHECK_NO_THROW(uvalue = pt->get<uint64_t>(param_map));
  BOOST_CHECK_EQUAL(uvalue, (uint64_t)4);
  BOOST_CHECK_NO_THROW(dvalue = pt->get<double>(param_map));
  BOOST_CHECK_CLOSE(dvalue, (double)4, 1e-10);

  BOOST_CHECK_THROW(pt->get<Fmi::DateTime>(param_map), Fmi::Exception);
  BOOST_CHECK_THROW(pt->get<std::string>(param_map), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(simple_param_from_array_with_default)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE(
      "+ [Testing reading scalar parameter from an array element with default value provided]");

  const std::string fn = create_config("test", "time[4]", "foo", "\"${test[3]:201210241526}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(not item.plain_text);
  BOOST_REQUIRE(item.param_ref);
  BOOST_CHECK_EQUAL(*item.param_ref, std::string("test"));
  BOOST_CHECK(item.param_ind);
  BOOST_CHECK_EQUAL(*item.param_ind, (std::size_t)3);
  BOOST_CHECK(item.default_value);
  BOOST_CHECK_EQUAL(*item.default_value, std::string("201210241526"));

  const Fmi::DateTime t0(Fmi::DateTime::from_string("2012-10-24 15:26"));

  const Fmi::DateTime t1(Fmi::DateTime::from_string("2012-10-24 12:34"));
  const Fmi::DateTime t2(Fmi::DateTime::from_string("2012-10-24 13:11"));
  const Fmi::DateTime t3(Fmi::DateTime::from_string("2012-10-24 14:02"));
  const Fmi::DateTime t4(Fmi::DateTime::from_string("2012-10-24 15:43"));

  RequestParameterMap param_map(false);
  Fmi::DateTime value;
  BOOST_CHECK_NO_THROW(value = pt->get<Fmi::DateTime>(param_map));
  BOOST_CHECK_EQUAL(value, t0);
  add(param_map, "test", t1);
  BOOST_REQUIRE_NO_THROW(value = pt->get<Fmi::DateTime>(param_map));
  add(param_map, "test", t2);
  add(param_map, "test", t3);
  add(param_map, "test", t4);
  BOOST_CHECK_NO_THROW(value = pt->get<Fmi::DateTime>(param_map));
  BOOST_CHECK_EQUAL(value, t4);
}

BOOST_AUTO_TEST_CASE(reference_to_non_existing_request_parameter)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing reference to non existing parameter]");

  const std::string fn = create_config("test", "string", "foo", "\"${bar}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(request_of_copying_entire_array_to_scalar)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing an attempt to read an array parameter as a scalar]");

  const std::string fn = create_config("test", "string[3]", "foo", "\"${test}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)), Fmi::Exception);
}

BOOST_AUTO_TEST_CASE(using_absent_parameters)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing an use of absent parameters]");

  const std::string fn = create_config("test", "string[3]", "foo", "\"${}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "foo", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(item.absent);
  BOOST_CHECK(not item.param_ref);
  BOOST_CHECK(not item.param_ind);
  BOOST_CHECK(not item.default_value);

  double foo;
  RequestParameterMap param_map(false);
  add(param_map, "test", 1.0);
  BOOST_CHECK_THROW(pt->get<double>(param_map), Fmi::Exception);
  BOOST_CHECK_NO_THROW(pt->get(param_map, &foo));
}


BOOST_AUTO_TEST_CASE(parameter_definition_missing_from_config)
{
  using namespace SmartMet;
  using namespace SmartMet::Plugin::WFS;

  BOOST_TEST_MESSAGE("+ [Testing case when parameter definition is missing from config (warning expected)]");

  const std::string fn = create_config("test", "string[3]", "foo", "\"${}\"");
  std::shared_ptr<StoredQueryConfig> config;
  BOOST_REQUIRE_NO_THROW(config.reset(new StoredQueryConfig(fn, nullptr)));
  unlink(fn.c_str());

  std::shared_ptr<ScalarParameterTemplate> pt;
  BOOST_REQUIRE_NO_THROW(pt.reset(new ScalarParameterTemplate(*config, "bar", false)));

  const ParameterTemplateItem& item = pt->get_item();
  BOOST_CHECK(item.absent);
  BOOST_CHECK(not item.param_ref);
  BOOST_CHECK(not item.param_ind);
  BOOST_CHECK(not item.default_value);
}

#include "stored_queries/StoredAtomQueryHandlerBase.h"
#include "StoredQueryHandlerFactoryDef.h"
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/variant.hpp>
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>
#include <smartmet/spine/Value.h>
#include <cassert>
#include <limits>
#include <sstream>
#include <vector>

namespace bw = SmartMet::Plugin::WFS;
namespace pt = boost::posix_time;
namespace ph = boost::placeholders;
using SmartMet::Spine::Value;

bw::StoredAtomQueryHandlerBase::StoredAtomQueryHandlerBase(
    SmartMet::Spine::Reactor* reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl& plugin_data,
    boost::optional<std::string> template_file_name)

    : bw::StoredQueryParamRegistry(config)
    , bw::SupportsExtraHandlerParams(config, true)
    , bw::StoredQueryHandlerBase(reactor, config, plugin_data, template_file_name)
{
  try
  {
    const std::string url = config->get_mandatory_config_param<std::string>("url_template.url");

    const std::vector<std::string> url_params =
        config->get_mandatory_config_array<std::string>("url_template.params");

    url_generator.reset(new UrlTemplateGenerator(url, url_params));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bw::StoredAtomQueryHandlerBase::~StoredAtomQueryHandlerBase() {}

void bw::StoredAtomQueryHandlerBase::init_handler() {}

void bw::StoredAtomQueryHandlerBase::query(const bw::StoredQuery& query,
                                           const std::string& language,
					   const boost::optional<std::string> &hostname,
                                           std::ostream& output) const
{
  try
  {
    const char* PS_NAME = "param_sets";

    const int debug_level = get_config()->get_debug_level();
    if (debug_level > 0)
      query.dump_query_info(std::cout);

    CTPP::CDT hash;
    hash["language"] = language;
    hash["responseTimestamp"] =
        Fmi::to_iso_extended_string(get_plugin_impl().get_time_stamp()) + "Z";

    const RequestParameterMap& req_param_map = query.get_param_map();

    std::vector<boost::shared_ptr<RequestParameterMap> > param_sets;
    update_parameters(req_param_map, query.get_query_id(), param_sets);

    hash["numMatched"] = param_sets.size();
    hash["numReturned"] = param_sets.size();
    hash["queryNum"] = query.get_query_id();

    hash["hostname"] = QueryBase::HOSTNAME_SUBST;
    hash["protocol"] = QueryBase::PROTOCOL_SUBST;

    if(!hostname)
      throw Fmi::Exception(BCP, "Hostname unknown");

    CTPP::CDT h_hosts;
    h_hosts["wms"] = get_config()->get_hosts().getWMSHost(*hostname);
    hash["hosts"] = h_hosts;

    if (get_config()->get_hosts().getKeepApikeyFlag(*hostname))
    {
      hash["fmi_apikey"] = bw::QueryBase::FMI_APIKEY_SUBST;
      hash["fmi_apikey_prefix"] = bw::QueryBase::FMI_APIKEY_PREFIX_SUBST;
    }
    else
    {
      hash["fmi_apikey"] = "";
      hash["fmi_apikey_prefix"] = "";
    }

    if ((debug_level > 1) and param_sets.empty())
    {
      Fmi::Exception exception(BCP, "No data available!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      throw exception.disableStackTrace();
    }

    req_param_map.dump_params(hash["query_parameters"]);
    dump_named_params(req_param_map, hash["named_parameters"]);

    for (std::size_t i = 0; i < param_sets.size(); i++)
    {
      const auto& param_set = *param_sets.at(i);
      CTPP::CDT& hash_item = hash[PS_NAME][i];
      auto get_param_cb =
          boost::bind(&bw::StoredAtomQueryHandlerBase::get_param_callback, this,  ph::_1, &param_set);
      const std::string url = url_generator->generate(get_param_cb);
      hash_item["URL"] = url;

      std::set<std::string> param_names = param_set.get_keys();

      for (const auto& param_name : param_names)
      {
        int cnt = 0;
	const std::vector<SmartMet::Spine::Value> values = param_set.get_values(param_name);
	for (auto v : values)
	{
	  hash_item["params"][param_name][cnt++] = v.to_string();
	}
      }
    }

    format_output(hash, output, query.get_use_debug_format());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void bw::StoredAtomQueryHandlerBase::update_parameters(
    const bw::RequestParameterMap& request_params,
    int seq_id,
    std::vector<boost::shared_ptr<bw::RequestParameterMap> >& result) const
{
  try
  {
    result.clear();
    boost::shared_ptr<bw::RequestParameterMap> tmp(
	new bw::RequestParameterMap(request_params.get_map(true), true));
    tmp->add("queryNum", seq_id);
    result.push_back(tmp);
    std::cout << tmp->as_string() << std::endl;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<std::string> bw::StoredAtomQueryHandlerBase::get_param_callback(
    const std::string& param_name, const bw::RequestParameterMap* param_map) const
{
  try
  {
    assert(param_map);

    std::vector<std::string> result;

    const auto& param = get_param(param_name);
    boost::variant<SmartMet::Spine::Value, std::vector<SmartMet::Spine::Value> > param_value;
    boost::tribool found = param.get_value(param_value, *param_map, this);
    if (found)
    {
      const int which = param_value.which();
      if (which == 0 /* SmartMet::Spine::Value */)
      {
        std::string tmp = boost::get<SmartMet::Spine::Value>(param_value).to_string();
        result.push_back(tmp);
      }
      else /* std::vector<SmartMet::Spine::Value> */
      {
        const auto& vect = boost::get<std::vector<SmartMet::Spine::Value> >(param_value);
        for (std::size_t i = 0; i < vect.size(); i++)
        {
          std::string tmp = vect.at(i).to_string();
          result.push_back(tmp);
        }
      }
    }

    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

namespace
{
using namespace SmartMet::Plugin::WFS;

boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> wfs_stored_atom_handler_create(
    SmartMet::Spine::Reactor* reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl& plugin_data,
    boost::optional<std::string> template_file_name)
{
  try
  {
    StoredAtomQueryHandlerBase* qh =
        new StoredAtomQueryHandlerBase(reactor, config, plugin_data, template_file_name);
    boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> result(qh);
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace

SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_stored_atom_handler_base_factory(
    &wfs_stored_atom_handler_create);

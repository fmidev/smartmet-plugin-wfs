#include "StoredQueryHandlerFactoryDef.h"
#include "DummyStoredQueryHandler.h"

namespace bw = SmartMet::Plugin::WFS;

namespace
{
  const char* P_ID_SCALAR = "ScalarParam";
  const char* P_ID_ARRAY = "ArrayParam";
}

bw::DummyStoredQueryHandler::DummyStoredQueryHandler(SmartMet::Spine::Reactor* reactor,
						     StoredQueryConfig::Ptr config,
						     PluginImpl& plugin_impl,
						     std::optional<std::string>  /*template_file_name*/)
  : bw::StoredQueryParamRegistry(config)
  , bw::SupportsExtraHandlerParams(config)
  , bw::StoredQueryHandlerBase(reactor, config, plugin_impl, std::optional<std::string>())
{
  try
  {
    register_scalar_param<std::string>(P_ID_SCALAR, "Scalar parameter for testing only", false);
    register_array_param<std::string>(P_ID_ARRAY, "Array parameter for testing only", 0, 99, 1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bw::DummyStoredQueryHandler::~DummyStoredQueryHandler() = default;

void bw::DummyStoredQueryHandler::init_handler()
{
}

std::string bw::DummyStoredQueryHandler::get_handler_description() const
{
    return "Simple stored query handler for testing purposes only";
}

void
bw::DummyStoredQueryHandler::query(const StoredQuery& query,
				   const std::string&  /*language*/,
				   const std::optional<std::string>&  /*hostname*/,
				   std::ostream& output) const
{
  try
    {
      auto params = query.get_param_map();

      Json::Value result = Json::objectValue;

      std::set<std::string> keys = params.get_keys();
      for (const auto& k : keys)
	{
	  std::vector<SmartMet::Spine::Value> values = params.get_values(k);
	  if (!values.empty()) {
	    Json::Value& tmp = result[k];
	    if (values.size() == 1) {
	      tmp = values[0].to_string();
	    } else {
	      for (std::size_t i = 0; i < values.size(); i++) {
		tmp[Json::ArrayIndex(i)] = values[i].to_string();
	      }
	    }
	  }
	}
      output << result;
    }
  catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
}

namespace
{
using namespace SmartMet::Plugin::WFS;

std::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> dummy_handler_create(
    SmartMet::Spine::Reactor* reactor,
    StoredQueryConfig::Ptr config,
    PluginImpl& plugin_data,
    std::optional<std::string> template_file_name)
{
  try
  {
    auto* qh =
        new DummyStoredQueryHandler(reactor, config, plugin_data, template_file_name);
    std::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> result(qh);
    return result;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace

SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef wfs_dummy_handler_factory(
    &dummy_handler_create);

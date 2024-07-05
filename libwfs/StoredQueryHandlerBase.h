#pragma once

#include "PluginImpl.h"
#include "StandardPresentationParameters.h"
#include "StoredQuery.h"
#include "StoredQueryConfig.h"
#include "StoredQueryHandlerInitBase.h"
#include "StoredQueryMap.h"
#include "StoredQueryParamRegistry.h"
#include "SupportsExtraHandlerParams.h"

#include <spine/CRSRegistry.h>
#include <spine/Reactor.h>
#include <spine/Value.h>

#include <macgyver/TemplateFormatter.h>
#include <macgyver/ValueFormatter.h>

#include <optional>
#include <memory>
#include <boost/thread.hpp>

namespace Fmi
{
class TemplateFormatter;
}

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class PluginImpl;
class StoredQuery;
class StoredQueryMap;

class StoredQueryHandlerBase : virtual protected SupportsExtraHandlerParams,
                               virtual protected StoredQueryHandlerInitBase
{
  SmartMet::Spine::Reactor* reactor;
  StoredQueryConfig::Ptr config;
  bool hidden{false};

 protected:
  const PluginImpl& plugin_impl;
  std::optional<std::string> template_file;  // needed by StoredFlashQueryHandler

 public:
  StoredQueryHandlerBase(SmartMet::Spine::Reactor* reactor,
                         StoredQueryConfig::Ptr config,
                         PluginImpl& plugin_impl,
                         std::optional<std::string> template_file_name);

  ~StoredQueryHandlerBase() override;

  void perform_init();

  virtual std::string get_query_name() const;

  /**
   *   @brief Returns short description of stored query handler
   *
   *   The return value of this method is expected to be same for all
   *   stored queries which use the same handler
   */
  virtual std::string get_handler_description() const = 0;

  virtual std::string get_title(const std::string& language) const;

  virtual std::vector<std::string> get_return_types() const;

  /**
   *   @brief Performs initial processing of stored query parameters
   *
   *   The default action (in base class) is to simply return
   *   original parameters unchanged.
   */
  virtual std::shared_ptr<RequestParameterMap> process_params(
      const std::string& stored_query_id, std::shared_ptr<RequestParameterMap> orig_params) const;

  virtual void query(const StoredQuery& query,
                     const std::string& language,
                     const std::optional<std::string>& hostname,
                     std::ostream& output) const = 0;

  /**
   *   @brief Check whether the request must be redirected to a different
   *          stored request
   *
   *   @param query The StoredQuery object
   *   @param new_stored_query_id The redirected stored query ID (only need to be set
   *          when return value is @c true)
   *   @retval false no redirection (the original request is OK)
   *   @retval true the request must be redirected to a different stored query handler.
   *          In this case method @b must put redirected stored query ID into the second
   *          parameter
   *
   *   The return value is @b false in the base class.
   */
  virtual bool redirect(const StoredQuery& query, std::string& new_stored_query_id) const;

  inline std::shared_ptr<const StoredQueryConfig> get_config() const { return config; }
  const StoredQueryMap& get_stored_query_map() const;

  inline bool is_hidden() const { return hidden; }
  inline std::set<std::string> get_handler_param_names() const
  {
    return StoredQueryParamRegistry::get_param_names();
  }

  const std::string& get_data_source() const;
  bool is_gridengine_disabled() const;

  std::map<std::string, HandlerFactorySummary::ParamInfo> get_param_info() const;

protected:
  virtual void init_handler();

  std::shared_ptr<Fmi::TemplateFormatter> get_formatter(bool debug_format) const;

  inline SmartMet::Spine::Reactor* get_reactor() const { return reactor; }
  inline const PluginImpl& get_plugin_impl() const { return plugin_impl; }
  void format_output(CTPP::CDT& hash, std::ostream& output, bool debug_format) const;

  static std::pair<std::string, std::string> get_2D_coord(
      std::shared_ptr<SmartMet::Spine::CRSRegistry::Transformation> transformation,
      double X,
      double Y);

  static void set_2D_coord(
      std::shared_ptr<SmartMet::Spine::CRSRegistry::Transformation> transformation,
      double X,
      double Y,
      CTPP::CDT& hash);

  static void set_2D_coord(
      std::shared_ptr<SmartMet::Spine::CRSRegistry::Transformation> transformation,
      const std::string& X,
      const std::string& Y,
      CTPP::CDT& hash);
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

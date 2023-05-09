#pragma once

#include "ArrayParameterTemplate.h"
#include "HandlerFactorySummary.h"
#include "RequestParameterMap.h"
#include "ScalarParameterTemplate.h"
#include "StoredQueryConfig.h"
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <json/value.h>
#include <map>
#include <set>
#include <typeinfo>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class SupportsExtraHandlerParams;
class ScalarParameterTemplate;
class ArrayParameterTemplate;

/**
 *   @brief Virtual base class for implementing registry of stored query
 *          parameters
 */
class StoredQueryParamRegistry : public StoredQueryConfig::Wrapper
{
  struct ParamRecBase
  {
    std::string name;
    std::string type_name;
    std::string description;

    virtual ~ParamRecBase();
  };

  struct ScalarParameterRec : public ParamRecBase
  {
    boost::shared_ptr<ScalarParameterTemplate> param_def;
    bool required;

     ~ScalarParameterRec() override;
  };

  struct ArrayParameterRec : public ParamRecBase
  {
    boost::shared_ptr<ArrayParameterTemplate> param_def;
    std::size_t min_size;
    std::size_t max_size;
    std::size_t step;

    ~ArrayParameterRec() override;
  };

 public:
  StoredQueryParamRegistry(StoredQueryConfig::Ptr config);

  ~StoredQueryParamRegistry() override;

  /**
   *   @brief Resolve provided stored query handler parameters from provided pre-processed
   *          query parameters and configuration.
   */
  boost::shared_ptr<RequestParameterMap> resolve_handler_parameters(
      const RequestParameterMap& src, const SupportsExtraHandlerParams* extra_params = nullptr) const;

  std::set<std::string> get_param_names() const;

  std::map<std::string, HandlerFactorySummary::ParamInfo> get_param_info() const;

  template <typename ParamType>
  void register_scalar_param(const std::string& name,
                             const std::string& description,
                             bool required = true,
			     bool silent = false);

  template <typename ParamType>
  void register_array_param(const std::string& name,
                            const std::string& description,
                            std::size_t min_size = 0,
                            std::size_t max_size = std::numeric_limits<uint16_t>::max(),
                            std::size_t step = 0,
			    bool silent = false);

  void register_scalar_param(const std::string& name,
                             const std::string& description,
                             boost::shared_ptr<ScalarParameterTemplate> param_def,
                             bool required);

  void register_array_param(const std::string& name,
                            const std::string& description,
                            boost::shared_ptr<ArrayParameterTemplate> param_def,
                            std::size_t min_size = 0,
                            std::size_t max_size = std::numeric_limits<uint16_t>::max());

  void silence_param_init_warnings(bool enable) { silence_param_init_warnings_ = enable; }

 private:
  void add_param_rec(boost::shared_ptr<ParamRecBase> rec);

 private:
  bool silence_param_init_warnings_{false};
  std::map<std::string, boost::shared_ptr<ParamRecBase> > param_map;
  std::map<std::string, int> supported_type_names;
};

template <typename ParamType>
void StoredQueryParamRegistry::register_scalar_param(const std::string& name,
                                                     const std::string& description,
                                                     bool required,
						     bool silent)
{
  boost::shared_ptr<ScalarParameterRec> rec(new ScalarParameterRec);
  rec->name = name;
  rec->description = description;
  rec->param_def.reset(new ScalarParameterTemplate(*get_config(), name,
						   silence_param_init_warnings_ or silent));
  rec->type_name = typeid(ParamType).name();
  rec->required = required;
  add_param_rec(rec);
}

template <typename ParamType>
void StoredQueryParamRegistry::register_array_param(const std::string& name,
                                                    const std::string& description,
                                                    std::size_t min_size,
                                                    std::size_t max_size,
                                                    std::size_t step,
						    bool silent)
{
  boost::shared_ptr<ArrayParameterRec> rec(new ArrayParameterRec);
  rec->name = name;
  rec->description = description;
  rec->param_def.reset(new ArrayParameterTemplate(*get_config(), name, min_size, max_size,
						  silence_param_init_warnings_ or silent));
  rec->type_name = typeid(ParamType).name();
  rec->min_size = min_size;
  rec->max_size = max_size;
  rec->step = step;
  add_param_rec(rec);
}

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

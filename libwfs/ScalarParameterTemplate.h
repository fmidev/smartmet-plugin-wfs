#pragma once

#include "ParameterTemplateBase.h"
#include "ParameterTemplateItem.h"
#include <spine/Value.h>
#include <map>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class SupportsExtraHandlerParams;

class ScalarParameterTemplate : public ParameterTemplateBase
{
 public:
  // explicit is intentionally used below to avoid ambiguity between conversions
  // -  const char* -> std::string
  // -  const char* -> bool
  // which neither GCC nor CLANG++ detect (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=104365)
  explicit ScalarParameterTemplate(StoredQueryConfig& config,
				   const char* config_path,
				   bool silent)
    : ScalarParameterTemplate(config, std::string(config_path), silent)
  {
  }

  explicit ScalarParameterTemplate(StoredQueryConfig& config,
				   const std::string& config_path,
				   bool silent);

  ScalarParameterTemplate(StoredQueryConfig& config,
                          const std::string& base_path,
                          const std::string& config_path,
			  bool silent);

  ~ScalarParameterTemplate() override;

  inline const ParameterTemplateItem& get_item() const { return item; }
  /**
   *   @brief Gets the value of parameter from request parameter map
   *
   *   In case of HTTP KVP request (POST with contant type application/x-www-form-urlencoded
   *   or simple GET request) this map is generated by
   *   SmartMet::Plugin::WFS::StoredQueryHandlerBase::parse_kvp_parameters.
   */
  SmartMet::Spine::Value get_value(const RequestParameterMap& req_param_map,
                                   const SupportsExtraHandlerParams* extra_params = nullptr) const;

  bool get_value(SmartMet::Spine::Value& result,
                 const RequestParameterMap& req_param_map,
                 const SupportsExtraHandlerParams* extra_params = nullptr) const;

  boost::tribool get_value(
      std::variant<SmartMet::Spine::Value, std::vector<SmartMet::Spine::Value> >& result,
      const RequestParameterMap& req_param_map,
      const SupportsExtraHandlerParams* extra_params = nullptr,
      bool strict = true) const override;

  template <typename ValueType>
  ValueType get(
      const RequestParameterMap& req_param_map,
      const SupportsExtraHandlerParams* extra_params = nullptr) const
    {
        try {
            const auto tmp = get_optional<ValueType>(req_param_map, extra_params);
            if (tmp) {
                return *tmp;
            } else {
                const std::string name = this->get_config_path();
                throw Fmi::Exception::Trace(BCP, "Mandatory scalar query parameter '"
                    + name + "' missing").disableStackTrace();
            }
        } catch (...) {
            throw Fmi::Exception::Trace(BCP, "Operation failed!");
        }
    }

  template <typename ValueType>
  bool get(const RequestParameterMap& req_param_map,
           ValueType* dest,
           const SupportsExtraHandlerParams* extra_params = nullptr) const
    {
        try {
            const auto tmp = Getter<ValueType>(*this)(req_param_map, extra_params);
            if (tmp) {
                *dest = *tmp;
                return true;
            } else {
                return false;
            }
        } catch (...) {
            throw Fmi::Exception::Trace(BCP, "Operation failed!");
        }
    }

  template <typename ValueType>
  std::optional<ValueType> get_optional(
      const RequestParameterMap& req_param_map,
      const SupportsExtraHandlerParams* extra_params = nullptr) const
    {
        try {
            return Getter<ValueType>(*this)(req_param_map, extra_params);
        } catch (...) {
            throw Fmi::Exception::Trace(BCP, "Operation failed!");
        }
    }

 private:
  template<typename ValueType>
  class Getter
  {
   public:
      Getter(const ScalarParameterTemplate& spt) : spt(spt) {}

      std::optional<ValueType> operator()(
          const RequestParameterMap& req_param_map,
          const SupportsExtraHandlerParams* extra_params = nullptr) const
      {
          SmartMet::Spine::Value tmp;
          bool found = spt.get_value(tmp, req_param_map, extra_params);
          if (found) {
              return std::optional<ValueType>(extract(tmp));
          } else {
              return std::nullopt;
          }
      }

   private:
      ValueType extract(const SmartMet::Spine::Value& src) const
      {
          return src.get<ValueType>();
      }

  private:
      const ScalarParameterTemplate& spt;
  };

 private:
  void init(bool silent);

 private:
  ParameterTemplateItem item;
};

template <>
Fmi::DateTime
ScalarParameterTemplate::Getter<Fmi::DateTime>::extract(const SmartMet::Spine::Value& src) const;

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

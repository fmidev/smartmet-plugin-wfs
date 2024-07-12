#pragma once

#include "StoredQueryConfig.h"
#include <memory>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
struct MeteoParameterOptionItem
{
  size_t precision{1};  // The precision defines the number of digits to the right of the decimal
                     // point.
  double missing_value{32700.0};
  std::string missing_text;
  unsigned short sensor_first{1};
  unsigned short sensor_last{1};
  unsigned short sensor_step{1};
  MeteoParameterOptionItem() :  missing_text("NaN") {}
};

using MeteoParameterOptions = std::map<std::string, MeteoParameterOptionItem>;

/**
 *   @brief The class implementing options for meteo parameters.
 */
class SupportsMeteoParameterOptions
{
 public:
  SupportsMeteoParameterOptions(std::shared_ptr<SmartMet::Plugin::WFS::StoredQueryConfig> config);

  virtual ~SupportsMeteoParameterOptions();

  /**
   * @brief Change the default precision.
   */
  void setDefaultPrecision(const size_t& value);

  /**
   * @brief Change the default missing text.
   */
  void setDefaultMissingText(const std::string& text);

  /**
   * @brief Change the default missing value.
   */
  void setDefaultMissingValue(const double& value);

  /**
   * @brief Change the default sensor first number.
   */
  void setDefaultSensorFirst(const unsigned short& value);

  /**
   * @brief Change the default sensor last number.
   */
  void setDefaultSensorLast(const unsigned short& value);

  /**
   * @brief Change the default sensor step number.
   */
  void setDefaultSensorStep(const unsigned short& value);

  /**
   * @brief Get meteoparameter options.
   * @param[in] name Meteorological parameter name (case insensitive)
   * @return Meteorological parameter option item.
   */
  std::shared_ptr<MeteoParameterOptionItem> get_meteo_parameter_options(
      const std::string& name) const;

  bool have_meteo_param_options(const std::string& name) const;

 private:
  int debug_level;
  MeteoParameterOptions options_map;
  MeteoParameterOptionItem default_option_item;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

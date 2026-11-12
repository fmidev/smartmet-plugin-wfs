#pragma once

#include "RequestParameterMap.h"
#include "StoredQueryParamRegistry.h"
#include "SupportsExtraHandlerParams.h"
#include <timeseries/TimeSeriesGeneratorOptions.h>
#include <timeseries/TimeSeriesInclude.h>
#include <memory>

namespace SmartMet::Plugin::WFS
{
class StoredQueryConfig;
}  // namespace SmartMet::Plugin::WFS

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
/**
 *   @brief Base class for implementing time related stored queries
 *          parameter support
 */
class SupportsTimeParameters : protected virtual SupportsExtraHandlerParams,
                               protected virtual StoredQueryParamRegistry
{
 public:
  SupportsTimeParameters(const std::shared_ptr<SmartMet::Plugin::WFS::StoredQueryConfig>& config);

  ~SupportsTimeParameters() override;

  std::shared_ptr<TS::TimeSeriesGeneratorOptions> get_time_generator_options(
      const RequestParameterMap &param_values) const;

 private:
  int debug_level;

 protected:
  static const char *P_HOURS;
  static const char *P_TIMES;
  static const char *P_BEGIN_TIME;
  static const char *P_START_STEP;
  static const char *P_END_TIME;
  static const char *P_TIME_STEP;
  static const char *P_NUM_STEPS;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#pragma once

#include "RequiresGeoEngine.h"
#include "StoredQueryConfig.h"
#include "StoredQueryParamRegistry.h"
#include "SupportsExtraHandlerParams.h"
#include <macgyver/LocalDateTime.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
/**
 *  @brief Base class for adding time zone support to stored query handler
 */
class SupportsTimeZone : protected virtual SupportsExtraHandlerParams,
                         protected virtual StoredQueryParamRegistry,
                         protected virtual RequiresGeoEngine
{
 public:
    SupportsTimeZone(SmartMet::Spine::Reactor* reactor, boost::shared_ptr<StoredQueryConfig> config);

  ~SupportsTimeZone() override;

  std::string get_tz_name(const RequestParameterMap& param_values) const;

  Fmi::TimeZonePtr get_tz_for_site(double longitude,
                                                   double latitude,
                                                   const std::string& tz_name) const;

  Fmi::TimeZonePtr get_time_zone(const std::string& tz_name) const;

  static std::string format_local_time(const Fmi::DateTime& utc_time,
                                       Fmi::TimeZonePtr tz);
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#pragma once

#include <engines/contour/Options.h>
#include <engines/querydata/Q.h>
#include <gis/Types.h>
#include <grid-content/queryServer/definition/Query.h>
#include <macgyver/DateTime.h>
#include <spine/Value.h>
#include <timeseries/TimeSeriesGenerator.h>
#include <timeseries/TimeSeriesInclude.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class OGRSpatialReference;
namespace SmartMet::Spine
{
class Parameter;
}  // namespace SmartMet::Spine

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
struct ContourQueryParameter
{
  const SmartMet::Spine::Parameter& parameter;
  const SmartMet::Engine::Querydata::Q& q;
  OGRSpatialReference& sr;
  SmartMet::Spine::BoundingBox bbox;
  std::string tz_name;
  bool smoothing{false};
  unsigned short smoothing_degree{2};
  unsigned short smoothing_size{2};
  TS::TimeSeriesGenerator::LocalTimeList tlist;
  mutable QueryServer::Query gridQuery;

  ContourQueryParameter(const SmartMet::Spine::Parameter& p,
                        const SmartMet::Engine::Querydata::Q& qe,
                        OGRSpatialReference& spref)
      : parameter(p),
        q(qe),
        sr(spref),
        bbox(-180.0, -90.0, 180.0, 90.0),
        tz_name("UTC")
        
  {
  }
  virtual ~ContourQueryParameter() = default;
};

struct CoverageQueryParameter : ContourQueryParameter
{
  CoverageQueryParameter(const SmartMet::Spine::Parameter& p,
                         const SmartMet::Engine::Querydata::Q& q,
                         OGRSpatialReference& sr)
      : ContourQueryParameter(p, q, sr)
  {
  }

  std::vector<SmartMet::Engine::Contour::Range> limits;
};

struct IsolineQueryParameter : ContourQueryParameter
{
  IsolineQueryParameter(const SmartMet::Spine::Parameter& p,
                        const SmartMet::Engine::Querydata::Q& q,
                        OGRSpatialReference& sr)
      : ContourQueryParameter(p, q, sr)
  {
  }

  std::string unit;
  std::vector<double> isovalues;
};

// contains timestamp and corresponding geometry
// for example heavy snow areas at 12:00
struct WeatherAreaGeometry
{
  Fmi::DateTime timestamp;
  OGRGeometryPtr geometry;

  WeatherAreaGeometry(const Fmi::DateTime& t, OGRGeometryPtr g)
      : timestamp(t), geometry(std::move(g))
  {
  }
};

struct ContourQueryResult
{
  std::string name;
  std::string unit;
  std::vector<WeatherAreaGeometry> area_geoms;
};

// contains weather name and result set for a query
// for example heavy snow areas at 12:00, 13:00, 14:00
struct CoverageQueryResult : ContourQueryResult
{
  double lolimit = 0.0;
  double hilimit = 0.0;
};

struct IsolineQueryResult : ContourQueryResult
{
  double isovalue = 0.0;
};

using ContourQueryResultPtr = std::shared_ptr<ContourQueryResult>;
using CoverageQueryResultPtr = std::shared_ptr<CoverageQueryResult>;
using IsolineQueryResultPtr = std::shared_ptr<IsolineQueryResult>;

using ContourQueryResultSet = std::vector<ContourQueryResultPtr>;

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

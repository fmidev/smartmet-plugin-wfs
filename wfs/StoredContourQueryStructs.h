#pragma once

#include "PluginImpl.h"
#include "StoredQueryConfig.h"
#include "StoredQueryHandlerBase.h"
#include "StoredQueryHandlerFactoryDef.h"
#include "SupportsBoundingBox.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsTimeParameters.h"
#include "SupportsTimeZone.h"

#include <engines/contour/Engine.h>
#include <engines/querydata/Engine.h>
#include <grid-content/queryServer/definition/Query.h>

#include <gis/OGR.h>

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
      : timestamp(t), geometry(g)
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
  double lolimit;
  double hilimit;
};

struct IsolineQueryResult : ContourQueryResult
{
  double isovalue;
};

using ContourQueryResultPtr = boost::shared_ptr<ContourQueryResult>;
using CoverageQueryResultPtr = boost::shared_ptr<CoverageQueryResult>;
using IsolineQueryResultPtr = boost::shared_ptr<IsolineQueryResult>;

using ContourQueryResultSet = std::vector<ContourQueryResultPtr>;

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

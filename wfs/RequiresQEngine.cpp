#include "RequiresQEngine.h"
#include <boost/algorithm/string/join.hpp>
#include <engines/querydata/MetaQueryOptions.h>
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>
#include <set>

namespace ba = boost::algorithm;
namespace qe = SmartMet::Engine::Querydata;

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{

std::string RequiresQEngine::get_qengine_origintime_qualifier(
    const std::string& producer, const std::optional<Fmi::DateTime>& origin_time) const
{
  try
  {
    if (!q_engine || producer.empty())
      return {};

    qe::MetaQueryOptions opt;
    opt.setProducer(producer);
    if (origin_time)
      opt.setOriginTime(*origin_time);

    std::set<std::string> qualifiers;
    for (const auto& meta_info : q_engine->getEngineMetadata(opt))
      qualifiers.insert(meta_info.producer + ':' + Fmi::to_iso_string(meta_info.originTime));

    return ba::join(qualifiers, ",");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#include "RequiresGridEngine.h"
#include <boost/algorithm/string/join.hpp>
#include <grid-content/contentServer/definition/GenerationInfo.h>
#include <macgyver/Exception.h>
#include <set>

namespace ba = boost::algorithm;

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{

std::string RequiresGridEngine::get_grid_generation_qualifier(
    const std::vector<std::string>& producer_aliases) const
{
  try
  {
    if (!grid_engine || !grid_engine->isEnabled())
      return {};

    auto content_server = grid_engine->getContentServer_sptr();
    if (!content_server)
      return {};

    std::set<std::string> qualifiers;
    for (const auto& alias : producer_aliases)
    {
      // Resolve the WFS producer alias into the real grid producer name(s).
      const std::string mapping_name = grid_engine->getProducerName(alias);
      std::vector<std::string> producer_names;
      grid_engine->getProducerNameList(mapping_name, producer_names);
      if (producer_names.empty())
        producer_names.push_back(mapping_name);

      for (const auto& producer_name : producer_names)
      {
        T::GenerationInfo info;
        if (content_server->getLastGenerationInfoByProducerNameAndStatus(
                0, producer_name, T::GenerationInfo::Status::Ready, info) == 0)
          qualifiers.insert(producer_name + ':' + info.mAnalysisTime);
      }
    }

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

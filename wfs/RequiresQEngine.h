#pragma once

#include "StoredQueryHandlerInitBase.h"
#include <spine/Reactor.h>
#include <engines/querydata/Engine.h>
#include <macgyver/DateTime.h>
#include <optional>
#include <string>
namespace SmartMet
{
namespace Plugin
{
namespace WFS
{

class RequiresQEngine : protected virtual StoredQueryHandlerInitBase
{
public:
  RequiresQEngine(SmartMet::Spine::Reactor* reactor)
    {
        add_init_action(
            "Acquire QEngine",
            [this, reactor]() {
                q_engine = reactor->getEngine<SmartMet::Engine::Querydata::Engine>("Querydata");
            });
    }

protected:
  /**
   *   @brief Builds a response cache key qualifier from the model origin time
   *
   *   Returns a string holding the origin (analysis) time of the querydata
   *   model(s) matching the given producer, so that the value changes when a
   *   new model run becomes available and invalidates cached responses. When an
   *   explicit origin time is requested it is passed through unchanged. Returns
   *   an empty string when no matching model is found.
   */
  std::string get_qengine_origintime_qualifier(
      const std::string& producer,
      const std::optional<Fmi::DateTime>& origin_time = std::nullopt) const;

  std::shared_ptr<SmartMet::Engine::Querydata::Engine> q_engine;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

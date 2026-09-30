#pragma once

#include "StoredQueryHandlerInitBase.h"
#include <spine/Reactor.h>
#include <engines/grid/Engine.h>
#include <string>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{

class RequiresGridEngine : protected virtual StoredQueryHandlerInitBase
{
public:
  RequiresGridEngine(SmartMet::Spine::Reactor* reactor)
    {
        add_init_action(
            "Acquire Grid engine",
            [this, reactor]() {
                grid_engine = reactor->getEngine<SmartMet::Engine::Grid::Engine>("grid");
            });
    }

protected:
  /**
   *   @brief Builds a response cache key qualifier from the latest model runs
   *
   *   Resolves each WFS producer alias into its real grid producer name(s) and
   *   returns a string holding the latest ready generation's analysis (origin)
   *   time for each of them. The value changes whenever a new model run becomes
   *   available, which invalidates cached responses. Returns an empty string
   *   when the grid engine is disabled or no generations are found.
   */
  std::string get_grid_generation_qualifier(
      const std::vector<std::string>& producer_aliases) const;

  std::shared_ptr<SmartMet::Engine::Grid::Engine> grid_engine;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

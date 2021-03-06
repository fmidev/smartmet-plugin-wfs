#include "Plugin.h"
#include <macgyver/Exception.h>
#include <spine/PluginTest.h>

using namespace std;

void prelude(SmartMet::Spine::Reactor& reactor)
{
  // Wait for qengine to finish
  sleep(5);

#if 1
  auto handlers = reactor.getURIMap();
  while (handlers.find("/wfs") == handlers.end())
  {
    sleep(1);
    handlers = reactor.getURIMap();
  }
  sleep(5);
#endif

  cout << endl << "Testing wfs plugin" << endl << "=========================" << endl;
}

int main(int argc, char* argv[])
{
  try
  {
    SmartMet::Spine::Options options;
    options.quiet = true;
    options.defaultlogging = false;
    options.configfile = "cnf/reactor.conf";

    if (!options.parse(argc, argv))
      exit(1);

    return SmartMet::Spine::PluginTest::test(options, prelude, 10);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Service call failed!", nullptr);
    exception.printError();
    return -1;
  }
}

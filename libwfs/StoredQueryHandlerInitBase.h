#pragma once

#include <functional>
#include <queue>
#include <string>
#include <utility>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{

class StoredQueryHandlerFactoryDef;

/**
 *   @brief Virtual base class for registering stored query handler initialization actions and executing them.
 */
class StoredQueryHandlerInitBase
{
protected:
    StoredQueryHandlerInitBase();
    StoredQueryHandlerInitBase(const StoredQueryHandlerInitBase&) = delete;
    StoredQueryHandlerInitBase& operator = (const StoredQueryHandlerInitBase&) = delete;

public:
    virtual ~StoredQueryHandlerInitBase();

    void execute_init_actions();

    void add_init_action(const std::string& name, std::function<void()> action);

private:
    std::queue<std::pair<std::string, std::function<void()> > > action_queue;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

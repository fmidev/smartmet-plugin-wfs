#pragma once

#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>
#include <optional>
#include <json/value.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
    class StoredQueryMap;
    class StoredQueryHandlerBase;

    /**
     *   @brief Summary of available stored query handler factory objects
     */
    struct HandlerFactorySummary
    {
    public:
        struct SizeInfo
        {
            std::size_t min_size;
            std::size_t max_size;
            std::size_t step;
        };

        struct StoredQueryInfo
        {
            std::string name;
            std::optional<std::string> template_name;
            std::vector<std::string> return_types;
        };

        struct ParamInfo
        {
            std::string name;
            bool mandatory;
            std::string type;
            std::string description;
            std::optional<SizeInfo> size_info;
        };

        struct FactoryInfo
        {
            std::string name;
            std::string description;
            std::map<std::string, ParamInfo> params;
            std::set<std::string> template_names;
            std::map<std::string, StoredQueryInfo> stored_queries;
        };

        HandlerFactorySummary(const std::string& prefix);
        HandlerFactorySummary(const HandlerFactorySummary& src);
        HandlerFactorySummary operator = (const HandlerFactorySummary&);

        virtual ~HandlerFactorySummary();

        void add_handler(const StoredQueryHandlerBase& handler);

        void write_html(
            std::ostream& os,
            const std::string& url_prefix,
            const std::optional<std::string>& name) const;

        Json::Value as_json(const std::optional<std::string>& name) const;

    private:
        const std::string prefix;
        std::map<std::string, std::shared_ptr<FactoryInfo> > factory_map;

    };

} // namespace WFS
} // namespace Plugin
} // namespace SmartMet

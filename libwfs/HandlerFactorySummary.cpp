#include "HandlerFactorySummary.h"
#include "StoredQueryHandlerBase.h"
#include <ostream>
#include <json/value.h>

#include <macgyver/DebugTools.h>

using namespace SmartMet::Plugin::WFS;

namespace
{
    void start_param_table(std::ostream& os)
    {
        os << "<br>";
        os << "<table border=\"1px solid black\" padding=\"10pt\">";
        os << "<tr>";
        os << "<th> </th>";
        os << "<th>Parameter</th>";
        os << "<th>Mandatory</th>";
        os << "<th>Type</th>";
        os << "<th>Min size</th>";
        os << "<th>Max size</th>";
        os << "<th>Step</th>";
        os << "<th>Description</th>";
        os << "</tr>\n";
    }

    void end_param_table(std::ostream& os)
    {
        os << "</table>\n";
    }

    void write_html(
        std::ostream& os,
        const HandlerFactorySummary::ParamInfo& p)
    {
        const bool is_array = static_cast<bool>(p.size_info);
        if (is_array) {
            os << "<tr>";
            os << "<td>Array</td>";
            os << "<td>" << p.name << "</td>";
            os << "<td>" << (p.size_info->min_size > 0 ? "yes" : "no") << "</td>";
            os << "<td>" << p.type << "</td>";
            os << "<td>" << p.size_info->min_size << "</td>";
            os << "<td>" << p.size_info->max_size << "</td>";
            os << "<td>" << p.size_info->step << "</td>";
            os << "<td>" << p.description << "</td>";
            os << "</tr>\n";
        } else {
            os << "<tr>";
            os << "<td>Scalar</td>";
            os << "<td>" << p.name << "</td>";
            os << "<td>" << (p.mandatory ? "yes" : "no") << "</td>";
            os << "<td>" << p.type << "</td>";
            os << "<td> </td>";
            os << "<td> </td>";
            os << "<td> </td>";
            os << "<td>" << p.description << "</td>";
            os << "</tr>\n";
        }
    }

    void start_stored_query_table(std::ostream& os)
    {
        os << "<table border=\"1px solid black\"; padding: \"5px\" >\n";
        os << "<tr>";
        os << "<th>Stored query</th>";
        os << "<th>Template</th>";
        os << "<th>Return types</th>";
        os << "</tr>\n";
    }

    void write_html(
        std::ostream& os,
        const std::string& prefix,
        const HandlerFactorySummary::StoredQueryInfo& s_info)
    {
        os << "<tr>";
        os << "<td><a href=\"" << prefix << "??SERVICE=WFS&VERSION=2.0.0"
           << "&request=DescribeStoredQueries"
           << "&storedquery_id=" << s_info.name
           << "\">" << s_info.name << "</a></td>";
        if (s_info.template_name) {
            os << "<td>" << *s_info.template_name << "</td>";
        } else {
            os << "<td> </td>";
        }

        if (s_info.return_types.size() == 1) {
            os << "<td>" << s_info.return_types.at(0) << "</td>";
        } else {
            os << "<td><ul>";
            for (const auto& item : s_info.return_types) {
                os << "<li>" << item << "</li>";
            }
            os << "</ul></td>";
        }
    }

    void end_stored_query_table(std::ostream& os)
    {
        os << "</table>\n";
    }

    void write_param_table(
        std::ostream& os,
        const HandlerFactorySummary::FactoryInfo& factory_info)
    {
        os << "<h3>Section handler_params</h3>\n";
        start_param_table(os);
        for (const auto& item : factory_info.params) {
            write_html(os, item.second);
        }
        end_param_table(os);
        os << std::endl;
    }

    void write_stored_query_table(
        std::ostream& os,
        const std::string& prefix,
        const HandlerFactorySummary::FactoryInfo& f_info)
    {
        os << "<h3>Stored queries</h3>\n";
        start_stored_query_table(os);
        for (const auto& item : f_info.stored_queries) {
            write_html(os, prefix, item.second);
        }
        end_stored_query_table(os);
    }
}

HandlerFactorySummary::HandlerFactorySummary(const std::string& prefix)
    : prefix(prefix)
{
}

HandlerFactorySummary::HandlerFactorySummary(const HandlerFactorySummary& src)
    : prefix(src.prefix)
{
}

HandlerFactorySummary::~HandlerFactorySummary() = default;



void HandlerFactorySummary::add_handler(
    const StoredQueryHandlerBase& handler)
{
    auto sq_conf = handler.get_config();
    const std::string constructor_name = sq_conf->get_constructor_name();
    const std::string query_id = sq_conf->get_query_id();

    std::shared_ptr<FactoryInfo> ci;
    auto insert_result = factory_map.insert(std::make_pair(constructor_name, ci));
    if (insert_result.second) {
        ci = std::make_shared<FactoryInfo>();
        ci->name = constructor_name;
        insert_result.first->second = ci;
    }

    const auto tmp = handler.get_param_info();
    ci = insert_result.first->second;
    std::copy(tmp.begin(), tmp.end(), std::inserter(ci->params, ci->params.begin()));

    StoredQueryInfo sqi;
    sqi.name = query_id;
    if (sq_conf->have_template_fn()) {
        sqi.template_name = sq_conf->get_template_fn();
        ci->template_names.insert(*sqi.template_name);
    }
    sqi.return_types = sq_conf->get_return_type_names();
    ci->stored_queries[sqi.name] = sqi;
}

void HandlerFactorySummary::write_html(
    std::ostream& os,
    const std::string& url_prefix,
    const boost::optional<std::string>& name) const
{
    if (name) {
        auto it = factory_map.find(*name);
        if (it == factory_map.end()) {
            return;
        }

        const auto ci = it->second;
        if (not ci.get()) {
            return;
        }

        os << "<h1>Stored query handler constructor: " << *name << "</h1>\n";
        os << "<table border=\"1px solid black\", padding: 5px; >\n";

        write_param_table(os, *ci);

        write_stored_query_table(os, prefix, *ci);

    } else {
        os << "<h1>Stored query handler constructors</h1>\n";
        os << "<br>\n";
        os << "<b>Only those stored query handler constructors, that are used for at least"
           << " one stored query are browsable</b>\n";
        os << "<br>\n";
        os << "<ul>\n";

        for (const auto& item : factory_map)
        {
            const auto ci = item.second;
            os << "<li> ";
            if (ci.get()) {
                os << "<a href=\"" << url_prefix << "/admin?request=constructors&handler="
                    << ci->name << "&format=html\">"
                   << ci->name << "</a>";
            } else {
                os << item.first;
            }
            os << "</li>\n";
        }
        os << "</ul>\n";
    }
}

Json::Value HandlerFactorySummary::as_json(const boost::optional<std::string>& name) const
{
    try {
        Json::Value result;
        Json::Value& constructors = result["constructors"];

        std::map<std::string, std::set<std::string> > template_summary;

        for (const auto& item : factory_map) {
            const auto& f_info = item.second;
            if (name and (*name != f_info->name)) {
                continue;
            }

            auto& f_out = constructors[f_info->name];
            auto& params_out = f_out["parameters"] = Json::objectValue;
            auto& sq_out = f_out["stored_queries"] = Json::arrayValue;
            for (const auto& p_item : f_info->params) {
                const auto& param = p_item.second;
                auto& p_out = params_out[param.name] = Json::objectValue;
                p_out["description"] = param.description;
                p_out["type"] = param.type;
                if (param.size_info) {
                    p_out["is_array"] = true;
                    p_out["mandatory"] = param.size_info->min_size >= 1;
                    p_out["min_size"] = param.size_info->min_size;
                    p_out["max_size"] = param.size_info->max_size;
                    p_out["step"] = param.size_info->step;
                } else {
                    p_out["is_array"] = false;
                    p_out["mandatory"] = param.mandatory;
                }
            }

            for (const auto& sq_item : f_info->stored_queries) {
                const auto& sq_info = sq_item.second;
                auto& sq_out_curr = sq_out.append(Json::objectValue);
                sq_out_curr["name"] = sq_info.name;
                sq_out_curr["return_types"] = Json::arrayValue;
                if (sq_info.template_name) {
                    sq_out_curr["template"] = *sq_info.template_name;
                    for (const auto& rt_name : sq_info.return_types) {
                        auto& tmp = template_summary[*sq_info.template_name];
                        tmp.insert(rt_name);
                        sq_out_curr["return_types"].append(Json::Value(rt_name));
                    }
                } else {
                    sq_out_curr["template_name"] = Json::nullValue;
                    for (const auto& rt_name : sq_info.return_types) {
                        sq_out_curr["return_types"].append(Json::Value(rt_name));
                    }
                }
            }
        }

        Json::Value templates(Json::objectValue);
        for (const auto& item : template_summary) {
            Json::Value tmp(Json::arrayValue);
            for (const auto& rt_name : item.second) {
                tmp.append(Json::Value(rt_name));
            }
            templates[item.first] = tmp;
        }

        result["templates"] = templates;
        return result;
    } catch (...) {
        Fmi::Exception err(BCP, "Operation failed");
        //std::cout << err;
        throw err;
    }
}


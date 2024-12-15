#include "rest_server_router.hpp"

namespace rest
{
    void Router::AddRoute(const std::string& path, boost::beast::http::verb method, Router::HandlerWithParams handler)
    {
        // Replace {:name} with regex group `([^/]+)` and extract parameter names
        boost::regex param_regex(R"(\{\:([a-zA-Z_][a-zA-Z0-9_]*)\})");
        std::string  regex_path = boost::regex_replace(path, param_regex, "([^/]+)");
        boost::regex full_regex("^" + regex_path + "$"); // Ensure full match

        std::vector<std::string> parameter_names;
        auto                     begin = boost::sregex_iterator(path.begin(), path.end(), param_regex);
        auto                     end   = boost::sregex_iterator();
        for (auto it = begin; it != end; ++it)
            parameter_names.push_back((*it)[1]);

        auto& info            = m_routes.try_emplace(path).first->second;
        info.pattern          = full_regex;
        info.parameter_names  = parameter_names;
        info.handlers[method] = std::move(handler);
    }

    Router::Response Router::Route(const Router::Request& req) const
    {
        for (const auto& [_, route] : m_routes)
        {
            boost::smatch match;
            std::string   target = req.target();
            if (boost::regex_match(target, match, route.pattern))
            {
                // Extract parameter values
                Params params;
                for (size_t i = 0; i < route.parameter_names.size(); ++i)
                {
                    params[route.parameter_names[i]] = match[i + 1]; // First group is at index 1
                }

                // Find and call the handler
                auto handler_it = route.handlers.find(req.method());
                if (handler_it != route.handlers.end())
                {
                    return handler_it->second(req, params);
                }
                else
                {
                    return Response{boost::beast::http::status::method_not_allowed, req.version()};
                }
            }
        }
        return Response{boost::beast::http::status::not_found, req.version()};
    }

} // namespace backend
// Copyright (C) 2024-2024 Aleksey Loginov
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Home page: https://github.com/victimsnino/TasksQueue/

#include "rest_router.hpp"

namespace rest
{
    void Router::AddRoute(const std::string& path, Request::Method method, Router::HandlerWithParams handler)
    {
        // Replace {:name} with regex group `([^/]+)` and extract parameter names
        std::regex  param_regex(R"(\{\:([a-zA-Z_][a-zA-Z0-9_]*)\})");
        std::string regex_path = std::regex_replace(path, param_regex, "([^/]+)");
        std::regex  full_regex("^" + regex_path + "$"); // Ensure full match

        std::vector<std::string> parameter_names;
        auto                     begin = std::sregex_iterator(path.begin(), path.end(), param_regex);
        auto                     end   = std::sregex_iterator();
        for (auto it = begin; it != end; ++it)
            parameter_names.push_back((*it)[1]);

        auto& info            = m_routes.try_emplace(path).first->second;
        info.pattern          = full_regex;
        info.parameter_names  = parameter_names;
        info.handlers[method] = std::move(handler);
    }

    Response Router::Route(const Request& req) const
    {
        for (const auto& [_, route] : m_routes)
        {
            std::smatch match;
            if (std::regex_match(req.path, match, route.pattern))
            {
                // Extract parameter values
                Params params;
                for (size_t i = 0; i < route.parameter_names.size(); ++i)
                {
                    params[route.parameter_names[i]] = match[i + 1]; // First group is at index 1
                }

                // Find and call the handler
                auto handler_it = route.handlers.find(req.method);
                if (handler_it != route.handlers.end())
                {
                    return handler_it->second(req, params);
                }
                else
                {
                    return Response{.status_code = Response::Status::MethodNotAllowed};
                }
            }
        }
        return Response{.status_code = Response::Status::NotFound};
    }

} // namespace rest

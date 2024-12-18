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
    namespace
    {
        std::unordered_map<std::string, std::string> ParseParams(std::string& url)
        {
            size_t query_start = url.find('?');
            if (query_start == std::string::npos)
                return {};

            auto query = std::string_view{url}.substr(query_start + 1);

            std::unordered_map<std::string, std::string> query_params;
            size_t                                       start = 0;
            while (true)
            {
                auto end   = query.find('&', start);
                auto param = query.substr(start, end == std::string::npos ? end : end - start);
                auto eq    = param.find('=');

                query_params[std::string{param.substr(0, eq)}] = eq != std::string::npos ? param.substr(eq + 1) : "";

                if (end == std::string::npos)
                    break;

                start = end + 1;
            }

            url.resize(query_start);

            return query_params;
        }
    } // namespace
    void Router::AddRoute(const std::string& path, Request::Method method, Router::HandlerWithParams handler)
    {
        if (path.empty() || path[0] != '/')
            throw std::invalid_argument("Path must start with '/'");

        if (!handler)
            throw std::invalid_argument("Handler cannot be null");

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
        if (req.content_type == ContentType::Unknown)
            return Response{.status_code = Response::Status::BadRequest, .body = "Unsupported or unknown content type"};

        if (req.method == Request::Method::Unknown)
            return Response{.status_code = Response::Status::BadRequest, .body = "Unknown method"};

        if (req.accept_content_type == ContentType::Unknown)
            return Response{.status_code = Response::Status::BadRequest, .body = "Unsupported or unknown accept content type"};

        std::string url    = req.path;
        Params      params = ParseParams(url);
        for (const auto& [_, route] : m_routes)
        {
            std::smatch match;
            if (!std::regex_match(url, match, route.pattern))
                continue;
            // Extract parameter values

            if (match.size() != route.parameter_names.size() + 1)
                return Response{.status_code = Response::Status::InternalServerError, .body = "Internal server error"};

            for (size_t i = 0; i < route.parameter_names.size(); ++i)
                params[route.parameter_names[i]] = match[i + 1]; // First group is at index 1

            // Find and call the handler
            auto handler_it = route.handlers.find(req.method);
            if (handler_it == route.handlers.end())
                return Response{.status_code = Response::Status::MethodNotAllowed};

            try
            {
                return handler_it->second(req, params);
            }
            catch (const std::exception& e)
            {
                return Response{.status_code = Response::Status::InternalServerError, .body = e.what()};
            }
        }
        return Response{.status_code = Response::Status::NotFound};
    }

} // namespace rest

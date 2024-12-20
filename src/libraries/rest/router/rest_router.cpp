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
        /**
         * Parses URL query parameters and modifies the input URL to remove them.
         * @param url Input URL string to parse. Will be modified to remove query parameters.
         * @return An unordered map containing key-value pairs of query parameters.
         *         Empty map if no query parameters are found.
         * 
         * @details Extracts query parameters from a URL string after the '?' character.
         *          Parameters are expected in the format 'key=value' separated by '&'.
         *          If a parameter has no value (no '=' sign), an empty string is used as value.
         *          The input URL is modified to remove the query string portion.
         * 
         * Example:
         * Input URL: "http://example.com?key1=value1&key2=&key3=value3"
         * Returns: {{"key1", "value1"}, {"key2", ""}, {"key3", "value3"}}
         * Modified URL becomes: "http://example.com"
         */
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
    } /**
     * Adds a new route to the router with URL parameter support.
     * @param path The URL path pattern (must start with '/'). Can include parameters in the format {:name}
     *            Example: "/users/{:id}/profile"
     * @param method The HTTP method for this route
     * @param handler Callback function to handle requests matching this route. The handler receives
     *               extracted URL parameters as arguments
     * @throws std::invalid_argument If path is empty, doesn't start with '/', or if handler is null
     * @details The function converts URL parameters in the format {:name} to regex patterns and stores
     *          the parameter names for later extraction. The path is converted to a regex pattern that
     *          ensures exact matching of the route.
     */
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

    /**
     * Routes an incoming HTTP request to the appropriate handler based on URL pattern matching.
     * 
     * @param req The incoming HTTP request to be routed
     * @return Response object containing the result of handling the request
     *         - 200 OK if request is successfully handled
     *         - 404 Not Found if no matching route is found
     *         - 405 Method Not Allowed if route exists but method is not supported
     *         - 500 Internal Server Error if pattern matching fails or handler throws exception
     * 
     * @throws None - All exceptions are caught and converted to 500 responses
     * 
     * @details The function performs the following steps:
     *          1. Extracts URL and parses query parameters
     *          2. Matches URL against registered route patterns
     *          3. Extracts URL parameters from matching route
     *          4. Finds and executes appropriate handler for the HTTP method
     *          5. Returns response from handler or error response if any step fails
     * 
     * @thread_safety This method is const and thread-safe if the handlers it calls are thread-safe
     */
    Response Router::Route(const Request& req) const
    {
        std::string url    = req.path;
        Params      params = ParseParams(url);
        for (const auto& [_, route] : m_routes)
        {
            std::smatch match;
            if (!std::regex_match(url, match, route.pattern))
                continue;

            if (match.size() != route.parameter_names.size() + 1)
                return Response{.status_code = Response::Status::InternalServerError, .body = "Internal server error", .content_type = ContentType::TextPlain};

            for (size_t i = 0; i < route.parameter_names.size(); ++i)
                params[route.parameter_names[i]] = match[i + 1]; // First group is at index 1

            // Find and call the handler
            auto handler_it = route.handlers.find(req.method);
            if (handler_it == route.handlers.end())
                return Response{.status_code = Response::Status::MethodNotAllowed, .content_type = ContentType::TextPlain};

            try
            {
                return handler_it->second(req, params);
            }
            catch (const std::exception& e)
            {
                return Response{.status_code = Response::Status::InternalServerError, .body = e.what(), .content_type = ContentType::TextPlain};
            }
        }
        return Response{.status_code = Response::Status::NotFound, .content_type = ContentType::TextPlain};
    }

} // namespace rest

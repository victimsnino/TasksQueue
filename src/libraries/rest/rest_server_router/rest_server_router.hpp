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

#pragma once

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/regex.hpp>

#include <span>
#include <vector>

namespace rest
{
    class Router
    {
    public:
        using Params            = std::unordered_map<std::string, std::string>;
        using Request           = boost::beast::http::request<boost::beast::http::string_body>;
        using Response          = boost::beast::http::response<boost::beast::http::string_body>;
        using HandlerWithParams = std::function<Response(const Request&, const Params&)>;

        Router() = default;

        void AddRoute(const std::string& path, boost::beast::http::verb method, HandlerWithParams handler);

        Response Route(const Request& req) const;

    private:
        struct RouteInfo
        {
            boost::regex                                                    pattern;
            std::vector<std::string>                                        parameter_names; // List of parameter names
            std::unordered_map<boost::beast::http::verb, HandlerWithParams> handlers;
        };

        std::unordered_map<std::string, RouteInfo> m_routes;
    };
} // namespace rest

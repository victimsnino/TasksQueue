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

#include <libraries/rest/core/rest_core.hpp>
#include <rfl/json.hpp>

#include <regex>
#include <unordered_map>
#include <vector>

namespace rest
{
    template<typename T>
    std::string Serialize(const T& v, rest::ContentType content_type)
    {
        switch (content_type)
        {
        case rest::ContentType::ApplicationJson:
            return rfl::json::write(v);
        case rest::ContentType::TextPlain:
            throw std::runtime_error("Unsupported accept content type");
        }
    }

    template<typename T>
    T DeSerialize(const std::string& v, rest::ContentType content_type)
    {
        switch (content_type)
        {
        case rest::ContentType::ApplicationJson:
            return rfl::json::read<T>(v).value();
        case rest::ContentType::TextPlain:
            throw std::runtime_error("Unsupported request content type");
        }
    }

    template<typename T>
    concept Serializable = requires(const T& v) { Serialize(v, {}); };

    template<typename T>
    concept Deserializable = requires(const T& v) { DeSerialize<T>("", {}); };

    class Router
    {
    public:
        using Params            = std::unordered_map<std::string, std::string>;
        using HandlerWithParams = std::function<Response(const Request&, const Params&)>;

        template<Serializable T>
        struct SerializableResponse
        {
            const Response::Status status_code;
            const T                body;
        };

        Router() = default;

        /**
         * @brief Adds a new route to the router
         * @param path The URL path pattern (e.g., "/users/{:id}")
         * @param method The HTTP method to handle
         * @param handler The callback to handle matching requests
         * @throws std::regex_error If the path pattern is invalid
         */
        void AddRoute(const std::string& path, Request::Method method, std::function<Response(const Request&, const Params&)> handler);

        template<Deserializable TRequest, Serializable TResponse>
        void AddRoute(const std::string& path, Request::Method method, std::function<SerializableResponse<TResponse>(const TRequest&, const Params&)> handler)
        {
            AddRoute(path, method, [handler](const Request& req, const Params& params) {
                std::optional<TRequest> body{};
                try
                {
                    body = DeSerialize<TRequest>(req.body, req.content_type);
                }
                catch (const std::exception& e)
                {
                    return Response{.status_code = Response::Status::BadRequest, .body = e.what()};
                }

                const auto  res = handler(body.value(), params);
                std::string body_str{};
                try
                {
                    body_str = Serialize(res.body, req.accept_content_type);
                }
                catch (const std::exception& e)
                {
                    return Response{.status_code = Response::Status::BadRequest, .body = e.what()};
                }
                return Response{.status_code = res.status_code, .body = std::move(body_str), .content_type = req.accept_content_type};
            });
        }

        /**
         * @brief Routes an incoming request to the appropriate handler
         * @param req The incoming HTTP request
         * @return HTTP response from the matching handler
         */
        [[nodiscard]] Response Route(const Request& req) const;

    private:
        struct RouteInfo
        {
            std::regex                                             pattern{};
            std::vector<std::string>                               parameter_names{}; // List of parameter names
            std::unordered_map<Request::Method, HandlerWithParams> handlers{};
        };

        std::unordered_map<std::string, RouteInfo> m_routes;
    };
} // namespace rest

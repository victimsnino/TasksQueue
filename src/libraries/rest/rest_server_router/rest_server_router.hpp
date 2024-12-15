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
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

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>

#include <boost/beast.hpp>
#include <libraries/rest/server/rest_server.hpp>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = net::ip::tcp;

auto MakeRequest(const std::string& path, const rest::ServerConfig& config)
{
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver     resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(config.address), config.port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(endpoint);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, path, 10};

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);
    return res;
}

struct Routes
{
    MAKE_MOCK2(Method, rest::Router::Response(rest::Router::Request, rest::Router::Params));
};

TEST_CASE("BackendServer provides correct api")
{
    Routes mock;
    auto   router = rest::Router{};
    router.AddRoute("/test", http::verb::get, [&mock](const auto& req, const auto& params) { return mock.Method(req, params); });

    const auto config     = rest::ServerConfig();
    auto       stop_token = rest::StartServer(router, config);

    SUBCASE("get /invalid")
    {
        const auto resp = MakeRequest("/invalid", config);
        REQUIRE(resp.result() == http::status::not_found);
        REQUIRE(resp.body() == "");
    }

    SUBCASE("get /test")
    {
        REQUIRE_CALL(mock, Method(trompeloeil::_, trompeloeil::_)).RETURN(http::response<http::string_body>{http::status::ok, 10});

        const auto resp = MakeRequest("/test", config);
        REQUIRE(resp.result() == http::status::ok);
    }

    stop_token.Stop();
}

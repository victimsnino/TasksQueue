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

#include <libraries/rest/rest_server_router/rest_server_router.hpp>

auto CreateRequest(boost::beast::http::verb method, std::string path)
{
    auto req = boost::beast::http::request<boost::beast::http::string_body>{};
    req.target(path);
    req.method(method);
    return req;
}

TEST_CASE("Router provide correct routing")
{
    rest::Router router{};
    SUBCASE("get empty")
    {
        REQUIRE(router.Route(CreateRequest(boost::beast::http::verb::get, {})).result() == boost::beast::http::status::not_found);
    }

    SUBCASE("get /test")
    {
        REQUIRE(router.Route(CreateRequest(boost::beast::http::verb::get, "/test")).result() == boost::beast::http::status::not_found);
    }

    SUBCASE("add get /test")
    {
        router.AddRoute("/test", boost::beast::http::verb::get, [](const auto&, const auto&) { return boost::beast::http::response<boost::beast::http::string_body>{}; });
        SUBCASE("get /test")
        {
            REQUIRE(router.Route(CreateRequest(boost::beast::http::verb::get, "/test")).result() == boost::beast::http::status::ok);
        }

        SUBCASE("get /test_2")
        {
            REQUIRE(router.Route(CreateRequest(boost::beast::http::verb::get, "/test_2")).result() == boost::beast::http::status::not_found);
        }

        SUBCASE("post /test")
        {
            REQUIRE(router.Route(CreateRequest(boost::beast::http::verb::post, "/test")).result() == boost::beast::http::status::method_not_allowed);
        }
        SUBCASE("add post /test")
        {
            router.AddRoute("/test", boost::beast::http::verb::post, [](const auto& req, const auto&) { return boost::beast::http::response<boost::beast::http::string_body>{boost::beast::http::status::processing, req.version()}; });
            SUBCASE("get /test")
            {
                REQUIRE(router.Route(CreateRequest(boost::beast::http::verb::get, "/test")).result() == boost::beast::http::status::ok);
            }

            SUBCASE("post /test")
            {
                REQUIRE(router.Route(CreateRequest(boost::beast::http::verb::post, "/test")).result() == boost::beast::http::status::processing);
            }
        }
    }
}

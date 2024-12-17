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

#include <libraries/rest/router/rest_router.hpp>


TEST_CASE("Router provide correct routing")
{
    rest::Router router{};
    SUBCASE("get empty")
    {
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::GET}).status_code == rest::Response::Status::NotFound);
    }

    SUBCASE("get /test")
    {
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::GET, .path = "/test"}).status_code == rest::Response::Status::NotFound);
    }

    SUBCASE("add get /test")
    {
        router.AddRoute("/test", rest::Request::Method::GET, [](const rest::Request&, const rest::Router::Params&) { return rest::Response{}; });
        SUBCASE("get /test")
        {
            REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::GET, .path = "/test"}).status_code == rest::Response::Status::Ok);
        }

        SUBCASE("get /test_2")
        {
            REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::GET, .path = "/test_2"}).status_code == rest::Response::Status::NotFound);
        }

        SUBCASE("post /test")
        {
            REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::POST, .path = "/test"}).status_code == rest::Response::Status::MethodNotAllowed);
        }
        SUBCASE("add post /test")
        {
            router.AddRoute("/test", rest::Request::Method::POST, [](const rest::Request&, const rest::Router::Params&) { return rest::Response{.status_code = rest::Response::Status::Processing}; });
            SUBCASE("get /test")
            {
                REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::GET, .path = "/test"}).status_code == rest::Response::Status::Ok);
            }

            SUBCASE("post /test")
            {
                REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::POST, .path = "/test"}).status_code == rest::Response::Status::Processing);
            }
        }
    }
}

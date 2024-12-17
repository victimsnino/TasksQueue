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
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::NotFound);
    }

    SUBCASE("get /test")
    {
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::NotFound);
    }

    SUBCASE("add get /test")
    {
        router.AddRoute("/test", rest::Request::Method::Get, [](const rest::Request&, const rest::Router::Params&) { return rest::Response{}; });
        SUBCASE("get /test")
        {
            REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::Ok);
        }

        SUBCASE("get /test_2")
        {
            REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test_2", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::NotFound);
        }

        SUBCASE("post /test")
        {
            REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Post, .path = "/test", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::MethodNotAllowed);
        }
        SUBCASE("add post /test")
        {
            router.AddRoute("/test", rest::Request::Method::Post, [](const rest::Request&, const rest::Router::Params&) { return rest::Response{.status_code = rest::Response::Status::Processing}; });
            SUBCASE("get /test")
            {
                REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::Ok);
            }

            SUBCASE("post /test")
            {
                REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Post, .path = "/test", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::Processing);
            }
        }
    }
    SUBCASE("exception inside handler")
    {
        router.AddRoute("/test", rest::Request::Method::Get, [](const rest::Request&, const rest::Router::Params&) -> rest::Response { throw std::runtime_error("test"); });
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::InternalServerError);
    }
    SUBCASE("pattern with parameter")
    {
        router.AddRoute("/test/{:id}/subtest", rest::Request::Method::Get, [](const rest::Request&, const rest::Router::Params& params) {
            REQUIRE(params.at("id") == "135");
            return rest::Response{.status_code = rest::Response::Status::Ok};
        });
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test/135/subtest", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::Ok);
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test/135", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::NotFound);
    }
    SUBCASE("invalid content-type")
    {
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test", .content_type = rest::ContentType::Unknown, .accept_content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::BadRequest);
    }
    SUBCASE("invalid accept-content-type")
    {
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test", .content_type = rest::ContentType::TextPlain, .accept_content_type = rest::ContentType::Unknown}).status_code == rest::Response::Status::BadRequest);
    }
    SUBCASE("invalid method")
    {
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Unknown, .path = "/test", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::BadRequest);
    }
    SUBCASE("query params")
    {
        router.AddRoute("/test/", rest::Request::Method::Get, [](const rest::Request&, const rest::Router::Params& params) {
            REQUIRE(params.at("key") == "value");
            return rest::Response{.status_code = rest::Response::Status::Ok};
        });
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test/?key=value", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::Ok);
    }
    SUBCASE("query params and path params")
    {
        router.AddRoute("/test/{:id}/subtest", rest::Request::Method::Get, [](const rest::Request&, const rest::Router::Params& params) {
            REQUIRE(params.at("key") == "value");
            REQUIRE(params.at("key2") == "value2");
            REQUIRE(params.at("id") == "23");
            return rest::Response{.status_code = rest::Response::Status::Ok};
        });
        REQUIRE(router.Route(rest::Request{.method = rest::Request::Method::Get, .path = "/test/23/subtest?key=value&key2=value2", .content_type = rest::ContentType::TextPlain}).status_code == rest::Response::Status::Ok);
    }
}

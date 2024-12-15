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
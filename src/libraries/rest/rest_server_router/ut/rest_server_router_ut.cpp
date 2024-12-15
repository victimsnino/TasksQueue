#include <doctest/doctest.h>

#include <libraries/rest/rest_server_router/rest_server_router.hpp>


TEST_CASE("Router provide correct routing")
{
    rest::Router router{};
    SUBCASE("route empty")
    {
        REQUIRE(router.Route(boost::beast::http::request<boost::beast::http::string_body>{}).result() == boost::beast::http::status::not_found);
    }
    SUBCASE("route unkown")
    {
        auto req = boost::beast::http::request<boost::beast::http::string_body>{};
        req.target("/unknown");
        REQUIRE(router.Route(req).result() == boost::beast::http::status::not_found);
    }
}
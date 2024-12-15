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

#include <boost/beast.hpp>

#include <libraries/backend/data_storage/interface/data_storage_mock.hpp>
#include <libraries/backend/server/backend_server.hpp>
#include <libraries/backend/tasks_manager/tasks_manager.hpp>

#include <iostream>
#include <memory>
#include <thread>


namespace beast = boost::beast;    
namespace http = beast::http;       
namespace net = boost::asio;        
using tcp = net::ip::tcp;       

auto MakeRequest(const std::string& path, const backend::ServerConfig& config)
{
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(config.address), config.port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(endpoint);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, path, 10};
    // req.set(http::field::host, host);
    // req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

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

TEST_CASE("BackendServer provides correct api")
{
    auto mock       = std::make_shared<MockDataStorage>();
    const auto config = backend::ServerConfig();
    auto stop_token = backend::StartServer(backend::TasksManager{mock}, config);

    SUBCASE("get /invalid")
    {
        const auto resp = MakeRequest("/invalid", config);
        REQUIRE(resp.result() == http::status::not_found);
        REQUIRE(resp.body() == "");
    }
    SUBCASE("get /tasks for empty")
    {
        REQUIRE_CALL(*mock, GetTasks()).RETURN(std::vector<backend::interface::Task>{});
        const auto resp = MakeRequest("/tasks", config);
        REQUIRE(resp.result() == http::status::ok);
        // REQUIRE(resp == "")
        // REQUIRE(resp == drogon::ContentType::CT_APPLICATION_JSON);
        // REQUIRE(resp->body() == "[]");
    }
    // SUBCASE("get /tasks with 1 task")
    // {
    //     backend::interface::Task task{.id = 1, .payload = {.name = "name", .description = "description"}};
    //     REQUIRE_CALL(*mock, GetTasks()).RETURN(std::vector<backend::interface::Task>{task});
    //     const auto resp = MakeRequest("/tasks");
    //     REQUIRE(resp->statusCode() == 200);
    //     REQUIRE(resp->contentType() == drogon::ContentType::CT_APPLICATION_JSON);
    //     REQUIRE(resp->body() == R"([{"description":"description","id":1,"name":"name"}])");
    // }

    stop_token.Stop();
}

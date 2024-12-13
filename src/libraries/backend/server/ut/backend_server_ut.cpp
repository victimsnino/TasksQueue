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

#include <drogon/drogon.h>
#include <libraries/backend/data_storage/interface/data_storage_mock.hpp>
#include <libraries/backend/server/backend_server.hpp>
#include <libraries/backend/tasks_manager/tasks_manager.hpp>

#include <memory>

auto MakeRequest(const std::string& path)
{
    auto cli = drogon::HttpClient::newHttpClient("http://localhost:8080");
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath(path);
    const auto [res, resp] = cli->sendRequest(req);
    REQUIRE(res == drogon::ReqResult::Ok);
    REQUIRE(resp);
    return resp;
}

TEST_CASE("BackendServer provides correct api")
{
    static auto mock       = std::make_shared<MockDataStorage>();
    static auto stop_token = backend::StartServer(backend::TasksManager{mock}, backend::ServerConfig({}));

    SUBCASE("get /invalid")
    {
        const auto resp = MakeRequest("/invalid");
        REQUIRE(resp->statusCode() == 404);
        REQUIRE(resp->body() == "");
    }
    SUBCASE("get /tasks for empty")
    {
        REQUIRE_CALL(*mock, GetTasks()).RETURN(std::vector<backend::interface::Task>{});
        const auto resp = MakeRequest("/tasks");
        REQUIRE(resp->statusCode() == 200);
        REQUIRE(resp->contentType() == drogon::ContentType::CT_APPLICATION_JSON);
        REQUIRE(resp->body() == "[]");
    }
    SUBCASE("get /tasks with 1 task")
    {
        backend::interface::Task task{.id = 1, .payload = {.name = "name", .description = "description"}};
        REQUIRE_CALL(*mock, GetTasks()).RETURN(std::vector<backend::interface::Task>{task});
        const auto resp = MakeRequest("/tasks");
        REQUIRE(resp->statusCode() == 200);
        REQUIRE(resp->contentType() == drogon::ContentType::CT_APPLICATION_JSON);
        REQUIRE(resp->body() == R"([{"description":"description","id":1,"name":"name"}])");
    }
}

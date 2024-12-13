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
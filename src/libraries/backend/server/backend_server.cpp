#include "backend_server.hpp"

#include <drogon/drogon.h>


namespace backend::interface
{
    Json::Value ToJson(const Task& task)
    {
        Json::Value json;
        json["id"]          = task.id;
        json["name"]        = task.payload.name;
        json["description"] = task.payload.description;
        return json;
    }
} // namespace backend::interface

namespace backend
{
    namespace
    {
        void FillRoutes(const TasksManager& manager)
        {
            drogon::app().registerHandler("/tasks",
                                          [manager](const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
                                            Json::Value res{Json::arrayValue};
                                            for (const auto& task : manager.GetTasks()) {
                                                res.append(ToJson(task));
                                            }
                                            callback(drogon::HttpResponse::newHttpJsonResponse(res));
                                          },
                                          {drogon::Get});
        }
    } // namespace
    StopHandler::StopHandler(std::jthread th)
        : m_th{std::move(th)}
    {
    }
    StopHandler::~StopHandler() noexcept
    {
        Stop();
    }

    void StopHandler::Stop()
    {
        if (!m_th.joinable())
            return;
        
        m_th.request_stop();
        m_th.join();
    }

    void StopHandler::Detach()
    {
        m_th.detach();
    }

    StopHandler StartServer(const TasksManager& tasks_manager, const ServerConfig& config)
    {
        drogon::app()
            .addListener("0.0.0.0", config.port)
            .setThreadNum(config.threads)
            .setCustom404Page(drogon::HttpResponse::newHttpResponse(drogon::HttpStatusCode::k404NotFound, drogon::ContentType::CT_TEXT_PLAIN))
            .disableSigtermHandling();


        FillRoutes(tasks_manager);

        std::atomic_bool started{};
        std::jthread     th{[&started](std::stop_token stop_token) {
            std::stop_callback cb{stop_token, [] {
                                      drogon::app().getLoop()->queueInLoop([]() { 
                                        drogon::app().quit(); 
                                    });
                                  }};
            drogon::app().getLoop()->queueInLoop([&] { started = true; });
            drogon::app().run();
        }};

        while (!started)
        {
        };

        return StopHandler{std::move(th)};
    }
} // namespace backend
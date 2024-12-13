#pragma once

#include <libraries/backend/tasks_manager/tasks_manager.hpp>

#include <thread>

namespace backend
{
    class StopHandler
    {
    public:
        explicit StopHandler(std::jthread th);
        ~StopHandler() noexcept;
        void Stop();
        void Detach();

    private:
        std::jthread m_th;
    };

    struct ServerConfig
    {
        uint16_t port    = 8080;
        size_t   threads = 1;
    };

    StopHandler StartServer(const TasksManager& tasks_manager, const ServerConfig& config);
} // namespace backend
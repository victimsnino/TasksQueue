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

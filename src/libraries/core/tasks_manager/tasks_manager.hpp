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

#include <libraries/core/interfaces/task/task.hpp>

#include <memory>
#include <vector>

namespace core::interfaces
{
    struct DataStorage;
} // namespace core::interfaces

namespace core
{
    class TasksManager
    {
    public:
        explicit TasksManager(std::shared_ptr<interfaces::DataStorage> storage);

        interfaces::Task              CreateTask(const interfaces::TaskPayload& payload) const;
        std::vector<interfaces::Task> GetTasks() const;

    private:
        std::shared_ptr<interfaces::DataStorage> m_storage{};
    };
} // namespace core

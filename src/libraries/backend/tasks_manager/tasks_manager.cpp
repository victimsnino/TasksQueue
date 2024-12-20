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

#include "tasks_manager.hpp"

#include <libraries/backend/data_storage/interface/data_storage.hpp>

#include <utility>

namespace backend
{
    TasksManager::TasksManager(std::shared_ptr<interface::DataStorage> storage)
        : m_storage{std::move(storage)}
    {
    }

    interface::Task TasksManager::CreateTask(const interface::TaskPayload& payload) const
    {
        return m_storage->CreateTask(payload);
    }

    std::vector<interface::Task> TasksManager::GetTasks() const
    {
        return m_storage->GetTasks();
    }
} // namespace backend

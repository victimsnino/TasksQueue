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
    /**
     * Constructor for TasksManager class.
     * @param storage A shared pointer to the DataStorage instance that will be used for task persistence.
     *               Ownership of the storage pointer is transferred to TasksManager.
     */
    TasksManager::TasksManager(std::shared_ptr<DataStorage> storage)
        : m_storage{std::move(storage)}
    {
    }

    /**
     * Creates a new task with the given payload.
     * @param payload The task payload containing task configuration and data
     * @return A newly created Task object
     */
    Task TasksManager::CreateTask(const TaskPayload& payload) const
    {
        return m_storage->CreateTask(payload);
    }

    /**
     * Retrieves all tasks from the storage.
     * @return A vector containing all stored tasks.
     */
    std::vector<Task> TasksManager::GetTasks() const
    {
        return m_storage->GetTasks();
    }
} // namespace backend

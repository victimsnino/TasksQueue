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

#include "in_memory_storage.hpp"

#include <algorithm>

namespace backend::data_storage
{
    /**
 * Default constructor for InMemoryStorage class.
 */
InMemoryStorage::InMemoryStorage()  = default;
    /**
 * Virtual destructor for InMemoryStorage class.
 */
InMemoryStorage::~InMemoryStorage() = default;

    /**
     * Creates a new task with the given payload and stores it in memory.
     * @param payload The task payload containing task details
     * @return The newly created Task object with an auto-generated ID
     * @thread_safety Thread-safe. Protected by internal mutex
     * @note Task IDs are generated sequentially starting from 0
     */
    Task InMemoryStorage::CreateTask(const TaskPayload& payload)
    {
        std::lock_guard _{m_mutex};
        m_tasks.emplace_back(Task{.id = m_id++, .payload = payload});
        return m_tasks.back();
    }

    /**
     * Deletes a task with the specified index from the storage if it exists.
     * @param index The unique identifier of the task to delete
     * @note Thread-safe operation protected by mutex
     * @details Uses binary search to locate the task. If the task is not found,
     *          the function returns silently without any effect.
     */
    void InMemoryStorage::DeleteTask(size_t index)
    {
        std::lock_guard _{m_mutex};
        const auto      itr = std::ranges::lower_bound(m_tasks, index, std::ranges::less{}, &Task::id);
        if (itr == m_tasks.end() || itr->id != index)
            return;

        m_tasks.erase(itr);
    }

    /**
     * Retrieves all tasks from the in-memory storage.
     * @return A copy of the stored tasks vector
     * @thread_safety Thread-safe. Uses shared lock for concurrent read access
     */
    std::vector<Task> InMemoryStorage::GetTasks() const
    {
        std::shared_lock _{m_mutex};
        return m_tasks;
    }

} // namespace backend::data_storage

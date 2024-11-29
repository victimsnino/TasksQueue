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

namespace core::storages
{
    interfaces::Task InMemoryStorage::CreateTask(const interfaces::TaskPayload& payload)
    {
        m_tasks.emplace_back(interfaces::Task{.id = m_id++, .payload = payload});
        return m_tasks.back();
    }

    void InMemoryStorage::DeleteTask(size_t index)
    {
        const auto itr = std::ranges::lower_bound(m_tasks, index, std::ranges::less{}, &interfaces::Task::id);
        if (itr == m_tasks.end() || itr->id != index)
            return;

        m_tasks.erase(itr);
    }

    std::vector<interfaces::Task> InMemoryStorage::GetTasks() const
    {
        return m_tasks;
    }

} // namespace core::storages

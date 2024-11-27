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

#include <libraries/core/interfaces/data_storage/data_storage.hpp>

namespace core
{
    tasks_manager::tasks_manager(std::shared_ptr<interfaces::data_storage> storage)
        : m_storage{storage}
    {
    }

    interfaces::task tasks_manager::create_task(const interfaces::task_payload& payload) const
    {
        return m_storage->create_task(payload);
    }

    std::vector<interfaces::task> tasks_manager::get_tasks() const
    {
        return m_storage->get_tasks();
    }
} // namespace core

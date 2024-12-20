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

#include <libraries/backend/interface/task/task.hpp>

#include <vector>

namespace backend
{
    struct DataStorage
    {
        /**
 * Virtual destructor for DataStorage class.
 * Enables proper cleanup of derived class objects through base class pointers.
 */
virtual ~DataStorage() = default;

        virtual Task              CreateTask(const TaskPayload& payload) = 0;
        virtual void              DeleteTask(size_t index)               = 0;
        /**
 * Retrieves the list of tasks.
 * @return A vector containing all tasks
 * @note This is a pure virtual function that must be implemented by derived classes
 */
virtual std::vector<Task> GetTasks() const                       = 0;
    };
} // namespace backend

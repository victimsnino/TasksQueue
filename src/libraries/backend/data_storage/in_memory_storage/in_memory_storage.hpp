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

#include <libraries/backend/data_storage/interface/data_storage.hpp>

#include <shared_mutex>

namespace backend::data_storage
{
    class InMemoryStorage final : public DataStorage
    {
    public:
        InMemoryStorage();
        ~InMemoryStorage() override;

        Task              CreateTask(const TaskPayload& payload) override;
        void              DeleteTask(size_t index) override;
        std::vector<Task> GetTasks() const override;

    private:
        mutable std::shared_mutex m_mutex{};
        std::vector<Task> m_tasks{};
        size_t            m_id{};
    };
} // namespace backend::data_storage

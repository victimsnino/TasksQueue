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

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>

#include <libraries/core/data_storage/interface/data_storage.hpp>
#include <libraries/core/tasks_manager/tasks_manager.hpp>

struct MockDataStorage final : public trompeloeil::mock_interface<core::interface::DataStorage>
{
    IMPLEMENT_MOCK1(CreateTask);
    IMPLEMENT_MOCK1(DeleteTask);
    IMPLEMENT_CONST_MOCK0(GetTasks);
};

TEST_CASE("TasksManager forwards calls to storage")
{
    auto                  mock = std::make_shared<MockDataStorage>();
    core::TasksManager    manager{mock};
    trompeloeil::sequence s{};

    core::interface::TaskPayload payload{.name = "name", .description = "description"};
    core::interface::Task        task{.id = 123, .payload = {.name = "name2", .description = "description2"}};

    SUBCASE("CreateTask")
    {
        REQUIRE_CALL(*mock, CreateTask(payload)).RETURN(task).IN_SEQUENCE(s);

        manager.CreateTask(payload);
    }

    SUBCASE("GetTasks")
    {
        const auto res = std::vector{task};
        REQUIRE_CALL(*mock, GetTasks()).RETURN(res).IN_SEQUENCE(s);

        REQUIRE(manager.GetTasks() == res);
    }
}

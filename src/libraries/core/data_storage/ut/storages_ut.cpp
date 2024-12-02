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

#include <libraries/core/data_storage/in_memory_storage/in_memory_storage.hpp>

TEST_CASE("every storage satisfy storage requirements")
{
    constexpr auto test = [](core::interface::DataStorage&& storage) // NOLINT
    {
        SUBCASE("manipulate storage")
        {
            core::interface::TaskPayload payload{.name = "name", .description = "description"};
            const auto                   task_0 = core::interface::Task{.id = 0, .payload = payload};
            REQUIRE(storage.CreateTask(payload) == task_0);

            auto new_payload        = payload;
            new_payload.name        = "name2";
            new_payload.description = "description2";
            const auto task_1       = core::interface::Task{.id = 1, .payload = new_payload};

            REQUIRE(storage.CreateTask(new_payload) == task_1);

            REQUIRE(storage.GetTasks() == std::vector{task_0, task_1});

            SUBCASE("delete task 0")
            {
                storage.DeleteTask(0);
                REQUIRE(storage.GetTasks() == std::vector{task_1});
            }

            SUBCASE("delete non-existing task")
            {
                storage.DeleteTask(10000);
                REQUIRE(storage.GetTasks() == std::vector{task_0, task_1});
            }

            SUBCASE("delete task 1")
            {
                storage.DeleteTask(1);
                REQUIRE(storage.GetTasks() == std::vector{task_0});

                SUBCASE("delete task 0")
                {
                    storage.DeleteTask(0);
                    REQUIRE(storage.GetTasks() == std::vector<core::interface::Task>{});
                }
                SUBCASE("add task 3")
                {
                    const auto task_2 = core::interface::Task{.id = 2, .payload = payload};

                    REQUIRE(storage.CreateTask(payload) == task_2);
                    REQUIRE(storage.GetTasks() == std::vector{task_0, task_2});

                    SUBCASE("delete task 0")
                    {
                        storage.DeleteTask(0);
                        REQUIRE(storage.GetTasks() == std::vector{task_2});
                    }
                }
            }
        }
    };

    SUBCASE("InMemoryStorage")
    {
        test(core::data_storage::InMemoryStorage{});
    }
}

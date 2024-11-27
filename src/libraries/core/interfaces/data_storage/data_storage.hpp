#pragma once

#include <libraries/core/interfaces/task/task.hpp>

#include <vector>

namespace core::interfaces
{
    struct data_storage
    {
        virtual ~data_storage() = default;

        virtual task              create_task(const task_payload& payload) = 0;
        virtual std::vector<task> get_tasks() const                        = 0;
    };
} // namespace core::interfaces

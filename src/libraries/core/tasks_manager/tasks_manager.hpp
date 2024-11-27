#pragma once

#include <libraries/core/interfaces/task/task.hpp>

#include <memory>
#include <vector>

namespace core::interfaces
{
    struct data_storage;
} // namespace core::interfaces

namespace core
{
    class tasks_manager
    {
    public:
        explicit tasks_manager(std::shared_ptr<interfaces::data_storage> storage);

        interfaces::task              create_task(const interfaces::task_payload& payload) const;
        std::vector<interfaces::task> get_tasks() const;

    private:
        std::shared_ptr<interfaces::data_storage> m_storage{};
    };
} // namespace core

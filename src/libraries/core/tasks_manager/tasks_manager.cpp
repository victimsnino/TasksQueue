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

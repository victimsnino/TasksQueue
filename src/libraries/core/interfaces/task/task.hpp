#pragma once

#include <cstddef>
#include <string>

namespace core::interfaces
{
    struct task_payload
    {
        std::string name{};
        std::string description{};
    };

    struct task
    {
        size_t       id{};
        task_payload payload{};
    };
} // namespace core::interfaces

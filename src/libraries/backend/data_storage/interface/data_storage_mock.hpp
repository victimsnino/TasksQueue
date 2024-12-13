#pragma once

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>

#include <libraries/backend/data_storage/interface/data_storage.hpp>

struct MockDataStorage final : public trompeloeil::mock_interface<backend::interface::DataStorage>
{
    IMPLEMENT_MOCK1(CreateTask);
    IMPLEMENT_MOCK1(DeleteTask);
    IMPLEMENT_CONST_MOCK0(GetTasks);
};
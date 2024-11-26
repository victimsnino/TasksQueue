#include <doctest/doctest.h>

#include <libraries/core/task/task.hpp>

TEST_CASE("test_task")
{
    REQUIRE(core::task{}.test(1) == 1);
}

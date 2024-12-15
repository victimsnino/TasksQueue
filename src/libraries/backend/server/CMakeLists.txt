# Copyright (C) 2024-2024 Aleksey Loginov
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# Home page: https://github.com/victimsnino/TasksQueue/

add_subdirectory(router)
add_subdirectory(server_impl)

tq_add_static_library(
    TARGET_NAME
        backend_server
    SOURCES
        backend_server.cpp
        backend_server.hpp
    PRIVATE
        boost::boost
        backend_server_impl
        backend_server_router
    PUBLIC
        tasks_manager
    ADD_TESTS_WITH_MOCK
    TEST_LIBS
        in_memory_storage
)

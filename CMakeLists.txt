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

cmake_minimum_required(VERSION 3.14)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds are not supported. "
      "Please read the BUILDING document before trying to build this project. "
      "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first.")
endif()

project(
  TasksQueue
  VERSION 0.0.1
  DESCRIPTION ""
  HOMEPAGE_URL "https://github.com/victimsnino/TasksQueue"
  LANGUAGES C CXX)

include(cmake/variables.cmake)
include(cmake/dependencies.cmake)
include(cmake/coverage.cmake)

add_subdirectory(src)

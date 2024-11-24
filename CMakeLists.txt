cmake_minimum_required(VERSION 3.14)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
      FATAL_ERROR
      "In-source builds are not supported. "
      "Please read the BUILDING document before trying to build this project. "
      "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()

project(
    TasksQueue
    VERSION 0.0.1
    DESCRIPTION ""
    HOMEPAGE_URL "https://github.com/victimsnino/TasksQueue"
    LANGUAGES CXX
)

include(cmake/variables.cmake)
include(cmake/dependencies.cmake)

add_subdirectory(src)
find_package(Threads)

macro(handle_3rdparty TARGET_NAME)
  get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
  if(${TARGET_TYPE} STREQUAL "INTERFACE_LIBRARY")
    target_compile_options(${TARGET_NAME} INTERFACE "-w")
  else()
    target_compile_options(${TARGET_NAME} PRIVATE "-w")
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_CLANG_TIDY "")
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_CPPCHECK "")
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER 3rdparty)
  endif()

  set_target_properties(
    ${TARGET_NAME}
    PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
               $<TARGET_PROPERTY:${TARGET_NAME},INTERFACE_INCLUDE_DIRECTORIES>)
endmacro()

macro(fetch_library_extended NAME URL TAG TARGET_NAME)
  include(FetchContent)
  set(BUILD_SHARED_LIBS
      OFF
      CACHE INTERNAL "Build SHARED libraries")

  set(FETCHCONTENT_QUIET FALSE)

  FetchContent_Declare(
    ${NAME}
    GIT_REPOSITORY ${URL}
    GIT_TAG ${TAG}
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    GIT_SUBMODULES "")

  FetchContent_MakeAvailable(${NAME})
  handle_3rdparty(${TARGET_NAME})
endmacro()

macro(fetch_library NAME URL TAG)
  find_package(${NAME} QUIET)
  if(NOT ${NAME}_FOUND)
    message("-- Fetching ${NAME}...")
    fetch_library_extended(${NAME} ${URL} ${TAG} ${NAME})
  endif()
endmacro()

if(BUILD_TESTS)
  include(CTest)
  fetch_library(doctest https://github.com/doctest/doctest.git v2.4.11)
  fetch_library(trompeloeil https://github.com/rollbear/trompeloeil.git v48)

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/doctest_main.cpp "#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN\n#include <doctest/doctest.h>\n")
  add_library(doctest_main STATIC ${CMAKE_CURRENT_BINARY_DIR}/doctest_main.cpp)
  target_link_libraries(doctest_main PUBLIC doctest::doctest)

  include(doctest)
endif()

macro(tq_parse_arguments)
  set(TQ_LIBRARY_OPTIONS ADD_TESTS ADD_TESTS_WITH_MOCK)
  set(TQ_LIBRARY_VALUES TARGET_NAME LIBRARY_TYPE)
  set(TQ_LIBRARY_MULTI_VALUES SOURCES PRIVATE PUBLIC INTERFACE)
  cmake_parse_arguments(PARSED "${TQ_LIBRARY_OPTIONS}" "${TQ_LIBRARY_VALUES}" "${TQ_LIBRARY_MULTI_VALUES}" ${ARGN})
endmacro()

function(tq_handle_library)
  tq_parse_arguments(${ARGN})
  target_link_libraries(${PARSED_TARGET_NAME} PRIVATE ${PARSED_PRIVATE} PUBLIC ${PARSED_PUBLIC} INTERFACE ${PARSED_INTERFACE})
  target_include_directories(${PARSED_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  set_target_properties(${PARSED_TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

  if((PARSED_ADD_TESTS OR PARSED_ADD_TESTS_WITH_MOCK) AND BUILD_TESTS)
    file(GLOB_RECURSE FILES ${CMAKE_CURRENT_SOURCE_DIR}/ut/*)
    set(TEST_LIBS doctest_main)
    if (PARSED_ADD_TESTS_WITH_MOCK)
      LIST(APPEND TEST_LIBS trompeloeil::trompeloeil)
    endif()
    tq_add_executable(
      TARGET_NAME
        ${PARSED_TARGET_NAME}_test
      SOURCES
        ${FILES}
      PRIVATE
        ${PARSED_PUBLIC}
        ${PARSED_TARGET_NAME}
        ${TEST_LIBS}
      )
    doctest_discover_tests(${PARSED_TARGET_NAME}_test)
  endif()
endfunction()

function(tq_add_library)
  tq_parse_arguments(${ARGN})
  if (DEFINED PARSED_LIBRARY_TYPE AND PARSED_LIBRARY_TYPE STREQUAL "INTERFACE")
    add_library(${PARSED_TARGET_NAME} INTERFACE)
  else()
    add_library(${PARSED_TARGET_NAME} ${PARSED_LIBRARY_TYPE} ${PARSED_SOURCES})
  endif()
  tq_handle_library(${ARGN})
endfunction()

function(tq_add_executable)
  tq_parse_arguments(${ARGN})
  add_executable(${PARSED_TARGET_NAME} ${PARSED_SOURCES})
  tq_handle_library(${ARGN})
endfunction()


macro(tq_parse_arguments)
  set(TQ_LIBRARY_OPTIONS ADD_TESTS ADD_TESTS_WITH_MOCK)
  set(TQ_LIBRARY_VALUES TARGET_NAME)
  set(TQ_LIBRARY_MULTI_VALUES SOURCES PRIVATE PUBLIC INTERFACE PROTO)
  cmake_parse_arguments(PARSED "${TQ_LIBRARY_OPTIONS}" "${TQ_LIBRARY_VALUES}" "${TQ_LIBRARY_MULTI_VALUES}" ${ARGN})
endmacro()

function(tq_handle_library)
  tq_parse_arguments(${ARGN})
  target_link_libraries(${PARSED_TARGET_NAME} PRIVATE ${PARSED_PRIVATE} PUBLIC ${PARSED_PUBLIC} INTERFACE ${PARSED_INTERFACE})
  set_target_properties(${PARSED_TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

  if (PARSED_PROTO)
    get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION )
    get_target_property(PROTOC_PATH protobuf::protoc LOCATION)

    execute_process(
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
          --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}
          --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
          --grpc_out=${CMAKE_CURRENT_SOURCE_DIR}
          --plugin=protoc-gen-grpc=${grpc_cpp_plugin_location}
          ${PARSED_PROTO}
        RESULT_VARIABLE PROTOC_RESULT
        ERROR_VARIABLE PROTOC_ERROR
    )

    if (NOT PROTOC_RESULT EQUAL 0)
        message(FATAL_ERROR "Protobuf generation failed:\n${PROTOC_ERROR}")
    endif()

    foreach(proto_file ${PARSED_PROTO})
        get_filename_component(proto_basename ${proto_file} NAME_WE)

        list(APPEND GENERATED_SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/${proto_basename}.pb.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/${proto_basename}.grpc.pb.cc
        )

        list(APPEND GENERATED_HEADERS
            ${CMAKE_CURRENT_SOURCE_DIR}/${proto_basename}.pb.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${proto_basename}.grpc.pb.h
        )
    endforeach()

    # Add the generated files to your target
    set_source_files_properties(${GENERATED_SOURCES} ${GENERATED_HEADERS} PROPERTIES GENERATED TRUE)
    target_sources(${PARSED_TARGET_NAME} PRIVATE ${GENERATED_SOURCES} ${GENERATED_HEADERS})
  endif()

  if(PARSED_ADD_TESTS OR PARSED_ADD_TESTS_WITH_MOCK)
    set(EXTRA)
    if (PARSED_ADD_TESTS_WITH_MOCK)
      list(APPEND EXTRA ADD_TESTS_WITH_MOCK)
    endif()
    tq_add_test_executable_in_ut_folder(
        TARGET_NAME
          ${PARSED_TARGET_NAME}_ut
        PRIVATE
          ${PARSED_PUBLIC}
          ${PARSED_TARGET_NAME}
          ${EXTRA}
          test_utils
        )
  endif()
endfunction()

function(tq_add_test_executable_in_ut_folder)
  if(NOT BUILD_TESTS)
    return()
  endif()

  tq_parse_arguments(${ARGN})

  set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ut)
  if(NOT EXISTS ${TEST_DIR})
    message(FATAL_ERROR "${TEST_DIR} directory not found")
  endif()
  file(GLOB_RECURSE FILES ${TEST_DIR}/*)
  if (NOT FILES)
    message(FATAL_ERROR "${TEST_DIR} is empty")
  endif()

  set(TEST_LIBS doctest_main)
  if (PARSED_ADD_TESTS_WITH_MOCK)
      list(APPEND TEST_LIBS trompeloeil::trompeloeil)
  endif()

  tq_add_executable(
    TARGET_NAME
      ${PARSED_TARGET_NAME}
    SOURCES
      ${FILES}
    PRIVATE
      ${PARSED_PUBLIC}
      ${PARSED_PRIVATE}
      ${TEST_LIBS}
    )
  doctest_discover_tests(${PARSED_TARGET_NAME})
endfunction()

function(tq_add_static_library)
  tq_parse_arguments(${ARGN})
  add_library(${PARSED_TARGET_NAME} STATIC ${PARSED_SOURCES} ${PARSED_PROTO})
  target_include_directories(${PARSED_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  tq_handle_library(${ARGN})
endfunction()

function(tq_add_interface_library)
  tq_parse_arguments(${ARGN})
  add_library(${PARSED_TARGET_NAME} INTERFACE ${PARSED_SOURCES} ${PARSED_PROTO})
  tq_handle_library(${ARGN})
endfunction()

function(tq_add_executable)
  tq_parse_arguments(${ARGN})
  add_executable(${PARSED_TARGET_NAME} ${PARSED_SOURCES} ${PARSED_PROTO})
  target_include_directories(${PARSED_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  tq_handle_library(${ARGN})
endfunction()

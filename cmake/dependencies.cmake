find_package(Threads)

macro(handle_3rdparty TARGET_NAME)
  get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
  if (${TARGET_TYPE} STREQUAL "INTERFACE_LIBRARY")
    target_compile_options(${TARGET_NAME} INTERFACE "-w")
  else()
    target_compile_options(${TARGET_NAME} PRIVATE "-w")
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_CLANG_TIDY "")
    set_target_properties(${TARGET_NAME} PROPERTIES CXX_CPPCHECK "")
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER 3rdparty)
  endif()

  set_target_properties(${TARGET_NAME} PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${TARGET_NAME},INTERFACE_INCLUDE_DIRECTORIES>)
endmacro()

macro(fetch_library_extended NAME URL TAG TARGET_NAME)
  Include(FetchContent)
  set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build SHARED libraries")

  Set(FETCHCONTENT_QUIET FALSE)

  FetchContent_Declare(
    ${NAME}
    GIT_REPOSITORY ${URL}
    GIT_TAG        ${TAG}
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    GIT_SUBMODULES ""
  )

  FetchContent_MakeAvailable(${NAME})
  handle_3rdparty(${TARGET_NAME})
endmacro()

macro(fetch_library NAME URL TAG)
  find_package(${NAME} QUIET)
  if (NOT ${NAME}_FOUND)
    message("-- Fetching ${NAME}...")
    fetch_library_extended(${NAME} ${URL} ${TAG} ${NAME})
  endif()
endmacro()
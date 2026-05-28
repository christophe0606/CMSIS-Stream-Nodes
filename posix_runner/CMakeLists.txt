cmake_minimum_required(VERSION 3.22)

project(app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMSIS_STREAM_INSTALL_PREFIX "C:/cmake_packages" CACHE PATH
    "CMSIS-Stream installation prefix")
list(PREPEND CMAKE_PREFIX_PATH "${CMSIS_STREAM_INSTALL_PREFIX}")

find_package(cmsis-stream CONFIG REQUIRED)

get_filename_component(APP_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(APP_RUNNER "posix" CACHE STRING "CMSIS-Stream runner")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/app_graph/board_config.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/app_graph/board_config.cmake")
endif()

set(APP_BOARD "" CACHE STRING "Board name to use with the selected runner")
if(APP_BOARD STREQUAL "")
    if(DEFINED APP_GENERATED_BOARD)
        set(APP_BOARD "${APP_GENERATED_BOARD}")
    else()
        set(APP_BOARD "default")
    endif()
endif()

if(NOT APP_BOARD STREQUAL "default")
    set(APP_HARDWARE_TARGET "${APP_RUNNER}/${APP_BOARD}" CACHE STRING "Runner/board hardware parameter target")
elseif(DEFINED APP_GENERATED_HARDWARE_TARGET)
    set(APP_HARDWARE_TARGET "${APP_GENERATED_HARDWARE_TARGET}" CACHE STRING "Runner/board hardware parameter target")
else()
    set(APP_HARDWARE_TARGET "${APP_RUNNER}/default" CACHE STRING "Runner/board hardware parameter target")
endif()

set(APP_HARDWARE_DIR "${APP_REPO_ROOT}/boards/${APP_HARDWARE_TARGET}")
if(NOT EXISTS "${APP_HARDWARE_DIR}/hardware_params.h")
    message(FATAL_ERROR "Hardware target '${APP_HARDWARE_TARGET}' does not provide ${APP_HARDWARE_DIR}/hardware_params.h")
endif()

file(GLOB_RECURSE APP_C_SOURCES CONFIGURE_DEPENDS
    "${APP_REPO_ROOT}/src/csrc/*.c"
)

file(GLOB APP_BOARD_C_SOURCES CONFIGURE_DEPENDS
    "${APP_HARDWARE_DIR}/*.c"
)

add_executable(app
    main.cpp
    config/stream_init.cpp
    app_graph/app_params.c
    app_graph/scheduler_app.cpp
    ${APP_C_SOURCES}
    ${APP_BOARD_C_SOURCES}
)

target_include_directories(app
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/config"
        "${CMAKE_CURRENT_SOURCE_DIR}/app_graph"
        "${APP_HARDWARE_DIR}"
        "${APP_REPO_ROOT}/src"
        "${APP_REPO_ROOT}/src/csrc"
)

target_link_libraries(app
    PRIVATE
        cmsis_stream::posix_runtime
)

if(MSVC)
    set_property(TARGET app PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    target_compile_definitions(app PRIVATE
        _ITERATOR_DEBUG_LEVEL=0
    )
endif()

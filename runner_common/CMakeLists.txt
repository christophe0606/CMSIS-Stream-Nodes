if(NOT DEFINED APP_REPO_ROOT)
    get_filename_component(APP_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

if(NOT DEFINED APP_TARGET)
    set(APP_TARGET "app")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/app_graph/board_config.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/app_graph/board_config.cmake")
endif()

if(NOT DEFINED APP_RUNNER OR APP_RUNNER STREQUAL "")
    if(DEFINED APP_GENERATED_RUNNER)
        set(APP_RUNNER "${APP_GENERATED_RUNNER}")
    else()
        message(FATAL_ERROR "APP_RUNNER must be defined by the runner entry point")
    endif()
endif()

if(NOT DEFINED APP_RUNNER_CONFIG_DIR OR APP_RUNNER_CONFIG_DIR STREQUAL "")
    message(FATAL_ERROR "APP_RUNNER_CONFIG_DIR must point to the runner configuration headers")
endif()

set(APP_BOARD "" CACHE STRING "Board name to use with the selected runner")
if(APP_BOARD STREQUAL "")
    if(DEFINED APP_GENERATED_BOARD)
        set(APP_BOARD "${APP_GENERATED_BOARD}")
    elseif(APP_RUNNER STREQUAL "posix")
        message(FATAL_ERROR "APP_BOARD must be set for the POSIX runner. Use one of: Windows, Mac, Linux.")
    else()
        set(APP_BOARD "default")
    endif()
endif()

if(APP_RUNNER STREQUAL "posix" AND APP_BOARD STREQUAL "default")
    if(DEFINED APP_GENERATED_BOARD AND NOT APP_GENERATED_BOARD STREQUAL "default")
        set(APP_BOARD "${APP_GENERATED_BOARD}")
    else()
        message(FATAL_ERROR "The POSIX runner has no default board. Use one of: Windows, Mac, Linux.")
    endif()
endif()

if(DEFINED APP_GENERATED_RUNNER AND APP_GENERATED_RUNNER STREQUAL APP_RUNNER)
    set(APP_USE_GENERATED_HARDWARE_TARGET TRUE)
else()
    set(APP_USE_GENERATED_HARDWARE_TARGET FALSE)
endif()

if(APP_RUNNER STREQUAL "posix")
    set(APP_HARDWARE_TARGET "${APP_RUNNER}/${APP_BOARD}" CACHE STRING "Runner/board hardware parameter target" FORCE)
elseif(NOT APP_BOARD STREQUAL "default")
    set(APP_HARDWARE_TARGET "${APP_RUNNER}/${APP_BOARD}" CACHE STRING "Runner/board hardware parameter target" FORCE)
elseif(APP_USE_GENERATED_HARDWARE_TARGET AND DEFINED APP_GENERATED_HARDWARE_TARGET)
    set(APP_HARDWARE_TARGET "${APP_GENERATED_HARDWARE_TARGET}" CACHE STRING "Runner/board hardware parameter target" FORCE)
else()
    set(APP_HARDWARE_TARGET "${APP_RUNNER}/default" CACHE STRING "Runner/board hardware parameter target" FORCE)
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

set(APP_COMMON_SOURCES
    ${APP_RUNNER_SOURCES}
    "${CMAKE_CURRENT_LIST_DIR}/config/stream_init.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/app_graph/app_params.c"
    "${CMAKE_CURRENT_LIST_DIR}/app_graph/scheduler_app.cpp"
    ${APP_C_SOURCES}
    ${APP_BOARD_C_SOURCES}
)

set(APP_COMMON_INCLUDE_DIRS
    "${APP_RUNNER_CONFIG_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/config"
    "${CMAKE_CURRENT_LIST_DIR}/app_graph"
    "${APP_HARDWARE_DIR}"
    "${APP_REPO_ROOT}/src"
    "${APP_REPO_ROOT}/src/csrc"
)

if(TARGET ${APP_TARGET})
    # Some runners create the application target before this common CMake is included.
    target_sources(${APP_TARGET}
        PRIVATE
            ${APP_COMMON_SOURCES}
    )
else()
    add_executable(${APP_TARGET}
        ${APP_COMMON_SOURCES}
    )
endif()

target_include_directories(${APP_TARGET}
    PRIVATE
        ${APP_COMMON_INCLUDE_DIRS}
)

if(DEFINED APP_RUNNER_INCLUDE_DIRS AND NOT "${APP_RUNNER_INCLUDE_DIRS}" STREQUAL "")
    target_include_directories(${APP_TARGET}
        PRIVATE
            ${APP_RUNNER_INCLUDE_DIRS}
    )
endif()

if(DEFINED APP_RUNNER_COMPILE_DEFINITIONS AND NOT "${APP_RUNNER_COMPILE_DEFINITIONS}" STREQUAL "")
    target_compile_definitions(${APP_TARGET}
        PRIVATE
            ${APP_RUNNER_COMPILE_DEFINITIONS}
    )
endif()

if(COMMAND zephyr_include_directories)
    zephyr_include_directories(${APP_COMMON_INCLUDE_DIRS})
endif()

if(DEFINED APP_RUNNER_LIBRARIES AND NOT "${APP_RUNNER_LIBRARIES}" STREQUAL "")
    target_link_libraries(${APP_TARGET}
        PRIVATE
            ${APP_RUNNER_LIBRARIES}
    )
endif()

if(MSVC)
    set_property(TARGET ${APP_TARGET} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded")
    target_compile_definitions(${APP_TARGET} PRIVATE
        _ITERATOR_DEBUG_LEVEL=0
    )
endif()

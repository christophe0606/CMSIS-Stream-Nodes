cmake_minimum_required(VERSION 3.22)

project(app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMSIS_STREAM_INSTALL_PREFIX "C:/cmake_packages" CACHE PATH
    "CMSIS-Stream installation prefix")
list(PREPEND CMAKE_PREFIX_PATH "${CMSIS_STREAM_INSTALL_PREFIX}")

find_package(cmsis-stream CONFIG REQUIRED)
find_package(CMSISDSP CONFIG REQUIRED)

get_filename_component(APP_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(APP_RUNNER "posix" CACHE STRING "CMSIS-Stream runner")
set(APP_RUNNER_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(APP_RUNNER_LIBRARIES cmsis_stream::posix_runtime CMSISDSP::CMSISDSP)
set(APP_BOARD "" CACHE STRING "Board name to use with the selected runner")

set(APP_EFFECTIVE_BOARD "${APP_BOARD}")
if(APP_EFFECTIVE_BOARD STREQUAL "" AND EXISTS "${APP_REPO_ROOT}/runner_common/app_graph/board_config.cmake")
    include("${APP_REPO_ROOT}/runner_common/app_graph/board_config.cmake")
    set(APP_EFFECTIVE_BOARD "${APP_GENERATED_BOARD}")
endif()
if(APP_EFFECTIVE_BOARD STREQUAL "default" AND EXISTS "${APP_REPO_ROOT}/runner_common/app_graph/board_config.cmake")
    include("${APP_REPO_ROOT}/runner_common/app_graph/board_config.cmake")
    if(DEFINED APP_GENERATED_BOARD AND NOT APP_GENERATED_BOARD STREQUAL "default")
        set(APP_EFFECTIVE_BOARD "${APP_GENERATED_BOARD}")
    endif()
endif()
if(APP_EFFECTIVE_BOARD STREQUAL "")
    message(FATAL_ERROR "APP_BOARD must be set for the POSIX runner. Use one of: Windows, Mac, Linux.")
endif()
if(APP_EFFECTIVE_BOARD STREQUAL "default")
    message(FATAL_ERROR "The POSIX runner has no default board. Use one of: Windows, Mac, Linux.")
endif()

set(APP_USES_MICROPHONE_SOURCE FALSE)
set(APP_GENERATED_NODES_FILE "${APP_REPO_ROOT}/runner_common/app_graph/AppNodes.hpp")
if(EXISTS "${APP_GENERATED_NODES_FILE}")
    file(READ "${APP_GENERATED_NODES_FILE}" APP_GENERATED_NODES)
    if(APP_GENERATED_NODES MATCHES "MicrophoneSource")
        set(APP_USES_MICROPHONE_SOURCE TRUE)
    endif()
endif()

option(APP_ENABLE_PORTAUDIO_MICROPHONE "Enable POSIX microphone capture through PortAudio" ON)
if(APP_ENABLE_PORTAUDIO_MICROPHONE AND APP_USES_MICROPHONE_SOURCE)
    find_package(PortAudio CONFIG QUIET)
    find_package(unofficial-portaudio CONFIG QUIET)

    if(TARGET PortAudio::PortAudio)
        list(APPEND APP_RUNNER_LIBRARIES PortAudio::PortAudio)
        set(APP_PORTAUDIO_TARGET PortAudio::PortAudio)
        set(APP_PORTAUDIO_RUNTIME_DIRS "$<TARGET_FILE_DIR:PortAudio::PortAudio>")
        list(APPEND APP_RUNNER_COMPILE_DEFINITIONS CMSIS_STREAM_NODES_USE_PORTAUDIO)
    elseif(TARGET portaudio)
        list(APPEND APP_RUNNER_LIBRARIES portaudio)
        set(APP_PORTAUDIO_TARGET portaudio)
        set(APP_PORTAUDIO_RUNTIME_DIRS "$<TARGET_FILE_DIR:portaudio>")
        list(APPEND APP_RUNNER_COMPILE_DEFINITIONS CMSIS_STREAM_NODES_USE_PORTAUDIO)
    elseif(TARGET unofficial::portaudio::portaudio)
        list(APPEND APP_RUNNER_LIBRARIES unofficial::portaudio::portaudio)
        set(APP_PORTAUDIO_TARGET unofficial::portaudio::portaudio)
        set(APP_PORTAUDIO_RUNTIME_DIRS "$<TARGET_FILE_DIR:unofficial::portaudio::portaudio>")
        list(APPEND APP_RUNNER_COMPILE_DEFINITIONS CMSIS_STREAM_NODES_USE_PORTAUDIO)
    else()
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(PORTAUDIO QUIET IMPORTED_TARGET portaudio-2.0)
        endif()

        if(TARGET PkgConfig::PORTAUDIO)
            list(APPEND APP_RUNNER_LIBRARIES PkgConfig::PORTAUDIO)
            set(APP_PORTAUDIO_RUNTIME_DIRS ${PORTAUDIO_LIBRARY_DIRS})
            list(APPEND APP_RUNNER_COMPILE_DEFINITIONS CMSIS_STREAM_NODES_USE_PORTAUDIO)
        else()
            message(WARNING "PortAudio was not found; POSIX MicrophoneSource will compile but return underflow at runtime")
        endif()
    endif()
endif()

add_executable(app
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

add_subdirectory("${APP_REPO_ROOT}/runner_common" "${CMAKE_CURRENT_BINARY_DIR}/runner_common")

if(WIN32 AND DEFINED APP_PORTAUDIO_TARGET)
    add_custom_command(TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${APP_PORTAUDIO_TARGET}>
            $<TARGET_FILE_DIR:app>
        COMMAND_EXPAND_LISTS
    )
endif()

if(UNIX AND DEFINED APP_PORTAUDIO_RUNTIME_DIRS)
    set_property(TARGET app APPEND PROPERTY
        BUILD_RPATH "${APP_PORTAUDIO_RUNTIME_DIRS}"
    )
endif()

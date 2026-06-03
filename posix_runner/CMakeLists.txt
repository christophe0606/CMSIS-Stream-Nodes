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
set(APP_MIC_SAMPLE_RATE "16000" CACHE STRING "POSIX microphone sample rate")
set(APP_MIC_CHANNELS "1" CACHE STRING "POSIX microphone channel count")
set(APP_MIC_FRAMES_PER_BUFFER "0" CACHE STRING "POSIX microphone frames per PortAudio buffer, or 0 for unspecified")
list(APPEND APP_RUNNER_COMPILE_DEFINITIONS
    APP_MIC_SAMPLE_RATE=${APP_MIC_SAMPLE_RATE}
    APP_MIC_CHANNELS=${APP_MIC_CHANNELS}
    APP_MIC_FRAMES_PER_BUFFER=${APP_MIC_FRAMES_PER_BUFFER}
)

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
        list(APPEND APP_RUNNER_COMPILE_DEFINITIONS CMSIS_STREAM_NODES_USE_PORTAUDIO)
    elseif(TARGET portaudio)
        list(APPEND APP_RUNNER_LIBRARIES portaudio)
        list(APPEND APP_RUNNER_COMPILE_DEFINITIONS CMSIS_STREAM_NODES_USE_PORTAUDIO)
    elseif(TARGET unofficial::portaudio::portaudio)
        list(APPEND APP_RUNNER_LIBRARIES unofficial::portaudio::portaudio)
        list(APPEND APP_RUNNER_COMPILE_DEFINITIONS CMSIS_STREAM_NODES_USE_PORTAUDIO)
    else()
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(PORTAUDIO QUIET IMPORTED_TARGET portaudio-2.0)
        endif()

        if(TARGET PkgConfig::PORTAUDIO)
            list(APPEND APP_RUNNER_LIBRARIES PkgConfig::PORTAUDIO)
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

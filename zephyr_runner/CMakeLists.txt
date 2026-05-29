# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)


find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(APP_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(APP_RUNNER "zephyr" CACHE STRING "CMSIS-Stream runner")
set(APP_RUNNER_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(APP_RUNNER_SOURCES "${CMAKE_CURRENT_LIST_DIR}/main.cpp")

add_subdirectory("${APP_REPO_ROOT}/runner_common" "${CMAKE_CURRENT_BINARY_DIR}/runner_common")

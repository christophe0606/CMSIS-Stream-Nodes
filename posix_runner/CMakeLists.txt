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

add_executable(app
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

add_subdirectory("${APP_REPO_ROOT}/runner_common" "${CMAKE_CURRENT_BINARY_DIR}/runner_common")

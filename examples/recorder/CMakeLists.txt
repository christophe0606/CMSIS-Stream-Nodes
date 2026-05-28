cmake_minimum_required(VERSION 3.22)

project(recorder LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMSIS_STREAM_INSTALL_PREFIX "C:/cmake_packages" CACHE PATH
    "CMSIS-Stream installation prefix")
list(PREPEND CMAKE_PREFIX_PATH "${CMSIS_STREAM_INSTALL_PREFIX}")

find_package(cmsis-stream CONFIG REQUIRED)

get_filename_component(RECORDER_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

file(GLOB_RECURSE RECORDER_C_SOURCES CONFIGURE_DEPENDS
    "${RECORDER_REPO_ROOT}/src/csrc/*.c"
)

add_executable(recorder
    main.cpp
    config/stream_init.cpp
    recorder_graph/recorder_params.c
    recorder_graph/scheduler_recorder.cpp
    ${RECORDER_C_SOURCES}
)

target_include_directories(recorder
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/config"
        "${CMAKE_CURRENT_SOURCE_DIR}/recorder_graph"
        "${RECORDER_REPO_ROOT}/src"
        "${RECORDER_REPO_ROOT}/src/csrc"
)

target_link_libraries(recorder
    PRIVATE
        cmsis_stream::posix_runtime
)

if(MSVC)
    set_property(TARGET recorder PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    target_compile_definitions(recorder PRIVATE
        _ITERATOR_DEBUG_LEVEL=0
    )
endif()

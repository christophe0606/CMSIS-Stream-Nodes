cmake_minimum_required(VERSION 3.24)

# This file is intended to be consumed with add_subdirectory().  It may also be
# configured directly, which is useful for checking the adapter in isolation.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(tflite_micro_cmake LANGUAGES C CXX)
endif()

set(TFLITE_PATH
    "${CMAKE_CURRENT_LIST_DIR}/../tflite-micro"
    CACHE PATH
    "Path to the root of the tensorflow/tflite-micro source repository")
get_filename_component(TFLITE_PATH "${TFLITE_PATH}" ABSOLUTE)

if(NOT EXISTS "${TFLITE_PATH}/MODULE.bazel" OR
   NOT EXISTS "${TFLITE_PATH}/tensorflow/lite/micro/micro_interpreter.cc")
  message(FATAL_ERROR
    "TFLITE_PATH does not point to a tflite-micro checkout: ${TFLITE_PATH}")
endif()

option(TFLM_FETCH_DEPENDENCIES
  "Fetch FlatBuffers, gemmlowp, and ruy when local paths are not supplied" ON)
set(TFLM_FLATBUFFERS_PATH "" CACHE PATH "Optional path to a FlatBuffers checkout")
set(TFLM_GEMMLOWP_PATH "" CACHE PATH "Optional path to a gemmlowp checkout")
set(TFLM_RUY_PATH "" CACHE PATH "Optional path to a ruy checkout")

# Dependency revisions mirror MODULE.bazel in the TFLM checkout for which this
# adapter was written.  They are header-only dependencies for this target.
if(TFLM_FETCH_DEPENDENCIES)
  include(FetchContent)

  if(NOT TFLM_FLATBUFFERS_PATH)
    FetchContent_Declare(tflm_flatbuffers
      GIT_REPOSITORY https://github.com/google/flatbuffers.git
      GIT_TAG v25.9.23
      GIT_SHALLOW TRUE
      SOURCE_SUBDIR cmake-do-not-add)
    FetchContent_MakeAvailable(tflm_flatbuffers)
    set(TFLM_FLATBUFFERS_PATH "${tflm_flatbuffers_SOURCE_DIR}")
  endif()

  if(NOT TFLM_GEMMLOWP_PATH)
    FetchContent_Declare(tflm_gemmlowp
      GIT_REPOSITORY https://github.com/google/gemmlowp.git
      GIT_TAG fda83bdc38b118cc6b56753bd540caa49e570745
      SOURCE_SUBDIR cmake-do-not-add)
    FetchContent_MakeAvailable(tflm_gemmlowp)
    set(TFLM_GEMMLOWP_PATH "${tflm_gemmlowp_SOURCE_DIR}")
  endif()

  if(NOT TFLM_RUY_PATH)
    FetchContent_Declare(tflm_ruy
      GIT_REPOSITORY https://github.com/google/ruy.git
      GIT_TAG 54774a7a2cf85963777289193629d4bd42de4a59
      SOURCE_SUBDIR cmake-do-not-add)
    FetchContent_MakeAvailable(tflm_ruy)
    set(TFLM_RUY_PATH "${tflm_ruy_SOURCE_DIR}")
  endif()
endif()

foreach(_dependency IN ITEMS FLATBUFFERS GEMMLOWP RUY)
  if(NOT TFLM_${_dependency}_PATH OR
     NOT EXISTS "${TFLM_${_dependency}_PATH}")
    message(FATAL_ERROR
      "TFLM_${_dependency}_PATH is missing. Set it explicitly or enable "
      "TFLM_FETCH_DEPENDENCIES.")
  endif()
endforeach()

# Match the reference-kernel source selection in
# tensorflow/lite/micro/tools/make/sources.inc without invoking GNU Make.
file(GLOB_RECURSE _tflm_shared_sources CONFIGURE_DEPENDS
  "${TFLITE_PATH}/tensorflow/*.c"
  "${TFLITE_PATH}/tensorflow/*.cc")
list(FILTER _tflm_shared_sources EXCLUDE REGEX
  "[/\\\\]tensorflow[/\\\\]lite[/\\\\]micro[/\\\\]")
list(FILTER _tflm_shared_sources EXCLUDE REGEX
  "[/\\\\]tensorflow[/\\\\]lite[/\\\\]experimental[/\\\\]")
list(FILTER _tflm_shared_sources EXCLUDE REGEX
  "[/\\\\]tensorflow[/\\\\]lite[/\\\\]array\\.cc$")

file(GLOB _tflm_runtime_sources CONFIGURE_DEPENDS
  "${TFLITE_PATH}/tensorflow/lite/micro/*.cc"
  "${TFLITE_PATH}/tensorflow/lite/micro/arena_allocator/*.cc"
  "${TFLITE_PATH}/tensorflow/lite/micro/memory_planner/*.cc"
  "${TFLITE_PATH}/tensorflow/lite/micro/tflite_bridge/*.cc"
  "${TFLITE_PATH}/tensorflow/lite/micro/kernels/*.cc")

list(FILTER _tflm_runtime_sources EXCLUDE REGEX "_test\\.cc$")
list(FILTER _tflm_runtime_sources EXCLUDE REGEX
  "[/\\\\](fake_micro_context|mock_micro_graph|test_helpers|test_helper_custom_ops)\\.cc$")

add_library(tflite-micro STATIC
  ${_tflm_shared_sources}
  ${_tflm_runtime_sources})
add_library(TFLM::tflite-micro ALIAS tflite-micro)

target_compile_features(tflite-micro PUBLIC cxx_std_17)
target_compile_definitions(tflite-micro PUBLIC
  TF_LITE_STATIC_MEMORY
  TF_LITE_DISABLE_X86_NEON
  TF_LITE_USE_CTIME)

target_include_directories(tflite-micro
  PUBLIC
    "${TFLITE_PATH}"
    "${TFLM_FLATBUFFERS_PATH}/include"
    "${TFLM_GEMMLOWP_PATH}"
    "${TFLM_RUY_PATH}")

if(MSVC)
  set_property(TARGET tflite-micro PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded")
  target_compile_options(tflite-micro PRIVATE /GR- /EHsc /MP)
  target_compile_definitions(tflite-micro PRIVATE
    _ITERATOR_DEBUG_LEVEL=0
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX)
else()
  target_compile_options(tflite-micro PRIVATE
    -fno-exceptions
    -fno-rtti
    -fno-threadsafe-statics
    -ffunction-sections
    -fdata-sections)
endif()

set_target_properties(tflite-micro PROPERTIES
  CXX_EXTENSIONS OFF
  POSITION_INDEPENDENT_CODE OFF)

unset(_dependency)
unset(_tflm_runtime_sources)
unset(_tflm_shared_sources)

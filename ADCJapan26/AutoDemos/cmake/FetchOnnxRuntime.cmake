# FetchOnnxRuntime.cmake
# Creates an IMPORTED target onnxruntime::onnxruntime backed by a static lib.
#
# Source: https://github.com/csukuangfj/onnxruntime-libs (pre-built static packages)
#
# Override the auto-download by setting ONNXRUNTIME_LIB_DIR before including this file:
#   cmake ... -DONNXRUNTIME_LIB_DIR=/path/to/onnxruntime-linux-aarch64-static_lib-1.20.1
#
# The expected layout inside ONNXRUNTIME_LIB_DIR:
#   lib/libonnxruntime.a
#   include/onnxruntime_cxx_api.h   (or include/onnxruntime/core/session/...)

if(TARGET onnxruntime::onnxruntime)
    return()
endif()

set(ONNXRUNTIME_VERSION "1.20.1" CACHE STRING "ONNX Runtime version to fetch from csukuangfj/onnxruntime-libs")

if(DEFINED ONNXRUNTIME_LIB_DIR)
    set(_ort_dir "${ONNXRUNTIME_LIB_DIR}")
    message(STATUS "ONNX Runtime: using manual install at ${_ort_dir}")
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(_ort_arch "aarch64")
    else()
        set(_ort_arch "x86_64")
    endif()

    set(_ort_pkg "onnxruntime-linux-${_ort_arch}-static_lib-${ONNXRUNTIME_VERSION}")
    set(_ort_url
        "https://github.com/csukuangfj/onnxruntime-libs/releases/download/v${ONNXRUNTIME_VERSION}/${_ort_pkg}.tar.bz2")

    message(STATUS "ONNX Runtime: fetching ${_ort_pkg}")
    include(FetchContent)
    FetchContent_Declare(onnxruntime_static
        URL      "${_ort_url}"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(onnxruntime_static)
    set(_ort_dir "${onnxruntime_static_SOURCE_DIR}")
endif()

find_library(_ort_lib
    NAMES onnxruntime
    PATHS "${_ort_dir}"
    PATH_SUFFIXES lib lib64
    NO_DEFAULT_PATH
    REQUIRED)

find_path(_ort_inc
    NAMES onnxruntime_cxx_api.h
    PATHS "${_ort_dir}"
    PATH_SUFFIXES include include/onnxruntime/core/session
    NO_DEFAULT_PATH
    REQUIRED)

add_library(onnxruntime::onnxruntime STATIC IMPORTED GLOBAL)
set_target_properties(onnxruntime::onnxruntime PROPERTIES
    IMPORTED_LOCATION             "${_ort_lib}"
    INTERFACE_INCLUDE_DIRECTORIES "${_ort_inc}"
    INTERFACE_LINK_LIBRARIES      "pthread;dl;m")

message(STATUS "ONNX Runtime: ${_ort_lib}")

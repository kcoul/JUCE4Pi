# FetchOnnxRuntime.cmake
# Downloads the official ONNX Runtime prebuilt package for the target platform
# and creates an IMPORTED target onnxruntime::onnxruntime.
#
# For Linux aarch64 this downloads the shared lib release from:
#   https://github.com/microsoft/onnxruntime/releases
#
# Override the auto-download by setting ONNXRUNTIME_LIB_DIR before including:
#   cmake ... -DONNXRUNTIME_LIB_DIR=/path/to/onnxruntime-linux-aarch64-1.26.0
#
# Expected layout inside ONNXRUNTIME_LIB_DIR:
#   include/onnxruntime/core/session/onnxruntime_cxx_api.h
#   lib/libonnxruntime.so  (or libonnxruntime.so.1.x.x)

if(TARGET onnxruntime::onnxruntime)
    return()
endif()

set(ONNXRUNTIME_VERSION "1.26.0" CACHE STRING "ONNX Runtime version")

if(DEFINED ONNXRUNTIME_LIB_DIR)
    set(_ort_dir "${ONNXRUNTIME_LIB_DIR}")
    message(STATUS "ONNX Runtime: using manual install at ${_ort_dir}")
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(_ort_arch "aarch64")
    else()
        set(_ort_arch "x64")
    endif()

    set(_ort_pkg "onnxruntime-linux-${_ort_arch}-${ONNXRUNTIME_VERSION}")
    set(_ort_url
        "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/${_ort_pkg}.tgz")

    message(STATUS "ONNX Runtime: fetching ${_ort_pkg}")
    include(FetchContent)
    FetchContent_Declare(onnxruntime_fetch
        URL      "${_ort_url}"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(onnxruntime_fetch)
    # FetchContent does not strip the archive top-level directory.
    # The .tgz extracts to SOURCE_DIR/onnxruntime-linux-<arch>-<ver>/
    set(_ort_dir "${onnxruntime_fetch_SOURCE_DIR}/${_ort_pkg}")
    if(NOT EXISTS "${_ort_dir}")
        set(_ort_dir "${onnxruntime_fetch_SOURCE_DIR}")
    endif()
endif()

find_path(_ort_inc
    NAMES onnxruntime_cxx_api.h
    PATHS "${_ort_dir}"
    PATH_SUFFIXES
        include/onnxruntime/core/session
        include
    NO_DEFAULT_PATH
    REQUIRED)

find_library(_ort_lib
    NAMES onnxruntime
    PATHS "${_ort_dir}"
    PATH_SUFFIXES lib lib64
    NO_DEFAULT_PATH
    REQUIRED)

# Expose the .so directory so callers can copy it next to the binary.
get_filename_component(ONNXRUNTIME_LIB_RUNTIME_DIR "${_ort_lib}" DIRECTORY)
set(ONNXRUNTIME_SHARED_LIBS "" CACHE INTERNAL "")
file(GLOB _ort_sofiles "${ONNXRUNTIME_LIB_RUNTIME_DIR}/libonnxruntime*.so*")
set(ONNXRUNTIME_SHARED_LIBS "${_ort_sofiles}" CACHE INTERNAL "ONNX Runtime shared libs to deploy")

add_library(onnxruntime::onnxruntime SHARED IMPORTED GLOBAL)
set_target_properties(onnxruntime::onnxruntime PROPERTIES
    IMPORTED_LOCATION             "${_ort_lib}"
    INTERFACE_INCLUDE_DIRECTORIES "${_ort_inc}")

message(STATUS "ONNX Runtime: ${_ort_lib}")

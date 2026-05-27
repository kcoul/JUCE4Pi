# FetchOnnxRuntime.cmake
# Downloads the official ONNX Runtime prebuilt package for the target platform
# and creates an IMPORTED target onnxruntime::onnxruntime.
#
# Supports Linux (aarch64, x64) and Windows (x64, arm64).
#
# Override the auto-download by setting ONNXRUNTIME_LIB_DIR before including:
#   cmake ... -DONNXRUNTIME_LIB_DIR=/path/to/onnxruntime-<platform>-<arch>-1.26.0
#
# Expected layout inside ONNXRUNTIME_LIB_DIR:
#   include/onnxruntime_cxx_api.h  (or include/onnxruntime/core/session/)
#   lib/libonnxruntime.so          (Linux)
#   lib/onnxruntime.dll + onnxruntime.lib  (Windows)

if(TARGET onnxruntime::onnxruntime)
    return()
endif()

set(ONNXRUNTIME_VERSION "1.26.0" CACHE STRING "ONNX Runtime version")

if(DEFINED ONNXRUNTIME_LIB_DIR)
    set(_ort_dir "${ONNXRUNTIME_LIB_DIR}")
    message(STATUS "ONNX Runtime: using manual install at ${_ort_dir}")
else()
    if(WIN32)
        # Windows ARM64 packages use "arm64", not "aarch64"
        if(CMAKE_GENERATOR_PLATFORM MATCHES "ARM64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
            set(_ort_arch "arm64")
        else()
            set(_ort_arch "x64")
        endif()
        set(_ort_pkg "onnxruntime-win-${_ort_arch}-${ONNXRUNTIME_VERSION}")
        set(_ort_url
            "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/${_ort_pkg}.zip")
    else()
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            set(_ort_arch "aarch64")
        else()
            set(_ort_arch "x64")
        endif()
        set(_ort_pkg "onnxruntime-linux-${_ort_arch}-${ONNXRUNTIME_VERSION}")
        set(_ort_url
            "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/${_ort_pkg}.tgz")
    endif()

    message(STATUS "ONNX Runtime: fetching ${_ort_pkg}")
    include(FetchContent)
    FetchContent_Declare(onnxruntime_fetch
        URL      "${_ort_url}"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(onnxruntime_fetch)
    # FetchContent may or may not strip the archive top-level directory depending
    # on CMake version. Search both: SOURCE_DIR/<pkg-name>/ and SOURCE_DIR/ directly.
    set(_ort_search_dirs
        "${onnxruntime_fetch_SOURCE_DIR}/${_ort_pkg}"
        "${onnxruntime_fetch_SOURCE_DIR}")
    message(STATUS "ONNX Runtime: searching in ${_ort_search_dirs}")
endif()

if(NOT DEFINED _ort_search_dirs)
    set(_ort_search_dirs "${_ort_dir}")
endif()

find_path(_ort_inc
    NAMES onnxruntime_cxx_api.h
    PATHS ${_ort_search_dirs}
    PATH_SUFFIXES
        include/onnxruntime/core/session
        include
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
    REQUIRED)

find_library(_ort_lib
    NAMES onnxruntime
    PATHS ${_ort_search_dirs}
    PATH_SUFFIXES lib lib64
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
    REQUIRED)

get_filename_component(_ort_lib_dir "${_ort_lib}" DIRECTORY)

add_library(onnxruntime::onnxruntime SHARED IMPORTED GLOBAL)

if(WIN32)
    # _ort_lib is the import lib (.lib); IMPORTED_LOCATION must point to the .dll
    find_file(_ort_dll
        NAMES onnxruntime.dll
        PATHS "${_ort_lib_dir}"
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
        REQUIRED)
    set_target_properties(onnxruntime::onnxruntime PROPERTIES
        IMPORTED_LOCATION             "${_ort_dll}"
        IMPORTED_IMPLIB               "${_ort_lib}"
        INTERFACE_INCLUDE_DIRECTORIES "${_ort_inc}")
    file(GLOB _ort_deploy "${_ort_lib_dir}/onnxruntime*.dll")
    set(ONNXRUNTIME_SHARED_LIBS "${_ort_deploy}" CACHE INTERNAL "ONNX Runtime shared libs to deploy")
    message(STATUS "ONNX Runtime: ${_ort_dll} (implib: ${_ort_lib})")
else()
    set_target_properties(onnxruntime::onnxruntime PROPERTIES
        IMPORTED_LOCATION             "${_ort_lib}"
        INTERFACE_INCLUDE_DIRECTORIES "${_ort_inc}")
    file(GLOB _ort_deploy "${_ort_lib_dir}/libonnxruntime*.so*")
    set(ONNXRUNTIME_SHARED_LIBS "${_ort_deploy}" CACHE INTERNAL "ONNX Runtime shared libs to deploy")
    message(STATUS "ONNX Runtime: ${_ort_lib}")
endif()

# FindHailoRT.cmake
# Locates the HailoRT SDK and creates the HailoRT::libhailort imported target.
# Delegates to the package's own HailoRTConfig.cmake with platform-specific hints.
#
# Search order:
#   1. HailoRT_ROOT cache/env var, CMAKE_PREFIX_PATH
#   2. Windows: C:/Program Files/HailoRT  (official installer default)
#   3. All platforms: CMake package registry, standard system paths
#
# Result variables (set by underlying HailoRTConfig.cmake):
#   HailoRT_FOUND           TRUE when located
#   HailoRT_VERSION         version string, e.g. "5.2.0"
#   HailoRT::libhailort     IMPORTED target (include dirs + link library)

if(TARGET HailoRT::libhailort)
    set(HailoRT_FOUND TRUE)
    return()
endif()

set(_hailo_hints "")
if(WIN32)
    list(APPEND _hailo_hints "C:/Program Files/HailoRT")
    if(DEFINED ENV{HAILORT_INSTALL_DIR})
        list(PREPEND _hailo_hints "$ENV{HAILORT_INSTALL_DIR}")
    endif()
endif()

# CONFIG skips Find modules — no recursion risk.
find_package(HailoRT CONFIG QUIET HINTS ${_hailo_hints})

if(HailoRT_FOUND)
    if(NOT HailoRT_FIND_QUIETLY)
        get_target_property(_hailo_inc HailoRT::libhailort INTERFACE_INCLUDE_DIRECTORIES)
        if(HailoRT_VERSION)
            message(STATUS "HailoRT: found v${HailoRT_VERSION} — ${_hailo_inc}")
        else()
            message(STATUS "HailoRT: found — ${_hailo_inc}")
        endif()
        unset(_hailo_inc)
    endif()
else()
    if(HailoRT_FIND_REQUIRED)
        if(WIN32)
            message(FATAL_ERROR
                "HailoRT not found.\n"
                "Install the HailoRT SDK from hailo.ai/developer-zone/\n"
                "or set HailoRT_ROOT to the install prefix.")
        else()
            message(FATAL_ERROR
                "HailoRT not found.\n"
                "Install the hailort package or extract the .deb under HAILO_CROSS_PREFIX.\n"
                "Or set HailoRT_ROOT to the install prefix.")
        endif()
    endif()
endif()

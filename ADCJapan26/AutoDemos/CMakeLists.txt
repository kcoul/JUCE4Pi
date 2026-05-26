cmake_minimum_required(VERSION 3.22)
project(AutoDemos VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Root cmake dir — already set by parent when built from JUCE4Pi root,
# otherwise resolve relative to this file (../../cmake = JUCE4Pi/cmake/).
if(NOT DEFINED JUCE4PI_CMAKE_DIR)
    set(JUCE4PI_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
endif()
list(APPEND CMAKE_MODULE_PATH "${JUCE4PI_CMAKE_DIR}")

set(AUTODEMOS_JUCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/JUCE"
    CACHE PATH "Path to the JUCE source tree (default: JUCE4Pi repo root libs/JUCE)")

if(NOT EXISTS "${AUTODEMOS_JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE not found at ${AUTODEMOS_JUCE_DIR}\n"
        "Ensure you cloned JUCE4Pi with --recurse-submodules, or pass -DAUTODEMOS_JUCE_DIR=/path/to/JUCE")
endif()

if(NOT TARGET juce::juce_core)
    add_subdirectory("${AUTODEMOS_JUCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE")
endif()

add_subdirectory(DataBridge)
add_subdirectory(VREngine)

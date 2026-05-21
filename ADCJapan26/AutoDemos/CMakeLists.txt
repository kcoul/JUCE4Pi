cmake_minimum_required(VERSION 3.22)
project(AutoDemos VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow standalone builds; when built as part of JUCE4Pi, JUCE is already
# configured by the GENISYS submodule processed before us.
if(NOT TARGET juce::juce_core)
    set(AUTODEMOS_JUCE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/../surge/libs/JUCE"
        CACHE PATH "Path to the JUCE source tree")
    add_subdirectory("${AUTODEMOS_JUCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE")
endif()

add_subdirectory(DataBridge)
add_subdirectory(VREngine)

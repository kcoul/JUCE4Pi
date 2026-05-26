cmake_minimum_required(VERSION 3.22)
project(JUCE4Pi VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE4PI_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${JUCE4PI_CMAKE_DIR}")

set(JUCE4PI_JUCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE"
    CACHE PATH "Path to the JUCE source tree")

if(NOT EXISTS "${JUCE4PI_JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE not found at ${JUCE4PI_JUCE_DIR}\n"
        "Run: git submodule update --init libs/JUCE")
endif()

if(NOT TARGET juce::juce_core)
    add_subdirectory("${JUCE4PI_JUCE_DIR}" JUCE)
endif()

option(BUILD_ADC21 "Build ADC21 targets" OFF)
if(BUILD_ADC21)
    add_subdirectory(ADC21)
endif()

add_subdirectory(ADCJapan26/AutoDemos)

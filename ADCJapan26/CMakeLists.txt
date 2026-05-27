cmake_minimum_required(VERSION 3.28)
project(JUCE4Pi VERSION 0.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

add_subdirectory(surge)
add_subdirectory(GENISYS)
add_subdirectory(AutoDemos)

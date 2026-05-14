cmake_minimum_required(VERSION 3.16)
project(JUCE4Pi VERSION 0.1)

option(BUILD_JUCE_PROTOTYPE_EXAMPLES "Build JUCE prototype examples" OFF)

add_subdirectory(ADCJapan26)

if(BUILD_JUCE_PROTOTYPE_EXAMPLES)
  add_subdirectory(ADC21)
endif()

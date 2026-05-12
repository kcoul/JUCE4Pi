cmake_minimum_required(VERSION 3.16)
project(JUCE4Pi VERSION 0.1)

# Including custom cmake rules
include(CMake/clang-cxx-dev-tools.cmake)

#Compile commands, useful for some IDEs like VS-Code
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

#Minimum MacOS target, set globally
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version" FORCE)

option(UniversalBinary "Build universal binary for mac" OFF)

if (UniversalBinary)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "")
endif()

#static linking in Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

#Adds all the module sources so they appear correctly in the IDE
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

#set any of these to "ON" if you want to build one of the juce examples
#or extras (Projucer/AudioPluginHost, etc):
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)

#By default, we are not building the provided plugin/app examples from ADC21:
option(BUILD_JUCE_PROTOTYPE_EXAMPLES "Build JUCE prototype examples" OFF)

# ADCJapan26 adds surge (which adds libs/JUCE and SurgeMidiToOscBridge),
# establishing the juce:: targets used by everything else.
add_subdirectory(ADCJapan26)

if (${BUILD_JUCE_PROTOTYPE_EXAMPLES})
    add_subdirectory(ADC21)
endif()

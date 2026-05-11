project(ADC21CPanel VERSION 0.0.1)

set(TargetName ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)

if (UNIX AND NOT APPLE)
    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")
    include (Findpigpio)
endif()

#helper function to add all images from a given folder:
function(add_images_from_directory target imagesSubFolder)
    set (ImagesDir ${CMAKE_CURRENT_LIST_DIR}/${imagesSubFolder})
    file(GLOB_RECURSE images
            "${ImagesDir}/*.jpg"
            "${ImagesDir}/*.png"
            "${ImagesDir}/*.jpeg")

    if(NOT images STREQUAL "")
        set (ImagesTarget "${target}-Images")
        juce_add_binary_data(${ImagesTarget} SOURCES ${images})
        target_link_libraries(${target} PRIVATE ${ImagesTarget})
    endif()
endfunction()

if(ARM)
    find_package(pigpio REQUIRED)
endif()

juce_add_gui_app(${TargetName} PRODUCT_NAME "ADC21CPanel")

#add_images_from_directory needs to be below juce_add_gui_app
add_images_from_directory(${TargetName} Images)

#==============
#TARGET_SOURCES
#==============

set (SRC_LIST
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/MainWindow.cpp)

if(ARM)
        set (SRC_LIST ${SRC_LIST}
                Source/RED.c
                Source/rotary_encoder.cpp)
endif()

target_sources(${TargetName} PRIVATE
        ${SRC_LIST})

#==========================
#TARGET_COMPILE_DEFINITIONS
#==========================

target_compile_definitions(${TargetName} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_PROJECT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_VERSION>"
        JUCE_DISPLAY_SPLASH_SCREEN=0
        #MICROPHONE_PERMISSION_ENABLED=TRUE
        #MICROPHONE_PERMISSION_TEXT="macOS requires permission for Microphone access" 
	)

#=====================
#TARGET_LINK_LIBRARIES
#=====================

target_link_libraries(${TargetName} PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        hack_audio_gui
        ff_meters)

if (ARM)
    target_link_libraries(${TargetName} PRIVATE
            ${pigpio_LIBRARY}
            ${pigpiod_if2_LIBRARY}
            pthread)
endif()
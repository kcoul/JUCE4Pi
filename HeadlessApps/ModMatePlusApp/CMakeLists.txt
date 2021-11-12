project(ModMatePlusApp VERSION 0.0.1)

set (BaseTargetName ModMatePlusApp)

set(CMAKE_CXX_STANDARD 17)

if (UNIX AND NOT APPLE)
    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")
    include (Findpigpio)
endif()

#helper function to add all images from a given folder:
function(add_images_from_directory target imagesSubFolder)
    set (ImagesDir ${CMAKE_CURRENT_LIST_DIR}/${imagesSubFolder})
    file(GLOB_RECURSE images
            "${ImagesDir}/*.svg"
            "${ImagesDir}/*.jpg"
            "${ImagesDir}/*.png"
            "${ImagesDir}/*.jpeg")

    if(NOT images STREQUAL "")
        set (ImagesTarget "${target}-Images")
        juce_add_binary_data(${ImagesTarget} SOURCES ${images})
        target_link_libraries(${target} PRIVATE ${ImagesTarget})
    endif()
endfunction()

if(UNIX AND NOT APPLE)
    find_package(ALSA REQUIRED)
    set(NATIVE_AUDIO_LIBS ${ALSA_LIBRARIES})
    find_package(pigpio REQUIRED)
endif()

juce_add_gui_app(${BaseTargetName} PRODUCT_NAME "ModMatePlusApp")

#add_images_from_directory needs to be below juce_add_gui_app
add_images_from_directory(${BaseTargetName} Images)

#==============
#TARGET_SOURCES
#==============

set (SRC_LIST
        Source/Main.cpp)

if(UNIX AND NOT APPLE)
        set (SRC_LIST ${SRC_LIST}
                Source/RED.c
                Source/rotary_encoder.cpp)
endif()

target_sources(${BaseTargetName} PRIVATE
        ${SRC_LIST})

#==========================
#TARGET_COMPILE_DEFINITIONS
#==========================

target_compile_definitions(${BaseTargetName} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${BaseTargetName},JUCE_PROJECT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${BaseTargetName},JUCE_VERSION>"
        JUCE_DISPLAY_SPLASH_SCREEN=0 
        )

target_link_libraries(${BaseTargetName} PRIVATE
        juce_audio_utils
        juce_recommended_config_flags
        juce_recommended_lto_flags
        juce_recommended_warning_flags)

if (UNIX AND NOT APPLE)
    target_link_libraries(${BaseTargetName} PRIVATE
            ${pigpio_LIBRARY}
            ${pigpiod_if2_LIBRARY}
            pthread)
endif()

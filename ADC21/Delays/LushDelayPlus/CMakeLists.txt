project(LushDelayPlus VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

set (BaseTargetName LushDelayPlus)

set(USING_CMAKE true CACHE BOOL "")

#helper functions to add all resources from a folder tree,
#but without changing the original Projucer-based structure:
function(add_resources_from_directories target fontsSubFolder graphicsSubFolder)
    set (FontsDir ${CMAKE_CURRENT_LIST_DIR}/${fontsSubFolder})
    set (GraphicsDir ${CMAKE_CURRENT_LIST_DIR}/${graphicsSubFolder})

    file(GLOB_RECURSE resources
            "${FontsDir}/Dancing_Script/static/*.ttf"
            "${FontsDir}/Dancing_Script/*.ttf"
            "${FontsDir}/Montserrat/*.ttf")

    file(GLOB_RECURSE graphics
            "${GraphicsDir}/svg/*.svg")

    #This line illustrates pretty well how confusing CMake can be
    #https://stackoverflow.com/questions/7533502/how-can-i-merge-multiple-lists-of-files-together-with-cmake
    list(APPEND resources ${graphics})

    if(NOT resources STREQUAL "")
        set (ResourcesTarget "${target}-Resources")
        juce_add_binary_data(${ResourcesTarget} SOURCES ${resources})
        target_link_libraries(${target} PRIVATE ${ResourcesTarget})
    endif()
endfunction()

juce_add_plugin("${BaseTargetName}"
        # VERSION ...                               # Set this if the plugin version is different to the project version
        # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
        # ICON_SMALL ...
        COMPANY_NAME "MyCompany"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Spbt
        PLUGIN_CODE Lshp
        FORMATS AU VST3 Standalone
        PRODUCT_NAME "LushDelayPlus")

#add_resources_from_directories needs to be below juce_add_plugin
add_resources_from_directories(${BaseTargetName} Fonts Graphics)
target_sources(${BaseTargetName} PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_compile_definitions(${BaseTargetName}
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

if(${USING_CMAKE})
    target_compile_definitions(${BaseTargetName} PUBLIC
            USING_CMAKE=1)
endif()

target_link_libraries(${BaseTargetName} PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        #stm_dsp
        #stm_gui
        )

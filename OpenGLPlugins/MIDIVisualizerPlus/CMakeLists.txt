project(MIDIVisualizerPlus VERSION 0.0.1)

set (BaseTargetName MIDIVisualizerPlus)

#helper function to add all resources from a given folder:
function(add_resources_from_directory target resourcesSubFolder)
    set (ResourcesDir ${CMAKE_CURRENT_LIST_DIR}/${resourcesSubFolder})
    file(GLOB_RECURSE resources
            "${ResourcesDir}/*.obj"
            "${ResourcesDir}/*.svg"
            "${ResourcesDir}/*.jpg"
            "${ResourcesDir}/*.png"
            "${ResourcesDir}/*.jpeg")

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
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT TRUE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Juce
        PLUGIN_CODE Dem0
        FORMATS AU VST3 Standalone
        PRODUCT_NAME "MIDIVisualizerPlus")

# Needs to be below juce_add_plugin
add_resources_from_directory(${BaseTargetName} Resources)

target_sources(${BaseTargetName} PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_compile_definitions(${BaseTargetName}
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(${BaseTargetName} PRIVATE
        juce::juce_audio_utils
        juce::juce_opengl
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

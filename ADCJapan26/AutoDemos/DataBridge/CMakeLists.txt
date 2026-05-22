juce_add_plugin(DataBridge
    PRODUCT_NAME        "DataBridge"
    COMPANY_NAME        "QNX"
    VERSION             "0.1"
    PLUGIN_MANUFACTURER_CODE Gnsy
    PLUGIN_CODE         GmOp
    FORMATS             VST3 Standalone
    IS_MIDI_EFFECT      TRUE
    NEEDS_MIDI_INPUT    TRUE
    NEEDS_MIDI_OUTPUT   FALSE
    IS_SYNTH            FALSE)

target_sources(DataBridge
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_include_directories(DataBridge
    PRIVATE
        Source)

target_compile_definitions(DataBridge
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(DataBridge
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_gui_basics
        juce::juce_osc
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

juce_add_plugin(SimpleReverbPlus
    COMPANY_NAME "Suzuki Kengo"  
    COMPANY_COPYRIGHT "Suzuki Kengo"
    COMPANY_WEBSITE "https://suzuki-kengo.net"
    COMPANY_EMAIL "zukkesukke23@gmail.com"
    IS_SYNTH FALSE    
    PLUGIN_MANUFACTURER_CODE Keng
    PLUGIN_CODE Srvp
    FORMATS 
            Standalone 
            AU
            VST3 
    VST3_CATEGORIES "Fx" 
    AU_MAIN_TYPE "kAudioUnitType_Effect"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This applicaion requires a permission to use an audio input device of your computer. By Default, Built-In microphone will be used."
    PRODUCT_NAME "SimpleReverbPlus")        

target_compile_features(SimpleReverbPlus PUBLIC cxx_std_17)


target_compile_definitions(SimpleReverbPlus
    PUBLIC
    JUCE_WEB_BROWSER=0  
    JUCE_USE_CURL=0     
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_sources(SimpleReverbPlus PRIVATE
    PluginEditor.cpp
    PluginProcessor.cpp
    CustomLookAndFeel.cpp 
    RotarySlider.cpp 
)

target_link_libraries(SimpleReverbPlus PUBLIC
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    )

juce_generate_juce_header(SimpleReverbPlus)
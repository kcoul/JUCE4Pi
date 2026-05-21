juce_add_gui_app(VREngine
    PRODUCT_NAME "VREngine"
    VERSION      "0.1")

target_sources(VREngine
    PRIVATE
        Source/Main.cpp)

target_include_directories(VREngine
    PRIVATE
        Source)

option(VRENGINE_HAS_HAILO "Enable Hailo NPU voice input via HailoRT" OFF)

target_compile_definitions(VREngine
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        VRENGINE_HAS_HAILO=$<BOOL:${VRENGINE_HAS_HAILO}>
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VREngine,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VREngine,JUCE_VERSION>")

target_link_libraries(VREngine
    PRIVATE
        juce::juce_gui_basics
        juce::juce_osc
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

# When Hailo is enabled, add VoiceInput_Hailo.cpp and link HailoRT.
# This mirrors the BRIDGE_HAS_HAILO pattern in SurgeMidiToOscBridge.
if(VRENGINE_HAS_HAILO)
    target_sources(VREngine PRIVATE Source/VoiceInput_Hailo.cpp)
    find_package(HailoRT REQUIRED)
    target_link_libraries(VREngine PRIVATE HailoRT::libhailort)
endif()

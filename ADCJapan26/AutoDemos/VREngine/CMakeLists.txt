juce_add_gui_app(VREngine
    PRODUCT_NAME "VREngine"
    VERSION      "0.1")

target_sources(VREngine
    PRIVATE
        Source/Main.cpp)

target_include_directories(VREngine
    PRIVATE
        Source)

target_compile_definitions(VREngine
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VREngine,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VREngine,JUCE_VERSION>")

target_link_libraries(VREngine
    PRIVATE
        juce::juce_gui_basics
        juce::juce_osc
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

find_package(HailoRT REQUIRED)

target_sources(VREngine PRIVATE Source/VoiceInput_Hailo.cpp)

include(FetchOnnxRuntime)

# libonnxruntime.so ships next to the binary; tell the dynamic linker to look there.
set_target_properties(VREngine PROPERTIES
    BUILD_RPATH  "$ORIGIN"
    INSTALL_RPATH "$ORIGIN")

target_link_libraries(VREngine PRIVATE
    HailoRT::libhailort
    onnxruntime::onnxruntime
    juce::juce_audio_devices)

# Windows: copy the fetched onnxruntime*.dll next to the binary so the correct
# version is loaded instead of any ancient system-installed onnxruntime.dll.
if(WIN32)
    foreach(_ort_dll ${ONNXRUNTIME_SHARED_LIBS})
        add_custom_command(TARGET VREngine POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${_ort_dll}"
                    "$<TARGET_FILE_DIR:VREngine>"
            COMMENT "Copying ${_ort_dll} next to VREngine binary")
    endforeach()
endif()

# Download Silero VAD ONNX model and embed it via incbin.
set(_silero_model  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/silero_vad.onnx")
set(_incbin_dir    "${CMAKE_CURRENT_SOURCE_DIR}/../libs/incbin")
set(_gen_dir       "${CMAKE_CURRENT_BINARY_DIR}/generated")

if(NOT EXISTS "${_silero_model}")
    message(STATUS "VREngine: downloading Silero VAD model...")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
    file(DOWNLOAD
        "https://github.com/snakers4/silero-vad/raw/master/src/silero_vad/data/silero_vad.onnx"
        "${_silero_model}"
        TIMEOUT 60
        STATUS _dl_status)
    list(GET _dl_status 0 _dl_code)
    if(NOT _dl_code EQUAL 0)
        list(GET _dl_status 1 _dl_msg)
        file(REMOVE "${_silero_model}")
        message(FATAL_ERROR "VREngine: Silero VAD model download failed: ${_dl_msg}\n"
            "Download manually to ${_silero_model} then reconfigure.")
    endif()
endif()

file(MAKE_DIRECTORY "${_gen_dir}")
set(_embed_src "${_gen_dir}/silero_onnx_embed.c")

if(WIN32)
    # MSVC/clang-cl do not support GAS inline assembly (.incbin); generate a C array instead.
    add_custom_command(
        OUTPUT  "${_embed_src}"
        COMMAND "${CMAKE_COMMAND}"
                -DINPUT="${_silero_model}"
                -DOUTPUT="${_embed_src}"
                -DSYMBOL=SileroVadOnnx
                -DPREFIX=g
                -P "${JUCE4PI_CMAKE_DIR}/BinaryToCArray.cmake"
        DEPENDS "${_silero_model}"
        COMMENT "Embedding Silero VAD model as C array")
else()
    set(SILERO_ONNX_PATH "${_silero_model}")
    configure_file("${_incbin_dir}/silero_onnx_embed.c.in" "${_embed_src}" @ONLY)
endif()

target_sources(VREngine PRIVATE "${_embed_src}")
target_include_directories(VREngine PRIVATE "${_incbin_dir}")
target_compile_definitions(VREngine PRIVATE VRENGINE_HAS_EMBEDDED_SILERO_VAD=1)

# Windows: avoid min/max macro conflicts from COM headers pulled in by HailoRT.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(VREngine PRIVATE NOMINMAX=1)
endif()

# Download Whisper HEF models if not already present.
set(_hailo_models_dir "${CMAKE_CURRENT_SOURCE_DIR}/Resources/models/hailo10h")
set(VRENGINE_HAILO_MODELS_VERSION "5.3.0" CACHE STRING "Hailo model package version for Whisper HEF downloads")
file(MAKE_DIRECTORY "${_hailo_models_dir}")
foreach(_hef_name Tiny Base Small)
    set(_hef_dst "${_hailo_models_dir}/Whisper-${_hef_name}.hef")
    if(NOT EXISTS "${_hef_dst}")
        message(STATUS "VREngine: downloading Whisper-${_hef_name}.hef ...")
        file(DOWNLOAD
            "https://dev-public.hailo.ai/v${VRENGINE_HAILO_MODELS_VERSION}/blob/Whisper-${_hef_name}.hef"
            "${_hef_dst}"
            TIMEOUT 120
            STATUS _hef_status)
        list(GET _hef_status 0 _hef_code)
        if(NOT _hef_code EQUAL 0)
            list(GET _hef_status 1 _hef_msg)
            file(REMOVE "${_hef_dst}")
            message(WARNING "VREngine: failed to download Whisper-${_hef_name}.hef: ${_hef_msg}\n"
                "Download manually to ${_hef_dst} then reconfigure.")
        else()
            message(STATUS "VREngine: downloaded Whisper-${_hef_name}.hef")
        endif()
    endif()
endforeach()

# Copy Whisper HEF models next to the binary at build time.
foreach(_hef_name Tiny Base Small)
    set(_hef_src "${_hailo_models_dir}/Whisper-${_hef_name}.hef")
    if(EXISTS "${_hef_src}")
        add_custom_command(TARGET VREngine POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory
                    "$<TARGET_FILE_DIR:VREngine>/models/hailo10h"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${_hef_src}"
                    "$<TARGET_FILE_DIR:VREngine>/models/hailo10h/Whisper-${_hef_name}.hef"
            COMMENT "Copying Whisper-${_hef_name}.hef next to VREngine binary")
    endif()
endforeach()

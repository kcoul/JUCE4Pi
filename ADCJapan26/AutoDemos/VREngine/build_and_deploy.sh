#!/usr/bin/env bash
# Build VREngine for QNX (aarch64) and optionally deploy to target via SCP.
#
# Usage:
#   ./build_and_deploy.sh                          # build only
#   ./build_and_deploy.sh user@192.168.1.100       # build + deploy
#   ./build_and_deploy.sh user@192.168.1.100 /opt  # build + deploy to custom path
#
# Requires the QNX SDP 8.0 environment sourced via qnxsdp-env.sh.

set -euo pipefail

source "$HOME/qnx800/qnxsdp-env.sh"

TARGET_TRIPLE="${QNX_TARGET_TRIPLE:-12.2.0,gcc_ntoaarch64le}"
DEPLOY_HOST="${1:-}"
DEPLOY_PATH="${2:-/tmp}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
JUCE_ROOT="$SCRIPT_DIR/../../surge/libs/JUCE"
SRC_DIR="$SCRIPT_DIR/Source"
BUILD_DIR="$SCRIPT_DIR/build/qnx_${TARGET_TRIPLE//,/_}"

mkdir -p "$BUILD_DIR"

COMMON=(
    "-V${TARGET_TRIPLE}"
    -std=gnu++17
    -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    -DJUCE_USE_CURL=0
    -DJUCE_WEB_BROWSER=0
    -DJUCE_JACK=0
    -DJUCE_ALSA=1
    -DJUCE_USE_FONTCONFIG=0
    "-I$JUCE_ROOT"
    "-I$JUCE_ROOT/modules"
    "-I$SRC_DIR"
)

echo "Compiling JUCE modules..."
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_core/juce_core.cpp"                        -o "$BUILD_DIR/juce_core.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_core/juce_core_CompilationTime.cpp"        -o "$BUILD_DIR/juce_core_CompilationTime.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_events/juce_events.cpp"                    -o "$BUILD_DIR/juce_events.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_graphics/juce_graphics.cpp"                -o "$BUILD_DIR/juce_graphics.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_graphics/juce_graphics_Harfbuzz.cpp"       -o "$BUILD_DIR/juce_graphics_Harfbuzz.o"
qcc "-V${TARGET_TRIPLE}" -DSB_CONFIG_UNITY=1 -I"$JUCE_ROOT" -I"$JUCE_ROOT/modules" \
    -c "$JUCE_ROOT/modules/juce_graphics/unicode/sheenbidi/Source/SheenBidi.c"             -o "$BUILD_DIR/SheenBidi.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_data_structures/juce_data_structures.cpp"  -o "$BUILD_DIR/juce_data_structures.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics.cpp"            -o "$BUILD_DIR/juce_gui_basics.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics_2.cpp"          -o "$BUILD_DIR/juce_gui_basics_2.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics_3.cpp"          -o "$BUILD_DIR/juce_gui_basics_3.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics_4.cpp"          -o "$BUILD_DIR/juce_gui_basics_4.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics_5.cpp"          -o "$BUILD_DIR/juce_gui_basics_5.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_audio_basics/juce_audio_basics.cpp"        -o "$BUILD_DIR/juce_audio_basics.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_audio_devices/juce_audio_devices.cpp"      -o "$BUILD_DIR/juce_audio_devices.o"
q++ "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_osc/juce_osc.cpp"                          -o "$BUILD_DIR/juce_osc.o"

echo "Compiling VREngine..."
q++ "${COMMON[@]}" -c "$SRC_DIR/Main.cpp" -o "$BUILD_DIR/Main.o"

echo "Linking..."
q++ "-V${TARGET_TRIPLE}" \
    "$BUILD_DIR/juce_core.o" \
    "$BUILD_DIR/juce_core_CompilationTime.o" \
    "$BUILD_DIR/juce_events.o" \
    "$BUILD_DIR/juce_graphics.o" \
    "$BUILD_DIR/juce_graphics_Harfbuzz.o" \
    "$BUILD_DIR/SheenBidi.o" \
    "$BUILD_DIR/juce_data_structures.o" \
    "$BUILD_DIR/juce_gui_basics.o" \
    "$BUILD_DIR/juce_gui_basics_2.o" \
    "$BUILD_DIR/juce_gui_basics_3.o" \
    "$BUILD_DIR/juce_gui_basics_4.o" \
    "$BUILD_DIR/juce_gui_basics_5.o" \
    "$BUILD_DIR/juce_audio_basics.o" \
    "$BUILD_DIR/juce_audio_devices.o" \
    "$BUILD_DIR/juce_osc.o" \
    "$BUILD_DIR/Main.o" \
    -lscreen -lasound -lsocket -lz -lexpat \
    -o "$BUILD_DIR/VREngine"

echo "Built: $BUILD_DIR/VREngine"

if [[ -n "$DEPLOY_HOST" ]]; then
    echo "Deploying to $DEPLOY_HOST:$DEPLOY_PATH ..."
    ssh "$DEPLOY_HOST" "mkdir -p '$DEPLOY_PATH'"
    scp "$BUILD_DIR/VREngine" "$DEPLOY_HOST:$DEPLOY_PATH/VREngine"
    echo "Deployed. Run with: ssh $DEPLOY_HOST '$DEPLOY_PATH/VREngine'"
fi

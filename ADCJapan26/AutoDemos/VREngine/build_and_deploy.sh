#!/usr/bin/env bash
# Cross-compile VREngine for aarch64 Ubuntu Linux (RPi 5) from x86_64 Linux / WSL2.
# Optionally deploys the binary to the target via SCP.
#
# Prerequisites (WSL2/Ubuntu host):
#   sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
#
# Usage:
#   ./build_and_deploy.sh                          # build only
#   ./build_and_deploy.sh user@192.168.1.100       # build + deploy to /tmp
#   ./build_and_deploy.sh user@192.168.1.100 /opt  # build + deploy to custom path
#
# If your cross-compiler needs a sysroot (for finding target X11/ALSA headers),
# set SYSROOT before calling: export SYSROOT=/path/to/rpi5-sysroot

set -euo pipefail

CXX="${CXX:-aarch64-linux-gnu-g++}"
CC="${CC:-aarch64-linux-gnu-gcc}"
DEPLOY_HOST="${1:-}"
DEPLOY_PATH="${2:-/tmp}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
JUCE_ROOT="$SCRIPT_DIR/../../surge/libs/JUCE"
SRC_DIR="$SCRIPT_DIR/Source"
BUILD_DIR="$SCRIPT_DIR/build/aarch64-linux"

SYSROOT_FLAGS=()
if [[ -n "${SYSROOT:-}" ]]; then
    SYSROOT_FLAGS=("--sysroot=$SYSROOT")
fi

mkdir -p "$BUILD_DIR"

COMMON=(
    "${SYSROOT_FLAGS[@]}"
    -std=gnu++17
    -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    -DJUCE_USE_CURL=0
    -DJUCE_WEB_BROWSER=0
    -DJUCE_JACK=0
    -DJUCE_ALSA=1
    -DJUCE_USE_FONTCONFIG=1
    "-I$JUCE_ROOT"
    "-I$JUCE_ROOT/modules"
    "-I$SRC_DIR"
)

echo "Compiling JUCE modules..."
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_core/juce_core.cpp"                        -o "$BUILD_DIR/juce_core.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_core/juce_core_CompilationTime.cpp"        -o "$BUILD_DIR/juce_core_CompilationTime.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_events/juce_events.cpp"                    -o "$BUILD_DIR/juce_events.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_graphics/juce_graphics.cpp"                -o "$BUILD_DIR/juce_graphics.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_graphics/juce_graphics_Harfbuzz.cpp"       -o "$BUILD_DIR/juce_graphics_Harfbuzz.o"
$CC  "${SYSROOT_FLAGS[@]}" -DSB_CONFIG_UNITY=1 -I"$JUCE_ROOT" -I"$JUCE_ROOT/modules" \
    -c "$JUCE_ROOT/modules/juce_graphics/unicode/sheenbidi/Source/SheenBidi.c"             -o "$BUILD_DIR/SheenBidi.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_data_structures/juce_data_structures.cpp"  -o "$BUILD_DIR/juce_data_structures.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics.cpp"            -o "$BUILD_DIR/juce_gui_basics.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics_2.cpp"          -o "$BUILD_DIR/juce_gui_basics_2.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics_3.cpp"          -o "$BUILD_DIR/juce_gui_basics_3.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics_4.cpp"          -o "$BUILD_DIR/juce_gui_basics_4.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_gui_basics/juce_gui_basics_5.cpp"          -o "$BUILD_DIR/juce_gui_basics_5.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_audio_basics/juce_audio_basics.cpp"        -o "$BUILD_DIR/juce_audio_basics.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_audio_devices/juce_audio_devices.cpp"      -o "$BUILD_DIR/juce_audio_devices.o"
$CXX "${COMMON[@]}" -c "$JUCE_ROOT/modules/juce_osc/juce_osc.cpp"                          -o "$BUILD_DIR/juce_osc.o"

echo "Compiling VREngine..."
$CXX "${COMMON[@]}" -c "$SRC_DIR/Main.cpp" -o "$BUILD_DIR/Main.o"

echo "Linking..."
$CXX "${SYSROOT_FLAGS[@]}" \
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
    -lpthread -ldl -lrt -lasound \
    -lX11 -lXext -lXinerama -lXcursor -lXrandr -lXcomposite -lXdamage \
    -lGL -lfreetype -lfontconfig \
    -lz -lexpat \
    -o "$BUILD_DIR/VREngine"

echo "Built: $BUILD_DIR/VREngine"

if [[ -n "$DEPLOY_HOST" ]]; then
    echo "Deploying to $DEPLOY_HOST:$DEPLOY_PATH ..."
    ssh "$DEPLOY_HOST" "mkdir -p '$DEPLOY_PATH'"
    scp "$BUILD_DIR/VREngine" "$DEPLOY_HOST:$DEPLOY_PATH/VREngine"
    echo "Deployed. Run with: ssh $DEPLOY_HOST '$DEPLOY_PATH/VREngine'"
fi

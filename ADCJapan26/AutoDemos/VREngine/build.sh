#!/usr/bin/env bash
# Cross-compile VREngine for aarch64 Ubuntu/RPi OS (RPi5) from WSL2.
#
# Prerequisites (WSL2/Ubuntu host):
#   sudo apt install cmake ninja-build gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
#   sudo dpkg --add-architecture arm64
#   sudo apt update
#   sudo apt install \
#       libasound2-dev:arm64 \
#       libfreetype-dev:arm64 libfontconfig1-dev:arm64 \
#       libx11-dev:arm64 libxrandr-dev:arm64 libxinerama-dev:arm64 \
#       libxcursor-dev:arm64 libxext-dev:arm64 \
#       libonnxruntime-dev:arm64
#
# Usage:
#   ./build.sh             # VRENGINE_HAS_HAILO=ON   (default)
#   ./build.sh --no-hailo  # VRENGINE_HAS_HAILO=OFF  (OSC + UI only)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
AUTODEMOS_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"   # AutoDemos/ is the CMake source root
BUILD_DIR="$SCRIPT_DIR/build"
NATIVE_BUILD="$BUILD_DIR/native"
CROSS_BUILD="$BUILD_DIR/aarch64-linux"
DIST_DIR="$BUILD_DIR/dist"
TOOLCHAIN="$AUTODEMOS_DIR/cmake/toolchain-aarch64-linux.cmake"

HAILO=ON
if [[ "${1:-}" == "--no-hailo" ]]; then
    HAILO=OFF
fi

# ── Step 1: Native configure (bootstraps juceaide) ───────────────────────────
echo "=== Step 1: Native configure (bootstraps juceaide) ==="
cmake -S "$AUTODEMOS_DIR" -B "$NATIVE_BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DVRENGINE_HAS_HAILO=OFF \
    -G Ninja

JUCEAIDE_EXE="$(find "$NATIVE_BUILD/JUCE" -name "juceaide" -type f 2>/dev/null | head -1)"
if [[ -z "$JUCEAIDE_EXE" || ! -x "$JUCEAIDE_EXE" ]]; then
    echo "ERROR: juceaide binary not found under $NATIVE_BUILD/JUCE"
    exit 1
fi
echo "  juceaide: $JUCEAIDE_EXE"

# ── Step 2: Cross-compile VREngine for aarch64 ──────────────────────────────
echo ""
echo "=== Step 2: Cross-compile VREngine for aarch64 (VRENGINE_HAS_HAILO=$HAILO) ==="
cmake -S "$AUTODEMOS_DIR" -B "$CROSS_BUILD" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DJUCE_JUCEAIDE_PATH="$JUCEAIDE_EXE" \
    -DVRENGINE_HAS_HAILO="$HAILO" \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja
cmake --build "$CROSS_BUILD" --target VREngine

# ── Step 3: Collect binary and models into dist/ ─────────────────────────────
mkdir -p "$DIST_DIR"
echo ""
echo "=== Collecting ==="

src="$(find "$CROSS_BUILD" -type f -name "VREngine" ! -path "*/_deps/*" 2>/dev/null | head -1)"
if [[ -n "$src" ]]; then
    cp "$src" "$DIST_DIR/VREngine"
    echo "  → dist/VREngine"
else
    echo "  WARNING: VREngine binary not found in $CROSS_BUILD"
fi

HEF_SRC="$(find "$CROSS_BUILD" -type d -name "hailo10h" ! -path "*/_deps/*" 2>/dev/null | head -1)"
if [[ -n "$HEF_SRC" && -d "$HEF_SRC" ]]; then
    mkdir -p "$DIST_DIR/models/hailo10h"
    if cp "$HEF_SRC/"*.hef "$DIST_DIR/models/hailo10h/" 2>/dev/null; then
        echo "  → dist/models/hailo10h/*.hef"
    fi
fi

echo ""
echo "=== Done ==="
echo "  Run:  python deploy.py --target-ip <pi-ip>"

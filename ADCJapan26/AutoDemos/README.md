# AutoDemos

Touchscreen demo apps for Raspberry Pi 5 with Hailo-10H NPU, built for the ADC Japan 2026 presentation.

| Binary | Role |
|---|---|
| `DataBridge` | VST3 host — bridges SurgeXT OSC output to MIDI/OSC for connected devices |
| `VREngine` | Touchscreen voice-command UI — Hailo Whisper ASR + Silero VAD |

---

## Building VREngine for RPi5 (cross-compile from x86_64 Ubuntu / WSL2)

### 1. Configure apt sources

See the GENISYS README for the full Ubuntu multiarch apt setup (amd64 pinned to `archive.ubuntu.com`, arm64 to `ports.ubuntu.com`).

### 2. Install cross toolchain and target libraries

```bash
sudo apt update
sudo apt install cmake ninja-build gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

sudo apt install \
    libasound2-dev:arm64 \
    libfreetype-dev:arm64 libfontconfig1-dev:arm64 \
    libx11-dev:arm64 libxrandr-dev:arm64 libxinerama-dev:arm64 \
    libxcursor-dev:arm64 libxext-dev:arm64
```

ONNX Runtime (required for Silero VAD in the Hailo build) is fetched automatically at CMake configure time as a pre-built static library from [csukuangfj/onnxruntime-libs](https://github.com/csukuangfj/onnxruntime-libs). No separate apt install is needed. To use a manually installed build instead, pass `-DONNXRUNTIME_LIB_DIR=/path/to/onnxruntime` to CMake.

### 3. Build

```bash
cd ADCJapan26/AutoDemos
./VREngine/build.sh             # default: VRENGINE_HAS_HAILO=ON
./VREngine/build.sh --no-hailo  # OSC + UI only, no HailoRT SDK required
```

Binary collected into `VREngine/build/dist/`.

### 4. Deploy to RPi5

```bash
python VREngine/deploy.py --target-ip 192.168.1.100
```

---

## Running

```bash
# VREngine — touchscreen voice-command UI
~/vrengine/VREngine [--model tiny|base|small]
```

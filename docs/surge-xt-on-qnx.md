# Building and running Surge XT on QNX

End-to-end recipe for cross-compiling Surge XT to a QNX aarch64 target from a
Windows host, getting it onto a Raspberry Pi, and driving it. Verified
2026-09-01 on a Pi 5 running QNX 8.0, with sound out of a HiFiBerry DAC8x.

Linux/WSL hosts work too - the presets carry both - but everything below is the
Windows path, which needs no WSL at all.

---

## Prerequisites

| | |
|---|---|
| QNX SDP 8.0 | `%USERPROFILE%\qnx800`, licence in `%USERPROFILE%\.qnx` |
| Visual Studio 2022 Build Tools | the **host** compiler - see the juceaide note below |
| Ninja | the presets expect `C:\ninja-win\ninja.exe` |
| CMake | 3.15+ |

## Use the `port/juce8` JUCE branch

Surge XT 1.4.0 **does not build against JUCE 9**. In JUCE 9 `Drawable` moved from
`juce_gui_basics` to `juce_graphics` and stopped deriving from `Component`, and
Surge treats Drawables as Components in several places:

    SurgeGUIEditor.cpp: 'class juce::Drawable' has no member named 'setBounds'
    SurgeGUIEditor.cpp: cannot convert 'juce::Drawable' to 'juce::Component&'

`port/juce8` (JUCE 8.0.15) has the same QNX work - including the threaded
software present that the rendering improvements depend on - and still has
`Drawable : public Component`, so Surge builds. Check you are getting the
graphics work rather than an older pin:

    git show <rev>:modules/juce_gui_basics/native/juce_Windowing_qnx.cpp | grep -c PresentThread

Expect a non-zero count. The older `qnx-ports` line predates it.

## Build

Two things trip this up, both easy once known.

**juceaide needs a compiler for the HOST, not the target.** It generates
BinaryData and icons and must run on the build machine. JUCE handles this
correctly when cross compiling - `extras/Build/juceaide/CMakeLists.txt` has a
`if(CMAKE_CROSSCOMPILING)` block that scrubs `CC`, `CXX`, `LD`, `AR` and
re-invokes CMake so the sub-configure finds the host toolchain - but it can only
find a host compiler that is actually on `PATH`. Launch the build from a
**Visual Studio developer shell**, or call `vcvars64.bat` first. From a plain
shell it fails with a misleading message:

    Failed to configure juceaide
      No CMAKE_C_COMPILER could be found.

That is a missing host compiler, not a broken port.

**HailoRT defaults ON for QNX targets** and is not usually installed for QNX, so
the configure dies in `find_package(HailoRT REQUIRED)`. Turn it off unless you
are building the voice-command bridge backend.

    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    cmake --preset qnx-surge -DSURGE_MIDI_OSC_BRIDGE_ENABLE_HAILO=OFF
    cmake --build --preset build-qnx-surge

Output: `build/surge_xt_products/SurgeXT`, ~32 MB, `ELF 64-bit LSB pie
executable, ARM aarch64, interpreter /usr/lib/ldqnx-64.so.2`.

## The target needs a working io-snd

Surge is not special here, but it is unforgiving: its **Sidechain input bus is
enabled by default**, so the standalone opens stereo *in* and stereo *out* - a
full-duplex open on a card that may only be configured for playback.

On the HiFiBerry 8x boards that needs an `io-snd` whose `bcm2712_pcm` can:

- run full duplex at all, and
- advertise a **range** of channel counts rather than one fixed count, with a
  channel map per count. A device advertising only 8 channels rejects Surge's
  stereo open outright.

The relevant config keys are `tx_voices=2:8` / `rx_voices=2:8` plus
`tx_chmap` / `rx_chmap` with one map per supported count. Without the maps,
io-snd fails the open with `No matching chmap found for N voices`.

## Deploy

The binary alone gives you the init patch and nothing else.

**The data directory is assembled from two places, and missing the second is a
silent failure.** `resources/data` holds the patches, wavetables and skins;
`resources/surge-shared` holds files Surge expects at the *root* of the data
path - `configuration.xml`, `windows.wt`, `memoryWavetable.wt`,
`paramdocumentation.xml`. `SurgeStorage` reads `datapath / "configuration.xml"`
during init, and without it Surge starts, runs, plays the init patch and accepts
OSC quite happily - but the patch browser stays empty and `/patch/load` silently
does nothing. There is no error message. Copy both:

    cd resources/data
    tar czf surge-data-core.tgz fx_presets modulator_presets tuning_library         wavetables skins patches_factory impulses_factory
    cd ../surge-shared
    tar czf surge-shared.tgz .

`resources/data` is 489 MB in full; the factory core above is about 58 MB
unpacked. The rest is third-party content you can add later by unpacking
`patches_3rdparty`, `wavetables_3rdparty` and `impulses_3rdparty` into the same
directory.

Copy both over, unpack the data anywhere writable, and point Surge at it with
`SURGE_DATA_HOME` - no system install required:

    scp build/surge_xt_products/SurgeXT       target:io-snd/bin/
    scp surge-data-core.tgz surge-shared.tgz  target:/tmp/
    ssh target 'mkdir -p ~/surge-data && cd ~/surge-data         && tar xzf /tmp/surge-data-core.tgz && tar xzf /tmp/surge-shared.tgz'

That yields 639 factory patches and 179 wavetables. Sanity check the result:
`configuration.xml` and `windows.wt` must sit at the top level beside
`patches_factory`, not nested inside it.

### Two layouts exist - pick one deliberately

`ADCJapan26/surge/deploy.py` already automates this and uses Surge's **portable
mode**: a `SurgeXTData/` directory sitting beside the binary, which
`SurgeStorage` finds by walking up from the executable.

    ~/surge/
      SurgeXT
      SurgeXTData/
        patches_factory/  wavetables/  configuration.xml  ...

`SURGE_DATA_HOME` is the alternative and overrides everything, which is handy
when the binary lives somewhere else - for example beside an `io-snd`
deployment so one `LD_LIBRARY_PATH` covers both. Both work; they are different
conventions, so do not half-apply one and then debug the other.

## Run

`LD_LIBRARY_PATH` must point at the same `libasound` the running `io-snd` was
built with, or the client library and the service disagree and every PCM open
fails with a confusing `-48 / Not supported`. Do **not** use `sudo` - it strips
`LD_LIBRARY_PATH`.

    cd ~/io-snd
    export LD_LIBRARY_PATH=$PWD/lib/dll:$PWD/lib
    export SURGE_DATA_HOME=$HOME/surge-data
    ./bin/SurgeXT &

The target needs a working Screen stack - check `screen` and a window manager
are running and `/dev/screen` exists.

## Driving it

QNX has no MIDI: io-snd's `libasound` ships sequencer symbols as ABI stubs that
return `-ENXIO`, and there is no rawmidi at all. Surge therefore takes **OSC**,
and the QNX preset sets `SURGE_STANDALONE_AUTO_OSC_IN=ON` so it listens on
startup.

    UDP 53280   OSC in
    UDP 53281   OSC out

Useful messages:

| address | args |
|---|---|
| `/mnote` | note, velocity (velocity 0 = off) |
| `/mnote/rel` | note release |
| `/fnote` | frequency, velocity |
| `/allnotesoff` | - |
| `/patch/load` | full path, without the `.fxp` |
| `/q/all_params` | query |

`extras/SurgeMidiToOscBridge` builds for the **host** (`cmake --preset
host-midi-osc-bridge`) and turns a MIDI controller into exactly these messages.
Its target IP defaults to `127.0.0.1`, so point it at the board.

Note that this puts your LAN in the control path: latency and jitter you feel
while playing are partly the network, not the QNX audio stack. Measure the audio
path separately if that distinction matters.

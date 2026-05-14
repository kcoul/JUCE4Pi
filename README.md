# JUCE4Pi
Resources for JUCE-based audio development on the Raspberry Pi

## Setup

This repository is intended to work as a template for reducing the complexity around embedded
audio development. It uses submodules for both active development repos and dormant third-party
dependencies:

- Active submodules are repos you are likely to edit directly. In this tree, the helper scripts
  classify submodules under the same GitHub owner as the root repo as active.
- Dormant submodules are vendor or third-party repos that should usually stay pinned to the
  exact commit selected by the parent repo.

After cloning, initialize submodules, then run the dry-run classifier:

```console
git submodule update --init --recursive
./scripts/manage-active-submodules.sh
```

On Windows:

```console
git submodule update --init --recursive
pwsh -NoProfile -File .\scripts\manage-active-submodules.ps1
```

The dry run prints a table showing which submodules are considered active or dormant. To apply the
active-submodule setup, run:

```console
./scripts/manage-active-submodules.sh --apply
```

On Windows:

```console
pwsh -NoProfile -File .\scripts\manage-active-submodules.ps1 -Apply
```

Apply mode attaches active submodules to a real branch when possible and installs a pre-commit guard
that refuses commits from detached HEAD. This keeps day-to-day work on forked or owned submodules
branch-based, while leaving ordinary third-party dependencies pinned.

## QNX

## Raspberry Pi OS / Ubuntu 
```console
sudo snap install cmake --classic

sudo apt update

sudo apt install ninja-build

sudo apt install libasound2-dev \
libjack-jackd2-dev ladspa-sdk \
libcurl4-openssl-dev  \
libfreetype6-dev \
libx11-dev libxcomposite-dev libxcursor-dev libxext-dev \
libxinerama-dev libxrandr-dev libxrender-dev \
libglu1-mesa-dev mesa-common-dev

sudo apt install libwebkit2gtk-4.1-dev
```

## Raspberry Pi Only

```console

sudo apt install pigpio
```

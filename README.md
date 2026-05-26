# JUCE4Pi
Resources for JUCE-based audio development on the Raspberry Pi

## Setup

This repository is intended to work as a starting point for reducing the complexity around embedded
audio development. For QNX development, Windows or Linux can work equally well as a Host OS.
For Raspberry Pi OS or Ubuntu development, native Linux or WSL2 as a Host OS is required.

## QNX (Setup Instructions coming)

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

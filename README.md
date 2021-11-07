# JUCE4Pi
Resources for JUCE-based audio development on the Raspberry Pi

```console
sudo apt update
sudo apt install cmake
sudo apt install clang
sudo apt install libasound2-dev \
libjack-jackd2-dev ladspa-sdk \
libcurl4-openssl-dev  \
libfreetype6-dev \
libx11-dev libxcomposite-dev libxcursor-dev libxext-dev \
libxinerama-dev libxrandr-dev libxrender-dev \
libwebkit2gtk-4.0-dev \
libglu1-mesa-dev mesa-common-dev
```

Raspberry Pi only

````console
sudo apt install pigpio
wget -O Findpigpio.cmake https://raw.githubusercontent.com/joan2937/pigpio/master/util/Findpigpio.cmake
sudo cp ./Findpigpio.cmake /usr/share/cmake-3.16/Modules



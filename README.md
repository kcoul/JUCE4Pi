# JUCE4Pi
Resources for JUCE-based audio development on the Raspberry Pi

Raspberry Pi / Linux

```console
git clone https://github.com/juce-framework/JUCE.git
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

````console
#Raspberry Pi

sudo apt install pigpio

#Linux

wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
cd pigpio-master
make
sudo make install

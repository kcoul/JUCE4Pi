#pragma once

//Can't seem to pick up this header without including it specifically,
//which is problematic. It makes sense to develop the resource management
//system in this CMake repo further, perhaps by centralizing it, and then
//selecting which images to include based on the target.

//It's also currently unclear how to pick out specific resources from the
//vector, i.e. by name, as is done using the Projucer's BinaryData.

#include "../../../cmake-build-debug/OpenGLPlugins/MIDIVisualizerPlus/juce_binarydata_MIDIVisualizerPlus-Resources/JuceLibraryCode/BinaryData.h"

//Represents a binary data image:
struct RawData
{
    explicit RawData(int index)
    {
        data = BinaryData::getNamedResource(BinaryData::namedResourceList[index], size);
    }

    const char* data;
    int size;
};

//Returns a vector of all existing binary data assets:
inline std::vector<RawData> getBinaryDataAssets()
{
    std::vector<RawData> assets;

    for (int index = 0; index < BinaryData::namedResourceListSize; ++index)
        assets.emplace_back(index);

    return assets;
}

//returns all binary data images as a vector:
inline std::vector<juce::Image> getBinaryDataImages()
{
    std::vector<juce::Image> images;

    for (auto& asset: getBinaryDataAssets())
    {
        auto image = juce::ImageCache::getFromMemory(asset.data, asset.size);

        if (image.isValid())
            images.emplace_back(image);
    }

    return images;
}

//This checks if a binarydata asset could be loaded as an image, but returns raw data so it is
//compatible with the BuiltInTexture CTOR
inline std::vector<RawData> getBinaryDataImagesAsRaw()
{
    std::vector<RawData> assets;

    for (auto& asset: getBinaryDataAssets())
    {
        auto image = juce::ImageCache::getFromMemory(asset.data, asset.size);

        if (image.isValid())
            assets.emplace_back(asset);
    }

    return assets;
}
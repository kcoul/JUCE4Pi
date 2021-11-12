#pragma once

#include "CommonHeader.h"

//Helper classes to automatically fetch all binary data assets

//Represents a binary data object:
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
/*
 ==============================================================================
 
 Ross Hoyt
 Textures.h
 ==============================================================================
 */

#pragma once
#include "GLUtils.h"

struct DemoTexture
{
    virtual ~DemoTexture() {}
    virtual bool applyTo (juce::OpenGLTexture&) = 0;
    
    juce::String name;
};

struct BuiltInTexture   : public DemoTexture
{
    BuiltInTexture (const char* nm, const void* imageData, size_t imageSize)
    : image (GLUtils::resizeImageToPowerOfTwo (juce::ImageFileFormat::loadFrom (imageData, imageSize)))
    {
        name = nm;
    }
    
    juce::Image image;
    
    bool applyTo (juce::OpenGLTexture& texture) override
    {
        texture.loadImage (image);
        return false;
    }
};

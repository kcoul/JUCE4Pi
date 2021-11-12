#pragma once
#include "../../stm_dsp/analyzers/stm_LevelAnalyzer.h"
#include "../../stm_dsp/utilities/stm_RollingAverage.h"
#include "../../stm_dsp/utilities/stm_RingBuffer.h"

namespace stm {

class LevelMeterSkin : public juce::Component
{
public:
    LevelMeterSkin() {}
    ~LevelMeterSkin() {}
    
    void paint (juce::Graphics& g) override
    {
//        g.fillAll (juce::Colours::grey.withAlpha(0.2f));   // background

        auto levelRect = getLocalBounds();
        auto maxedRect = levelRect;
        
        if (getHeight() > getWidth()) { // Vertical
            auto width = getWidth();
            maxedRect = levelRect.removeFromTop(width);
            
            auto height = levelRect.getHeight();
            levelRect.removeFromTop(height - height * dbScaled());
        } else { // Horizontal
            auto height = getHeight();
            maxedRect = levelRect.removeFromRight(height);
            
            auto width = levelRect.getWidth();
            levelRect.setWidth(width * dbScaled());
        }
        
        g.setColour (juce::Colours::white.withAlpha(0.7f));
        g.fillRect (levelRect);
        
        if (isMaxed) {
            g.setColour (juce::Colour::fromFloatRGBA(0.9f, 0.2f, 0.2f, 1.0f));
            g.fillRect (maxedRect);
        }
    }

    void resized() override
    {
        
    }
    
    float dbScaled() {
        auto db = juce::Decibels::gainToDecibels(level);
        return juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);
    }
    
    void setMaxed() {
        isMaxed = true;
        maxedSeconds = 1.0f;
    }
    
    void timePassed(float seconds) {
        if (maxedSeconds > 0.0f) {
            maxedSeconds -= seconds;
        } else {
            isMaxed = false;
        }
    }
    
    float level = 0.5f;
    
private:
    bool isMaxed = false;
    float maxedSeconds = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeterSkin)
};




class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter(RingBuffer& _buffer) : buffer(_buffer){
        prepare(buffer.processSpec);
        startTimerHz(60); //Used for animated components
    }
    
    void paint (juce::Graphics& g) override {}
    
    void resized() override {
        auto bounds = getLocalBounds();
        if (getWidth() > getHeight()){
            // Horizontal
            auto height = getHeight() / meters.size();
            for (auto meter : meters) {
                meter->setBounds(bounds.removeFromTop(height));
            }
        } else {
            // Vertical
            auto width = getWidth() / meters.size();
            for (auto meter : meters) {
                meter->setBounds(bounds.removeFromLeft(width));
            }
        }
    }
        
    void reset() {
        for (auto& level : levels){
            level.reset();
        }
    }
    
private:
    
    RingBuffer& buffer;
    juce::OwnedArray<LevelMeterSkin> meters;
    std::vector<RollingAverageBiased> levels;
    double sampleRate;
    
    void prepare (juce::dsp::ProcessSpec& spec) {
        sampleRate = spec.sampleRate;
        
        for (int iChannel = 0; iChannel < spec.numChannels ; iChannel++){
            auto level = RollingAverageBiased();
            level.prepare(spec.sampleRate, 0.001f, 0.2f);
            levels.push_back(level);
            
            auto* meter = new LevelMeterSkin();
            addAndMakeVisible(meter);
            meters.add(meter);
        }
    }
    
    void timerCallback() override {
        process(buffer.popBlock());
        for (auto channel = 0 ; channel < levels.size() ; channel++) {
            meters[channel]->level = levels[channel].get();
            if (levels[channel].get() > 1.0f) {
                meters[channel]->setMaxed();
            }
            meters[channel]->repaint();
        }
    }
    
    
    void process (const juce::dsp::AudioBlock<float>& processBlock)
    {
        for (int iChannel = 0; iChannel < processBlock.getNumChannels() ; iChannel++){
            for (int iSample = 0; iSample < processBlock.getNumSamples() ; iSample++){
                auto sample = processBlock.getSample(iChannel, iSample);
                levels[iChannel].push(std::abs(sample));
            }
        }
        
        for (auto meter : meters){
            meter->timePassed(processBlock.getNumSamples() / sampleRate);
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};

} // namespace stm

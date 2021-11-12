/*
  ==============================================================================

    LushDelayEngine.h
    Created: 2 May 2020 7:31:14pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once

#include "CommonHeader.h"
#include "LushDelayLine.h"

class LushDelayEngine {
public:

    LushDelayEngine()
    {
        //state.addParameterListener(Params::idDry, this);
        //state.addParameterListener(Params::idWet, this);
        //state.addParameterListener(Params::idBypass, this);
    }
    /*
    LushDelayEngine(juce::AudioProcessorValueTreeState& s) : delayLine(s)
    {

    }
    */
    ~LushDelayEngine(){
        //state.removeParameterListener(Params::idDry, this);
        //state.removeParameterListener(Params::idWet, this);
        //state.removeParameterListener(Params::idBypass, this);
    }

    void init(float dry, float wet, bool bypass,
              float modDepth, float modRate, float delay, int taps,
              float spread, float offsetLR, float feedbackDirect,
              float feedbackCross, float lowPass, float highPass, float pan,
              float allpass)
    {
        dryWetMix.setDryDecibels(dry);
        dryWetMix.setWetDecibels(wet);
        matchedBypass.setActive(bypass);

        delayLine.init(modDepth, modRate, delay, taps,
                       spread, offsetLR, feedbackDirect,
                       feedbackCross, lowPass, highPass, pan,
                       allpass);
    }

    void prepare(const juce::dsp::ProcessSpec& spec) {
        dryWetMix.prepare(spec);
        matchedBypass.prepare(spec);
        dryBuffer.setSize(static_cast<int>(spec.numChannels),
                          static_cast<int>(spec.maximumBlockSize));
        delayLine.prepare(spec);
        
        //updateParameters();
    }
    
    void process(juce::AudioBuffer<float>& buffer) {
        auto outBlock = juce::dsp::AudioBlock<float> (buffer);
        
        //Lush is designed as a stereo delay and only processes 2 buffers.
        jassert(outBlock.getNumChannels() == 2);
        
        auto numSamples = outBlock.getNumSamples();
        auto dryBlock = juce::dsp::AudioBlock<float>(dryBuffer).getSubBlock(0, numSamples);
        dryBlock.copyFrom(outBlock);
        //outBlock.clear();
        
        delayLine.process(outBlock);
        
        dryWetMix.process(dryBlock, outBlock);
        matchedBypass.process(dryBlock, outBlock);
    }
    
    void reset() {
        dryWetMix.reset();
        matchedBypass.reset();
        delayLine.reset();
    }

    void setDry(float newValue)
    {
        dryWetMix.setDryDecibels(newValue);
    }

    void setWet(float newValue)
    {
        dryWetMix.setWetDecibels(newValue);
    }

    void setBypass(bool newValue)
    {
        matchedBypass.setActive(newValue);
    }

    void setDepth(float newValue)
    {
        delayLine.setDepth(newValue);
    }

    void setRate(float newValue)
    {
        delayLine.setRate(newValue);
    }

    void setDelay(float newValue)
    {
        delayLine.setDelay(newValue);
    }

    void setTaps(int newValue)
    {
        delayLine.setTaps(newValue);
    }

    void setSpread(float newValue)
    {
        delayLine.setSpread(newValue);
    }

    void setOffset(float newValue)
    {
        delayLine.setOffset(newValue);
    }

    void setFBDirect(float newValue)
    {
        delayLine.setFBDirect(newValue);
    }

    void setFBCross(float newValue)
    {
        delayLine.setFBCross(newValue);
    }

    void setLowPass(float newValue)
    {
        delayLine.setLowPass(newValue);
    }

    void setHighPass(float newValue)
    {
        delayLine.setHighPass(newValue);
    }

    void setPan(float newValue)
    {
        delayLine.setPan(newValue);
    }
/*
    void parameterChanged(const juce::String& parameterID, float newValue ) {
        
        if (parameterID == Params::idDry) {
            dryWetMix.setDryDecibels(newValue);
        } else if (parameterID == Params::idWet) {
            dryWetMix.setWetDecibels(newValue);
        } else if (parameterID == Params::idBypass) {
            matchedBypass.setActive(newValue > 0.5 ? true : false);
        }
    }
*/
    void analyzePosition(juce::AudioPlayHead::CurrentPositionInfo& currentPositionInfo){
        if (currentBPM != currentPositionInfo.bpm
            || currentTimeSigDenominator != currentPositionInfo.timeSigDenominator)
        {
            // The DAW has changed the duration of a note.
            // If "snap" is set, we need to update "delay".
        }
    }
    
private:
    //juce::AudioProcessorValueTreeState& state;
    stm::DryWetMix dryWetMix;
    stm::MatchedBypass matchedBypass;
    LushDelayLine delayLine;

    juce::AudioSampleBuffer dryBuffer;
    
    double currentBPM;
    int currentTimeSigDenominator;

    /*
    void updateParameters(){
        dryWetMix.setDryDecibels(*state.getRawParameterValue(Params::idDry));
        dryWetMix.setWetDecibels(*state.getRawParameterValue(Params::idWet));
        matchedBypass.setActive(*state.getRawParameterValue(Params::idBypass));
    }
    */
};

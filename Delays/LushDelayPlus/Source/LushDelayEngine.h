/*
  ==============================================================================

    LushDelayEngine.h
    Created: 2 May 2020 7:31:14pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once
#if USING_CMAKE
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "../../../Modules/stm_dsp/stm_dsp.h"
#else
#include <JuceHeader.h>
#endif
#include "Params.h"
#include "LushDelayLine.h"

class LushDelayEngine: public juce::AudioProcessorValueTreeState::Listener {
public:
    LushDelayEngine(juce::AudioProcessorValueTreeState& s) : state(s), delayLine(s)
    {
        state.addParameterListener(Params::idDry, this);
        state.addParameterListener(Params::idWet, this);
        state.addParameterListener(Params::idBypass, this);
    }
    
    ~LushDelayEngine(){
        state.removeParameterListener(Params::idDry, this);
        state.removeParameterListener(Params::idWet, this);
        state.removeParameterListener(Params::idBypass, this);
    }
    
    void prepare(const juce::dsp::ProcessSpec& spec) {
        dryWetMix.prepare(spec);
        matchedBypass.prepare(spec);
        dryBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
        delayLine.prepare(spec);
        
        updateParameters();
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
    
    void parameterChanged(const juce::String& parameterID, float newValue ) {
        
        if (parameterID == Params::idDry) {
            dryWetMix.setDryDecibels(newValue);
        } else if (parameterID == Params::idWet) {
            dryWetMix.setWetDecibels(newValue);
        } else if (parameterID == Params::idBypass) {
            matchedBypass.setActive(newValue > 0.5 ? true : false);
        }
    }
    
    void analyzePosition(juce::AudioPlayHead::CurrentPositionInfo& currentPositionInfo){
        if (currentBPM != currentPositionInfo.bpm
            || currentTimeSigDenominator != currentPositionInfo.timeSigDenominator)
        {
            // The DAW has changed the duration of a note.
            // If "snap" is set, we need to update "delay".
        }
    }
    
private:
    juce::AudioProcessorValueTreeState& state;
    stm::DryWetMix dryWetMix;
    stm::MatchedBypass matchedBypass;
    LushDelayLine delayLine;
    
    AudioSampleBuffer dryBuffer;
    
    double currentBPM;
    int currentTimeSigDenominator;
    
    void updateParameters(){
        dryWetMix.setDryDecibels(*state.getRawParameterValue(Params::idDry));
        dryWetMix.setWetDecibels(*state.getRawParameterValue(Params::idWet));
        matchedBypass.setActive(*state.getRawParameterValue(Params::idBypass));
    }
};

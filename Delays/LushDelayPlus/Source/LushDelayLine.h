/*
  ==============================================================================

    LushDelayLine.h
    Created: 3 May 2020 2:09:54am
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
#include "StereoDelayLine.h"
#include "HighLowFilter.h"

class LushDelayLine: public juce::AudioProcessorValueTreeState::Listener {
public:
    LushDelayLine(juce::AudioProcessorValueTreeState& s) : state(s), delayLine(s)
    {
        state.addParameterListener(Params::idModDepth, this);
        state.addParameterListener(Params::idModRate, this);
        state.addParameterListener(Params::idDelay, this);
        state.addParameterListener(Params::idTaps, this);
        state.addParameterListener(Params::idSpread, this);
        state.addParameterListener(Params::idOffsetLR, this);
        state.addParameterListener(Params::idFeedbackDirect, this);
        state.addParameterListener(Params::idFeedbackCross, this);
        state.addParameterListener(Params::idLowPass, this);
        state.addParameterListener(Params::idHighPass, this);
        state.addParameterListener(Params::idPan, this);
    }
    
    ~LushDelayLine() {
        state.removeParameterListener(Params::idModDepth, this);
        state.removeParameterListener(Params::idModRate, this);
        state.removeParameterListener(Params::idDelay, this);
        state.removeParameterListener(Params::idTaps, this);
        state.removeParameterListener(Params::idSpread, this);
        state.removeParameterListener(Params::idOffsetLR, this);
        state.removeParameterListener(Params::idFeedbackDirect, this);
        state.removeParameterListener(Params::idFeedbackCross, this);
        state.removeParameterListener(Params::idLowPass, this);
        state.removeParameterListener(Params::idHighPass, this);
        state.removeParameterListener(Params::idPan, this);
    }
    
    void prepare(const juce::dsp::ProcessSpec& spec) {
        modulator.prepare(spec, Params::MAX_MOD);
        highLowFilter.prepare(spec);
        delayLine.prepare(spec);
        panner.prepare();
        
        updateParameters();
    }
    
    void process(juce::dsp::AudioBlock<float>& processBlock) {
        panner.process(processBlock);
        highLowFilter.process(processBlock);
        modulator.process(processBlock);
        delayLine.process(processBlock);
    }
    
    void reset() {
        highLowFilter.reset();
        modulator.reset();
        delayLine.reset();
        panner.reset();
    }
    
    void parameterChanged(const juce::String& parameterID, float newValue ) override {
        stm::DebugDisplay::set(10, parameterID + ": " + juce::String(newValue));
        
        if (parameterID == Params::idModDepth) {
            modulator.setDepth(newValue);
        } else if (parameterID == Params::idModRate) {
            modulator.setRate(newValue);
        } else if (parameterID == Params::idDelay) {
            delayLine.setDelay(newValue);
        } else if (parameterID == Params::idTaps) {
            delayLine.setTaps((int)newValue);
        }  else if (parameterID == Params::idSpread) {
            delayLine.setSpread(newValue);
        }  else if (parameterID == Params::idOffsetLR) {
            delayLine.setOffset(newValue);
        } else if (parameterID == Params::idFeedbackDirect) {
            delayLine.setFBdirect(newValue);
        }  else if (parameterID == Params::idFeedbackCross) {
            delayLine.setFBcross(newValue);
        } else if (parameterID == Params::idLowPass) {
            highLowFilter.setLowPassFreq(newValue);
        } else if (parameterID == Params::idHighPass){
            highLowFilter.setHiPassFreq(newValue);
        } else if (parameterID == Params::idPan){
            //panner.setSinCentered(newValue);
            panner.setLinearCentered(newValue);
        }
    }
    
private:
    juce::AudioProcessorValueTreeState& state;
    //stm::BalancePanner panner;
    stm::StereoPanner panner;
    stm::FrequencyModulator modulator;
    StereoDelayLine delayLine;
    HighLowFilter highLowFilter;
    
    void updateParameters(){
        modulator.setDepth(*state.getRawParameterValue(Params::idModDepth));
        modulator.setRate(*state.getRawParameterValue(Params::idModRate));
        delayLine.setDelay(*state.getRawParameterValue(Params::idDelay));
        delayLine.setTaps((int)*state.getRawParameterValue(Params::idTaps));
        delayLine.setSpread(*state.getRawParameterValue(Params::idSpread));
        delayLine.setOffset(*state.getRawParameterValue(Params::idOffsetLR));
        delayLine.setFBdirect(*state.getRawParameterValue(Params::idFeedbackDirect));
        delayLine.setFBcross(*state.getRawParameterValue(Params::idFeedbackCross));
        highLowFilter.setLowPassFreq(*state.getRawParameterValue(Params::idLowPass));
        highLowFilter.setHiPassFreq(*state.getRawParameterValue(Params::idHighPass));
    }
};





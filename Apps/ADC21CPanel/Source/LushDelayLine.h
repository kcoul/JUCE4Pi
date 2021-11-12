/*
  ==============================================================================

    LushDelayLine.h
    Created: 3 May 2020 2:09:54am
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once
#include "CommonHeader.h"
#include "StereoDelayLine.h"
#include "HighLowFilter.h"

class LushDelayLine: public AudioProcessorValueTreeState::Listener {
public:
    LushDelayLine()
    {

    }

    void init(float modDepth, float modRate, float delay, int taps,
              float spread, float offsetLR, float feedbackDirect,
              float feedbackCross, float lowPass, float highPass, float pan,
              float allpass)
    {
        modulator.setDepth(modDepth);
        modulator.setRate(modRate);
        delayLine.setDelay(delay);
        delayLine.setTaps(taps);
        delayLine.setSpread(spread);
        delayLine.setOffset(offsetLR);
        delayLine.setFBdirect(feedbackDirect);
        delayLine.setFBcross(feedbackCross);
        highLowFilter.setLowPassFreq(lowPass);
        highLowFilter.setHiPassFreq(highPass);
        //panner.setSinCentered(pan);
        panner.setLinearCentered(pan);

        delayLine.setAllPass(allpass);
    }

    void prepare(const dsp::ProcessSpec& spec) {
        modulator.prepare(spec, Params::MAX_MOD);
        highLowFilter.prepare(spec);
        delayLine.prepare(spec);
        panner.prepare();
        
        //updateParameters();
    }
    
    void process(dsp::AudioBlock<float>& processBlock) {
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

    void setDepth(float newValue)
    {
        modulator.setDepth(newValue);
    }

    void setRate(float newValue)
    {
        modulator.setRate(newValue);
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
        delayLine.setFBdirect(newValue);
    }

    void setFBCross(float newValue)
    {
        delayLine.setFBcross(newValue);
    }

    void setLowPass(float newValue)
    {
        highLowFilter.setLowPassFreq(newValue);
    }

    void setHighPass(float newValue)
    {
        highLowFilter.setHiPassFreq(newValue);
    }

    void setPan(float newValue)
    {
        //panner.setSinCentered(newValue);
        panner.setLinearCentered(newValue);
    }

    void parameterChanged(const String& parameterID, float newValue ) override {
        stm::DebugDisplay::set(10, parameterID + ": " + String(newValue));
        
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
    //AudioProcessorValueTreeState& state;
    //stm::BalancePanner panner;
    stm::StereoPanner panner;
    stm::FrequencyModulator modulator;
    StereoDelayLine delayLine;
    HighLowFilter highLowFilter;

    /*
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
     */
};





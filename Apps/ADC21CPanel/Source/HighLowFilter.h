/*
  ==============================================================================

    HighLowFilter.h
    Created: 24 Apr 2020 5:24:04pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once
#include "CommonHeader.h"


class HighLowFilter {
public:
    
    HighLowFilter()
    {
        
    }
    
    void prepare (const dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        auto& lowPassFilter = processorChain.get<lowPassIndex>();
        lowPassFilter.setType(dsp::StateVariableTPTFilterType::lowpass);
        lowPassFilter.setResonance(static_cast<float>(1 / sqrt(2)));
        lowPassFilter.setCutoffFrequency(20000);
        auto& hiPassFilter = processorChain.get<hiPassIndex>();
        hiPassFilter.setType(dsp::StateVariableTPTFilterType::highpass);
        hiPassFilter.setResonance(static_cast<float>(1 / sqrt(2)));
        hiPassFilter.setCutoffFrequency(20);
        processorChain.prepare(spec);
    }
    
    void process (dsp::AudioBlock<float>& processBlock)
    {
        processorChain.process(dsp::ProcessContextReplacing<float>(processBlock));
    }
    
    void reset ()
    {
        processorChain.reset();
    }

    void setLowPassFreq(float newFreq){
        auto& lowPassFilter = processorChain.get<lowPassIndex>();
        lowPassFilter.setCutoffFrequency(newFreq);
    }

    void setHiPassFreq(float newFreq){
        auto& hiPassFilter = processorChain.get<hiPassIndex>();
        hiPassFilter.setCutoffFrequency(newFreq);
    }
    
private:
    //float Q = 1.0f;
    
    double sampleRate = 44000.0;
    
    enum {
        hiPassIndex,
        lowPassIndex
    };
    
    using Filter = dsp::StateVariableTPTFilter<float>;
    dsp::ProcessorChain<Filter, Filter> processorChain;
};

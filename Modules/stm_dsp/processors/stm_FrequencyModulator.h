#pragma once
#include "../utilities/stm_RecircBuffer.h"
#include "../utilities/stm_Oscillator.h"

using namespace juce;

namespace stm {

/**
 Delays a signal by the given maxDepth, so that the phase can be modulated by that amount.
 */
class FrequencyModulator {
    
public:
    /**Don't forget to call setDepth and setRate before use as well!*/
    void prepare (const dsp::ProcessSpec& spec, float maxDepthMS)
    {
        sampleRate = static_cast<float>(spec.sampleRate);
        float maxDelaySamples = maxDepthMS / 1000.0f * (float)sampleRate;
        offsetSamples = maxDelaySamples + 1.0f;
        int bufferSamples = int(maxDelaySamples) * 2 + 3;
        
        oscillator.prepare(sampleRate);
        buffer.prepare( bufferSamples, static_cast<int>(spec.numChannels));
    }
    
    void process (const dsp::AudioBlock<float>& processBlock)
    {
        auto numSamples = processBlock.getNumSamples();
        auto numChannels = processBlock.getNumChannels();
        
        for (auto sampleIndex = 0 ; sampleIndex < (int)numSamples ; sampleIndex++)
        {
            float delaySamples = oscillator.getNext() * amplitudeSamples + offsetSamples;
            
            for (auto channelIndex = 0 ; channelIndex < (int)numChannels ; channelIndex++)
            {
                buffer.push( processBlock.getSample(channelIndex, sampleIndex), channelIndex );
                processBlock.setSample( channelIndex, sampleIndex, buffer.getInterSample(delaySamples, channelIndex) );
            }
            
            buffer.increment();
        }
    }
    
    void reset ()
    {
        oscillator.reset();
        buffer.reset();
    }
    
    void setDepth (float ms)
    {
        amplitudeSamples = ms / 1000.0f * sampleRate;
        
        //jassert(amplitudeSamples < offsetSamples);
    }
    
    void setRate (float hz)
    {
        oscillator.setFrequency(hz);
    }
    
private:
    float offsetSamples = 0.0f;
    float amplitudeSamples = 0.0f;
    float sampleRate = 48000.0f;
    Oscillator<float> oscillator;
    RecircBuffer buffer;
};



} // namespace stm

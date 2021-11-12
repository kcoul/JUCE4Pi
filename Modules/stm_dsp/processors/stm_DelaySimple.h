#pragma once
#include "../utilities/stm_RecircBuffer.h"

namespace stm {

class DelaySimple : public dsp::ProcessorBase
{
public:
    void prepare (const dsp::ProcessSpec& spec) override {
        sampleRate = spec.sampleRate;
        //spec.maximumBlockSize;
        delayBuffer.prepare(bufferSamples, static_cast<int>(spec.numChannels));
    }
    
    void process (const dsp::ProcessContextReplacing<float>& context) override
    {
        auto& inBlock = context.getInputBlock();
        auto& outBlock = context.getOutputBlock();
        
        for (auto sampleIndex = 0 ; sampleIndex < (int)inBlock.getNumSamples() ; sampleIndex++)
        {
            for (auto channel = 0 ; channel < (int)inBlock.getNumChannels() ; channel++)
            {
                float inSample = inBlock.getSample(channel, sampleIndex);
                delayBuffer.push(inSample, channel);
                
                float outSample = delayBuffer.getSample(delaySamples, channel);
                outBlock.setSample(channel, sampleIndex, outSample);
            }
            
            delayBuffer.increment();
        }
    }
    
    void reset() override {
        delayBuffer.reset();
    }
    
    void setDelaySamples(int samples){
        delaySamples = samples;
    }
    
    void setBufferSamples(int samples){
        bufferSamples = samples;
    }
    
    void setDelaySeconds(float seconds){
        delaySamples = int ( float(sampleRate) * seconds );
    }

private:
    double sampleRate = 0.0;
    int delaySamples = 0;
    int bufferSamples = 1000;
    RecircBuffer delayBuffer;
};

} // namespace stm

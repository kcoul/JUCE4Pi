#pragma once
#include "../utilities/stm_Rampers.h"
#include "../utilities/stm_Measuring.h"
#include "../../stm_gui/displays/stm_DebugDisplay.h"
#include "../utilities/stm_RollingAverage.h"

using namespace juce;

namespace stm {

class MatchedBypass
{
public:
    void setActive (bool isActive){
        isBypassActive = isActive;
    }
    
    void prepare (const dsp::ProcessSpec& spec)
    {
        rollingWetRMS.prepare(spec.sampleRate, 0.5f);
        rollingDryRMS.prepare(spec.sampleRate, 0.5f);
    }
    
    /**
     The MatchedBypass processor only works with a ProcessContextNonReplacing.
     Using this processor with ProcessContextReplacing doesn't make sense.
     Because this processor expects separate inBlock and outBlock blocks.
     */
    void process (const dsp::ProcessContextReplacing<float>& context, const dsp::AudioBlock<float>& dryBlock)
    {
        auto& wetBlock = context.getOutputBlock();
        
        auto numSamples = wetBlock.getNumSamples();
        auto numChannels = wetBlock.getNumChannels();
        
        jassert (dryBlock.getNumSamples() == numSamples);
        
        for (int sampleIndex=0 ; sampleIndex<(int)numSamples ; sampleIndex++)
        {
            float gain = 1.0f;
            if (rollingDryRMS.get() > 0.0f) {
                gain = rollingWetRMS.get() / rollingDryRMS.get();
            }
            
            for (int channel=0 ; channel<(int)numChannels ; channel++)
            {
                float drySample = dryBlock.getSample(channel, sampleIndex);
                rollingDryRMS.push(drySample);
                rollingWetRMS.push(wetBlock.getSample(channel, sampleIndex));
                
                if (isBypassActive) {
                    wetBlock.setSample(channel, sampleIndex, drySample * gain);
                }
            }
        }
    }
    
    /**
     The MatchedBypass processor only works with a ProcessContextNonReplacing.
     Using this processor with ProcessContextReplacing doesn't make sense.
     Because this processor expects separate inBlock and outBlock blocks.
     */
    void process (const dsp::AudioBlock<float>& dryBlock, const dsp::AudioBlock<float>& wetBlock)
    {
        auto numSamples = wetBlock.getNumSamples();
        auto numChannels = wetBlock.getNumChannels();
        
        jassert (dryBlock.getNumSamples() == numSamples);
        
        for (int sampleIndex=0 ; sampleIndex<(int)numSamples ; sampleIndex++)
        {
            float gain = 1.0f;
            if (rollingDryRMS.get() > 0.0f) {
                gain = rollingWetRMS.get() / rollingDryRMS.get();
            }
            
            for (int channel=0 ; channel<(int)numChannels ; channel++)
            {
                float drySample = dryBlock.getSample(channel, sampleIndex);
                rollingDryRMS.push(drySample);
                rollingWetRMS.push(wetBlock.getSample(channel, sampleIndex));
                
                if (isBypassActive) {
                    wetBlock.setSample(channel, sampleIndex, drySample * gain);
                }
            }
        }
    }
    
    void reset()
    {
        rollingWetRMS.reset();
        rollingDryRMS.reset();
    }
    
private:
    bool isBypassActive = false;
    
    RollingRMS rollingWetRMS, rollingDryRMS;
};


} // namespace stm

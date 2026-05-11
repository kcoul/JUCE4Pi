#pragma once

using namespace juce;

namespace stm {

/**
 Assumes context's InputBlock = dry. OutputBlock = wet.
 Mixes the two with the supplied Dry and Wet gain.
 Gain values are ramped.
 */
class DryWetMix
{
public:
    
    void setDryLinear (float gain){
        dryGainRamper.updateTarget(gain);
    }
    void setDryDecibels (float dB){
        dryGainRamper.updateTarget(Decibels::decibelsToGain(dB));
    }
    void setWetLinear (float gain){
        wetGainRamper.updateTarget(gain);
    }
    void setWetDecibels (float dB){
        wetGainRamper.updateTarget(Decibels::decibelsToGain(dB));
    }
    
    void prepare (const dsp::ProcessSpec& /*spec*/) {
        dryGainRamper.prepare(1.0f, 0.001f);
        wetGainRamper.prepare(1.0f, 0.001f);
    }
    
    /**
     The dry/wet mix processor only works with a ProcessContextNonReplacing.
     Using this processor with ProcessContextReplacing doesn't make sense.
     */
    void process (const dsp::ProcessContextReplacing<float>& context, const dsp::AudioBlock<float>& dryBlock)
    {
        auto& inBlock = context.getInputBlock();
        auto& outBlock = context.getOutputBlock();
        
        jassert (dryBlock.getNumSamples() == inBlock.getNumSamples());
        
        for (auto sample = 0 ; sample < (int)inBlock.getNumSamples() ; sample++)
        {
            float dryGain = dryGainRamper.getNext();
            float wetGain = wetGainRamper.getNext();
            
            for (auto channel = 0 ; channel < (int)inBlock.getNumChannels() ; channel++)
            {
                float drySample = dryBlock.getSample(channel, sample);
                float wetSample = outBlock.getSample(channel, sample);
                float outputSample = drySample * dryGain + wetSample * wetGain;
                outBlock.setSample(channel, sample, outputSample);
            }
        }
    }
    
    /**
     The dry/wet mix processor only works with a ProcessContextNonReplacing.
     Using this processor with ProcessContextReplacing doesn't make sense.
     */
    void process (const dsp::AudioBlock<float>& dryBlock, const dsp::AudioBlock<float>& outBlock)
    {
        jassert (dryBlock.getNumSamples() == outBlock.getNumSamples());
        
        for (auto sample = 0 ; sample < (int)dryBlock.getNumSamples() ; sample++)
        {
            float dryGain = dryGainRamper.getNext();
            float wetGain = wetGainRamper.getNext();
            
            for (auto channel = 0 ; channel < (int)dryBlock.getNumChannels() ; channel++)
            {
                float drySample = dryBlock.getSample(channel, sample);
                float wetSample = outBlock.getSample(channel, sample);
                float outputSample = drySample * dryGain + wetSample * wetGain;
                outBlock.setSample(channel, sample, outputSample);
            }
        }
    }
    
    void reset() {
        dryGainRamper.reset();
        wetGainRamper.reset();
    }
    
private:
    RamperLinear dryGainRamper, wetGainRamper;
};

} // namespace stm

#pragma once

#include "../utilities/stm_Rampers.h"

namespace stm {

/**
 An abstract base class for saturators
 */
class SaturatorBase : dsp::ProcessorBase
{
public:
    
    virtual float saturateSample (float sample)=0;
    
    virtual void prepare (const dsp::ProcessSpec& /*spec*/) override {}
    
    void process (const dsp::ProcessContextReplacing<float>& context) override
    {
        auto& inBlock = context.getInputBlock();
        auto& outBlock = context.getOutputBlock();
        
        for (auto sample = 0 ; sample < (int)inBlock.getNumSamples() ; sample++)
        {
            for (auto channel = 0 ; channel < (int)inBlock.getNumChannels() ; channel++)
            {
                float saturatedSample = saturateSample(inBlock.getSample(channel, sample));
                
                outBlock.setSample(channel, sample, saturatedSample);
            }
        }
    }
    
    void reset() override {}
    
private:
    RamperLinear amountRamper;
};

/**
 A simple hyperbolic tangent saturator
 */
class SaturatorTanH : public SaturatorBase
{
public:
    
    float saturateSample (float sample) override
    {
        return tanh(sample);
    }
};

class SaturatorCrispyEven : dsp::ProcessorBase
{
    
};

/** The harmonic saturation engine that crispy is based on */
class SaturatorCrispy : dsp::ProcessorBase
{
public:
    SaturatorCrispy() {
        oddPowerRamper.prepare(0.0f, 0.1f);
        evenMixRamper.prepare(0.0f, 0.01f);
    }
    
    void prepare (const dsp::ProcessSpec& /*spec*/) override {
        
    }
    
    /** Set Even from 0.0 to 1.0 */
    void setEven(float even){
        evenMix = even;
        evenMixRamper.updateTarget(evenMix);
    }
    
    /** Set Odd from 0.0 to 1.0 */
    void setOdd(float odd){
        float power = odd * 2.0f;
        power += 1.0f; //Odd is now from 1.0 to 3.0
        power = power * power * power * power; //Odd is not from 1 to 76
        
        oddPower = power;
        oddPowerRamper.updateTarget(oddPower);
    }
    
    /**
    Sets the even and odd based on an odd/even mix between 0.0 and 1.0
    and an overall saturation between 0.0 and 1.0
    */
    void setMix(float oddEvenMix, float saturation){
        float odd = 1.0f - oddEvenMix;
        float even = oddEvenMix;

        if (oddEvenMix < 0.5){
            //odd = 1.0f;
            even = oddEvenMix / 0.5f;
        } else {
            even = 1.0f;
            //odd = (1.0f - oddEvenMix) / 0.5f;
        }

        odd *= saturation;
        even *= saturation;
        
        setEven(even);
        setOdd(odd);
    }
    
    float saturateOdd(float sample, float power){
        if (sample > 0) {
            return 1.0f - powf(1.0f-sample, power);
        } else {
            return -(1.0f - powf(1.0f+sample, power));
        }
    }
    
    float saturateEven(float sample, float mix){
        float wet = sample * sample;
        return wet * mix + sample * (1.0f - mix);
    }
    
    float saturateEvenOffset(float sample){
        float wet = sample * sample;
        wet -= 0.5f;
        wet *= 2.0f;
        return wet * evenMix + sample * (1.0f - evenMix);
    }
    
    /** Sample-by-sample saturation provides no ramping */
    float saturateSample(float sample)
    {
        sample = jlimit(-1.0f, 1.0f, sample);
        //sample = saturateOdd(sample, oddPower);
        sample = saturateEven(sample, evenMix);
        sample = saturateOdd(sample, oddPower);
        return sample;
    }
    
    /** The process function provides ramping */
    void process (const dsp::ProcessContextReplacing<float>& context) override
    {
        auto& inBlock = context.getInputBlock();
        auto& outBlock = context.getOutputBlock();
        
        for (auto sampleIndex = 0 ; sampleIndex < (int)inBlock.getNumSamples() ; sampleIndex++)
        {
            oddPower = oddPowerRamper.getNext();
            evenMix = evenMixRamper.getNext();
            
            for (auto channel = 0 ; channel < (int)inBlock.getNumChannels() ; channel++)
            {
                float sample = inBlock.getSample(channel, sampleIndex);
                
                sample = saturateSample(sample);
                
                outBlock.setSample(channel, sampleIndex, sample);
            }
        }
    }
    
    void reset() override {
        oddPowerRamper.reset();
        evenMixRamper.reset();
    }
    
private:
    RamperLinear oddPowerRamper;
    RamperLinear evenMixRamper;
    float oddPower;
    float evenMix;
};

} //namespace stm

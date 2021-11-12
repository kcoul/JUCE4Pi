/*
  ==============================================================================

    OscillatingPosition.cpp
    Created: 20 Nov 2019 1:00:22pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once

template <typename SampleType>
class Oscillator {
    
public:
    Oscillator(){
        
    }
    
    ~Oscillator(){
        
    }
    
    void prepare(SampleType sr)
    {
        sampleRate = sr;
    }
    
    void setFrequency(SampleType frequency){
        SampleType radsPerOscillation = MathConstants<SampleType>::twoPi;
        // frequency is effectively oscillations/second
        SampleType radsPerSecond = radsPerOscillation * frequency;
        // sampleRate is effectively samples/second
        radsPerSample = radsPerSecond / sampleRate;
    }
    
    void reset(){
        pos = 0.0;
    }
    
    SampleType getNext()
    {
        SampleType result = std::sin(pos);
        
        increment();
        
        return result;
    }
    
private:
    SampleType sampleRate = 48000.0f;
    SampleType radsPerSample = 0.0f;
    SampleType pos = 0.0f; //Current oscillator position, in radians.
    
    void increment()
    {
        pos += radsPerSample;
        
        if (pos > MathConstants<SampleType>::twoPi)
        {
            pos -= MathConstants<SampleType>::twoPi;
        }
    }
};

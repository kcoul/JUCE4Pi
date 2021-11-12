#pragma once
#include "../../stm_gui/displays/stm_DebugDisplay.h"

using namespace juce;

namespace stm {

class RecircBuffer {
public:
    AudioBuffer<float> buffer;
    
    void prepare(int numSamples, int numChannels = 1){
        buffer.setSize(numChannels, numSamples);
        reset();
    }
    
    void reset(){
        buffer.clear();
        index = 0;
    }
    
    /**
     Adds a sample to the buffer at the current index.
     Be sure to increment() after filling all channels to update the index.
    */
    void push(float sample, int channel = 0){
        buffer.setSample(channel, index, sample);
    }
    
    /**
     Linearly interpolates between samples to return non-integer sample delays.
     Be sure to get samples after push() and before increment()
     Otherwise, getSample(0) will return the oldest sample in the buffer.
     */
    float getInterSample(float delaySamples, int channel = 0){
        int delay0 = int (delaySamples);
        float frac = delaySamples - float (delay0);
        float y0 = getSample(delay0, channel);
        float y1 = getSample(delay0 + 1, channel);
        float rise = y1 - y0;
        return y0 + rise * frac;
    }
    
    /**
     Returns a sample at the current index.
     Be sure to get samples after push() and before increment()
     Otherwise, getSample(0) will return the oldest sample in the buffer.
     */
    float getSample(int delaySamples, int channel = 0){
        
        int delayIndex = index - delaySamples;
        if (delayIndex < 0){
            delayIndex = buffer.getNumSamples() + delayIndex;
        }
        return buffer.getSample(channel, delayIndex);
    }
    
    void increment(){
        index += 1;
        index %= buffer.getNumSamples();
    }
    
private:
    int index = 0;
};

} // namespace stm

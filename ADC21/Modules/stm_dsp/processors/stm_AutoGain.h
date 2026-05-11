#pragma once

#include "../../stm_gui/displays/stm_DebugDisplay.h"
#include "../utilities/stm_RollingAverage.h"

namespace stm {

/**
 Maintains a record of the highest value from the last "sustain" number of pushes.
 This ended up being much more involved than I expected.
 */
class MaxTracker
{
public:
    
    void prepare(int _sustain) {
        sustain = _sustain;
        regionSustain = sustain / numRegions;
        reset();
    }
    
    void reset() {
        max = 0.0f;
        maxCount = 0;
        regionMax = 0.0f;
        regionMaxCount = 0;
        
        regionMaxs.clear();
        for (int i=0 ; i<numRegions ; i++){
            regionMaxs.push_back(0.0f);
        }
    }
    
    float push(float val){
        //Update the current max
        if (val > max) {
            max = val;
            maxCount = 0;
        }
        else if (maxCount > sustain) {
            max = 0.0f;
            maxCount = 0;
            for (int i=0 ; i < (int)regionMaxs.size() ; i++){
                if (regionMaxs[static_cast<unsigned long>(i)] > max) {
                    max = regionMaxs[static_cast<unsigned long>(i)];
                    maxCount = i * regionSustain;
                }
            }
        }
        
        //Update region max
        if (val > regionMax) {
            regionMax = val;
        }
        
        //Update region count
        if (regionMaxCount > regionSustain){
            regionMaxs.push_front(regionMax);
            regionMaxs.pop_back();
            regionMax = 0.0f;
            regionMaxCount = 0;
        }
        
        maxCount += 1;
        regionMaxCount += 1;
        
        return max;
    }
    
private:
    float max = 0.0f;
    int maxCount = 0;
    int sustain = 0;
    
    const int numRegions = 5;
    float regionMax = 0.0f;
    int regionMaxCount = 0;
    int regionSustain = 0;
    
    std::deque<float> regionMaxs = {};
};

/**
 Automatically adds gain to the signal to bring signal ceiling to the target level.
 Then provides a method to remove the added gain from each sample.
 Note that the process class introduces tanh distortion.
*/
class AutoGain
{
public:
    
    void setAmount (float newAmount) {
        amount = newAmount;
    }
    
    void prepare (const dsp::ProcessSpec& spec)
    {
        float sustainSamples = static_cast<float>(spec.sampleRate * sustain / 1000.0f);
        levelTracker.prepare(static_cast<int>(sustainSamples));
        gainValues.setSize(static_cast<int>(spec.numChannels),
                           static_cast<int>(spec.maximumBlockSize));
        reset();
    }
    
    void reset ()
    {
        currentLevel = 0.0f;
        currentGain = 1.0f;
        levelTracker.reset();
        gainValues.clear();
    }
    
    /**
     Automatically adds gain to the signal to bring peaks to the target level.
     See parameters below.
     Channel-Linked to avoid stereo weirdness.
     */
    void process(const dsp::ProcessContextReplacing<float>& context)
    {
        auto& inBlock = context.getInputBlock();
        auto& outBlock = context.getOutputBlock();
        
        stm::DebugDisplay::set(1, "currentLevel: " + String(currentLevel));
        stm::DebugDisplay::set(2, "currentGain: " + String(currentGain));
        
        for (auto sampleIndex=0 ; sampleIndex<(int)inBlock.getNumSamples() ; sampleIndex++)
        {
            //Find the maximum sample level of all channels
            float channelMax = 0.0f;
            for (auto channelIndex=0 ; channelIndex<(int)inBlock.getNumChannels() ; channelIndex++)
            {
                float level = abs(inBlock.getSample(channelIndex, sampleIndex));
                
                if (level > channelMax){
                    channelMax = level;
                }
            }
            
            //Determine the current level of the signal
            //Adjust gain accordingly.
            float maxLevel = levelTracker.push(channelMax);
            updateLevel(maxLevel);
            updateGain();
                
            //Save the gain value in a buffer so it can be removed later in processRemoveGain()
            gainValues.setSample(0, sampleIndex, currentGain);
            
            //Apply gain to all channels.
            for (auto channelIndex=0 ; channelIndex<(int)inBlock.getNumChannels() ; channelIndex++)
            {
                float sample = inBlock.getSample(channelIndex, sampleIndex);
                
                sample = sample * currentGain;
                // This saturation algorithm is applied after gain
                // To prevent sudden peaks from breaching +/- 1.0
                // without the sharp, aliasing effect of clipping
                sample = saturator.saturateSample(sample);
                outBlock.setSample(channelIndex, sampleIndex, sample);
            }
        }
    }
    
    /**
     This uses the gainValues buffer created in process() to remove added gain by sample.
     */
    void processRemoveGain(const dsp::ProcessContextReplacing<float>& context)
    {
        juce::dsp::AudioBlock<const float> inBlock = context.getInputBlock();
        juce::dsp::AudioBlock<float> outBlock = context.getOutputBlock();
        
        for (auto sampleIndex=0 ; sampleIndex<(int)inBlock.getNumSamples() ; sampleIndex++)
        {
            float sampleGain = gainValues.getSample(0, sampleIndex);
            
            for (auto channelIndex=0 ; channelIndex<(int)inBlock.getNumChannels() ; channelIndex++)
            {
                float sample = inBlock.getSample(channelIndex, sampleIndex);
                outBlock.setSample(channelIndex, sampleIndex, sample / sampleGain );
            }
        }
    }

private:
    
    //--------------     USER PARAMETERS     ---------------
    
    //Should be between 0.0 and 1.0
    float amount = 0.0f;
    
    
    //------------     CONSTANT PARAMETERS     -------------
    
    //The minimum gain used on the incoming signal
    //Dropping this below 1.0 means that the incoming signal may be reduced.
    const float minGain = 0.5f;
    
    //The max gain used on the incoming signal
    const float maxGain = 100.0f;
    
    //The desired signal ceiling (between 0.0 and 1.0)
    //Gain is adjusted to bring the incoming signal's peaks to this level.
    const float targetLevel = 0.8f;
    
    //Time in ms that the currentLevel is maintained after a peak.
    const float sustain = 50.0f; // ms
    
    //Gain increases slowly to bring signal level to target
    //Similar to release on a compressor.
    const float rateOfIncrease = 0.01f; // gain/ms
    
    //Gain decreases quickly when signal level goes over target
    //Similar to attack on a compressor.
    const float rateOfDecrease = -1.0f; // gain/ms
    
    
    //--------------    INTERNAL VARIABLES     -------------
    float currentLevel = 0.0f;
    float currentGain = 1.0f;
    float targetGain = 1.0f;
    
    MaxTracker levelTracker;
    
    AudioSampleBuffer gainValues;
    stm::SaturatorTanH saturator;
    
    
    //--------------     HELPER FUNCTIONS     --------------
    void updateLevel(float maxLevel){
        if (!approximatelyEqual(currentLevel, maxLevel))
        {
            currentLevel = maxLevel;
            targetGain = targetLevel / currentLevel;
            targetGain = jlimit(minGain, maxGain, targetGain);
            targetGain = (targetGain - 1.0f) * amount + 1.0f;
        }
    }
    
    void updateGain(){
        if (!approximatelyEqual(targetGain, currentGain))
        {
            float move = targetGain - currentGain;
            
            if (move < rateOfDecrease)
            {
                currentGain += rateOfDecrease;
            } else if (move > rateOfIncrease)
            {
                currentGain += rateOfIncrease;
            } else {
                currentGain = targetGain;
            }
        }
    }

};

} // namespace stm


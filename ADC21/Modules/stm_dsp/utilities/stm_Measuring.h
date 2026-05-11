#pragma once

using namespace juce;

namespace stm {


class Measuring {
    
public:
    
    /** Return the root mean squared level for a region of all channels in an audioblock. */
    template<typename SampleType>
    static inline SampleType getRMSLevel (const dsp::AudioBlock<const SampleType>& block, int startSample, int numSamples) noexcept
    {
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= block.getNumSamples());
        
        if (numSamples <= 0 || block.getNumSamples() == 0 || block.getNumChannels() == 0)
            return SampleType(0);
        
        double sum = 0.0;
        
        for (int channelIndex=0 ; channelIndex<block.getNumChannels() ; channelIndex++)
        {
            auto channelRMS = getRMSLevel(block, channelIndex, startSample, numSamples);
            sum += channelRMS * channelRMS;
        }
        
        return static_cast<SampleType> (std::sqrt ( sum / block.getNumChannels() ));
    }
    
    /** Return the root mean squared level for a region of all channels in an audioblock. */
    template<typename SampleType>
    static inline SampleType getRMSLevel (const dsp::AudioBlock<SampleType>& block, int startSample, int numSamples) noexcept
    {
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= block.getNumSamples());
        
        if (numSamples <= 0 || block.getNumSamples() == 0 || block.getNumChannels() == 0)
            return SampleType(0);
        
        double sum = 0.0;
        
        for (int channelIndex=0 ; channelIndex<block.getNumChannels() ; channelIndex++)
        {
            auto channelRMS = getRMSLevel(block, channelIndex, startSample, numSamples);
            sum += channelRMS * channelRMS;
        }
        
        return static_cast<SampleType> (std::sqrt ( sum / block.getNumChannels() ));
    }
    
    /** Returns the root mean squared level for a region of a channel in an audioblock. */
    template<typename SampleType>
    static inline SampleType getRMSLevel (const dsp::AudioBlock<const SampleType>& block, int channel, int startSample, int numSamples) noexcept
    {
        jassert (isPositiveAndBelow (channel, block.getNumChannels()));
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= block.getNumSamples());

        if (numSamples <= 0 || channel < 0 || channel >= block.getNumChannels() || block.getNumSamples() == 0)
            return SampleType(0);

        auto* data = block.getChannelPointer(channel);
        double sum = 0.0;

        for (int i = startSample; i < numSamples; i++)
        {
            sum += data[i] * data[i];
        }

        return static_cast<SampleType> (std::sqrt (sum / numSamples));
    }
    
    /** Returns the root mean squared level for a region of a channel in an audioblock. */
    template<typename SampleType>
    static inline SampleType getRMSLevel (const dsp::AudioBlock<SampleType>& block, int channel, int startSample, int numSamples) noexcept
    {
        jassert (isPositiveAndBelow (channel, block.getNumChannels()));
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= block.getNumSamples());

        if (numSamples <= 0 || channel < 0 || channel >= block.getNumChannels() || block.getNumSamples() == 0)
            return SampleType(0);

        auto* data = block.getChannelPointer(channel);
        double sum = 0.0;

        for (int i = startSample; i < numSamples; i++)
        {
            sum += data[i] * data[i];
        }

        return static_cast<SampleType> (std::sqrt (sum / numSamples));
    }
};


} // namespace stm

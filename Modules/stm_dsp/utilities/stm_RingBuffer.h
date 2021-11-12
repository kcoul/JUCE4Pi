#pragma once

namespace stm {

/**
 Single-producer (audio thread)
 Single-consumer (gui thread)
 */
class RingBuffer {
public:
    RingBuffer() {
        
    }
    
    void prepare(const dsp::ProcessSpec& spec, double pullsPerSecond = 60.0f, int safetyFactor = 4){
        //The amount of samples that should be read with each consumer pull.
        const int samplesPerPull = int (spec.sampleRate / pullsPerSecond);
        //The amount of samples that should be written with each producer push.
        const int samplesPerPush = static_cast<const int>(spec.maximumBlockSize);
        
        const int safetySamples = jmax(samplesPerPull, samplesPerPush) * safetyFactor;
        maxReadSamples = samplesPerPull * safetyFactor;
        
        const int bufferSamples =  maxReadSamples + safetySamples;
        buffer.setSize(static_cast<int>(spec.numChannels), bufferSamples);
        
        processSpec = spec;
    }
    
    /**
     ONLY FUNCTION TO BE CALLED BY THE AUDIO THREAD!!!
     Adds a block of audio data to the buffer.
     This way the audio thread only needs to interact with the atomics once per buffer.
     */
    void pushBlock(const dsp::AudioBlock<float>& processBlock) {
        int writePos = writePosAtomic.load();
        
        for (int iSample = 0 ; iSample<(int)processBlock.getNumSamples() ; iSample++){
            
            for (int iChannel = 0 ; iChannel < (int)processBlock.getNumChannels() ; iChannel++){
                //Read sample from incoming AudioBlock
                float level = processBlock.getSample(iChannel, iSample);
                //Set next ring buffer sample
                buffer.setSample(iChannel, writePos, level);
            }
            increment(writePos);
            writeSamples++;
        }

        writePosAtomic.store(writePos);
    }
    
    /**
     Returns an audio block with any data between the readPos and writePosAtomic.
     Splits the block in two if wrapping occurs. Call this twice by the consumer to account for this.
     Jumps forward (bypassing data) if the distance between readPos and writePosAtomic is greater than maxReadSamples.
     */
    const juce::dsp::AudioBlock<float> popBlock(){
        int writePos = writePosAtomic.load();
        updateReadPos(writePos);
        
        int readLength = 0;
        if (writePos >= readPos){
            // We can send a continuous buffer
            readLength = writePos - readPos;
        } else {
            // We cannot send a continuous buffer. So send what we can.
            readLength = buffer.getNumSamples() - readPos;
        }
        auto startPos = readPos;
        increment(readPos, readLength);
        
        readSamples += readLength;
        
        return juce::dsp::AudioBlock<float>(buffer).getSubBlock(
            static_cast<size_t>(startPos), static_cast<size_t>(readLength));
    }
    
    void reset(){
        buffer.clear();
        readPos = 0;
        writePosAtomic.store(0);
    }
    
    juce::dsp::ProcessSpec processSpec = {44100.0, 2048, 2}; //sampleRate, (uint32) samplesPerBlock, (uint32) totalNumInputChannels
    
private:
    int writeSamples = 0;
    int readSamples = 0;
    
    std::atomic<int> writePosAtomic = {0};
    int readPos = 0;
    int maxReadSamples = 0;
    
    juce::AudioSampleBuffer buffer;
    
    void increment(int& index, int amount = 1){
        index += amount;
        index %= buffer.getNumSamples();
    }
    
    int getSamplesBetween(int leadIndex, int followIndex){
        if (leadIndex >= followIndex){
            return leadIndex - followIndex;
        } else {
            return leadIndex + buffer.getNumSamples() - followIndex;
        }
    }
    
    void updateReadPos(int writePos){
        int samplesBetween = getSamplesBetween(writePos, readPos);
        if (samplesBetween > maxReadSamples){
            int difference = samplesBetween - maxReadSamples;
            increment(readPos, difference);
        }
    }
};


} //namespace stm

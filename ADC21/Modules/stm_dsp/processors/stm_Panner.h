#pragma once
#include "../utilities/stm_Rampers.h"
#include "../utilities/stm_Mixer.h"

using namespace juce;

namespace stm {

class BalancePanner {
public:
    void prepare() {
        leftLevel.prepare(1.0f, 0.001f);
        rightLevel.prepare(1.0f, 0.001f);
    }
    
    void process(const dsp::AudioBlock<float>& processBlock) {
        //BalancePanner is only designed to pan between 2 channels.
        jassert(processBlock.getNumChannels() == 2);
        
        auto* left = processBlock.getChannelPointer(0);
        auto* right = processBlock.getChannelPointer(1);
        
        for (auto iSample = 0 ; iSample < (int)processBlock.getNumSamples() ; iSample++)
        {
            left[iSample] *= leftLevel.getNext();
            right[iSample] *= rightLevel.getNext();
        }
    }
    
    void reset() {
        leftLevel.reset();
        rightLevel.reset();
    }
    
    void setLinear(float newPan) {
        auto balance = Balancer::getLinear(newPan);
        leftLevel.updateTarget(balance.left);
        rightLevel.updateTarget(balance.right);
    }
    
    void setLinearCentered(float newPan) {
        auto balance = Balancer::getLinearCentered(newPan);
        leftLevel.updateTarget(balance.left);
        rightLevel.updateTarget(balance.right);
    }
    
    void setSin(float newPan) {
        auto balance = Balancer::getSin(newPan);
        leftLevel.updateTarget(balance.left);
        rightLevel.updateTarget(balance.right);
    }
    
    void setSinCentered(float newPan) {
        auto balance = Balancer::getSinCentered(newPan);
        leftLevel.updateTarget(balance.left);
        rightLevel.updateTarget(balance.right);
    }
    
private:
    RamperLinear leftLevel;
    RamperLinear rightLevel;
};

class StereoPanner {
public:
    void prepare() {
        pan.prepare(0.5f, 0.001f);
    }
    
    void process(const dsp::AudioBlock<float>& processBlock) {
        //BalancePanner is only designed to pan between 2 channels.
        jassert(processBlock.getNumChannels() == 2);
        
        for (auto iSample = 0 ; iSample < (int)processBlock.getNumSamples() ; iSample++)
        {
            auto mix = Mixer::getLinearCentered(pan.getNext());
            
            float leftSample = processBlock.getSample(0, iSample);
            float rightSample = processBlock.getSample(1, iSample);
            
            float leftSampleOut = leftSample * mix.left + rightSample * mix.rightCross;
            float rightSampleOut = rightSample * mix.right + leftSample * mix.leftCross;
            
            processBlock.setSample(0, iSample, leftSampleOut);
            processBlock.setSample(1, iSample, rightSampleOut);
        }
    }
    
    void reset(){
        pan.reset();
    }
    
    void setLinearCentered(float newPan){
        pan.updateTarget(newPan);
    }
    
private:
    RamperLinear pan;
};

} // namespace stm

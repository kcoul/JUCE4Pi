/*
  ==============================================================================

    Params.h
    Created: 2 May 2020 7:24:40pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once
#if USING_CMAKE
#include <juce_audio_processors/juce_audio_processors.h>
#else
#include <JuceHeader.h>
#endif

class Params
{
public:
    constexpr static const float MAX_DELAY = 1000.0f; //ms
    constexpr static const float MAX_MOD = 1.0f; //ms
    constexpr static const float MAX_SPREAD = 300.0f; //ms
    constexpr static const float MAX_OFFSET = 1.0f; //ratio of delay
    constexpr static const int MAX_TAPS = 10;
    constexpr static const float MIN_DELAY = MAX_MOD;
    // MAX_BUFFER represents the maximum amount of time (in ms) that a delay buffer must
    // hold in order to satisfy all possible parameter combinations.
    constexpr static const float MAX_BUFFER = MAX_DELAY * (1.0f + MAX_OFFSET) + MAX_SPREAD;
    
    inline static const juce::String idDry = "dry";
    inline static const juce::String idWet = "wet";
    inline static const juce::String idBypass = "bypass";
    inline static const juce::String idDelay = "delay";
    inline static const juce::String idPan = "pan";
    inline static const juce::String idTaps = "taps";
    inline static const juce::String idSpread = "spread";
    inline static const juce::String idOffsetLR = "offsetLR";
    inline static const juce::String idAllpass = "allpass";
    inline static const juce::String idFeedbackDirect = "feedbackDirect";
    inline static const juce::String idFeedbackCross = "feedbackCross";
    inline static const juce::String idHighPass = "highPass";
    inline static const juce::String idLowPass = "lowPass";
    inline static const juce::String idModDepth = "modDepth";
    inline static const juce::String idModRate = "modRate";
    inline static const juce::String idSnap = "snap";

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
        auto delayRange = juce::NormalisableRange<float>(MIN_DELAY, MAX_DELAY);
        delayRange.setSkewForCentre(100.0f);
        
        auto gainRange = juce::NormalisableRange<float>(-100.0f, 0.0f);
        gainRange.setSkewForCentre(-18.0f);
        
        auto frequencyRange = juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f);
        frequencyRange.setSkewForCentre(640.0f);
        
        // auto tapsRange = juce::NormalisableRange<float>(1.0f, 10.0f, 1.0f);
        
        auto modRateRange = juce::NormalisableRange<float>(0.0f, 10.0f);
        modRateRange.setSkewForCentre(2.0f);
        
        auto spreadRange = juce::NormalisableRange<float>(0.0f, 100.0f);
        spreadRange.setSkewForCentre(1.0f);
        
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idDry, "Dry Enabled", gainRange, 0.0f) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idWet, "Wet dB", gainRange, -6.0f) );
        params.push_back( std::make_unique<juce::AudioParameterBool>(idBypass, "Matched Bypass", false) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idDelay, "Delay", delayRange, 100.0f) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idPan, "Pan", 0.0f, 1.0f, 0.5f));
        params.push_back( std::make_unique<juce::AudioParameterInt>(idTaps, "Taps", 1, MAX_TAPS, 1) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idSpread, "Spread", spreadRange, 1.0f) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idOffsetLR, "L/R Offset", 0.0f, 1.0f, 0.5f ) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idAllpass, "Allpass", 0.0f, 1.0f, 0.0f ) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idFeedbackDirect, "Direct Feedback", 0.0f, 1.0f, 0.0f) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idFeedbackCross, "Cross Feedback", 0.0f, 1.0f, 0.0f) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idHighPass, "High Pass Hz", frequencyRange, 20.0f) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idLowPass, "Low Pass Hz", frequencyRange, 20000.0f) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idModDepth, "Mod Depth", 0.0f, 1.0f, 0.0f) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idModRate, "Mod Rate (Hz)", modRateRange, 2.0f) );
        params.push_back( std::make_unique<juce::AudioParameterFloat>(idSnap, "Snap Note", 0.0f, 1.0f, 0.0f) );
        
        return { params.begin(), params.end() };
    }
};

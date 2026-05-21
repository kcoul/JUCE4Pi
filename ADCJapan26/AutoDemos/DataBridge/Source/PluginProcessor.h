#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_osc/juce_osc.h>
#include "AvasMapping.h"

class GenisysPluginProcessor  : public juce::AudioProcessor
{
public:
    GenisysPluginProcessor();

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "DataBridge"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& dest) override;
    void setStateInformation (const void* data, int size) override;

    // Exposed to editor
    juce::String targetHost { AvasMapping::surgeOscHost };
    int          targetPort { AvasMapping::surgeOscPort };

    float        lastCcValues[128] = {};
    int          currentGear       = 0;  // 0 = no gear engaged

    std::function<void()> onStateChanged;

private:
    void sendOsc (const juce::String& address, float value);

    juce::OSCSender sender;
    bool            senderConnected = false;

    void ensureConnected();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenisysPluginProcessor)
};

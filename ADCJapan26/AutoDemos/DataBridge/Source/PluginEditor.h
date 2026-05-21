#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AvasMapping.h"

class GenisysPluginEditor  : public juce::AudioProcessorEditor,
                             private juce::Timer
{
public:
    explicit GenisysPluginEditor (GenisysPluginProcessor& p);
    ~GenisysPluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildConnection();

    GenisysPluginProcessor& proc;

    // Connection config
    juce::Label      hostLabel   { {}, "SurgeXT Host:" };
    juce::TextEditor hostEditor;
    juce::Label      portLabel   { {}, "Port:" };
    juce::TextEditor portEditor;
    juce::TextButton connectBtn  { "Reconnect" };

    // Gear display
    juce::Label      gearLabel   { {}, "Gear" };

    // Per-CC live bars (one per mapped CC)
    struct CcRow
    {
        juce::Label  label;
        juce::Label  valueLabel;
        float        currentValue = 0.0f;
    };
    CcRow ccRows[AvasMapping::numCcMappings];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenisysPluginEditor)
};

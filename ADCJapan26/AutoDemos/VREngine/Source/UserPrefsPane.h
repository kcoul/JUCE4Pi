#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_osc/juce_osc.h>

// Represents a named sound profile that maps to a SurgeXT user patch.
struct SoundProfile
{
    juce::String name;          // display name, e.g. "ESE Normal"
    juce::String patchName;     // SurgeXT /patch/load_user argument, e.g. "GENISYS/ESE_Normal"
    juce::String voiceKeyword;  // word(s) Whisper output is matched against
};

// A panel showing a list of sound profiles that can be activated by button or voice command.
// Sends /patch/load_user to SurgeXT when a profile is selected.
class UserPrefsPane  : public juce::Component
{
public:
    UserPrefsPane()
    {
        titleLabel.setText ("Engine Sound", juce::dontSendNotification);
        titleLabel.setFont (juce::FontOptions (18.0f, juce::Font::bold));
        addAndMakeVisible (titleLabel);

        lastCommandLabel.setJustificationType (juce::Justification::centred);
        lastCommandLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
        addAndMakeVisible (lastCommandLabel);

        diagLabel.setJustificationType (juce::Justification::centred);
        diagLabel.setFont (juce::FontOptions (11.0f));
        diagLabel.setColour (juce::Label::textColourId, juce::Colours::skyblue);
        addAndMakeVisible (diagLabel);

        for (auto& p : profiles)
            addProfile (p);

        bool connected = sender.connect (surgeHost, surgePort);
        lastCommandLabel.setText (connected ? "OSC ready (" + surgeHost + ":" + juce::String (surgePort) + ")"
                                            : "OSC connect FAILED",
                                  juce::dontSendNotification);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (12);
        titleLabel.setBounds (area.removeFromTop (28));
        area.removeFromTop (6);

        int btnH = 52;
        for (auto& btn : profileButtons)
            btn->setBounds (area.removeFromTop (btnH).reduced (0, 4));

        lastCommandLabel.setBounds (area.removeFromTop (24));
        diagLabel.setBounds (area.removeFromTop (20));
    }

    void showVadActivity (float prob)
    {
        diagLabel.setText ("VAD " + juce::String (juce::roundToInt (prob * 100)) + "% — speech detected",
                           juce::dontSendNotification);
    }

    void showWhisperStart (int utterSamples)
    {
        const int ms = utterSamples * 1000 / 16000;
        diagLabel.setText ("STT: " + juce::String (ms) + "ms -> Whisper...",
                           juce::dontSendNotification);
    }

    // Called by VREngine when Whisper produces a transcript.
    void handleVoiceInput (const juce::String& text)
    {
        diagLabel.setText ({}, juce::dontSendNotification);
        lastCommandLabel.setText ("Heard: \"" + text + "\"", juce::dontSendNotification);

        auto lower = text.toLowerCase();
        for (int i = 0; i < (int) profiles.size(); ++i)
            if (lower.contains (profiles[i].voiceKeyword.toLowerCase()))
                activateProfile (i);
    }

    juce::String surgeHost = "127.0.0.1";
    int          surgePort = 53280;

private:
    const std::vector<SoundProfile> profiles =
    {
        { "Modern",  "/QNX/Pads/Modern",  "modern" },
        { "ICE",     "/QNX/Pads/ICE",     "ice"    },
    };

    juce::OSCSender sender;
    juce::Label titleLabel;
    juce::Label lastCommandLabel;
    juce::Label diagLabel;
    std::vector<std::unique_ptr<juce::TextButton>> profileButtons;
    int activeIndex = 0;

    void addProfile (const SoundProfile& p)
    {
        auto btn = std::make_unique<juce::TextButton> (p.name);
        int idx = (int) profileButtons.size();
        btn->onClick = [this, idx] { activateProfile (idx); };
        addAndMakeVisible (*btn);
        profileButtons.push_back (std::move (btn));
    }

    void activateProfile (int idx)
    {
        if (idx < 0 || idx >= (int) profiles.size()) return;
        activeIndex = idx;

        for (int i = 0; i < (int) profileButtons.size(); ++i)
            profileButtons[i]->setToggleState (i == activeIndex, juce::dontSendNotification);

        lastCommandLabel.setText ("Active: " + profiles[idx].name, juce::dontSendNotification);
        sendPatchLoad (profiles[idx].patchName);
    }

    void sendPatchLoad (const juce::String& patchName)
    {
        bool ok = sender.send ("/patch/load_user", patchName);
        lastCommandLabel.setText (ok ? "Sent: " + patchName : "Send FAILED: " + patchName,
                                  juce::dontSendNotification);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UserPrefsPane)
};

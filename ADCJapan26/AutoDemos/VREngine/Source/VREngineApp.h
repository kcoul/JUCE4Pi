#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "UserPrefsPane.h"

// ---------------------------------------------------------------------------
// Whisper/NPU voice input — compiled in only when VRENGINE_HAS_HAILO=1.
// When the flag is off the UI still works; voice input is via a typed field.
// ---------------------------------------------------------------------------
#if VRENGINE_HAS_HAILO
// Forward-declared; real implementation in VoiceInput_Hailo.cpp
class VoiceInputThread;
#endif

class MainWindow  : public juce::DocumentWindow
{
public:
    explicit MainWindow (const juce::String& name)
        : DocumentWindow (name,
                          juce::Desktop::getInstance().getDefaultLookAndFeel()
                              .findColour (juce::ResizableWindow::backgroundColourId),
                          DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar (true);
        setContentOwned (new MainComponent(), true);
        setResizable (true, true);
        centreWithSize (400, 320);
        setVisible (true);
    }

    void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }

    // -----------------------------------------------------------------------
    struct MainComponent  : public juce::Component
    {
        UserPrefsPane prefsPane;

        // Typed input — always available; on Hailo builds also wired from VoiceInputThread
        juce::TextEditor  voiceSimInput;
        juce::TextButton  submitBtn { "Send" };
        juce::Label       inputLabel { {}, "Voice (typed):" };

        MainComponent()
        {
            addAndMakeVisible (prefsPane);
            addAndMakeVisible (inputLabel);
            addAndMakeVisible (voiceSimInput);
            addAndMakeVisible (submitBtn);

            submitBtn.onClick = [this]
            {
                prefsPane.handleVoiceInput (voiceSimInput.getText());
                voiceSimInput.clear();
            };

            setSize (400, 320);
        }

        void resized() override
        {
            auto area = getLocalBounds().reduced (8);
            auto inputRow = area.removeFromBottom (36).reduced (0, 4);
            submitBtn.setBounds    (inputRow.removeFromRight (70));
            inputLabel.setBounds   (inputRow.removeFromLeft  (100));
            voiceSimInput.setBounds (inputRow);
            prefsPane.setBounds    (area);
        }
    };
};

class VREngineApp  : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "VREngine"; }
    const juce::String getApplicationVersion() override { return "0.1"; }

    void initialise (const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override { mainWindow.reset(); }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

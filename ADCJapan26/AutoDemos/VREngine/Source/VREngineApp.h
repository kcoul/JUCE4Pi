#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "UserPrefsPane.h"

#if VRENGINE_HAS_HAILO
#include "VoiceInput_Hailo.h"
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
        centreWithSize (400, 300);
        setVisible (true);
    }

    void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }

    // -----------------------------------------------------------------------
    struct MainComponent  : public juce::Component
    {
        UserPrefsPane prefsPane;

#if VRENGINE_HAS_HAILO
        std::unique_ptr<VoiceInputThread> voiceThread;
#endif

        MainComponent()
        {
            addAndMakeVisible (prefsPane);
            setSize (400, 300);

#if VRENGINE_HAS_HAILO
            voiceThread = std::make_unique<VoiceInputThread>();
            voiceThread->start ([this] (const juce::String& text) {
                prefsPane.handleVoiceInput (text);
            });
#endif
        }

        void resized() override
        {
            prefsPane.setBounds (getLocalBounds().reduced (8));
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

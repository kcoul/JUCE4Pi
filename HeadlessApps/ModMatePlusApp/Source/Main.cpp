
#include "MainContentComponent.h"
#include "Headless.h"
#include <juce_gui_basics/juce_gui_basics.h>

class Application    : public juce::JUCEApplication
{
public:
    //==============================================================================
    Application() = default;

    const juce::String getApplicationName() override       { return "ModMatePlusApp"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }

    void initialise (const juce::String&) override
    {
        if (juce::Desktop::getInstance().isHeadless())
            headless.reset(new Headless());
        else
            mainWindow.reset (new MainWindow ("ModMatePlusApp", new MainContentComponent, *this));
    }

    void shutdown() override
    {
        if (juce::Desktop::getInstance().isHeadless())
            headless = nullptr;
        else
            mainWindow = nullptr;
    }

private:
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (const juce::String& name, juce::Component* c, JUCEApplication& a)
            : DocumentWindow (name, juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                                .findColour (ResizableWindow::backgroundColourId),
                              juce::DocumentWindow::allButtons),
              app (a)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (c, true);

           #if JUCE_ANDROID || JUCE_IOS
            setFullScreen (true);
           #else
            setResizable (true, false);
            setResizeLimits (300, 250, 10000, 10000);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            app.systemRequestedQuit();
        }

    private:
        JUCEApplication& app;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<Headless> headless;
};

//==============================================================================
START_JUCE_APPLICATION (Application)

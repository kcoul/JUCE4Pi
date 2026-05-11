#include "MainWindow.h"

namespace GuiApp
{
constexpr bool isMobile()
{
#if JUCE_IOS || JUCE_ANDROID
    return true;
#else
    return false;
#endif
}

MainWindow::MainWindow(const String& name)
    : DocumentWindow(name, getBackgroundColour(), allButtons)
{
    setName("ADC21CPanel");
    setUsingNativeTitleBar(true);
    setContentOwned(new MainComponent(), true);

    //TODO: Test mobile interface connecting to RPi Hotspot or similar
    if (isMobile())
        setFullScreen(true);
    else
    {
        //Gui was designed for 800 x 480
        getConstrainer()->setFixedAspectRatio(1.66667);
        //Sufficient for demo purposes, in real app, maybe constrain
        //to largest detected display for multi-monitor setups
        auto display = Desktop::getInstance().getDisplays().getPrimaryDisplay();
        if (display)
        {
            Rectangle<int> r = display->userArea;
                int x = r.getWidth();
                int y = r.getHeight();

            setResizeLimits (800,
                            480,
                            x,
                            y);
        }
        setResizable(true, true);
        centreWithSize(getWidth(), getHeight());
        setFullScreen(true);
    }

    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

Colour MainWindow::getBackgroundColour()
{
    return juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
        ResizableWindow::backgroundColourId);
}

} // namespace GuiApp

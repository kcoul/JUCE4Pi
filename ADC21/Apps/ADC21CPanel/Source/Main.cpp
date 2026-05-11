#include "MainWindow.h"

namespace GuiApp
{
class GuiAppTemplateApplication : public juce::JUCEApplication
{
public:
    const String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
    const String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const String& /*commandLine*/) override
    {
        if (!juce::Desktop::getInstance().isHeadless())
            mainWindow = std::make_unique<MainWindow>(getApplicationName());
        else //run in headless mode
        {
            std::cout << "Running in headless mode" << std::endl;
        }
    }

    void shutdown() override { mainWindow.reset(); }

    void systemRequestedQuit() override { quit(); }

    void anotherInstanceStarted(const String& /*commandLine*/) override {}

private:
    std::unique_ptr<MainWindow> mainWindow;
};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(GuiAppTemplateApplication)

} // namespace GuiApp

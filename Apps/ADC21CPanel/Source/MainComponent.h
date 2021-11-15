#pragma once

#include "CommonHeader.h"
#include <functional>
#include <thread>
#include "LushDelayEngine.h"
#include "Params.h"

#define USE_PIGPIOD 0

#if JUCE_LINUX
    #include <cstdlib>
    #include <sstream>

    #if USE_ALSA_API
        #include <alsa/asoundlib.h>
    #endif

    #if __arm__
        #if USE_PIGPIOD
            extern "C"
            {
            #include "RED.h"
            }
            #include <pigpiod_if2.h>
        #else
            #include "rotary_encoder.hpp"
            #include <pigpio.h>
        #endif
    #endif
#endif

namespace GuiApp
{

template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
    template <typename... Args>
    static Ret callback(Args... args) { return func(args...); }
    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    void paint (juce::Graphics&) override;
    void resized() override;

    //TODO: Resolve auto-init issues on Linux/RPi
    void initialiseDevice();
private:
    bool enablePassthrough = false;

    template <class T>
    bool convertFromStr(std::string &str, T *var) {
        std::istringstream ss(str);
        return (ss >> *var);
    }

    template <class T>
    std::string convertToStr(T *var) {
        std::ostringstream ss;
        ss << *var;
        return ss.str();
    }

    //==============================================================================
    juce::Slider parameterSlider;
    void parameterSliderChanged();
    juce::Label sliderLabel;

    juce::ImageButton logoImageComponent;

    HackAudio::Button topButton;
    void topButtonClicked();
    juce::Label topButtonLabel;

    HackAudio::Button bottomButton;
    void bottomButtonClicked();
    juce::Label bottomButtonLabel;

    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meter { foleys::LevelMeter::MeterFlags::Default };
    foleys::LevelMeterSource meterSource;

#if JUCE_LINUX
    #if USE_ALSA_API
        snd_mixer_t *handle;
        const char *card = "default";

        long min, max;
        snd_mixer_selem_id_t *sid;
        const char *selem_name = "Master";
        snd_mixer_elem_t* elem;

        long min_capture, max_capture;
        snd_mixer_selem_id_t *sid_capture;
        const char *selem_name_capture = "Capture";
        snd_mixer_elem_t* elem_capture;
    #endif

    #if __arm__
        unsigned int TOP_BUTTON_PIN = 25;
        #if USE_PIGPIOD
            char *optHost   = NULL;
            char *optPort   = NULL;
            int optGlitch = 1000;
            int optSeconds = 0;
            int optMode = RED_MODE_DETENT;
            int pi;
            RED_t *renc;
            unsigned int EDGE = 2;
            void pigpiod_cbf(int way);
            void handleTopButton(int pi, unsigned int gpio, unsigned int edge, uint32_t tick);
            void handleBottomButton(int pi, unsigned int gpio, unsigned int edge, uint32_t tick);
        #else
            std::unique_ptr<re_decoder> renc;
            void pigpio_cbf(int way);
            void handleTopButton(int gpio, int edge, uint32_t tick);
            void handleBottomButton(int gpio, int edge, uint32_t tick);
        #endif
    #endif
#endif

    bool initialised_pigpio = false;
    bool channel2Enabled = false;

    juce::AudioDeviceSelectorComponent selector {
        deviceManager, 2, 2,
        2, 2,
        false, false,
        true, false};

    juce::ToggleButton settingsButton;
    void settingsButtonClicked();

    HackAudio::Button quitCPanelButton;
    static void quitCPanelButtonClicked();
    juce::Label quitCPanelLabel;

    HackAudio::Button enablePassthroughButton;
    void enablePassthroughButtonClicked();
    juce::Label enablePassthroughLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

} // namespace GuiApp

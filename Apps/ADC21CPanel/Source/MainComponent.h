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

// Initialize the static member.
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

    juce::dsp::Reverb::Parameters params;
    juce::dsp::Reverb leftReverb, rightReverb;

    bool enableReverb = false;

    LushDelayEngine lushDelayEngine;

    juce::NormalisableRange<float> delayRange = NormalisableRange<float>(Params::MIN_DELAY, Params::MAX_DELAY);
    juce::NormalisableRange<float> gainRange = NormalisableRange<float>(-100.0f, 0.0f);
    juce::NormalisableRange<float> frequencyRange = NormalisableRange<float>(20.0f, 20000.0f, 0.1f);
    // juce::NormalisableRange<float> tapsRange = NormalisableRange<float>(1.0f, 10.0f, 1.0f);
    juce::NormalisableRange<float> modRateRange = NormalisableRange<float>(0.0f, 10.0f);
    juce::NormalisableRange<float> spreadRange = NormalisableRange<float>(0.0f, 100.0f);

    std::unique_ptr<AudioParameterFloat> idDry;
    std::unique_ptr<AudioParameterFloat> idWet;
    std::unique_ptr<AudioParameterBool> idBypass;
    std::unique_ptr<AudioParameterFloat> idDelay;
    std::unique_ptr<AudioParameterFloat> idPan;
    std::unique_ptr<AudioParameterInt> idTaps;
    std::unique_ptr<AudioParameterFloat> idSpread;
    std::unique_ptr<AudioParameterFloat> idOffsetLR;
    std::unique_ptr<AudioParameterFloat> idAllpass;
    std::unique_ptr<AudioParameterFloat> idFeedbackDirect;
    std::unique_ptr<AudioParameterFloat> idFeedbackCross;
    std::unique_ptr<AudioParameterFloat> idHighPass;
    std::unique_ptr<AudioParameterFloat> idLowPass;
    std::unique_ptr<AudioParameterFloat> idModDepth;
    std::unique_ptr<AudioParameterFloat> idModRate;
    std::unique_ptr<AudioParameterFloat> idSnap;

    bool enableDelay = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

} // namespace GuiApp

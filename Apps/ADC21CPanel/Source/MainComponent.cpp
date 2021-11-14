#include "MainComponent.h"
#include "BinaryHelper.h"
namespace GuiApp
{

//==============================================================================
MainComponent::MainComponent()
{
    //On RPi: this is the resolution of a Waveshare 3.5" touchscreen display.
    //(On Mobile: setSize() is ignored but height/width must be set and non-zero)
    setSize (800, 480);

    setOpaque(true);

//Selector Pane
    addAndMakeVisible(selector);
    selector.setVisible(false);

    quitCPanelLabel.setText("Quit CPanel", juce::NotificationType::dontSendNotification);
    quitCPanelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(quitCPanelLabel);
    quitCPanelLabel.setVisible(false);

    quitCPanelButton.setButtonStyle(HackAudio::Button::ButtonStyle::BarToggle);
    quitCPanelButton.onClick = [] { quitCPanelButtonClicked(); };
    quitCPanelButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(quitCPanelButton);
    quitCPanelButton.setVisible(false);

    enablePassthroughLabel.setText("Enable Passthrough",
                                   juce::NotificationType::dontSendNotification);
    enablePassthroughLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(enablePassthroughLabel);
    enablePassthroughLabel.setVisible(false);

    enablePassthroughButton.setButtonStyle(HackAudio::Button::ButtonStyle::BarToggle);
    enablePassthroughButton.onClick = [this] { enablePassthroughButtonClicked(); };
    addAndMakeVisible(enablePassthroughButton);
    enablePassthroughButton.setVisible(false);

//Main Pane
    parameterSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    parameterSlider.setRange(1.0, 1000.0, 5.0);
    parameterSlider.onValueChange = [this] { parameterSliderChanged(); };
    addAndMakeVisible(parameterSlider);

    sliderLabel.setText("Parameter", juce::NotificationType::dontSendNotification);
    sliderLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sliderLabel);

    auto images = getBinaryDataImages();

    logoImageComponent.setImages(true, true, true,
                                 images.front(), 1.0f, juce::Colours::transparentWhite,
                                 images.front(), 1.0f, juce::Colours::transparentWhite,
                                 images.front(), 1.0f, juce::Colours::transparentWhite);
    logoImageComponent.onClick = [this] { settingsButtonClicked(); };
    addAndMakeVisible(logoImageComponent);

    topButton.setButtonStyle(HackAudio::Button::ButtonStyle::BarToggle);
    topButton.onClick = [this] { topButtonClicked(); };
    addAndMakeVisible(topButton);

    topButtonLabel.setText("Enable Reverb", juce::NotificationType::dontSendNotification);
    topButtonLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(topButtonLabel);

    bottomButton.setButtonStyle(HackAudio::Button::ButtonStyle::BarToggle);
    bottomButton.onClick = [this] { bottomButtonClicked(); };
    addAndMakeVisible(bottomButton);

    bottomButtonLabel.setText("Enable Delay", juce::NotificationType::dontSendNotification);
    bottomButtonLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bottomButtonLabel);

    lnf.setColour(foleys::LevelMeter::lmBackgroundColour, topButton.findColour(HackAudio::midgroundColourId));
    lnf.setColour (foleys::LevelMeter::lmMeterGradientLowColour, juce::Colours::green);
    lnf.setColour(foleys::LevelMeter::lmTicksColour,
                  topButton.findColour(HackAudio::backgroundColourId));

    meter.setLookAndFeel (&lnf);
    meter.setMeterSource (&meterSource);
    addAndMakeVisible (meter);

#if JUCE_LINUX
    #if USE_ALSA_API
        snd_mixer_open(&handle, 0);
        snd_mixer_attach(handle, card);
        snd_mixer_selem_register(handle, NULL, NULL);
        snd_mixer_load(handle);

        snd_mixer_selem_id_alloca(&sid);
        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, selem_name);
        elem = snd_mixer_find_selem(handle, sid);
        if (elem)
            snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

        snd_mixer_selem_id_set_name(sid, selem_name_capture);
        elem_capture = snd_mixer_find_selem(handle, sid);
        if (elem_capture)
            snd_mixer_selem_get_capture_volume_range(elem_capture, &min_capture, &max_capture);
    #endif

    #if __arm__
        #if USE_PIGPIOD
            pi = pigpio_start(optHost, optPort);
            if (pi < 0)
                std::cout << "Couldn't connect to pigpiod, did you forget to launch it?" << std::endl;
            else
            {
                Callback<void(int)>::func = std::bind(&MainComponent::pigpiod_cbf, this, std::placeholders::_1);
                void (*c_func)(int) = static_cast<decltype(c_func)>(Callback<void(int)>::callback);
                renc = RED(pi, 24, 23, optMode, c_func);
                RED_set_glitch_filter(renc, optGlitch);

                set_mode(pi, BUTTON_PIN, PI_INPUT);
                set_pull_up_down(pi, BUTTON_PIN, PI_PUD_UP);

                Callback<void(int, unsigned int, unsigned int, uint32_t)>::func =
                    std::bind(&MainComponent::handleButton, this,
                              std::placeholders::_1, std::placeholders::_2,
                              std::placeholders::_3, std::placeholders::_4);
                void (*c_func2)(int, unsigned int, unsigned int, uint32_t) =
                    static_cast<decltype(c_func2)>(Callback<void(int, unsigned int, unsigned int,
                                                                 uint32_t)>::callback);
                callback(pi, BUTTON_PIN, EDGE, c_func2);
                initialised_pigpio = true;
            }
        #else
            //pigpio clocking conflicts with sound card...
            //https://github.com/joan2937/pigpio/issues/87
            gpioCfgClock(5, 0, 0);
            int ret = gpioInitialise();
            if (ret < 0)
            {
                std::cout << "Couldn't initialise pigpio. Ret=" << ret << std::endl;
            }
            else
            {
                Callback<void(int)>::func = std::bind(&MainComponent::pigpio_cbf, this, std::placeholders::_1);
                void (*c_func)(int) = static_cast<decltype(c_func)>(Callback<void(int)>::callback);
                renc = std::make_unique<re_decoder>(24, 23, c_func);

                gpioSetMode(TOP_BUTTON_PIN, PI_INPUT);
                gpioSetPullUpDown(TOP_BUTTON_PIN, PI_PUD_UP);

                Callback<void(int, int, uint32_t)>::func =
                    std::bind(&MainComponent::handleTopButton, this,
                              std::placeholders::_1, std::placeholders::_2,
                              std::placeholders::_3);

                void (*c_func2)(int, int, uint32_t) =
                static_cast<decltype(c_func2)>(Callback<void(int, int,
                                                             uint32_t)>::callback);
                gpioSetAlertFunc(TOP_BUTTON_PIN, c_func2);

                initialised_pigpio = true;
            }
        #endif
    #endif
#endif

    setAudioChannels(2,2);

//Delay Init
    delayRange.setSkewForCentre(100.0f);
    gainRange.setSkewForCentre(-18.0f);
    frequencyRange.setSkewForCentre(640.0f);
    modRateRange.setSkewForCentre(2.0f);
    spreadRange.setSkewForCentre(1.0f);
    idDry = std::make_unique<AudioParameterFloat>(Params::idDry, "Dry Enabled", gainRange, 0.0f);
    idWet = std::make_unique<AudioParameterFloat>(Params::idWet, "Wet dB", gainRange, -6.0f);
    idBypass = std::make_unique<AudioParameterBool>(Params::idBypass, "Matched Bypass", false);
    idDelay = std::make_unique<AudioParameterFloat>(Params::idDelay, "Delay", delayRange, 100.0f);
    idPan = std::make_unique<AudioParameterFloat>(Params::idPan, "Pan", 0.0f, 1.0f, 0.5f);
    idTaps = std::make_unique<AudioParameterInt>(Params::idTaps, "Taps", 1, Params::MAX_TAPS, 1);
    idSpread = std::make_unique<AudioParameterFloat>(Params::idSpread, "Spread", spreadRange, 1.0f);
    idOffsetLR = std::make_unique<AudioParameterFloat>(Params::idOffsetLR, "L/R Offset", 0.0f, 1.0f, 0.5f );
    idAllpass = std::make_unique<AudioParameterFloat>(Params::idAllpass, "Allpass", 0.0f, 1.0f, 0.0f );
    idFeedbackDirect = std::make_unique<AudioParameterFloat>(Params::idFeedbackDirect, "Direct Feedback", 0.0f, 1.0f, 0.1f);
    idFeedbackCross = std::make_unique<AudioParameterFloat>(Params::idFeedbackCross, "Cross Feedback", 0.0f, 1.0f, 0.1f);
    idHighPass = std::make_unique<AudioParameterFloat>(Params::idHighPass, "High Pass Hz", frequencyRange, 20.0f);
    idLowPass = std::make_unique<AudioParameterFloat>(Params::idLowPass, "Low Pass Hz", frequencyRange, 20000.0f);
    idModDepth = std::make_unique<AudioParameterFloat>(Params::idModDepth, "Mod Depth", 0.0f, 1.0f, 0.0f);
    idModRate = std::make_unique<AudioParameterFloat>(Params::idModRate, "Mod Rate (Hz)", modRateRange, 2.0f);
    idSnap = std::make_unique<AudioParameterFloat>(Params::idSnap, "Snap Note", 0.0f, 1.0f, 0.0f);

    lushDelayEngine.init(idDry->get(), idWet->get(), idBypass->get(), idModDepth->get(), idModRate->get(),
                         idDelay->get(), idTaps->get(), idSpread->get(), idOffsetLR->get(), idFeedbackDirect->get(),
                         idFeedbackCross->get(), idLowPass->get(), idHighPass->get(), idPan->get(), idAllpass->get());
}

//TODO: Auto-initialising seems to be unreliable... :(
void MainComponent::initialiseDevice()
{
    juce::AudioDeviceManager::AudioDeviceSetup setup = selector.deviceManager.getAudioDeviceSetup();
#if JUCE_LINUX && __arm__
    setup.inputDeviceName = "Jack Audio Connection Kit";
    setup.outputDeviceName = "Jack Audio Connection Kit";
#elif JUCE_LINUX
    setup.inputDeviceName = "Scarlett 2i2 USB, USB Audio; Direct hardware device without any conversions";
    setup.outputDeviceName = "Scarlett 2i2 USB, USB Audio; Direct hardware device without any conversions";
    setup.sampleRate = 48000.0;
    setup.bufferSize = 512;
#endif
    setup.sampleRate = 48000.0;
    setup.bufferSize = 16;
    juce::String error = selector.deviceManager.setAudioDeviceSetup(setup, true);

    if (error.isNotEmpty())
        std::cout << error << std::endl;
}

void MainComponent::quitCPanelButtonClicked()
{
    juce::JUCEApplicationBase::quit();
}

void MainComponent::enablePassthroughButtonClicked()
{
    enablePassthrough = enablePassthroughButton.getToggleState();
}

MainComponent::~MainComponent()
{
    shutdownAudio();

#if JUCE_LINUX
    #if USE_ALSA_API
        snd_mixer_close(handle);
    #endif
    #if __arm__
        #if USE_PIGPIOD
            if (initialised_pigpio)
            {
                RED_cancel(renc);
                pigpio_stop(pi);
            }
        #else
            if (initialised_pigpio)
            {
                renc->re_cancel();
                gpioTerminate();
            }
        #endif
    #endif
#endif
    meter.setLookAndFeel (nullptr);
}

#if JUCE_LINUX && __arm__
#if USE_PIGPIOD
void MainComponent::pigpiod_cbf(int way)
{
    std::cout << "way=" << way << std::endl;
    juce::MessageManager::callAsync([this, way]
    {
        inputGainSlider.setValue(way*5.0f, juce::dontSendNotification);
    });
    inputGainChanged();
}
#else
void MainComponent::pigpio_cbf(int way)
{
    std::cout << "way=" << way << std::endl;
    juce::MessageManager::callAsync([this, way]
    {
        parameterSlider.setValue(parameterSlider.getValue()-way*5.0f,
                              juce::dontSendNotification);
    });
    parameterSliderChanged();
}
#endif

#if USE_PIGPIOD
void MainComponent::handleTopButton(int pi, unsigned int gpio, unsigned int edge, uint32_t tick)
#else
void MainComponent::handleTopButton(int gpio, int edge, uint32_t tick)
#endif
{
    std::cout << "press=" << edge << std::endl;
    if (edge == 1)
    {
        juce::MessageManager::callAsync([this]
        {
            bottomButton.startTimerHz(60);
            bottomButton.setToggleState(!bottomButton.getToggleState(),
                                      juce::dontSendNotification);
            bottomButtonClicked();
        });
    }
}

#if USE_PIGPIOD
void MainComponent::handleBottomButton(int pi, unsigned int gpio, unsigned int edge, uint32_t tick)
#else
void MainComponent::handleBottomButton(int gpio, int edge, uint32_t tick)
#endif
{
    std::cout << "press=" << edge << std::endl;
    if (edge == 1)
    {
        juce::MessageManager::callAsync([this]
        {
          bottomButton.startTimerHz(60);
          bottomButton.setToggleState(!bottomButton.getToggleState(),
                                      juce::dontSendNotification);
          bottomButtonClicked();
        });
    }
}
#endif

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    meterSource.resize (2, static_cast<int>(sampleRate * 0.1 / samplesPerBlockExpected));

//Reverb Setup
    juce::dsp::ProcessSpec spec{};

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<uint32>(samplesPerBlockExpected);
    spec.numChannels = 1;

    leftReverb.prepare(spec);
    rightReverb.prepare(spec);

//Delay Setup
    auto numOutputChannels = 2;
    lushDelayEngine.prepare({ sampleRate, (uint32) samplesPerBlockExpected, (uint32) numOutputChannels });
}

void MainComponent::releaseResources()
{
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    meterSource.measureBlock (*bufferToFill.buffer);

    if (!enablePassthrough)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    if (channel2Enabled)
    {
        //Mix input 2 with input 1
        bufferToFill.buffer->addFrom(0, 0, *bufferToFill.buffer,
                                      1, 0, bufferToFill.buffer->getNumSamples());
    }

    if (enableReverb)
    {
        params.roomSize = 0.5f /**apvts.getRawParameterValue ("Room Size")*/;
        params.damping = 0.5f /**apvts.getRawParameterValue ("Damping")*/;
        params.width = 0.5f /**apvts.getRawParameterValue ("Width")*/;
        params.wetLevel = 0.5f /**apvts.getRawParameterValue ("Dry/Wet")*/;
        params.dryLevel = 0.5f /*1.0f - *apvts.getRawParameterValue ("Dry/Wet")*/;
        params.freezeMode = false /**apvts.getRawParameterValue ("Freeze")*/;

        leftReverb.setParameters(params);
        rightReverb.setParameters(params);

        juce::dsp::AudioBlock<float> block(*bufferToFill.buffer);

        auto leftBlock = block.getSingleChannelBlock(0);
        auto rightBlock = block.getSingleChannelBlock(1);

        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

        leftReverb.process(leftContext);
        rightReverb.process(rightContext);
    }

    if (enableDelay)
    {
        lushDelayEngine.process(*bufferToFill.buffer);
    }
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (topButton.findColour(HackAudio::midgroundColourId));
}

void MainComponent::resized()
{
    //Tic-tac-toe resizable layout (3x3)
    auto bounds = getBounds();

    int thirdOfWidth = bounds.getWidth()/3;
    int thirdOfWidthPlusOne = thirdOfWidth+1;

    int mainHeight = bounds.getHeight();
    int thirdOfMainHeight = mainHeight / 3;

    int labelHeight = thirdOfMainHeight / 5;

    parameterSlider.setBounds(0, 0, thirdOfWidth, mainHeight - labelHeight);
    parameterSlider.setValue(100, juce::NotificationType::dontSendNotification);
    sliderLabel.setBounds(0, mainHeight - labelHeight, thirdOfWidth, labelHeight);

    logoImageComponent.setBounds(thirdOfWidthPlusOne, 0, thirdOfWidth, thirdOfMainHeight);

    topButton.setBounds(thirdOfWidthPlusOne,
                                    thirdOfMainHeight, thirdOfWidth,
                                    thirdOfMainHeight - labelHeight);
    topButtonLabel.setBounds(thirdOfWidthPlusOne,
                             (thirdOfMainHeight * 2) - labelHeight, thirdOfWidth, labelHeight);

    bottomButton.setBounds(thirdOfWidthPlusOne,
                                     thirdOfMainHeight * 2, thirdOfWidth,
                                     thirdOfMainHeight - labelHeight);
    bottomButtonLabel.setBounds(thirdOfWidthPlusOne, mainHeight-labelHeight, thirdOfWidth, labelHeight);

    meter.setBounds(thirdOfWidthPlusOne * 2, 0, thirdOfWidth, mainHeight);

    selector.setBounds(0, thirdOfMainHeight, bounds.getWidth(), mainHeight - 150);

    quitCPanelButton.setBounds(0,
                                   0, thirdOfWidth/2,
                               (thirdOfMainHeight - labelHeight)/2);
    quitCPanelLabel.setBounds(0,
                              (thirdOfMainHeight - labelHeight)/2, thirdOfWidth/2, labelHeight/2);

    enablePassthroughButton.setBounds(getWidth() - (thirdOfWidth/2),
                               0, thirdOfWidth/2,
                               (thirdOfMainHeight - labelHeight)/2);
    enablePassthroughLabel.setBounds(getWidth() - (thirdOfWidth/2),
                                      (thirdOfMainHeight - labelHeight)/2, thirdOfWidth/2, labelHeight/2);
}

void MainComponent::parameterSliderChanged()
{
#if FX_BOX
    double value = parameterSlider.getValue();
    lushDelayEngine.setDelay(static_cast<float>(value));
#elif CPANEL
    double inputGainValueDecibels = parameterSlider.getValue();
    double inputGainValue = (inputGainValueDecibels + 12.0) * 2.0;

    #if JUCE_LINUX
        #if __arm__
            std::string command = "amixer -c0 sset ADC " + convertToStr(&inputGainValue) + "&";
            system(command.c_str());
        #elif USE_ALSA_API
            if (elem_capture)
            {
                long normalizedValue = (long) inputGainValue * max_capture / 100;
                int retval = snd_mixer_selem_set_capture_volume_all(elem_capture, normalizedValue);
                if (retval)
                    std::cout << retval << std::endl;
            }
        #endif
    #endif
#endif
}

void MainComponent::topButtonClicked()
{
#if FX_BOX
    enableReverb = topButton.getToggleState();
#elif CPANEL
    #if JUCE_LINUX && __arm__
        if (topButton.getToggleState())
        {
            std::string command = R"(amixer -c0 sset "ADC Left Input" "{VIN1P, VIN1M}[DIFF]" &)";
            system(command.c_str());
        }
        else
        {
            std::string command = R"(amixer -c0 sset "ADC Left Input" "VINL1[SE]" &)";
            system(command.c_str());
        }
    #endif
#endif
}

void MainComponent::bottomButtonClicked()
{
#if FX_BOX
    enableDelay = bottomButton.getToggleState();
#elif CPANEL
    #if JUCE_LINUX && __arm__
        if (bottomButton.getToggleState())
        {
            std::string command = R"(amixer -c0 sset "ADC Right Input" "{VIN2P, VIN2M}[DIFF]" &)";
            system(command.c_str());
            channel2Enabled = true;
        }
        else
        {
            std::string command = R"(amixer -c0 sset "ADC Right Input" "VINR2[SE]" &)";
            system(command.c_str());
            channel2Enabled = false;
        }
    #endif
#endif
}

void MainComponent::settingsButtonClicked()
{
    logoImageComponent.setToggleState(!logoImageComponent.getToggleState(), juce::dontSendNotification);
    if (logoImageComponent.getToggleState())
    {
        parameterSlider.setVisible(false);
        sliderLabel.setVisible(false);
        topButton.setVisible(false);
        topButtonLabel.setVisible(false);
        bottomButton.setVisible(false);
        bottomButtonLabel.setVisible(false);
        meter.setVisible(false);

        quitCPanelButton.setVisible(true);
        quitCPanelLabel.setVisible(true);
        enablePassthroughButton.setVisible(true);
        enablePassthroughLabel.setVisible(true);
        selector.setVisible(true);
    }
    else
    {
        parameterSlider.setVisible(true);
        sliderLabel.setVisible(true);
        topButton.setVisible(true);
        topButtonLabel.setVisible(true);
        bottomButton.setVisible(true);
        bottomButtonLabel.setVisible(true);
        meter.setVisible(true);

        quitCPanelButton.setVisible(false);
        quitCPanelLabel.setVisible(false);
        enablePassthroughButton.setVisible(false);
        enablePassthroughLabel.setVisible(false);
        selector.setVisible(false);
    }
}
} // namespace GuiApp
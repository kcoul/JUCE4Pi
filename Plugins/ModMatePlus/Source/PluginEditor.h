#pragma once

#include "PluginProcessor.h"
#include "UnidirectionalSlider.h"

class CCLabel : public juce::Label
{
private:
    int& ccVar;

public:
    CCLabel(int& ccVarRef, const juce::Colour& colour, const juce::Justification justification);

    void mouseDown(const juce::MouseEvent&) override;
    void textWasChanged() override;
};

class ModMateAudioProcessorEditor   : public juce::AudioProcessorEditor
                                    , public juce::ChangeListener
                                    , public juce::Button::Listener
{
public:
    ModMateAudioProcessorEditor (ModMateAudioProcessor&);
    ~ModMateAudioProcessorEditor();

    // Component
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;

    // ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    
    // Button::Listener
    void buttonClicked(juce::Button*) override;

private:
    ModMateAudioProcessor& processor;

    // Controls
    juce::DrawableButton aboutButton;
    juce::Label pbUpLabel, pbDownLabel;
    CCLabel modWheelLabel, wheel2Label, wheel4Label, wheel67Label;
    UnidirectionalSlider pbUpSlider, pbDownSlider, modWheelSlider, wheel2Slider, wheel4Slider, wheel67Slider;
    CCLabel cc1Label, cc2Label, cc4Label, cc67Label;
    UnidirectionalSlider cc1Slider, cc2Slider, cc4Slider, cc67Slider;
    juce::ToggleButton pbUp_cc1Btn, pbUp_cc2Btn, pbUp_cc4Btn, pbUp_cc67Btn;
    juce::ToggleButton pbDn_cc1Btn, pbDn_cc2Btn, pbDn_cc4Btn, pbDn_cc67Btn;
    juce::ToggleButton modW_cc1Btn, modW_cc2Btn, modW_cc4Btn, modW_cc67Btn;
    juce::ToggleButton wh2_cc1Btn, wh2_cc2Btn, wh2_cc4Btn, wh2_cc67Btn;
    juce::ToggleButton wh4_cc1Btn, wh4_cc2Btn, wh4_cc4Btn, wh4_cc67Btn;
    juce::ToggleButton wh67_cc1Btn, wh67_cc2Btn, wh67_cc4Btn, wh67_cc67Btn;

    // Data model: see corresponding variables in PluginProcessor class
    float pitchBendUp, pitchBendDown, modWheel, wheel2, wheel4, wheel67;
    float cc1, cc2, cc4, cc67;
    ControlBitmap pbUp, pbDown, wheel, ctrl2, ctrl4, ctrl67;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModMateAudioProcessorEditor)
};

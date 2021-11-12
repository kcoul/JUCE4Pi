/*
  ==============================================================================

    ControlPanel.h
    Created: 3 May 2020 9:32:52pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once

#if USING_CMAKE
#else
#include <JuceHeader.h>
#endif
#include "KnobPair.h"
#include "Params.h"
#include "LushLookAndFeel.h"
#include "SliderPair.h"
#include "EqPair.h"

//==============================================================================
/*
*/
class ControlPanel    : public Component
{
public:
    ControlPanel(AudioProcessorValueTreeState& s)
        : state(s),
        stereoPair(s, "Stereo", Params::idPan, "Pan", Params::idOffsetLR, "Offset"),
        eqPair(s, "EQ", Params::idLowPass, "Low", Params::idHighPass, "High"),
        modulatePair(s, "Modulate", Params::idModDepth, "Depth", Params::idModRate, "Rate"),
        feedbackPair(s, "Feedback", Params::idFeedbackDirect, "Direct", Params::idFeedbackCross, "Cross"),
        diffusePair(s, "Diffuse", Params::idTaps, "Taps", Params::idSpread, "Spread")
    {
        initSlider(delaySlider, Params::idDelay, delaySliderAttachment);
        initLabel(delayLabel, "Delay");
        
        addAndMakeVisible(stereoPair);
        addAndMakeVisible(eqPair);
        addAndMakeVisible(modulatePair);
        addAndMakeVisible(feedbackPair);
        addAndMakeVisible(diffusePair);
    }

    ~ControlPanel()
    {
        
    }

    void paint (Graphics& g) override
    {
        
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(LushLookAndFeel::padding);
        
        int pairWidth = ( bounds.getWidth() - LushLookAndFeel::padding * 2 ) / 3;
        int pairHeight = ( bounds.getHeight() - LushLookAndFeel::padding) / 2;
        
        auto topBounds = bounds.removeFromTop(pairHeight);
        stereoPair.setBounds(topBounds.removeFromLeft(pairWidth));
        feedbackPair.setBounds(topBounds.removeFromRight(pairWidth));
        topBounds.reduce(LushLookAndFeel::padding, 0);
        delayLabel.setBounds(topBounds.removeFromTop(35));
        delaySlider.setBounds(topBounds);
        
        bounds.removeFromTop(LushLookAndFeel::padding);
        modulatePair.setBounds(bounds.removeFromLeft(pairWidth));
        diffusePair.setBounds(bounds.removeFromRight(pairWidth));
        eqPair.setBounds(bounds.reduced(LushLookAndFeel::padding, 0));
        
    }

private:
    AudioProcessorValueTreeState& state;
    
    SliderPair stereoPair;
    EqPair eqPair;
    KnobPair modulatePair, feedbackPair, diffusePair;
    
    Slider delaySlider;
    Label delayLabel;
    
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    std::unique_ptr<SliderAttachment> delaySliderAttachment;
    
    void initSlider(Slider& slider, String paramId, std::unique_ptr<SliderAttachment>& attachment) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
        attachment.reset(new SliderAttachment(state, paramId, slider));
    }
    void initLabel(Label& label, String text) {
        addAndMakeVisible(label);
        label.setText(text, dontSendNotification);
        label.setJustificationType(Justification::centred);
        label.setFont(Font(26.0f));
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};

/*
  ==============================================================================

    DryWetControl.h
    Created: 3 May 2020 10:59:50pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once

#if USING_CMAKE
#else
#include <JuceHeader.h>
#endif
#include "LushLookAndFeel.h"

//==============================================================================
/*
*/
class DryWetControl    : public Component, Slider::Listener
{
public:
    DryWetControl(AudioProcessorValueTreeState& s, String dryParamId, String wetParamId)
        : state(s)
        , dryId(dryParamId)
        , wetId(wetParamId)
    {
        initSlider(drySlider, dryId, dryAttachment);
        initSlider(wetSlider, wetId, wetAttachment);
        
        initLabel(dryLabel, "Dry");
        initLabel(wetLabel, "Wet");
        initLabel(dryValue, "");
        initLabel(wetValue, "");
        dryValue.setFont(Font(14.0f));
        wetValue.setFont(Font(14.0f));
        
        dryLabel.attachToComponent(&drySlider, false);
        wetLabel.attachToComponent(&wetSlider, false);
        
        sliderValueChanged(&drySlider);
        
        drySlider.addListener(this);
        wetSlider.addListener(this);
    }

    ~DryWetControl()
    {
        drySlider.removeListener(this);
        wetSlider.removeListener(this);
    }

    void paint (Graphics& g) override
    {
        
    }

    void resized() override
    {
        auto labelHeight = 20;
        auto bounds = getLocalBounds();
        bounds.removeFromTop(labelHeight);
        auto dryBounds = bounds.removeFromLeft(getWidth()/2);
        
        dryValue.setBounds(dryBounds.removeFromBottom(labelHeight));
        drySlider.setBounds(dryBounds);
        
        wetValue.setBounds(bounds.removeFromBottom(labelHeight));
        wetSlider.setBounds(bounds);
    }
    
    void sliderValueChanged (Slider *slider) override {
        setGainText(dryValue, drySlider.getValue());
        setGainText(wetValue, wetSlider.getValue());
        repaint();
    }

private:
    AudioProcessorValueTreeState& state;
    String dryId, wetId;
    Slider drySlider, wetSlider;
    Label dryLabel, wetLabel, dryValue, wetValue;
    
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    std::unique_ptr<SliderAttachment> dryAttachment, wetAttachment;
    
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
        label.setFont(Font(16.0f));
    }
    void setGainText(Label& label, float db) {
        auto dbString = String::toDecimalStringWithSignificantFigures(db, 2);
        dbString += "dB";
        label.setText(dbString, dontSendNotification);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DryWetControl)
};

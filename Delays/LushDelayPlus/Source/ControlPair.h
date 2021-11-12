/*
  ==============================================================================

    ControlPair.h
    Created: 3 May 2020 9:32:42pm
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
class ControlPair    : public Component, Slider::Listener
{
public:
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    
    ControlPair(AudioProcessorValueTreeState& s, String title, String topParamId, String topName, String bottomParamId, String bottomName)
        : state(s),
        topId(topParamId),
        bottomId(bottomParamId)
    {
        initSlider(topSlider, topId, topAttachment);
        initSlider(bottomSlider, bottomId, bottomAttachment);
        
        titleLabel.setJustificationType(Justification::centred);
        initLabel(titleLabel, title);
        
        initLabel(topLabel, topName);
        topLabel.setJustificationType(Justification::bottomLeft);
        
        initLabel(topValue, "");
        topValue.setFont(Font(16.0f));
        topValue.setJustificationType(Justification::bottomRight);
        
        initLabel(bottomLabel, bottomName);
        bottomLabel.setJustificationType(Justification::bottomLeft);
        
        initLabel(bottomValue, "");
        bottomValue.setFont(Font(16.0f));
        bottomValue.setJustificationType(Justification::bottomRight);
        
        topSlider.addListener(this);
        bottomSlider.addListener(this);
        sliderValueChanged(&topSlider);
    }

    virtual ~ControlPair()
    {
        topSlider.removeListener(this);
        bottomSlider.removeListener(this);
    }

    void paint (Graphics& g) override
    {
        float cornerSize = 20.0f;
        g.setColour(LushLookAndFeel::colourPanel);
        g.fillRoundedRectangle(0.0f, 0.0f, getWidth(), getHeight(), cornerSize);
    }
    
    void sliderValueChanged (Slider *slider) override {
        topValue.setText(getTopText(topSlider.getValue()), dontSendNotification);
        bottomValue.setText(getBottomText(bottomSlider.getValue()), dontSendNotification);
        repaint();
    }
    
    virtual void resized() override{
        auto bounds = getLocalBounds().reduced(3);
        titleLabel.setBounds(bounds.removeFromTop(30));

        auto topBounds = bounds.removeFromTop(bounds.getHeight()/2);
        topSlider.setBounds(topBounds.removeFromLeft(topBounds.getWidth()/2));
        topLabel.setBounds(topBounds.removeFromTop(topBounds.getHeight()/2));
        topValue.setBounds(topBounds);

        bottomSlider.setBounds(bounds.removeFromRight(bounds.getWidth()/2));
        bottomLabel.setBounds(bounds.removeFromTop(bounds.getHeight()/2));
        bottomValue.setBounds(bounds);
    }
    
    virtual void setBounds(Rectangle<int> bounds, Label& topLabel, Label& bottomLabel, Slider& topValue, Slider& bottomValue) = 0;
    
protected:
    virtual void initSlider(Slider& slider, String paramId, std::unique_ptr<SliderAttachment>& attachment) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(Slider::LinearHorizontal);
        slider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
        attachment.reset(new SliderAttachment(state, paramId, slider));
    }
    virtual void initLabel(Label& label, String text) {
        addAndMakeVisible(label);
        label.setText(text, dontSendNotification);
        label.setFont(Font(24.0f));
    }
    String getTopText(float value){
        return String::toDecimalStringWithSignificantFigures(value, 2);
    }
    String getBottomText(float value){
        return String::toDecimalStringWithSignificantFigures(value, 2);
    }
    
    Slider topSlider, bottomSlider;
    Label titleLabel, topLabel, bottomLabel, topValue, bottomValue;

private:
    AudioProcessorValueTreeState& state;
    
    String topId, bottomId;
    std::unique_ptr<SliderAttachment> topAttachment, bottomAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPair)
};


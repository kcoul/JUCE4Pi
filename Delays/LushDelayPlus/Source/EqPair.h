/*
  ==============================================================================

    EqPair.h
    Created: 11 May 2020 8:00:22am
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once

#if USING_CMAKE
#else
#include <JuceHeader.h>
#endif
//==============================================================================
/*
*/
class EqPair    : public Component, Slider::Listener
{
public:
    EqPair(AudioProcessorValueTreeState& s, String title, String topParamId, String topName, String bottomParamId, String bottomName)
        : state(s),
        topId(topParamId),
        bottomId(bottomParamId)
    {
        initSlider(topSlider, topId, topAttachment);
        initSlider(bottomSlider, bottomId, bottomAttachment);
        
        titleLabel.setJustificationType(Justification::centred);
        initLabel(titleLabel, title);
        titleLabel.setFont(Font(24.0f));
        
        initLabel(topValue, "");
        topValue.setFont(Font(16.0f));
        topValue.setJustificationType(Justification::centredLeft);
        
        initLabel(bottomValue, "");
        bottomValue.setFont(Font(16.0f));
        bottomValue.setJustificationType(Justification::centredRight);
        
        topSlider.addListener(this);
        bottomSlider.addListener(this);
        
        sliderValueChanged(&topSlider);
        
        lowIcon.setHP(true);
        addAndMakeVisible(highIcon);
        addAndMakeVisible(lowIcon);
    }

    ~EqPair()
    {
        topSlider.removeListener(this);
        bottomSlider.removeListener(this);
    }

    void paint (Graphics& g) override
    {
        g.setColour(LushLookAndFeel::colourPanel);
        g.fillRoundedRectangle(0.0f, 0.0f, getWidth(), getHeight(), cornerSize);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(3);
        titleLabel.setBounds(bounds.removeFromTop(30));
        
        auto topBounds = bounds.removeFromTop(bounds.getHeight()/2);
        topSlider.setBounds(topBounds.removeFromBottom(topBounds.getHeight()/2));
        topBounds.removeFromTop(15);
        highIcon.setBounds(topBounds.removeFromRight(topBounds.getWidth()/2));
        topValue.setBounds(topBounds);
        
        bottomSlider.setBounds(bounds.removeFromBottom(bounds.getHeight()/2));
        bounds.removeFromTop(15);
        lowIcon.setBounds(bounds.removeFromLeft(bounds.getWidth()/2));
        bottomValue.setBounds(bounds);
    }
    
    void sliderValueChanged (Slider *slider) override {
        setText(topValue, topSlider.getValue());
        setText(bottomValue, bottomSlider.getValue());
        repaint();
    }

private:
    AudioProcessorValueTreeState& state;
    
    const float cornerSize = 20.0f;
    
    String topId, bottomId;
    
    Slider topSlider, bottomSlider;
    Label titleLabel, topValue, bottomValue;
    stm::FilterIcon highIcon, lowIcon;
    
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    std::unique_ptr<SliderAttachment> topAttachment, bottomAttachment;
    
    void initSlider(Slider& slider, String paramId, std::unique_ptr<SliderAttachment>& attachment) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(Slider::LinearHorizontal);
        slider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
        attachment.reset(new SliderAttachment(state, paramId, slider));
    }
    
    void initLabel(Label& label, String text) {
        addAndMakeVisible(label);
        label.setText(text, dontSendNotification);
        label.setFont(Font(18.0f));
    }
    
    void setText(Label& label, float value) {
        auto valueString = String::toDecimalStringWithSignificantFigures(value, 2) + "hz";
        label.setText(valueString, dontSendNotification);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqPair)
};

/*
  ==============================================================================

    KnobPair.h
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
#include "ControlPair.h"

//==============================================================================
/*
*/
class KnobPair    : public Component, Slider::Listener
{
public:
    KnobPair(AudioProcessorValueTreeState& s, String title, String topParamId, String topName, String bottomParamId, String bottomName)
        : state(s),
        topId(topParamId),
        bottomId(bottomParamId)
    {
        initSlider(topSlider, topId, topAttachment);
        initSlider(bottomSlider, bottomId, bottomAttachment);

        titleLabel.setJustificationType(Justification::centred);
        initLabel(titleLabel, title);
        titleLabel.setFont(Font(24.0f));

        initLabel(topLabel, topName);
        topLabel.setJustificationType(Justification::centredLeft);

        initLabel(topValue, "");
        topValue.setFont(Font(16.0f));
        topValue.setJustificationType(Justification::topLeft);

        initLabel(bottomLabel, bottomName);
        bottomLabel.setJustificationType(Justification::centredRight);

        initLabel(bottomValue, "");
        bottomValue.setFont(Font(16.0f));
        bottomValue.setJustificationType(Justification::topRight);

        topSlider.addListener(this);
        bottomSlider.addListener(this);

        sliderValueChanged(&topSlider);
    }

    ~KnobPair()
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
        topSlider.setBounds(topBounds.removeFromLeft(topBounds.getWidth()/2));
        topLabel.setBounds(topBounds.removeFromTop(topBounds.getHeight()/2));
        topValue.setBounds(topBounds);

        bottomSlider.setBounds(bounds.removeFromRight(bounds.getWidth()/2));
        bottomLabel.setBounds(bounds.removeFromTop(bounds.getHeight()/2));
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
    Label titleLabel, topLabel, bottomLabel, topValue, bottomValue;

    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    std::unique_ptr<SliderAttachment> topAttachment, bottomAttachment;

    void initSlider(Slider& slider, String paramId, std::unique_ptr<SliderAttachment>& attachment) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
        attachment.reset(new SliderAttachment(state, paramId, slider));
    }
    void initLabel(Label& label, String text) {
        addAndMakeVisible(label);
        label.setText(text, dontSendNotification);
        label.setFont(Font(18.0f));
    }

    void setText(Label& label, float value) {
        auto dbString = String::toDecimalStringWithSignificantFigures(value, 2);
        label.setText(dbString, dontSendNotification);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobPair)
};

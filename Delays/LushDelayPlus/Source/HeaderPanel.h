/*
  ==============================================================================

    HeaderPanel.h
    Created: 2 May 2020 7:31:30pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once

#if USING_CMAKE
#else
#include <JuceHeader.h>
#endif
#include "Params.h"
#include "LushLookAndFeel.h"
#include "DryWetControl.h"
#include "DelayVisualizer.h"

//==============================================================================
/*
*/
class HeaderPanel    : public Component
{
public:
    HeaderPanel(AudioProcessorValueTreeState& s)
        : state(s)
        , delayVisualizer(s)
        , dryWetControl(s, Params::idDry, Params::idWet)
    {
        addAndMakeVisible(dryWetControl);
        addAndMakeVisible(delayVisualizer);
        initLabel(titleLabel, "Lush Delay");
        initSlider(allpassSlider, Params::idAllpass, allpassAttachment);
    }

    ~HeaderPanel()
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll(LushLookAndFeel::colourHeader);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(LushLookAndFeel::padding);
        auto topBounds = bounds.removeFromTop(90);
        titleLabel.setBounds(topBounds.removeFromLeft(getWidth() * 0.7f));
        dryWetControl.setBounds(topBounds);
        
        bounds.removeFromBottom(30);
        delayVisualizer.setBounds(bounds.reduced(50, 0));
        
        bounds = getLocalBounds();
        bounds.removeFromTop(getHeight()*2/3);
        bounds.removeFromLeft(getWidth()*2/3);
        //allpassSlider.setBounds(bounds);
    }

private:
    AudioProcessorValueTreeState& state;
    
    DelayVisualizer delayVisualizer;
    DryWetControl dryWetControl;
    Label titleLabel;
    
    Slider allpassSlider;
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    std::unique_ptr<SliderAttachment> allpassAttachment;
    
    void initLabel(Label& label, String text) {
        addAndMakeVisible(label);
        label.setText(text, dontSendNotification);
        label.setJustificationType(Justification::centredLeft);
        label.setFont(Font("Dancing Script", 65.0f, Font::plain));
    }
    
    void initSlider(Slider& slider, String paramId, std::unique_ptr<SliderAttachment>& attachment) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
        attachment.reset(new SliderAttachment(state, paramId, slider));
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderPanel)
};

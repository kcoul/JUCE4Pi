#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class UnidirectionalSlider	: public juce::Component, public juce::ChangeBroadcaster
{
public:
    UnidirectionalSlider(juce::Colour c) : colour(c), value(0.0f) {}
    virtual ~UnidirectionalSlider() = default;

	// Component
	void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    //void mouseUp(const MouseEvent&) override;

	// UnidirectionalSlider
	void setValue(float v);
    float getValue() { return value; }

private:
    juce::Colour colour;
    float value;
};

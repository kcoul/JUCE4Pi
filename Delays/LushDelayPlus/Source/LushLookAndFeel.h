/*
  ==============================================================================

    LushLookAndFeel.h
    Created: 2 May 2020 7:24:23pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once

#include <BinaryData.h>

class LushLookAndFeel : public LookAndFeel_V4
{
public:
    LushLookAndFeel()
    {
        setColour (TextButton::textColourOffId, colourLight);
        setColour (TextButton::textColourOnId, colourLight);
        setColour (Label::textColourId, colourLight);
        
        montserrat = Typeface::createSystemTypefaceFor(BinaryData::MontserratRegular_ttf, BinaryData::MontserratRegular_ttfSize);
        dancingScript = Typeface::createSystemTypefaceFor(BinaryData::DancingScriptRegular_ttf, BinaryData::DancingScriptRegular_ttfSize);

        LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(montserrat);
    }
    
    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
    {
        auto shadowThickness = 4.0f;
        auto outlineThickness = 4.0f;
        auto radius0 = jmin (width / 2, height / 2) - 4.0f;
        auto radius1 = radius0 - shadowThickness;
        auto radius2 = radius1 - outlineThickness;
        auto centreX = x + width  * 0.5f;
        auto centreY = y + height * 0.5f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        auto shadowGradient = ColourGradient(Colours::black.withAlpha(0.7f), centreX, centreY, Colours::transparentBlack, centreX, centreY - radius0, true);
        g.setGradientFill(shadowGradient);
        g.fillAll();

        auto outlineRect = Rectangle<float>(radius1*2, radius1*2).withCentre({centreX, centreY});
        g.setColour( colourControlOutline );
        g.fillEllipse (outlineRect);
        
        auto innerRect = Rectangle<float>(radius2*2, radius2*2).withCentre({centreX, centreY});
        g.setColour( colourControl);
        g.fillEllipse (innerRect);
        
        //Draw pointer
        Path p;
        auto pointerLength = radius0 * 0.6f;
        auto pointerThickness = radius0 * 0.1f;
        p.addRectangle (-pointerThickness * 0.5f, -radius0, pointerThickness, pointerLength);
        p.applyTransform (AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (colourLight);
        g.fillPath (p);
    }
    
    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour, bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown) override
    {
        auto cornerSize = 6.0f;
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);
        
        if (button.getToggleState()){
            g.setColour (colourAccent);
            g.drawRoundedRectangle (bounds, cornerSize, 3.0f);
        }
    }
    
    Typeface::Ptr getTypefaceForFont (const Font& font) override
    {
        if (font.getTypefaceName() == "Dancing Script")
        {
            return dancingScript;
        }
        
        return montserrat;
    }

    inline static const Colour
        colourAccent = Colour::fromHSV(2.0f/360.0f, 0.47f, 0.48f, 1.0f),
        colourLight = Colour::fromHSV(0.0f, 0.0f, 0.8f, 1.0f),
        colourHeader = Colour::fromHSV(0.0f, 0.0f, 0.0f, 0.72f),
        colourPanel = Colour::fromHSV(0.0f, 0.0f, 1.0f, 0.1f),
        colourControl = Colour::fromHSV(0.0f, 0.0f, 0.45f, 1.0f),
        colourControlOutline = Colour::fromHSV(0.0f, 0.0f, 0.4f, 1.0f)
    ;
    
    static const int padding = 10;

private:
    Typeface::Ptr montserrat, dancingScript;
    
};

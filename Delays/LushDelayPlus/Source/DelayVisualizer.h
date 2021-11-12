/*
  ==============================================================================

    DelayVisualizer.h
    Created: 4 May 2020 10:14:38am
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once

#if USING_CMAKE
#include <BinaryData.h>
#else
#include <JuceHeader.h>
#endif
#include "LushLookAndFeel.h"
#include "Params.h"

//==============================================================================
/*
*/
class DelayVisualizer    : public Component, public stm::ParameterAttachment::Listener
{
public:
    DelayVisualizer(AudioProcessorValueTreeState& s)
        : crossFBAttachment(s, Params::idFeedbackCross)
        , directFBAttachment(s, Params::idFeedbackDirect)
        , delayAttachment(s, Params::idDelay)
        , offsetAttachment(s, Params::idOffsetLR)
        , panAttachment(s, Params::idPan)
    {
        crossFBAttachment.setListener(this);
        directFBAttachment.setListener(this);
        delayAttachment.setListener(this);
        offsetAttachment.setListener(this);
        panAttachment.setListener(this);
        
        crossFBsvg = Drawable::createFromImageData(BinaryData::VisualizerCrossFB_svg, BinaryData::VisualizerCrossFB_svgSize);
        directFBsvg = Drawable::createFromImageData(BinaryData::VisualizerDirectFB_svg, BinaryData::VisualizerDirectFB_svgSize);
        leftsvg = Drawable::createFromImageData(BinaryData::VisualizerLeft_svg, BinaryData::VisualizerLeft_svgSize);
        rightsvg = Drawable::createFromImageData(BinaryData::VisualizerRight_svg, BinaryData::VisualizerRight_svgSize);

        addAndMakeVisible(*crossFBsvg);
        addAndMakeVisible(*directFBsvg);
        addAndMakeVisible(*leftsvg);
        addAndMakeVisible(*rightsvg);
        
        updateComponents();
    }

    ~DelayVisualizer()
    {
        
    }

    void paint (Graphics& g) override
    {
        
    }
    
    void paintOverChildren(Graphics& g) override
    {
        g.setColour(LushLookAndFeel::colourAccent);
        g.fillRoundedRectangle(topLabelBounds.toFloat(), 5);
        g.fillRoundedRectangle(bottomLabelBounds.toFloat(), 5);
        g.setColour(LushLookAndFeel::colourLight);
        g.drawFittedText(leftDelayText, topLabelBounds, Justification::centred, 1);
        g.drawFittedText(rightDelayText, bottomLabelBounds, Justification::centred, 1);
        
    }

    void resized() override
    {
        updateBounds();
        
        auto boundsF = bounds.toFloat();
        
        crossFBsvg->setTransformToFit (boundsF, RectanglePlacement::stretchToFit);
        directFBsvg->setTransformToFit (boundsF, RectanglePlacement::stretchToFit);
        leftsvg->setTransformToFit (boundsF, RectanglePlacement::stretchToFit);
        rightsvg->setTransformToFit (boundsF, RectanglePlacement::stretchToFit);
    }
    
    void valueUpdated(stm::ParameterAttachment* attachment, float newValue) override {
        updateComponents();
    }

private:
    std::unique_ptr<Drawable> crossFBsvg, directFBsvg, leftsvg, rightsvg;
    //Label leftDelayLabel, rightDelayLabel;
    String leftDelayText, rightDelayText;
    float whRatio = 400.0f / 160.0f;
    Rectangle<int> bounds, topLabelBounds, bottomLabelBounds;
    
    Colour crossFBColour = Colours::white, directFBColour = Colours::white, leftColour = Colours::white, rightColour = Colours::white;
    stm::ParameterAttachment crossFBAttachment, directFBAttachment, delayAttachment, offsetAttachment, panAttachment;
    
    void updateBounds(){
        bounds = getLocalBounds();
        int height = getHeight();
        int width = getWidth();
        if ((float)width / (float)height > whRatio){
            //too much width
            width = height * whRatio;
            int excessWidth = bounds.getWidth() - width;
            bounds.reduce(excessWidth/2, 0);
        } else {
            //too much height
            height = width / whRatio;
            int excessHeight = bounds.getHeight() - height;
            bounds.reduce(0, excessHeight/2);
        }
        
        stm::DebugDisplay::set(1, "height: " + String(height));
        stm::DebugDisplay::set(2, "getHeight: " + String(getHeight()));
        stm::DebugDisplay::set(3, "width: " + String(width));
        stm::DebugDisplay::set(4, "getWidth: " + String(getWidth()));
        
        int labelHeight = 24;
        int labelWidth = 70;
        int reduceY = height * 0.24 - labelHeight/2;
        int reduceX = (width - labelWidth)/2;
        
        auto labelBounds = bounds.reduced(reduceX, reduceY);
        topLabelBounds = labelBounds.removeFromTop(labelHeight);
        bottomLabelBounds = labelBounds.removeFromBottom(labelHeight);
    }
    
    void updateComponents(){
        //------     UPDATE LABELS     -----------
        float leftDelay = delayAttachment.getValue();
        float rightDelay = delayAttachment.getValue();
        
        auto balance = stm::Balancer::getLinearCentered(offsetAttachment.getValue());
        leftDelay *= 2.0f - balance.right;
        rightDelay *= 2.0f - balance.left;
        
//        if ( offset < 0.5f ){
//            float leftRatio = 1.0f - offset * 2.0f;
//            leftDelay *= (1.0f + leftRatio);
//        } else {
//            float rightRatio = ( offset-0.5f ) * 2.0f;
//            rightDelay *= (1.0f + rightRatio);
//        }
        
        leftDelayText = String::toDecimalStringWithSignificantFigures (leftDelay, 3) + "ms";
        rightDelayText = String::toDecimalStringWithSignificantFigures (rightDelay, 3) + "ms";
        
//        leftDelayLabel.setText(String::toDecimalStringWithSignificantFigures (leftDelay, 2) + "ms", dontSendNotification);
//        rightDelayLabel.setText(String::toDecimalStringWithSignificantFigures (rightDelay, 2) + "ms", dontSendNotification);
        
        //-------     UPDATE SVGs     ------------
        balance = stm::Balancer::getLinearCentered(panAttachment.getValue());
        leftsvg->replaceColour(leftColour, rightColour.withAlpha(balance.left));
        leftColour = rightColour.withAlpha(balance.left);
        rightsvg->replaceColour(rightColour, rightColour.withAlpha(balance.right));
        rightColour = rightColour.withAlpha(balance.right);
        
        crossFBsvg->replaceColour(crossFBColour, crossFBColour.withAlpha(crossFBAttachment.getValue()));
        crossFBColour = crossFBColour.withAlpha(crossFBAttachment.getValue());
        directFBsvg->replaceColour(directFBColour, directFBColour.withAlpha(directFBAttachment.getValue()));
        directFBColour = directFBColour.withAlpha(directFBAttachment.getValue());
        
        //-------------
        repaint();
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayVisualizer)
};

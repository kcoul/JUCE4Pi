/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LushDelayAudioProcessorEditor::LushDelayAudioProcessorEditor (LushDelayAudioProcessor& p)
    : AudioProcessorEditor (&p),
    processor (p),
    headerPanel(p.state),
    controlPanel(p.state)
{
    width = height * whRatio;
    int totalWidth = width;
    if (debug) totalWidth += debugWidth;
    setSize(totalWidth , height);
    setResizeLimits(300, 500, 800, 1080);
    setLookAndFeel(&lushLookAndFeel);
    
    addAndMakeVisible(headerPanel);
    addAndMakeVisible(controlPanel);
    addAndMakeVisible(bypassedVeil);
    addAndMakeVisible(debugDisplay);
    
    addAndMakeVisible(matchedBypassButton);
    matchedBypassButton.setButtonText("MATCHED BYPASS");
    matchedBypassButton.setClickingTogglesState(true);
    matchedBypassButtonAttachment.reset(new ButtonAttachment(p.state, Params::idBypass, matchedBypassButton));
    matchedBypassButton.addListener(this);
    
    updateWindow();
}

LushDelayAudioProcessorEditor::~LushDelayAudioProcessorEditor()
{
    matchedBypassButton.removeListener(this);
    setLookAndFeel(nullptr);
}

//==============================================================================
void LushDelayAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(LushLookAndFeel::colourAccent);
}

void LushDelayAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    //Bypass veil
    bypassedVeil.setBounds(bounds);

    //Debug display
    if (debug){
        debugDisplay.setBounds(bounds.removeFromLeft(debugWidth));
    }
    
    //Header
    int headerHeight = headerRatio * bounds.getHeight();
    auto headerBounds = bounds.removeFromTop(headerHeight);
    headerPanel.setBounds(headerBounds);
    
    //Matched Bypass Button
    //it needs to be rendered here so it can be rendered on top of the bypass veil
    matchedBypassButton.setBounds(headerBounds.removeFromBottom(40).reduced(5));

    //Control panel
    controlPanel.setBounds(bounds);
}

void LushDelayAudioProcessorEditor::buttonClicked (Button *button)
{
    if (button == &matchedBypassButton){
        updateWindow();
    }
}

void LushDelayAudioProcessorEditor::updateWindow()
{
    bool isBypassed = matchedBypassButton.getToggleState();
    
    if (isBypassed) {
        bypassedVeil.setVisible(true);
    } else {
        bypassedVeil.setVisible(false);
    }
}

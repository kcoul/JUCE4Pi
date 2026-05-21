#include "PluginEditor.h"

GenisysPluginEditor::GenisysPluginEditor (GenisysPluginProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setSize (420, 280);

    hostEditor.setText (proc.targetHost);
    portEditor.setText  (juce::String (proc.targetPort));
    hostEditor.setJustification (juce::Justification::centredLeft);
    portEditor.setJustification (juce::Justification::centredLeft);
    portEditor.setInputRestrictions (5, "0123456789");

    connectBtn.onClick = [this] { rebuildConnection(); };

    addAndMakeVisible (hostLabel);
    addAndMakeVisible (hostEditor);
    addAndMakeVisible (portLabel);
    addAndMakeVisible (portEditor);
    addAndMakeVisible (connectBtn);
    addAndMakeVisible (gearLabel);

    gearLabel.setFont (juce::FontOptions (28.0f, juce::Font::bold));
    gearLabel.setJustificationType (juce::Justification::centred);

    for (int i = 0; i < AvasMapping::numCcMappings; ++i)
    {
        ccRows[i].label.setText (AvasMapping::ccMappings[i].label, juce::dontSendNotification);
        ccRows[i].valueLabel.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (ccRows[i].label);
        addAndMakeVisible (ccRows[i].valueLabel);
    }

    proc.onStateChanged = [this] { repaint(); };

    startTimerHz (20);
}

GenisysPluginEditor::~GenisysPluginEditor()
{
    proc.onStateChanged = nullptr;
    stopTimer();
}

void GenisysPluginEditor::resized()
{
    auto area = getLocalBounds().reduced (12);

    // Connection row
    auto connRow = area.removeFromTop (28);
    hostLabel.setBounds  (connRow.removeFromLeft (90));
    hostEditor.setBounds (connRow.removeFromLeft (130).reduced (2));
    portLabel.setBounds  (connRow.removeFromLeft (36));
    portEditor.setBounds (connRow.removeFromLeft (60).reduced (2));
    connectBtn.setBounds (connRow.reduced (2));

    area.removeFromTop (8);

    // Gear display
    gearLabel.setBounds (area.removeFromRight (80));

    // CC rows
    int rowH = 36;
    for (int i = 0; i < AvasMapping::numCcMappings; ++i)
    {
        auto row = area.removeFromTop (rowH);
        ccRows[i].label.setBounds      (row.removeFromLeft (120));
        ccRows[i].valueLabel.setBounds (row.removeFromRight (50));
        // The remaining strip in `row` is the bar area, painted manually
    }
}

void GenisysPluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds().reduced (12);
    area.removeFromTop (36 + 8);  // skip connection row

    auto gearArea = area.removeFromRight (80);

    // Gear label content
    int gear = proc.currentGear;
    gearLabel.setText (gear > 0 ? ("G" + juce::String (gear)) : "--",
                       juce::dontSendNotification);

    // CC bar rows
    int rowH = 36;
    for (int i = 0; i < AvasMapping::numCcMappings; ++i)
    {
        int cc = AvasMapping::ccMappings[i].ccNumber;
        float val = proc.lastCcValues[cc];

        auto row = area.removeFromTop (rowH);
        row.removeFromLeft (120);
        row.removeFromRight (50);
        row.reduce (4, 6);

        // Background
        g.setColour (juce::Colours::darkgrey.darker());
        g.fillRoundedRectangle (row.toFloat(), 3.0f);

        // Fill
        auto fill = row.withWidth (juce::roundToInt (row.getWidth() * val));
        g.setColour (juce::Colour (0xff00aaff).withAlpha (0.85f));
        g.fillRoundedRectangle (fill.toFloat(), 3.0f);

        // Value text
        ccRows[i].valueLabel.setText (
            juce::String (juce::roundToInt (val * 100)) + "%",
            juce::dontSendNotification);
    }
}

void GenisysPluginEditor::timerCallback()
{
    repaint();
}

void GenisysPluginEditor::rebuildConnection()
{
    proc.targetHost = hostEditor.getText().trim();
    proc.targetPort = portEditor.getText().getIntValue();
}

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryHelper.h"

CCLabel::CCLabel(int& ccVarRef, const juce::Colour& colour, const juce::Justification justification)
    : ccVar(ccVarRef)
{
    setColour(Label::textColourId, colour);
    setJustificationType(justification);
    setText("-1", juce::NotificationType::sendNotification);
    setEditable(false, true, true);
}

void CCLabel::mouseDown(const juce::MouseEvent& evt)
{
    if (evt.mods.isRightButtonDown() || evt.mods.isCtrlDown())
    {
        juce::PopupMenu menu;
        for (int cn = 0; cn < 128; cn++)
        {
            juce::String controllerNameString = "CC" + juce::String(cn);
            const char* controllerName = juce::MidiMessage::getControllerName(cn);
            if (controllerName)
                controllerNameString += " " + juce::String(controllerName);
            menu.addItem(cn + 1, controllerNameString);
        }
        menu.addItem(129, "Channel Pressure (mono aftertouch)");
        int id = menu.show();
        if (id)
        {
            setText(juce::String(id - 1), juce::NotificationType::sendNotification);
        }
    }
    else Label::mouseDown(evt);
}

void CCLabel::textWasChanged()
{
    juce::String newValueStr = getText();
    if (newValueStr.toLowerCase() == "at")
    {
        setText("AT", juce::NotificationType::dontSendNotification);
        ccVar = 128;
    }
    else
    {
        int newCC = newValueStr.trimCharactersAtStart("cC").getIntValue();
        if (newCC >= 0 && newCC <= 128) ccVar = newCC;
        else newCC = ccVar;
        juce::String lblText(newCC == 128 ? "AT" : "cc" + juce::String(newCC));
        setText(lblText, juce::NotificationType::dontSendNotification);
    }
}

ModMateAudioProcessorEditor::ModMateAudioProcessorEditor(ModMateAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
    , aboutButton(juce::String("aboutBtn"), juce::DrawableButton::ButtonStyle::ImageFitted)
    , modWheelLabel(p.cc1In, juce::Colours::hotpink, juce::Justification::bottomLeft)
    , wheel2Label(p.cc2In, juce::Colours::greenyellow, juce::Justification::bottomLeft)
    , wheel4Label(p.cc4In, juce::Colours::cyan, juce::Justification::bottomLeft)
    , wheel67Label(p.cc67In, juce::Colours::darkorange, juce::Justification::bottomLeft)
    , pbUpSlider(juce::Colours::lime)
    , pbDownSlider(juce::Colours::red)
    , modWheelSlider(juce::Colours::hotpink)
    , wheel2Slider(juce::Colours::greenyellow)
    , wheel4Slider(juce::Colours::cyan)
    , wheel67Slider(juce::Colours::darkorange)
    , cc1Label(p.cc1Out, juce::Colours::hotpink, juce::Justification::centredRight)
    , cc2Label(p.cc2Out, juce::Colours::greenyellow, juce::Justification::centredRight)
    , cc4Label(p.cc4Out, juce::Colours::cyan, juce::Justification::centredRight)
    , cc67Label(p.cc67Out, juce::Colours::darkorange, juce::Justification::centredRight)
    , cc1Slider(juce::Colours::hotpink)
    , cc2Slider(juce::Colours::greenyellow)
    , cc4Slider(juce::Colours::cyan)
    , cc67Slider(juce::Colours::darkorange)
{

    std::unique_ptr<juce::Drawable> icon(juce::Drawable::createFromImageData(getBinaryDataAssets().front().data,
                                                                             getBinaryDataAssets().front().size));
    icon->replaceColour(juce::Colours::black, juce::Colours::aliceblue);
    aboutButton.setImages(icon.get());
    addAndMakeVisible(aboutButton);
    aboutButton.onClick = [] {
#ifdef BRANDED_VERSION
      juce::URL url(BRANDED_INFO_URL);
#else
      juce::URL url("https://github.com/getdunne/modmate");
#endif
        url.launchInDefaultBrowser();
    };

    addAndMakeVisible(pbUpLabel);
    pbUpLabel.setJustificationType(juce::Justification::bottomLeft);
    pbUpLabel.setText("pbUp", juce::NotificationType::dontSendNotification);

    addAndMakeVisible(pbDownLabel);
    pbDownLabel.setJustificationType(juce::Justification::bottomLeft);
    pbDownLabel.setText("pbDn", juce::NotificationType::dontSendNotification);

    addAndMakeVisible(modWheelLabel);
    addAndMakeVisible(wheel2Label);
    addAndMakeVisible(wheel4Label);
    addAndMakeVisible(wheel67Label);

    addAndMakeVisible(pbUpSlider);
    addAndMakeVisible(pbDownSlider);
    addAndMakeVisible(modWheelSlider);
    addAndMakeVisible(wheel2Slider);
    addAndMakeVisible(wheel4Slider);
    addAndMakeVisible(wheel67Slider);

    addAndMakeVisible(cc1Label);
    addAndMakeVisible(cc2Label);
    addAndMakeVisible(cc4Label);
    addAndMakeVisible(cc67Label);

    addAndMakeVisible(cc1Slider);
    addAndMakeVisible(cc2Slider);
    addAndMakeVisible(cc4Slider);
    addAndMakeVisible(cc67Slider);

    addAndMakeVisible(pbUp_cc1Btn);
    addAndMakeVisible(pbUp_cc2Btn);
    addAndMakeVisible(pbUp_cc4Btn);
    addAndMakeVisible(pbUp_cc67Btn);

    addAndMakeVisible(pbDn_cc1Btn);
    addAndMakeVisible(pbDn_cc2Btn);
    addAndMakeVisible(pbDn_cc4Btn);
    addAndMakeVisible(pbDn_cc67Btn);

    addAndMakeVisible(modW_cc1Btn);
    addAndMakeVisible(modW_cc2Btn);
    addAndMakeVisible(modW_cc4Btn);
    addAndMakeVisible(modW_cc67Btn);
    
    addAndMakeVisible(wh2_cc1Btn);
    addAndMakeVisible(wh2_cc2Btn);
    addAndMakeVisible(wh2_cc4Btn);
    addAndMakeVisible(wh2_cc67Btn);

    addAndMakeVisible(wh4_cc1Btn);
    addAndMakeVisible(wh4_cc2Btn);
    addAndMakeVisible(wh4_cc4Btn);
    addAndMakeVisible(wh4_cc67Btn);

    addAndMakeVisible(wh67_cc1Btn);
    addAndMakeVisible(wh67_cc2Btn);
    addAndMakeVisible(wh67_cc4Btn);
    addAndMakeVisible(wh67_cc67Btn);

    pitchBendUp = pitchBendDown = modWheel = -1.0f;
    wheel2 = wheel4 = wheel67 = -1.0f;
    cc1 = cc2 = cc4 = cc67 = -1.0f;
    pbUp.byteValue = pbDown.byteValue = wheel.byteValue = -1;
    ctrl2.byteValue = ctrl4.byteValue = ctrl67.byteValue = -1;
    changeListenerCallback(nullptr);

    processor.addChangeListener(this);

    pbUpSlider.addChangeListener(this);
    pbDownSlider.addChangeListener(this);
    modWheelSlider.addChangeListener(this);
    wheel2Slider.addChangeListener(this);
    wheel4Slider.addChangeListener(this);
    wheel67Slider.addChangeListener(this);

    cc1Slider.addChangeListener(this);
    cc2Slider.addChangeListener(this);
    cc4Slider.addChangeListener(this);
    cc67Slider.addChangeListener(this);
    
    pbUp_cc1Btn.addListener(this);
    pbUp_cc2Btn.addListener(this);
    pbUp_cc4Btn.addListener(this);
    pbUp_cc67Btn.addListener(this);

    pbDn_cc1Btn.addListener(this);
    pbDn_cc2Btn.addListener(this);
    pbDn_cc4Btn.addListener(this);
    pbDn_cc67Btn.addListener(this);
    
    modW_cc1Btn.addListener(this);
    modW_cc2Btn.addListener(this);
    modW_cc4Btn.addListener(this);
    modW_cc67Btn.addListener(this);
    
    wh2_cc1Btn.addListener(this);
    wh2_cc2Btn.addListener(this);
    wh2_cc4Btn.addListener(this);
    wh2_cc67Btn.addListener(this);

    wh4_cc1Btn.addListener(this);
    wh4_cc2Btn.addListener(this);
    wh4_cc4Btn.addListener(this);
    wh4_cc67Btn.addListener(this);

    wh67_cc1Btn.addListener(this);
    wh67_cc2Btn.addListener(this);
    wh67_cc4Btn.addListener(this);
    wh67_cc67Btn.addListener(this);

#ifdef BRANDED_VERSION
    setSize(722, 288);
#else
    setSize(650, 288);
#endif
}

ModMateAudioProcessorEditor::~ModMateAudioProcessorEditor()
{
    processor.removeChangeListener(this);
}

void ModMateAudioProcessorEditor::paint (juce::Graphics& g)
{
#ifdef BRANDED_VERSION
    juce::Image background = juce::ImageCache::getFromMemory(BinaryData::Background_png, BinaryData::Background_pngSize);
    g.drawImageAt(background, 0, 0);

    if (processor.isVST())
    {
        g.setColour(juce::Colours::lightgrey);
        g.setFont(10);
        g.drawText("VST PlugIn Technology by Steinberg Media Technologies",
                   juce::Rectangle<float>(440, 12, 250, 10), juce::Justification::left);
    }
#else
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
#endif
}

void ModMateAudioProcessorEditor::resized()
{
    auto aboutBox = getLocalBounds().reduced(2).removeFromTop(24).removeFromRight(24);
    aboutButton.setBounds(aboutBox);

    auto bounds = getLocalBounds().reduced(10, 0);
#ifdef BRANDED_VERSION
    bounds.removeFromLeft(72);
#endif

    auto column = bounds.removeFromLeft(50);
    pbUpLabel.setBounds(column.removeFromTop(26).withX(column.getX() - 8));
    column.removeFromTop(2);
    pbUpSlider.setBounds(column.removeFromLeft(20).reduced(0, 6));
    int colHeight = column.getHeight();
    pbUp_cc1Btn.setBounds(column.removeFromTop(colHeight / 4));
    pbUp_cc2Btn.setBounds(column.removeFromTop(colHeight / 4));
    pbUp_cc4Btn.setBounds(column.removeFromTop(colHeight / 4));
    pbUp_cc67Btn.setBounds(column);

    column = bounds.removeFromLeft(50);
    pbDownLabel.setBounds(column.removeFromTop(26).withX(column.getX() - 8));
    column.removeFromTop(2);
    pbDownSlider.setBounds(column.removeFromLeft(20).reduced(0, 6));
    pbDn_cc1Btn.setBounds(column.removeFromTop(colHeight / 4));
    pbDn_cc2Btn.setBounds(column.removeFromTop(colHeight / 4));
    pbDn_cc4Btn.setBounds(column.removeFromTop(colHeight / 4));
    pbDn_cc67Btn.setBounds(column);

    column = bounds.removeFromLeft(50);
    modWheelLabel.setBounds(column.removeFromTop(26).withX(column.getX() - 8));
    column.removeFromTop(2);
    modWheelSlider.setBounds(column.removeFromLeft(20).reduced(0, 6));
    modW_cc1Btn.setBounds(column.removeFromTop(colHeight / 4));
    modW_cc2Btn.setBounds(column.removeFromTop(colHeight / 4));
    modW_cc4Btn.setBounds(column.removeFromTop(colHeight / 4));
    modW_cc67Btn.setBounds(column);

    column = bounds.removeFromLeft(50);
    wheel2Label.setBounds(column.removeFromTop(26).withX(column.getX() - 8));
    column.removeFromTop(2);
    wheel2Slider.setBounds(column.removeFromLeft(20).reduced(0, 6));
    wh2_cc1Btn.setBounds(column.removeFromTop(colHeight / 4));
    wh2_cc2Btn.setBounds(column.removeFromTop(colHeight / 4));
    wh2_cc4Btn.setBounds(column.removeFromTop(colHeight / 4));
    wh2_cc67Btn.setBounds(column);

    column = bounds.removeFromLeft(50);
    wheel4Label.setBounds(column.removeFromTop(26).withX(column.getX() - 8));
    column.removeFromTop(2);
    wheel4Slider.setBounds(column.removeFromLeft(20).reduced(0, 6));
    wh4_cc1Btn.setBounds(column.removeFromTop(colHeight / 4));
    wh4_cc2Btn.setBounds(column.removeFromTop(colHeight / 4));
    wh4_cc4Btn.setBounds(column.removeFromTop(colHeight / 4));
    wh4_cc67Btn.setBounds(column);

    column = bounds.removeFromLeft(50);
    wheel67Label.setBounds(column.removeFromTop(26).withX(column.getX() - 8));
    column.removeFromTop(2);
    wheel67Slider.setBounds(column.removeFromLeft(20).reduced(0, 6));
    wh67_cc1Btn.setBounds(column.removeFromTop(colHeight / 4));
    wh67_cc2Btn.setBounds(column.removeFromTop(colHeight / 4));
    wh67_cc4Btn.setBounds(column.removeFromTop(colHeight / 4));
    wh67_cc67Btn.setBounds(column);

    column = bounds.removeFromLeft(50);
    column.removeFromTop(28);
    cc1Label.setBounds(column.removeFromTop(colHeight / 4));
    cc2Label.setBounds(column.removeFromTop(colHeight / 4));
    cc4Label.setBounds(column.removeFromTop(colHeight / 4));
    cc67Label.setBounds(column);

    bounds.removeFromTop(28);
    auto row = bounds.removeFromTop(colHeight / 4); row.reduce(0, 8);
    cc1Slider.setBounds(row);
    row = bounds.removeFromTop(colHeight / 4); row.reduce(0, 8);
    cc2Slider.setBounds(row);
    row = bounds.removeFromTop(colHeight / 4); row.reduce(0, 8);
    cc4Slider.setBounds(row);
    row = bounds; row.reduce(0, 8);
    cc67Slider.setBounds(row);
}

void ModMateAudioProcessorEditor::mouseDown(const juce::MouseEvent& /*evt*/)
{
#ifdef BRANDED_VERSION
    if (evt.getPosition().getX() < 72 && evt.getPosition().getY() < 72)
    {
        juce::URL url(BRANDING_URL);
        url.launchInDefaultBrowser();
    }
#endif
}

void ModMateAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* sender)
{
    if (sender == (juce::ChangeBroadcaster*)&pbUpSlider)
    {
        pitchBendUp = pbUpSlider.getValue();
        processor.pbUpChange(pitchBendUp);
    }
    else if (sender == (juce::ChangeBroadcaster*)&pbDownSlider)
    {
        pitchBendDown = pbDownSlider.getValue();
        processor.pbDownChange(pitchBendDown);
    }
    else if (sender == (juce::ChangeBroadcaster*)&modWheelSlider)
    {
        modWheel = modWheelSlider.getValue();
        processor.modWheelChange(modWheel);
    }
    else if (sender == (juce::ChangeBroadcaster*)&wheel2Slider)
    {
        wheel2 = wheel2Slider.getValue();
        processor.wheel2Change(wheel2);
    }
    else if (sender == (juce::ChangeBroadcaster*)&wheel4Slider)
    {
        wheel4 = wheel4Slider.getValue();
        processor.wheel4Change(wheel4);
    }
    else if (sender == (juce::ChangeBroadcaster*)&wheel67Slider)
    {
        wheel67 = wheel67Slider.getValue();
        processor.wheel67Change(wheel67);
    }
    else if (sender == (juce::ChangeBroadcaster*)&cc1Slider)
    {
        cc1 = cc1Slider.getValue();
        processor.cc1Change(cc1);
    }
    else if (sender == (juce::ChangeBroadcaster*)&cc2Slider)
    {
        cc2 = cc2Slider.getValue();
        processor.cc2Change(cc2);
    }
    else if (sender == (juce::ChangeBroadcaster*)&cc4Slider)
    {
        cc4 = cc4Slider.getValue();
        processor.cc4Change(cc4);
    }
    else if (sender == (juce::ChangeBroadcaster*)&cc67Slider)
    {
        cc67 = cc67Slider.getValue();
        processor.cc67Change(cc67);
    }
    else  // sender is nullptr (called on creation) or &processor
    {
        if (pitchBendUp != processor.pitchBendUp)
        {
            pitchBendUp = processor.pitchBendUp;
            pbUpSlider.setValue(pitchBendUp);
        }
        if (pitchBendDown != processor.pitchBendDown)
        {
            pitchBendDown = processor.pitchBendDown;
            pbDownSlider.setValue(pitchBendDown);
        }
        if (modWheel != processor.modWheel)
        {
            modWheel = processor.modWheel;
            modWheelSlider.setValue(modWheel);
        }
        if (wheel2 != processor.wheel2)
        {
            wheel2 = processor.wheel2;
            wheel2Slider.setValue(wheel2);
        }
        if (wheel4 != processor.wheel4)
        {
            wheel4 = processor.wheel4;
            wheel4Slider.setValue(wheel4);
        }
        if (wheel67 != processor.wheel67)
        {
            wheel67 = processor.wheel67;
            wheel67Slider.setValue(wheel67);
        }
        if (cc1 != processor.cc1)
        {
            cc1 = processor.cc1;
            cc1Slider.setValue(cc1);
        }
        if (cc2 != processor.cc2)
        {
            cc2 = processor.cc2;
            cc2Slider.setValue(cc2);
        }
        if (cc4 != processor.cc4)
        {
            cc4 = processor.cc4;
            cc4Slider.setValue(cc4);
        }
        if (cc67 != processor.cc67)
        {
            cc67 = processor.cc67;
            cc67Slider.setValue(cc67);
        }
        if (sender == nullptr || processor.presetLoaded || pbUp.byteValue != processor.pbUp.byteValue)
        {
            pbUp.byteValue = processor.pbUp.byteValue;
            pbUp_cc1Btn.setToggleState(pbUp.bits.cc1, juce::NotificationType::dontSendNotification);
            pbUp_cc2Btn.setToggleState(pbUp.bits.cc2, juce::NotificationType::dontSendNotification);
            pbUp_cc4Btn.setToggleState(pbUp.bits.cc4, juce::NotificationType::dontSendNotification);
            pbUp_cc67Btn.setToggleState(pbUp.bits.cc67, juce::NotificationType::dontSendNotification);
        }
        if (sender == nullptr || processor.presetLoaded || pbDown.byteValue != processor.pbDown.byteValue)
        {
            pbDown.byteValue = processor.pbDown.byteValue;
            pbDn_cc1Btn.setToggleState(pbDown.bits.cc1, juce::NotificationType::dontSendNotification);
            pbDn_cc2Btn.setToggleState(pbDown.bits.cc2, juce::NotificationType::dontSendNotification);
            pbDn_cc4Btn.setToggleState(pbDown.bits.cc4, juce::NotificationType::dontSendNotification);
            pbDn_cc67Btn.setToggleState(pbDown.bits.cc67, juce::NotificationType::dontSendNotification);
        }
        if (sender == nullptr || processor.presetLoaded || wheel.byteValue != processor.wheel.byteValue)
        {
            wheel.byteValue = processor.wheel.byteValue;
            modW_cc1Btn.setToggleState(wheel.bits.cc1, juce::NotificationType::dontSendNotification);
            modW_cc2Btn.setToggleState(wheel.bits.cc2, juce::NotificationType::dontSendNotification);
            modW_cc4Btn.setToggleState(wheel.bits.cc4, juce::NotificationType::dontSendNotification);
            modW_cc67Btn.setToggleState(wheel.bits.cc67, juce::NotificationType::dontSendNotification);
        }
        if (sender == nullptr || processor.presetLoaded || ctrl2.byteValue != processor.ctrl2.byteValue)
        {
            ctrl2.byteValue = processor.ctrl2.byteValue;
            wh2_cc1Btn.setToggleState(ctrl2.bits.cc1, juce::NotificationType::dontSendNotification);
            wh2_cc2Btn.setToggleState(ctrl2.bits.cc2, juce::NotificationType::dontSendNotification);
            wh2_cc4Btn.setToggleState(ctrl2.bits.cc4, juce::NotificationType::dontSendNotification);
            wh2_cc67Btn.setToggleState(ctrl2.bits.cc67, juce::NotificationType::dontSendNotification);
        }
        if (sender == nullptr || processor.presetLoaded || ctrl4.byteValue != processor.ctrl4.byteValue)
        {
            ctrl4.byteValue = processor.ctrl4.byteValue;
            wh4_cc1Btn.setToggleState(ctrl4.bits.cc1, juce::NotificationType::dontSendNotification);
            wh4_cc2Btn.setToggleState(ctrl4.bits.cc2, juce::NotificationType::dontSendNotification);
            wh4_cc4Btn.setToggleState(ctrl4.bits.cc4, juce::NotificationType::dontSendNotification);
            wh4_cc67Btn.setToggleState(ctrl4.bits.cc67, juce::NotificationType::dontSendNotification);
        }
        if (sender == nullptr || processor.presetLoaded || ctrl67.byteValue != processor.ctrl67.byteValue)
        {
            ctrl67.byteValue = processor.ctrl67.byteValue;
            wh67_cc1Btn.setToggleState(ctrl67.bits.cc1, juce::NotificationType::dontSendNotification);
            wh67_cc2Btn.setToggleState(ctrl67.bits.cc2, juce::NotificationType::dontSendNotification);
            wh67_cc4Btn.setToggleState(ctrl67.bits.cc4, juce::NotificationType::dontSendNotification);
            wh67_cc67Btn.setToggleState(ctrl67.bits.cc67, juce::NotificationType::dontSendNotification);
        }
        if (processor.presetLoaded)
        {
            modWheelLabel.setText("-1", juce::NotificationType::sendNotification);
            wheel2Label.setText("-1", juce::NotificationType::sendNotification);
            wheel4Label.setText("-1", juce::NotificationType::sendNotification);
            wheel67Label.setText("-1", juce::NotificationType::sendNotification);
            cc1Label.setText("-1", juce::NotificationType::sendNotification);
            cc2Label.setText("-1", juce::NotificationType::sendNotification);
            cc4Label.setText("-1", juce::NotificationType::sendNotification);
            cc67Label.setText("-1", juce::NotificationType::sendNotification);
            processor.presetLoaded = false;
        }
    }
}

void ModMateAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &pbUp_cc1Btn) pbUp.bits.cc1 = processor.pbUp.bits.cc1 = button->getToggleState();
    if (button == &pbUp_cc2Btn) pbUp.bits.cc2 = processor.pbUp.bits.cc2 = button->getToggleState();
    if (button == &pbUp_cc4Btn) pbUp.bits.cc4 = processor.pbUp.bits.cc4 = button->getToggleState();
    if (button == &pbUp_cc67Btn) pbUp.bits.cc67 = processor.pbUp.bits.cc67 = button->getToggleState();

    if (button == &pbDn_cc1Btn) pbDown.bits.cc1 = processor.pbDown.bits.cc1 = button->getToggleState();
    if (button == &pbDn_cc2Btn) pbDown.bits.cc2 = processor.pbDown.bits.cc2 = button->getToggleState();
    if (button == &pbDn_cc4Btn) pbDown.bits.cc4 = processor.pbDown.bits.cc4 = button->getToggleState();
    if (button == &pbDn_cc67Btn) pbDown.bits.cc67 = processor.pbDown.bits.cc67 = button->getToggleState();

    if (button == &modW_cc1Btn) wheel.bits.cc1 = processor.wheel.bits.cc1 = button->getToggleState();
    if (button == &modW_cc2Btn) wheel.bits.cc2 = processor.wheel.bits.cc2 = button->getToggleState();
    if (button == &modW_cc4Btn) wheel.bits.cc4 = processor.wheel.bits.cc4 = button->getToggleState();
    if (button == &modW_cc67Btn) wheel.bits.cc67 = processor.wheel.bits.cc67 = button->getToggleState();

    if (button == &wh2_cc1Btn) ctrl2.bits.cc1 = processor.ctrl2.bits.cc1 = button->getToggleState();
    if (button == &wh2_cc2Btn) ctrl2.bits.cc2 = processor.ctrl2.bits.cc2 = button->getToggleState();
    if (button == &wh2_cc4Btn) ctrl2.bits.cc4 = processor.ctrl2.bits.cc4 = button->getToggleState();
    if (button == &wh2_cc67Btn) ctrl2.bits.cc67 = processor.ctrl2.bits.cc67 = button->getToggleState();

    if (button == &wh4_cc1Btn) ctrl4.bits.cc1 = processor.ctrl4.bits.cc1 = button->getToggleState();
    if (button == &wh4_cc2Btn) ctrl4.bits.cc2 = processor.ctrl4.bits.cc2 = button->getToggleState();
    if (button == &wh4_cc4Btn) ctrl4.bits.cc4 = processor.ctrl4.bits.cc4 = button->getToggleState();
    if (button == &wh4_cc67Btn) ctrl4.bits.cc67 = processor.ctrl4.bits.cc67 = button->getToggleState();

    if (button == &wh67_cc1Btn) ctrl67.bits.cc1 = processor.ctrl67.bits.cc1 = button->getToggleState();
    if (button == &wh67_cc2Btn) ctrl67.bits.cc2 = processor.ctrl67.bits.cc2 = button->getToggleState();
    if (button == &wh67_cc4Btn) ctrl67.bits.cc4 = processor.ctrl67.bits.cc4 = button->getToggleState();
    if (button == &wh67_cc67Btn) ctrl67.bits.cc67 = processor.ctrl67.bits.cc67 = button->getToggleState();
}

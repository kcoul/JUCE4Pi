#include "PluginProcessor.h"
#include "PluginEditor.h"

ModMateAudioProcessor::ModMateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
    #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
#else
                      //FIXME: This line should not be needed in a pure MIDI effect, but there are
                      //FIXME: two issues where processBlock never gets called without it in standalones
                      //FIXME: Over here in ModMatePlus, we instead see that we don't get the callback
                      //FIXME: unless we set the audio to Jack, which we can't do unless we have audio I/O
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
)
#endif
{
    pitchBendUp = pitchBendDown = modWheel = 0.0f;
    wheel2 = wheel4 = wheel67 = 0.0f;
    cc1 = cc2 = cc4 = cc67 = 0.0f;
    pbUpChanged = pbDownChanged = modWheelChanged = false;
    wheel2Changed = wheel4Changed = wheel67Changed = false;
    cc1Changed = cc2Changed = cc4Changed = cc67Changed = false;
    presetLoaded = false;

    cc1In = cc1Out = 1;
    cc2In = cc2Out = 2;
    cc4In = cc4Out = 4;
    cc67In = cc67Out = 67;

    pbUp.byteValue = 0;
    pbDown.byteValue = 0;
    wheel.byteValue = 0;
    ctrl2.byteValue = 0;
    ctrl4.byteValue = 0;
    ctrl67.byteValue = 0;

    //Hardwire some params for the headless version
    //Note this won't work in a standalone plugin
    //where JUCE automatically creates a window on our behalf
    if (juce::Desktop::getInstance().isHeadless())
    {
        wheel.bits.cc1 = true;
        wheel.bits.cc2 = true;
        wheel.bits.cc4 = true;
        wheel.bits.cc67 = true;
    }

}

ModMateAudioProcessor::~ModMateAudioProcessor()
{
}

const juce::String ModMateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ModMateAudioProcessor::isVST() const
{
    return wrapperType == WrapperType::wrapperType_VST ||
           wrapperType == WrapperType::wrapperType_VST3;
}

bool ModMateAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ModMateAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ModMateAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ModMateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ModMateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int ModMateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ModMateAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const juce::String ModMateAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void ModMateAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
}

void ModMateAudioProcessor::prepareToPlay (double /*sampleRate*/, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ModMateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ModMateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
#endif
}
#endif

void ModMateAudioProcessor::pbUpChange(float ccVal)
{
    if (ccVal != pitchBendUp)
    {
        pitchBendUp = ccVal;
        pbUpChanged = true;
    }
}

void ModMateAudioProcessor::pbDownChange(float ccVal)
{
    if (ccVal != pitchBendDown)
    {
        pitchBendDown = ccVal;
        pbDownChanged = true;
    }
}

void ModMateAudioProcessor::modWheelChange(float ccVal)
{
    if (ccVal != modWheel)
    {
        modWheel = ccVal;
        modWheelChanged = true;
    }
}

void ModMateAudioProcessor::wheel2Change(float ccVal)
{
    if (ccVal != wheel2)
    {
        wheel2 = ccVal;
        wheel2Changed = true;
    }
}

void ModMateAudioProcessor::wheel4Change(float ccVal)
{
    if (ccVal != wheel4)
    {
        wheel4 = ccVal;
        wheel4Changed = true;
    }
}

void ModMateAudioProcessor::wheel67Change(float ccVal)
{
    if (ccVal != wheel67)
    {
        wheel67 = ccVal;
        wheel67Changed = true;
    }
}

void ModMateAudioProcessor::cc1Change(float ccVal)
{
    if (ccVal != cc1)
    {
        cc1 = ccVal;
        cc1Changed = true;
    }
}

void ModMateAudioProcessor::cc2Change(float ccVal)
{
    if (ccVal != cc2)
    {
        cc2 = ccVal;
        cc2Changed = true;
    }
}

void ModMateAudioProcessor::cc4Change(float ccVal)
{
    if (ccVal != cc4)
    {
        cc4 = ccVal;
        cc4Changed = true;
    }
}

void ModMateAudioProcessor::cc67Change(float ccVal)
{
    if (ccVal != cc67)
    {
        cc67 = ccVal;
        cc67Changed = true;
    }
}

void ModMateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // the audio buffer in a midi effect will have zero channels!
    jassert(buffer.getNumChannels() == 0);

    juce::MidiBuffer midiOut;

    float CC1 = cc1;
    float CC2 = cc2;
    float CC4 = cc4;
    float CC67 = cc67;

    bool somethingChanged = false;

    if (pbUpChanged)
    {
        pbUpChanged = false;
        if (pbUp.byteValue != 0)
        {
            int cval = int(pitchBendUp * 127 + 0.5f);
            if (pbUp.bits.cc1)
            {
                cc1 = pitchBendUp;
                if (cc1Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc1Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (pbUp.bits.cc2)
            {
                cc2 = pitchBendUp;
                if (cc2Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc2Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (pbUp.bits.cc4)
            {
                cc4 = pitchBendUp;
                if (cc4Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc4Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (pbUp.bits.cc67)
            {
                cc67 = pitchBendUp;
                if (cc67Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc67Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            sendChangeMessage();
        }
        else midiOut.addEvent(juce::MidiMessage::pitchWheel(1, 8192 + int(pitchBendUp * 8191)), 0);
    }
    if (pbDownChanged)
    {
        somethingChanged = true;
        pbDownChanged = false;
        if (pbDown.byteValue != 0)
        {
            int cval = int(pitchBendDown * 127 + 0.5f);
            if (pbDown.bits.cc1)
            {
                cc1 = pitchBendDown;
                if (cc1Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc1Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (pbDown.bits.cc2)
            {
                cc2 = pitchBendDown;
                if (cc2Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc2Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (pbDown.bits.cc4)
            {
                cc4 = pitchBendDown;
                if (cc4Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc4Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (pbDown.bits.cc67)
            {
                cc67 = pitchBendDown;
                if (cc67Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc67Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            sendChangeMessage();
        }
        else midiOut.addEvent(juce::MidiMessage::pitchWheel(1, 8192 - int(pitchBendDown * 8192)), 0);
    }
    if (modWheelChanged)
    {
        modWheelChanged = false;
        int cval = int(modWheel * 127 + 0.5f);
        if (wheel.byteValue != 0)
        {
            if (wheel.bits.cc1)
            {
                cc1 = modWheel;
                if (cc1Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc1Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (wheel.bits.cc2)
            {
                cc2 = modWheel;
                if (cc2Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc2Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (wheel.bits.cc4)
            {
                cc4 = modWheel;
                if (cc4Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc4Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (wheel.bits.cc67)
            {
                cc67 = modWheel;
                if (cc67Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc67Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            sendChangeMessage();
        }
        else midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc1In, cval), 0);
    }
    if (wheel2Changed)
    {
        wheel2Changed = false;
        int cval = int(wheel2 * 127 + 0.5f);
        if (ctrl2.byteValue != 0)
        {
            if (ctrl2.bits.cc1)
            {
                cc1 = wheel2;
                if (cc1Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc1Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (ctrl2.bits.cc2)
            {
                cc2 = wheel2;
                if (cc2Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc2Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (ctrl2.bits.cc4)
            {
                cc4 = wheel2;
                if (cc4Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc4Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (ctrl2.bits.cc67)
            {
                cc67 = wheel2;
                if (cc67Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc67Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            sendChangeMessage();
        }
        else midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc2In, cval), 0);
    }
    if (wheel4Changed)
    {
        wheel4Changed = false;
        int cval = int(wheel4 * 127 + 0.5f);
        if (ctrl4.byteValue != 0)
        {
            if (ctrl4.bits.cc1)
            {
                cc1 = wheel4;
                if (cc1Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc1Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (ctrl4.bits.cc2)
            {
                cc2 = wheel4;
                if (cc2Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc2Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (ctrl4.bits.cc4)
            {
                cc4 = wheel4;
                if (cc4Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc4Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (ctrl4.bits.cc67)
            {
                cc67 = wheel4;
                if (cc67Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc67Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            sendChangeMessage();
        }
        else midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc4In, cval), 0);
    }
    if (wheel67Changed)
    {
        wheel67Changed = false;
        int cval = int(wheel67 * 127 + 0.5f);
        if (ctrl67.byteValue != 0)
        {
            if (ctrl67.bits.cc1)
            {
                cc1 = wheel67;
                if (cc1Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc1Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (ctrl67.bits.cc2)
            {
                cc2 = wheel67;
                if (cc2Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc2Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (ctrl67.bits.cc4)
            {
                cc4 = wheel67;
                if (cc4Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc4Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            if (ctrl67.bits.cc67)
            {
                cc67 = wheel67;
                if (cc67Out < 128)
                    midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc67Out, cval), 0);
                else
                    midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), 0);
            }
            sendChangeMessage();
        }
        else midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc67In, cval), 0);
    }

    if (cc1Changed)
    {
        cc1Changed = false;
        if (cc1Out < 128)
            midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc1Out, int(cc1 * 127)), 0);
        else
            midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, int(cc1 * 127)), 0);
    }
    if (cc2Changed)
    {
        cc2Changed = false;
        if (cc2Out < 128)
            midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc2Out, int(cc2 * 127)), 0);
        else
            midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, int(cc2 * 127)), 0);
    }
    if (cc4Changed)
    {
        cc4Changed = false;
        if (cc4Out < 128)
            midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc4Out, int(cc4 * 127)), 0);
        else
            midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, int(cc4 * 127)), 0);
    }
    if (cc67Changed)
    {
        cc67Changed = false;
        if (cc67Out < 128)
            midiOut.addEvent(juce::MidiMessage::controllerEvent(1, cc67Out, int(cc67 * 127)), 0);
        else
            midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, int(cc67 * 127)), 0);
    }

    juce::MidiMessage msg;
    int samplePos;
    for (const auto meta : midiMessages)
    {
        msg = meta.getMessage();
        samplePos = meta.samplePosition;

        if (msg.isPitchWheel() && (pbDown.byteValue || pbUp.byteValue))
        {
            int pwv = msg.getPitchWheelValue();
            if (pwv >= 8192)
            {
                pitchBendUp = (pwv - 8192) / 8191.0f;
                bool pbdWas0 = (pitchBendDown == 0.0f);
                pitchBendDown = 0.0f;
                if (!pbdWas0)
                {
                    if (pbDown.bits.cc1)
                    {
                        if (cc1Out < 128)
                            midiOut.addEvent(juce::MidiMessage::controllerEvent(msg.getChannel(), cc1Out, 0), samplePos);
                        else
                            midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, 0), samplePos);
                        CC1 = 0.0f;
                        somethingChanged = true;
                    }
                    if (pbDown.bits.cc2)
                    {
                        if (cc2Out < 128)
                            midiOut.addEvent(juce::MidiMessage::controllerEvent(msg.getChannel(), cc2Out, 0), samplePos);
                        else
                            midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, 0), samplePos);
                        CC2 = 0.0f;
                        somethingChanged = true;
                    }
                    if (pbDown.bits.cc4)
                    {
                        if (cc4Out < 128)
                            midiOut.addEvent(juce::MidiMessage::controllerEvent(msg.getChannel(), cc4Out, 0), samplePos);
                        else
                            midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, 0), samplePos);
                        CC4 = 0.0f;
                        somethingChanged = true;
                    }
                    if (pbDown.bits.cc67)
                    {
                        if (cc67Out < 128)
                            midiOut.addEvent(juce::MidiMessage::controllerEvent(msg.getChannel(), cc67Out, 0), samplePos);
                        else
                            midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, 0), samplePos);
                        CC67 = 0.0f;
                        somethingChanged = true;
                    }
                }
                if (pbUp.bits.cc1) CC1 = pitchBendUp;
                if (pbUp.bits.cc2) CC2 = pitchBendUp;
                if (pbUp.bits.cc4) CC4 = pitchBendUp;
                if (pbUp.bits.cc67) CC67 = pitchBendUp;
            }
            else if (pwv < 8192)
            {
                pitchBendDown = (8192 - pwv) / 8191.0f;
                pitchBendUp = 0.0f;
                if (pbDown.bits.cc1) CC1 = pitchBendDown;
                if (pbDown.bits.cc2) CC2 = pitchBendDown;
                if (pbDown.bits.cc4) CC4 = pitchBendDown;
                if (pbDown.bits.cc67) CC67 = pitchBendDown;
            }
            somethingChanged = true;
        }
        else if (msg.isPitchWheel())
        {
            // pitchWheel used but not allocated to any output: ensure GUI updates
            int pwv = msg.getPitchWheelValue();
            if (pwv >= 8192)
            {
                pitchBendUp = (pwv - 8192) / 8191.0f;
                pitchBendDown = 0.0f;
            }
            else
            {
                pitchBendUp = 0.0f;
                pitchBendDown = (8192 - pwv) / 8191.0f;
            }
            somethingChanged = true;
            midiOut.addEvent(msg, samplePos);
        }
        else if ((cc1In == 128 && msg.isChannelPressure()) || msg.isControllerOfType(cc1In))
        {
            modWheel = ((cc1In == 128) ? msg.getChannelPressureValue() : msg.getControllerValue()) / 127.0f;
            somethingChanged = true;
            if (wheel.byteValue)
            {
                if (wheel.bits.cc1) CC1 = modWheel;
                if (wheel.bits.cc2) CC2 = modWheel;
                if (wheel.bits.cc4) CC4 = modWheel;
                if (wheel.bits.cc67) CC67 = modWheel;
            }
            else midiOut.addEvent(msg, samplePos);
        }
        else if ((cc2In == 128 && msg.isChannelPressure()) || msg.isControllerOfType(cc2In))
        {
            wheel2 = ((cc2In == 128) ? msg.getChannelPressureValue() : msg.getControllerValue()) / 127.0f;
            somethingChanged = true;
            if (ctrl2.byteValue)
            {
                if (ctrl2.bits.cc1) CC1 = wheel2;
                if (ctrl2.bits.cc2) CC2 = wheel2;
                if (ctrl2.bits.cc4) CC4 = wheel2;
                if (ctrl2.bits.cc67) CC67 = wheel2;
            }
            else midiOut.addEvent(msg, samplePos);
        }
        else if ((cc4In == 128 && msg.isChannelPressure()) || msg.isControllerOfType(cc4In))
        {
            wheel4 = ((cc4In == 128) ? msg.getChannelPressureValue() : msg.getControllerValue()) / 127.0f;
            somethingChanged = true;
            if (ctrl4.byteValue)
            {
                if (ctrl4.bits.cc1) CC1 = wheel4;
                if (ctrl4.bits.cc2) CC2 = wheel4;
                if (ctrl4.bits.cc4) CC4 = wheel4;
                if (ctrl4.bits.cc67) CC67 = wheel4;
            }
            else midiOut.addEvent(msg, samplePos);
        }
        else if ((cc67In == 128 && msg.isChannelPressure()) || msg.isControllerOfType(cc67In))
        {
            wheel67 = ((cc67In == 128) ? msg.getChannelPressureValue() : msg.getControllerValue()) / 127.0f;
            somethingChanged = true;
            if (ctrl67.byteValue)
            {
                if (ctrl67.bits.cc1) CC1 = wheel67;
                if (ctrl67.bits.cc2) CC2 = wheel67;
                if (ctrl67.bits.cc4) CC4 = wheel67;
                if (ctrl67.bits.cc67) CC67 = wheel67;
            }
            else midiOut.addEvent(msg, samplePos);
        }
        else
        {
            // all other messages are passed through
            midiOut.addEvent(msg, samplePos);
        }

        if (CC1 != cc1)
        {
            cc1 = CC1;
            int cval = int(cc1 * 127 + 0.5f);
            if (cc1Out < 128)
                midiOut.addEvent(juce::MidiMessage::controllerEvent(msg.getChannel(), cc1Out, cval), samplePos);
            else
                midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), samplePos);
            somethingChanged = true;
        }
        if (CC2 != cc2)
        {
            cc2 = CC2;
            int cval = int(cc2 * 127 + 0.5f);
            if (cc2Out < 128)
                midiOut.addEvent(juce::MidiMessage::controllerEvent(msg.getChannel(), cc2Out, cval), samplePos);
            else
                midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), samplePos);
            somethingChanged = true;
        }
        if (CC4 != cc4)
        {
            cc4 = CC4;
            int cval = int(cc4 * 127 + 0.5f);
            if (cc4Out < 128)
                midiOut.addEvent(juce::MidiMessage::controllerEvent(msg.getChannel(), cc4Out, cval), samplePos);
            else
                midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), samplePos);
            somethingChanged = true;
        }
        if (CC67 != cc67)
        {
            cc67 = CC67;
            int cval = int(cc67 * 127 + 0.5f);
            if (cc67Out < 128)
                midiOut.addEvent(juce::MidiMessage::controllerEvent(msg.getChannel(), cc67Out, cval), samplePos);
            else
                midiOut.addEvent(juce::MidiMessage::channelPressureChange(1, cval), samplePos);
            somethingChanged = true;
        }
        if (somethingChanged) sendChangeMessage();
    }

    midiMessages.swapWith(midiOut);
}

bool ModMateAudioProcessor::hasEditor() const
{
    return !juce::Desktop::getInstance().isHeadless();
}

juce::AudioProcessorEditor* ModMateAudioProcessor::createEditor()
{
    if (!juce::Desktop::getInstance().isHeadless())
        return new ModMateAudioProcessorEditor(*this);

    return nullptr;
}

void ModMateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml = juce::XmlElement("modBits");
    xml.setAttribute("pbUpBits", pbUp.byteValue);
    xml.setAttribute("pbDownBits", pbDown.byteValue);
    xml.setAttribute("wheelBits", wheel.byteValue);
    xml.setAttribute("ctrl2Bits", ctrl2.byteValue);
    xml.setAttribute("ctrl3Bits", ctrl4.byteValue);
    xml.setAttribute("ctrl4Bits", ctrl67.byteValue);
    xml.setAttribute("cc1In", cc1In);
    xml.setAttribute("cc2In", cc2In);
    xml.setAttribute("cc3In", cc4In);
    xml.setAttribute("cc4In", cc67In);
    xml.setAttribute("cc1Out", cc1Out);
    xml.setAttribute("cc2Out", cc2Out);
    xml.setAttribute("cc3Out", cc4Out);
    xml.setAttribute("cc4Out", cc67Out);
    copyXmlToBinary(xml, destData);
}

void ModMateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml = getXmlFromBinary(data, sizeInBytes);
    pbUp.byteValue = xml->getIntAttribute("pbUpBits", 0);
    pbDown.byteValue = xml->getIntAttribute("pbDownBits", 0);
    wheel.byteValue = xml->getIntAttribute("wheelBits", 0);
    ctrl2.byteValue = xml->getIntAttribute("ctrl2Bits", 0);
    ctrl4.byteValue = xml->getIntAttribute("ctrl3Bits", 0);
    ctrl67.byteValue = xml->getIntAttribute("ctrl4Bits", 0);
    cc1In = xml->getIntAttribute("cc1In", 1);
    cc2In = xml->getIntAttribute("cc2In", 2);
    cc4In = xml->getIntAttribute("cc3In", 4);
    cc67In = xml->getIntAttribute("cc4In", 67);
    cc1Out = xml->getIntAttribute("cc1Out", 1);
    cc2Out = xml->getIntAttribute("cc2Out", 2);
    cc4Out = xml->getIntAttribute("cc3Out", 4);
    cc67Out = xml->getIntAttribute("cc4Out", 67);
    presetLoaded = true;
    sendChangeMessage();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModMateAudioProcessor();
}

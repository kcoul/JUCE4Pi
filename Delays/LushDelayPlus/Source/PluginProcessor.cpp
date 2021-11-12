/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LushDelayAudioProcessor::LushDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
    :
#endif
state (*this, &undoManager, Identifier ("Lush"), Params::createParameterLayout()),
lushDelayEngine(state)
{
    
}

LushDelayAudioProcessor::~LushDelayAudioProcessor()
{
    
}

//==============================================================================
const String LushDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LushDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LushDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LushDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LushDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LushDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LushDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LushDelayAudioProcessor::setCurrentProgram (int index)
{
}

const String LushDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void LushDelayAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void LushDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    auto numInputChannels  = getTotalNumInputChannels();
    auto numOutputChannels = getTotalNumOutputChannels();
    lushDelayEngine.prepare({ sampleRate, (uint32) samplesPerBlock, (uint32) numOutputChannels });
    
    stm::DebugDisplay::set(10, "Inputs: " + String(numOutputChannels));
    stm::DebugDisplay::set(11, "Outputs: " + String(numInputChannels));
}

void LushDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LushDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void LushDelayAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Send the playhead to the audio engine.
    // This allows the engine to check if bpm or timeSigDenominator has changed.
    auto playhead = getPlayHead();
    if (playhead)
    {
        playhead->getCurrentPosition(currentPositionInfo);
        lushDelayEngine.analyzePosition(currentPositionInfo);
    }
    
    //Lush only supports stereo out
    jassert(totalNumOutputChannels == 2);
    //Lush supports mono or stereo input
    jassert(totalNumInputChannels < 3);
    
    if (totalNumInputChannels == 1) {
        // For mono input, we copy channel 0 across both outputs
        //buffer.copyFrom(1, 0, buffer.getReadPointer(0), buffer.getNumSamples());
    }

    lushDelayEngine.process(buffer);
}

//==============================================================================
bool LushDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* LushDelayAudioProcessor::createEditor()
{
    return new LushDelayAudioProcessorEditor (*this);
}

//==============================================================================
void LushDelayAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto stateCopy = state.copyState();
    std::unique_ptr<XmlElement> xml (stateCopy.createXml());
    copyXmlToBinary (*xml, destData);
}

void LushDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (state.state.getType()))
            state.replaceState (ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LushDelayAudioProcessor();
}

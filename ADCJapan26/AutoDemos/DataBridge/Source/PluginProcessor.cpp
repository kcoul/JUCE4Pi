#include "PluginProcessor.h"
#include "PluginEditor.h"

GenisysPluginProcessor::GenisysPluginProcessor()
    : AudioProcessor (BusesProperties())
{
}

void GenisysPluginProcessor::ensureConnected()
{
    if (! senderConnected)
        senderConnected = sender.connect (targetHost, targetPort);
}

void GenisysPluginProcessor::sendOsc (const juce::String& address, float value)
{
    ensureConnected();
    if (senderConnected)
        sender.send (address, value);
}

void GenisysPluginProcessor::processBlock (juce::AudioBuffer<float>& audio,
                                           juce::MidiBuffer& midi)
{
    audio.clear();
    bool stateChanged = false;

    for (const auto meta : midi)
    {
        auto msg = meta.getMessage();

        if (msg.isController())
        {
            int   cc  = msg.getControllerNumber();
            float val = (float) msg.getControllerValue() / 127.0f;
            lastCcValues[cc] = val;

            for (int i = 0; i < AvasMapping::numCcMappings; ++i)
            {
                const auto& m = AvasMapping::ccMappings[i];
                if (m.ccNumber == cc)
                {
                    float oscVal = m.outMin + val * (m.outMax - m.outMin);
                    sendOsc (m.oscAddress, oscVal);
                    stateChanged = true;
                }
            }
        }
        else if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            for (int i = 0; i < AvasMapping::numGears; ++i)
            {
                if (AvasMapping::gears[i].midiNote == note)
                {
                    currentGear = AvasMapping::gears[i].gearNumber;
                    sendOsc (AvasMapping::gearOscAddress, AvasMapping::gears[i].oscOctaveNorm);
                    stateChanged = true;
                    break;
                }
            }
        }
        else if (msg.isNoteOff())
        {
            // Sustain the gear state on note-off; gear holds until next gear note.
        }
    }

    if (stateChanged && onStateChanged)
        juce::MessageManager::callAsync ([this] { if (onStateChanged) onStateChanged(); });
}

juce::AudioProcessorEditor* GenisysPluginProcessor::createEditor()
{
    return new GenisysPluginEditor (*this);
}

void GenisysPluginProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = std::make_unique<juce::DynamicObject>();
    state->setProperty ("targetHost", targetHost);
    state->setProperty ("targetPort", targetPort);
    auto json = juce::JSON::toString (juce::var (state.release()));
    dest.replaceAll (json.toRawUTF8(), (size_t) json.getNumBytesAsUTF8());
}

void GenisysPluginProcessor::setStateInformation (const void* data, int size)
{
    auto json = juce::String::fromUTF8 ((const char*) data, size);
    auto v = juce::JSON::parse (json);
    if (v.isObject())
    {
        targetHost = v["targetHost"].toString();
        targetPort = (int) v["targetPort"];
        senderConnected = false;  // force reconnect with new settings
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GenisysPluginProcessor();
}

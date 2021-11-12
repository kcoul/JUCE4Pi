#pragma once

//Import module headers here
#include <ff_meters/ff_meters.h>
#include <hack_audio_gui/hack_audio_gui.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <stm_dsp/stm_dsp.h> //Depends on juce_dsp, juce_audio_processors
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "BinaryData.h"

namespace GuiApp
{
//To save some typing, we could import a few commonly used juce classes
//into our namespace
//using juce::Colour;
//using juce::Component;
//using juce::Graphics;
//using juce::String;
} // namespace GuiApp
/** 
     BEGIN_JUCE_MODULE_DECLARATION

      ID:               stm_dsp
      vendor:           Spenser Saling
      version:          1.0.0
      name:             Sailing To Mars DSP Module
      description:      A Digital Signal Processing Module for JUCE
      website:          https://sailingtomars.com/
      license:          MIT

      dependencies:     juce_core, juce_audio_basics, juce_dsp, juce_audio_processors, juce_events, stm_gui,

     END_JUCE_MODULE_DECLARATION
*/

#pragma once

#include "analyzers/stm_LevelAnalyzer.h"

#include "processors/stm_DelaySimple.h"
#include "processors/stm_DCFilter.h"
#include "processors/stm_Saturation.h"
#include "processors/stm_AutoGain.h"
#include "processors/stm_MatchedBypass.h"
#include "processors/stm_DryWetMix.h"
#include "processors/stm_Panner.h"
#include "processors/stm_FrequencyModulator.h"

#include "utilities/stm_Measuring.h"
#include "utilities/stm_Rampers.h"
#include "utilities/stm_RingBuffer.h"
#include "utilities/stm_RecircBuffer.h"
#include "utilities/stm_RollingAverage.h"
#include "utilities/stm_Primes.h"
#include "utilities/stm_Notes.h"
#include "utilities/stm_ParameterAttachment.h"
#include "utilities/stm_Oscillator.h"
#include "utilities/stm_Mixer.h"

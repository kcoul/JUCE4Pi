/** 
     BEGIN_JUCE_MODULE_DECLARATION

      ID:               stm_gui
      vendor:           Spenser Saling
      version:          1.0.0
      name:             Sailing To Mars GUI Module
      description:      A Graphical User Interface Module for JUCE
      website:          https://sailingtomars.com/
      license:          MIT

      dependencies:     juce_gui_basics, juce_events, stm_dsp

     END_JUCE_MODULE_DECLARATION
*/

#pragma once

#include "analyzers/stm_LevelMeter.h"

#include "lookandfeel/stm_LookAndFeel.h"

#include "displays/stm_RecircBufferDisplay.h"
#include "displays/stm_DebugDisplay.h"
#include "controls/stm_Slider2Axis.h"
#include "displays/stm_DisabledVeil.h"

#include "icons/stm_FilterIcon.h"

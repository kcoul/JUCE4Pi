/*
  ==============================================================================

    Params.h
    Created: 2 May 2020 7:24:40pm
    Author:  Spenser Saling

  ==============================================================================
*/

#pragma once
#include "CommonHeader.h"

class Params
{
public:
    constexpr static const float MAX_DELAY = 1000.0f; //ms
    constexpr static const float MAX_MOD = 1.0f; //ms
    constexpr static const float MAX_SPREAD = 300.0f; //ms
    constexpr static const float MAX_OFFSET = 1.0f; //ratio of delay
    constexpr static const int MAX_TAPS = 10;
    constexpr static const float MIN_DELAY = MAX_MOD;
    // MAX_BUFFER represents the maximum amount of time (in ms) that a delay buffer must
    // hold in order to satisfy all possible parameter combinations.
    constexpr static const float MAX_BUFFER = MAX_DELAY * (1.0f + MAX_OFFSET) + MAX_SPREAD;
    
    inline static const String idDry = "dry";
    inline static const String idWet = "wet";
    inline static const String idBypass = "bypass";
    inline static const String idDelay = "delay";
    inline static const String idPan = "pan";
    inline static const String idTaps = "taps";
    inline static const String idSpread = "spread";
    inline static const String idOffsetLR = "offsetLR";
    inline static const String idAllpass = "allpass";
    inline static const String idFeedbackDirect = "feedbackDirect";
    inline static const String idFeedbackCross = "feedbackCross";
    inline static const String idHighPass = "highPass";
    inline static const String idLowPass = "lowPass";
    inline static const String idModDepth = "modDepth";
    inline static const String idModRate = "modRate";
    inline static const String idSnap = "snap";
};

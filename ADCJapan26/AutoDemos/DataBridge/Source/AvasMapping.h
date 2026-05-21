#pragma once
#include <juce_core/juce_core.h>

// Default CC → SurgeXT OSC mappings for an AVAS/ESE gear-shift demo.
// All SurgeXT OSC params are normalised 0.0–1.0.

struct CcMapping
{
    int          ccNumber;
    juce::String label;
    juce::String oscAddress;
    float        outMin = 0.0f;  // value sent when CC == 0
    float        outMax = 1.0f;  // value sent when CC == 127
};

// Gear note assignments: MIDI note → octave offset index (0 = lowest gear).
// Notes C3(48) through A3(57) map to gears 1–6.
// Octave param is normalised; SurgeXT's octave range is -3..+3 (ct_pitch_octave).
// We represent each gear as a fraction of that range centred around 0.5.
struct GearDef
{
    int   midiNote;
    int   gearNumber;
    float oscOctaveNorm;  // normalised value for /param/a/osc/1/octave
};

namespace AvasMapping
{
    static const CcMapping ccMappings[] =
    {
        // CC, label, SurgeXT address, outMin, outMax
        { 1,  "Speed / RPM",  "/param/a/osc/1/pitch",      0.35f, 0.65f },
        { 11, "Throttle",     "/param/a/filter/1/cutoff",  0.25f, 0.80f },
        { 7,  "Load / Amp",   "/param/a/amp/volume",       0.10f, 0.90f },
    };
    static constexpr int numCcMappings = 3;

    // Octave offsets: gear 1 is lowest (idle sound), gear 6 is highest (motorway cruise).
    // SurgeXT octave range –3..+3; normalised: 0 = –3, 1 = +3.  0.5 = 0 (no shift).
    static const GearDef gears[] =
    {
        { 48, 1, 0.33f },  // C3  – gear 1: slight down (heavy engine lug)
        { 50, 2, 0.40f },  // D3  – gear 2
        { 52, 3, 0.47f },  // E3  – gear 3
        { 53, 4, 0.50f },  // F3  – gear 4: neutral
        { 55, 5, 0.55f },  // G3  – gear 5
        { 57, 6, 0.62f },  // A3  – gear 6: slight up (high-speed cruise)
    };
    static constexpr int numGears = 6;

    static constexpr int    surgeOscPort    = 53280;
    static const char*      surgeOscHost    = "127.0.0.1";
    static const char*      gearOscAddress  = "/param/a/osc/1/octave";
}

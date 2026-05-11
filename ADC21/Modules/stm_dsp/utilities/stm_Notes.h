#pragma once

using namespace juce;

namespace stm {

class Notes
{
public:
    
    /**
     Calculates the milliseconds per note.
     "note" should be in the format of the time signature denominator. i.e. 4 represents a 1/4 note.
     */
    static float ms(int note, int timeSignatureDenominator, float bpm){
        float beats = float(timeSignatureDenominator) / float(note);
        float beatsPerMillisecond = bpm / 60.0f / 1000.0f;
        return beats / beatsPerMillisecond;
    }
    static float msDotted(int note, int timeSignatureDenominator, float bpm){
        return ms(note, timeSignatureDenominator, bpm) * 1.5f;
    }
    static float msTriplet(int note, int timeSignatureDenominator, float bpm){
        return ms(note, timeSignatureDenominator, bpm) * 0.66666666f;
    }
    
private:

};
    
} //namespace stm

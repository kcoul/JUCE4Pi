#pragma once

using namespace juce;

namespace stm {

/**
 Represents separate left and right levels
 */
struct Balance {
    float left;
    float right;
};

/**
 Balance affects the left and right levels, but doesn't mix between the two.
 < 0.5 to the left
 > 0.5 to the right
*/
class Balancer {
public:
    /** Converts the mix to a human-readable string */
    static String toString(float mix){
        if (approximatelyEqual(mix, 0.5f)){
            return "Center";
        } else if (mix < 0.5f) {
            int percent = int(100.0f - mix * 200.0f);
            return String(percent) + "% L";
        } else {
            float percentF = (mix-0.5f) * 200.0f;
            return String(int(percentF)) + "% R";
        }
    }
    
    /** left & right = 0.5 at center */
    static Balance getLinear(float mix){
        return {
            1.0f - mix,
            mix
        };
    }
    /** left & right = 0.707 at center */
    static Balance getSin(float mix){
        //from 0 to pi/2 or 90 degrees
        //center is 45 degrees
        float rads = mix * MathConstants<float>::halfPi;
        
        return {
            std::cos(rads),
            std::sin(rads)
        };
    }
    /** left & right = 1.0 at center */
    static Balance getLinearCentered(float mix){
        if (mix < 0.5f) {
            return {
                1.0f,
                mix * 2.0f
            };
        } else {
            return {
                1.0f - (mix - 0.5f) * 2.0f,
                1.0f
            };
        }
    }
    /** left & right = 1.0 at center */
    static Balance getSinCentered(float mix){
        if (mix < 0.5f) {
            float rads = mix * 2.0f * MathConstants<float>::halfPi;
            return {
                1.0f,
                std::sin(rads)
            };
        } else {
            float rads = (mix - 0.5f) * 2.0f * MathConstants<float>::halfPi;
            return {
                std::cos(rads),
                1.0f
            };
        }
    }
};

/**
 Represents separate left and right levels
 As well as cross-mix between the left and right
 */
struct Mix {
    float left;
    float right;
    float leftCross;
    float rightCross;
};

/**
 Mix affects the left and right levels, and mixes between the two.
 < 0.5 to the left
 > 0.5 to the right
 */
class Mixer {
public:
    
    /** Left & Right = 1.0 @ center */
    static Mix getLinearCentered(float mix){
        if (mix < 0.5f){
            //To the left
            return {
                0.5f + mix,
                mix * 2.0f,
                0.0f,
                0.5f - mix
            };
        } else {
            //To the right
            return {
                2.0f - mix * 2.0f,
                1.5f - mix,
                mix - 0.5f,
                0.0f
            };
        }
    }
    
};

} // namespace stm

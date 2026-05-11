#pragma once

namespace stm {

// A base class for rampers.
class RamperBase {
public:
    virtual ~RamperBase(){}
    
    void prepare(float _current, float _delta){
        current = _current;
        target = current;
        //Ensure that delta is positive to allow for optimizations in getNext()
        delta = std::abs(_delta);
    }
    
    //Resetting the ramper is a useful way to snap to the target value without ramping.
    void reset(){
        current = target;
    }
    
    void updateTarget(float newTarget){
        target = newTarget;
        //Calculate direction here to optimize getNext()
        direction = (target > current) ? 1.0f : -1.0f;
    }
    
    virtual float getNext()=0;
    
protected:
    float current, target, delta, direction, start;
};


//Ramps linearly towards the target
class RamperLinear : public RamperBase {
public:
    
    float getNext() override {
        if (current != target){
            if (juce::isWithin(target, current, delta)){
                current = target;
            } else {
                current += delta * direction;
            }
        }
        return current;
    }
};

} // namespace stm

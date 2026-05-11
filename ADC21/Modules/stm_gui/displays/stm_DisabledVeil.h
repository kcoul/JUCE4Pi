#pragma once

using namespace juce;

namespace stm {


class DisabledVeil    : public Component
{
public:
    DisabledVeil()
    {
    }

    ~DisabledVeil()
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black.withAlpha(0.7f));
    }

    void resized() override
    {
        
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisabledVeil)
};


} // namespace stm

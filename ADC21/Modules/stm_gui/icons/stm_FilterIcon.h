#pragma once

using namespace juce;

namespace stm {


class FilterIcon    : public Component
{
public:
    FilterIcon() {}
    ~FilterIcon() {}
    
    void setHP(bool _isHP) {
        isHP = _isHP;
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colours::white);
        
        auto strokeType = PathStrokeType( thickness,
            PathStrokeType::JointStyle::curved,
            PathStrokeType::EndCapStyle::butt );
            
        AffineTransform transform = {};
        
        if (isHP){
            const float width = (float)getWidth();
            transform = { -1.0f,  0.0f, width, 0.0f, 1.0f, 0.0f };
        }
        
        g.strokePath(path, strokeType, transform);
    }

    void resized() override
    {
        float height = getHeight() - thickness;
        float width = getWidth() - thickness;
        float startX = thickness/2.0f;
        float startY = startX;
        
        if(width / height > w_hRatio) {
            //width is too big
            width = height * w_hRatio;
            startX += (getWidth() - width) / 2;
        }
        else {
            //height is too big
            height = width / w_hRatio;
            startY += (getHeight() - height) / 2;
        }
        
        path.clear();
        path.startNewSubPath(startX, startY);
        path.lineTo(startX + width * elbow, startY);
        path.lineTo(startX + width, startY + height);
    }

private:
    Path path;
    bool isHP = false;
    float w_hRatio = 1.7f;
    float thickness = 1.5f;
    float elbow = 0.7f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterIcon)
};


} // namespace stm

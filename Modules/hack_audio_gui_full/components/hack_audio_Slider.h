/* Copyright (C) 2017 by Antonio Lassandro, HackAudio LLC
 *
 * hack_audio_gui is provided under the terms of The MIT License (MIT):
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef HACK_AUDIO_SLIDER
#define HACK_AUDIO_SLIDER

namespace HackAudio
{

/**
 A custom slider using the juce::Slider API while adding custom styling and new methods.
*/
class Slider : public juce::Slider,
               private juce::Timer,
               private juce::Slider::Listener
{
public:

    Slider();

    /**
        Sets whether the slider should snap to a value when double clicked and, if so, what value
     
        @param shouldHaveDefault    whether the slider accepts double clicks
     
        @param defaultValue         the value the slider should snap too (must be in slider's range)
    */
    void setDefaultValue(bool shouldHaveDefault, double defaultValue);

    /**
        Sets the visibility state of the slider's decorative pips
    */
    void setPipState(bool shouldBeShown);

    /**
        Returns the current visibility of the slider's pips
    */
    bool getPipState() const;

    /**
        Sets the number of pips the slider should display, adjusting their spacing automatically
    */
    void setPipCount(int count);

    /**
        Returns the current number of pips for the slider
    */
    int  getPipCount() const;

    /**
     Sets the size of the slider with a 1:1 aspect ratio. Useful for rotary sliders

     @paremeter size     the size that both the width and height will be set to
     */
    void setSymmetricSize(int size);

private:

    void setPipScale();

    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp  (const juce::MouseEvent& e) override;

    bool keyPressed(const juce::KeyPress& key) override;

    void focusGained(juce::Component::FocusChangeType cause) override;
    void focusLost  (juce::Component::FocusChangeType cause) override;

    void enablementChanged() override;

    void timerCallback() override;

    void sliderValueChanged(juce::Slider*) override;

    void parentHierarchyChanged() override;

    void stopAnimation();

    void paint (juce::Graphics& g) override;
    void resized() override;

    bool isAnimating;
    float animationAcc;
    float animationVel;
    juce::Point<int> animationStart;
    juce::Point<int> animationEnd;

    bool resizeGuard;

    bool pipsShown;
    int minPipSize;
    int maxPipSize;
    int currentMinPipSize;
    int currentMaxPipSize;
    int pipClicked;

    juce::Array<juce::Point<int>> pipLocations;
    juce::Rectangle<int> trackArea;
    juce::Rectangle<int> thumbArea;
    juce::Rectangle<int> thumbSpan;
    juce::Rectangle<int> indicatorArea;

    bool isDraggable;
    bool isSettable;

    bool hasDefault;
    double sliderDefault;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Slider)

};

}

#endif

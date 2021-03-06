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

#ifndef HACK_AUDIO_SELECTOR_H
#define HACK_AUDIO_SELECTOR_H

namespace HackAudio
{

/**
 A component that allows a user to traverse through a list of options
*/
class Selector : public juce::Component,
                 private juce::Timer
{
public:

    Selector();

    /**
     Sets the style of justification to be used for positioning the text
    */
    void setJustificationType(juce::Justification justification);

    /**
     Returns the current justification type
    */
    juce::Justification getJustificationType() const;

    /**
     Adds an item to the list of the selector choices
     
     @param newItemText     the string of the new item
     @param itemIndex       where to place the item, use -1 to place the new item last in the array
    */
    void add(const juce::String& newItemText, int itemIndex = -1);
    void add(const char* newItemText, int itemIndex = -1);

    /**
     Adds an array of strings as new selector choices
     
     @param items               an array of strings
     @param startIndex          where to begin placing the new items
     @param numElementsToAdd    how many elements to insert (inserts all if < 0)
    */
    void addArray(const juce::StringArray& items, int startIndex = 0, int numElementsToAdd = -1);

    /**
     Inserts an item into the selector
     
     @param index           the index to insert the item at
     @param newItemText     the text of the new item
    */
    void insert(int index, const juce::String& newItemText);

    /**
     Changes the text of one of the exisiting items
    */
    void set(int itemIndex, const juce::String& newText);
    void set(int itemIndex, const char* newText);

    /**
     Clears all choices from the selector
    */
    void clear();

    /**
     Manually sets the current choice of the selector
    */
    void setCurrentIndex(int index);

    /**
     Returns the index of the currently displayed item
    */
    int getCurrentIndex() const;

    /**
     Returns the total number of items the selector holds
    */
    int getNumItems() const;

    /**
     Returns the text of the currently displayed item
    */
    juce::String getCurrentText() const;

    /**
     Returns the text of the item at a specific index
    */
    juce::String getItemText(int index) const;

    /**
     Searches the choices and returns the index if it finds the given string
    */
    int getIndexForItem(const juce::String& itemText) const;
    int getIndexForItem(const char* itemText) const;

    /**
     Sets the selector's font
    */
    void setFont(juce::Font font);

    /**
     Returns the selector's current font
    */
    juce::Font getFont() const;

    /**
        A class for receiving callbacks from a selector.
     
        To be told when a selector changes, you can register a Selector::Listener
        object using Selector::addListener()
     
        @see Selector::addListener, Selector::removeListener
    */
    class Listener
    {
    public:
        virtual ~Listener() {}

        /**
         Called when a selector moves forwards or backwards
        */
        virtual void selectorIndexChanged(Selector*, int index) = 0;

        /**
         Called when a selector has items added or removed
        */
        virtual void selectorItemsChanged(Selector*) {}

    };

    /**
     Adds a listener to be called when the selector changes items
    */
    void addListener(Listener* listener);

    /**
     Removes a previously registered listener
    */
    void removeListener(Listener* listener);

private:

    void indexChanged();
    void itemsChanged();

    void mouseUp(const juce::MouseEvent& e) override;

    bool keyPressed(const juce::KeyPress& key) override;

    void focusGained(juce::Component::FocusChangeType cause) override;
    void focusLost  (juce::Component::FocusChangeType cause) override;

    void enablementChanged() override;

    void timerCallback() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::ListenerList<Listener> listeners;

    juce::Font selectorFont;

    juce::Justification selectorJustification;

    int currentIndex;
    juce::Array<juce::String> selectorItems;

    juce::LinearSmoothedValue<float> colourInterpolation;

    HackAudio::NavigationButton selectLeft, selectRight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Selector)

};

}

#endif

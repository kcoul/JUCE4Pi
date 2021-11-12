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

#ifndef HACK_AUDIO_COLOURS_H
#define HACK_AUDIO_COLOURS_H

namespace HackAudio
{

/**
 The predefined colours for the HackAudio theme
*/
class Colours
{
public:
    static const juce::Colour
    Black,
    DarkGray,
    Gray,
    LightGray,
    White,

    Teal,
    Cyan,
    Violet,
    Magenta;

    /**
     Registers a colour for every available component. This affects all components even if they are invisible or disabled.
    */
    static void setGlobalColour(int colourId, juce::Colour newColour);

private:
    Colours();

    JUCE_DECLARE_NON_COPYABLE (Colours)

};

/**
 The standard colour IDs for the HackAudio theme
*/
enum ColourIds
{

    backgroundColourId,
    midgroundColourId,
    foregroundColourId,
    highlightColourId
    
};
    
}

#endif

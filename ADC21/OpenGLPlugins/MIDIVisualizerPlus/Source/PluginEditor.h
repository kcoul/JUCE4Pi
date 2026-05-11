/*
  ==============================================================================
Ross Hoyt
PluginEditor.h
The Plugin Window, and the GL Component are declared here.
They are defined PluginEditor.h

==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "PluginProcessor.h"
#include "WavefrontObjParser.h"
#include "Shape.h"
#include "Textures.h"
#include "Uniforms.h"
#include "Attributes.h"
#include "ShaderPreset.h"
#include "GLUtils.h"

#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================
/**
*/
class GLComponent; // forward declared
class PluginEditor  : public juce::AudioProcessorEditor
{
public:
    PluginEditor (PluginProcessor&, juce::MidiKeyboardState&);
    ~PluginEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // SLIDERS FOR GL WINDOW PARAMS THAT NEED TO BE ACCESSED BY CHILDREN
    juce::Slider zoomSlider;
    juce::Slider rotationSlider;
    
    // SHADER SELECTOR
    juce::ComboBox shaderPresetBox;
    
private:
    
    
    // RADIO BUTTONS:
    enum RadioButtonIds { ObjSelectorButtons = 1001 };
    // RB->OBJ SELECTOR RADIO BOX
    juce::GroupComponent* radioButtonsObjSelector;
    juce::ToggleButton* togglePianoKeyObj;
    juce::ToggleButton* toggleTeapotObj;
    
    
    // OBJ SELECTOR RADIO BOX
    juce::ToggleButton* toggleDrawControlMesh;
    //ToggleButton* toggleButton_TeapotObj;
    

    // LABELS
    juce::Label zoomLabel  { {}, "Zoom:" };
    juce::Label shaderPresetBoxLabel;
    //Label rotationLabel { {}, "Rotation:" };
    
    
    void initialise();
    
    void updateToggleState(juce::Button*, juce::String);
    
    void selectShaderPreset(int);
    void setShaderProgram(const juce::String&, const juce::String&);
    void updateShader();
    
    // class that created this class
    PluginProcessor& processor;
    

    juce::MidiKeyboardComponent midiKeyboardComponent;
    juce::MidiKeyboardState * midiKeyboardState;
    
    std::unique_ptr<GLComponent> glComponent;
    
    // GUI constants
    static const int MARGIN = 4, MAX_WINDOW_HEIGHT = 800, MAX_WINDOW_WIDTH = 1200 + 2 * MARGIN,
    MAX_KEYB_WIDTH = 1200, MAX_KEYB_HEIGHT = 82, BUTTON_WIDTH = 50, BUTTON_HEIGHT = 30;
    juce::Colour backgroundColor { 44,54,60 }; // stock bckgrd colour
    
    // Display Helper Method
    enum SubdividedOrientation { Vertical, Horizontal};
    static juce::Rectangle<int> getSubdividedRegion(const juce::Rectangle<int>, int, int, SubdividedOrientation);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED

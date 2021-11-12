/*
  ==============================================================================
Ross Hoyt
PluginEditor.cpp
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryResourceHelper.h"

//==============================================================================
class GLComponent : public juce::Component, private juce::OpenGLRenderer, public juce::Slider::Listener
{
public:
    GLComponent(juce::MidiKeyboardState &mKeybState, PluginEditor *par) :
    rotation (0.0f), scale (0.5f), rotationSpeed (0.1f),
    textureToUse (nullptr), lastTexture (nullptr)
    {
        parent = par;
        
        midiKeyboardState = &mKeybState;
        juce::Array<ShaderPreset> shaders = GLUtils::getShaderPresets();
        
        if (shaders.size() > 0)
        {
            newVertexShader = shaders.getReference (0).vertexShader;
            newFragmentShader = shaders.getReference (0).fragmentShader;
        }
        
        lastTexture = textureToUse = new BuiltInTexture ("Portmeirion", getBinaryDataImagesAsRaw().front().data,
                                                         getBinaryDataImagesAsRaw().front().size);
        
        openGLContext.setComponentPaintingEnabled (false);
        openGLContext.setContinuousRepainting (true);
        
        openGLContext.setRenderer (this);
        openGLContext.attachTo (*this);
    }
    
    ~GLComponent()
    {
        openGLContext.detach();
        openGLContext.setRenderer (nullptr);
        
        if (lastTexture != nullptr)
            delete lastTexture;
    }
    
    bool drawPianoKeys = true;
    bool drawControlMesh = true;
    
    juce::String newVertexShader, newFragmentShader;
private:
    
    void newOpenGLContextCreated() override
    {
        freeAllContextObjects();
    }
    
    void renderOpenGL() override
    {
        jassert (juce::OpenGLHelpers::isContextActive());
        
        const float desktopScale = (float) openGLContext.getRenderingScale();
        juce::OpenGLHelpers::clear (juce::Colours::cadetblue);
        
        updateShader();   // Check whether we need to compile a new shader
        
        if (shader == nullptr)
            return;
        
        juce::gl::glEnable (juce::gl::GL_DEPTH_TEST);
        juce::gl::glDepthFunc (juce::gl::GL_LESS);
        juce::gl::glEnable (juce::gl::GL_BLEND);
        juce::gl::glBlendFunc (juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);
        openGLContext.extensions.glActiveTexture (juce::gl::GL_TEXTURE0);
        juce::gl::glEnable (juce::gl::GL_TEXTURE_2D);
        
        juce::gl::glViewport (0, 0, juce::roundToInt (desktopScale * getWidth()), juce::roundToInt (desktopScale * getHeight()));
        texture.bind();
        
        juce::gl::glTexParameteri (juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_S, juce::gl::GL_REPEAT);
        juce::gl::glTexParameteri (juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_T, juce::gl::GL_REPEAT);
        
        shader->use();
        
        if (uniforms->projectionMatrix != nullptr)
            uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
        
        if (uniforms->texture != nullptr)
            uniforms->texture->set ((GLint) 0);
        
        if (uniforms->lightPosition != nullptr)
            uniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);
        
        
        for(int i = 0; i < 128; i++)
        {
            
            if (uniforms->viewMatrix != nullptr)
                uniforms->viewMatrix->setMatrix4 (getViewMatrix(i).mat, 1, false);
            
            // TODO could add automatic MidiChannel Sensing
            if(drawPianoKeys)
            {
                if(drawControlMesh)
                    shapePianoKey->drawControlMesh(openGLContext, *attributes);
                if(midiKeyboardState->isNoteOn(1, i))
                    shapePianoKey->draw (openGLContext, *attributes);
            }
            else
            {
                if(drawControlMesh)
                    shapeTeapot->drawControlMesh(openGLContext, *attributes);
                if(midiKeyboardState->isNoteOn(1, i))
                    shapeTeapot->draw(openGLContext, *attributes);
            }
            
        }
        
        // Reset the element buffers so child Components draw correctly
        openGLContext.extensions.glBindBuffer (juce::gl::GL_ARRAY_BUFFER, 0);
        openGLContext.extensions.glBindBuffer (juce::gl::GL_ELEMENT_ARRAY_BUFFER, 0);
        
        rotation += (float) rotationSpeed;
    }
    
    juce::Matrix3D<float> getProjectionMatrix() const
    {
        auto w = 1.0f / (scale + 0.1f);
        auto h = w * getLocalBounds().toFloat().getAspectRatio (false);
        return juce::Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
    }
    
    juce::Matrix3D<float> getViewMatrix(int i) const
    {
        int fundamentalPitch = i % 12;
        auto viewMatrix = draggableOrientation.getRotationMatrix()
        * juce::Vector3D<float> (0.0f, 1.0f, -10.0f);
        
        float angle = 360.0f / 12.0f;
        float rotY = juce::degreesToRadians(angle) * ((fundamentalPitch + 9) % 12);
        auto rotationMatrix = juce::Matrix3D<float>::rotation ({ 1.55f, rotY, -1.55f });
        // TODO Control Cylinder Radius with Slider
        float cylinderRadius = 2.0f;
        
        float translateY = i * -.5f + 30.0f;
        float translateX = cylinderRadius * sin(juce::degreesToRadians(fundamentalPitch * angle));
        float translateZ = cylinderRadius * cos(juce::degreesToRadians(fundamentalPitch * angle));
        auto translationVector = juce::Matrix3D<float>(juce::Vector3D<float> ({ translateX, translateY, translateZ}));
        
        return  rotationMatrix * translationVector * viewMatrix;
        
    }
    void openGLContextClosing() override
    {
        freeAllContextObjects();
        
        if (lastTexture != nullptr)
            setTexture (lastTexture);
    }
    
    void paint(juce::Graphics& /*g*/) override {}
    
    void setTexture (DemoTexture* t)
    {
        lastTexture = textureToUse = t;
    }
    
    void freeAllContextObjects()
    {
        shapePianoKey = nullptr;
        shader = nullptr;
        attributes = nullptr;
        uniforms = nullptr;
        texture.release();
    }
    
    void resized() override
    {
        draggableOrientation.setViewport (getLocalBounds());
    }
    
    juce::Draggable3DOrientation draggableOrientation;
    float rotation;
    float scale, rotationSpeed;
    
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    std::unique_ptr<Shape> shapePianoKey;
    std::unique_ptr<Shape> shapeTeapot;
    
    std::unique_ptr<Attributes> attributes;
    std::unique_ptr<Uniforms> uniforms;
    
    juce::OpenGLTexture texture;
    DemoTexture* textureToUse, *lastTexture;
    
    juce::OpenGLContext openGLContext;
    
    //==============================================================================
    
    
    void updateShader()
    {
        if (newVertexShader.isNotEmpty() || newFragmentShader.isNotEmpty())
        {
            std::unique_ptr<juce::OpenGLShaderProgram> newShader (new juce::OpenGLShaderProgram (openGLContext));
            juce::String statusText;
            
            if (newShader->addVertexShader (juce::OpenGLHelpers::translateVertexShaderToV3 (newVertexShader))
                && newShader->addFragmentShader (juce::OpenGLHelpers::translateFragmentShaderToV3 (newFragmentShader))
                && newShader->link())
            {
                shapePianoKey = nullptr;
                shapeTeapot = nullptr;
                attributes = nullptr;
                uniforms = nullptr;
                
                shader = std::move(newShader);
                shader->use();
                
                shapePianoKey = std::make_unique<Shape> (openGLContext, BinaryData::pianokey_rectangle_obj, false);
                shapeTeapot   = std::make_unique<Shape> (openGLContext, BinaryData::teapot_obj, true);
                attributes = std::make_unique<Attributes> (openGLContext, *shader);
                uniforms   = std::make_unique<Uniforms> (openGLContext, *shader);
                
                statusText = "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
            }
            else
            {
                statusText = newShader->getLastError();
            }
            
            newVertexShader = juce::String();
            newFragmentShader = juce::String();
        }
    }
    
    //==============================================================================
    // INPUT HANDLING
    
    void sliderValueChanged (juce::Slider*) override
    {
        scale = (float) parent->zoomSlider.getValue();
    }
    
    void mouseDown (const juce::MouseEvent& e) override
    {
        draggableOrientation.mouseDown (e.getPosition());
    }
    
    void mouseDrag (const juce::MouseEvent& e) override
    {
        draggableOrientation.mouseDrag (e.getPosition());
    }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& d) override
    {
        parent->zoomSlider.setValue(parent->zoomSlider.getValue() + d.deltaY);
    }
    juce::MidiKeyboardState * midiKeyboardState;
    PluginEditor *parent;
};

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p, juce::MidiKeyboardState& midiKeyboardState)
: AudioProcessorEditor (&p), processor (p), midiKeyboardComponent(midiKeyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // TOP MIDI KEYBOARD DISPLAY
    addAndMakeVisible(midiKeyboardComponent);
    
    // BOTTOM RIGHT - OPENGL DISPLAY
    glComponent = std::make_unique<GLComponent> (midiKeyboardState, this);
    addAndMakeVisible (glComponent.get());
    
    // LEFT - OBJ FILE SELECTOR (RADIO BOX GROUP)
    radioButtonsObjSelector = new juce::GroupComponent ("OBJ Selector", "Use Obj File:");
    addAndMakeVisible (radioButtonsObjSelector);
    togglePianoKeyObj = new juce::ToggleButton("Pianokey_rectangle.obj");
    toggleTeapotObj   = new juce::ToggleButton("Teapot.obj");
    togglePianoKeyObj->setRadioGroupId(ObjSelectorButtons);
    toggleTeapotObj  ->setRadioGroupId(ObjSelectorButtons);
    togglePianoKeyObj->onClick = [this] { updateToggleState (togglePianoKeyObj,   "Pianokey_rectangle.obj");   };
    toggleTeapotObj  ->onClick = [this] { updateToggleState (toggleTeapotObj, "Teapot.obj"); };
    addAndMakeVisible(togglePianoKeyObj);
    addAndMakeVisible(toggleTeapotObj);
    
    // LEFT - ZOOM SLIDER
    addAndMakeVisible (zoomSlider);
    zoomSlider.setRange (0.0, 1.0, 0.001);
    zoomSlider.addListener (glComponent.get());
    zoomSlider.setSliderStyle (juce::Slider::LinearVertical);
    zoomLabel.attachToComponent (&zoomSlider, false);
    addAndMakeVisible (zoomLabel);
    
    // LEFT RADIO BOX: Draw Points
    toggleDrawControlMesh = new juce::ToggleButton("Draw Points");
    toggleDrawControlMesh->onClick = [this] { updateToggleState (toggleDrawControlMesh, "Draw Points");   };
    addAndMakeVisible(toggleDrawControlMesh);
    
    
    addAndMakeVisible (shaderPresetBox);
    shaderPresetBox.onChange = [this] { selectShaderPreset (shaderPresetBox.getSelectedItemIndex()); };
    
    auto presets = GLUtils::getShaderPresets();
    
    for (int i = 0; i < presets.size(); ++i)
        shaderPresetBox.addItem (presets[i].name, i + 1);
    
    addAndMakeVisible (shaderPresetBoxLabel);
    shaderPresetBoxLabel.attachToComponent (&shaderPresetBox, true);

    initialise();
    
    //setResizable(true, true);
    setSize (1000, 725);
}

PluginEditor::~PluginEditor()
{
    glComponent = nullptr;
}

void PluginEditor::initialise()
{
    togglePianoKeyObj->setToggleState(true, juce::dontSendNotification);
    toggleDrawControlMesh->setToggleState(true, juce::dontSendNotification);
    zoomSlider .setValue (0.5);
    
}

void PluginEditor::updateToggleState (juce::Button* button, juce::String name)
{
    if(name == "Draw Points") glComponent->drawControlMesh = button->getToggleState();
    else if(button->getToggleState())
    {
        if(name == "Pianokey_rectangle.obj") glComponent->drawPianoKeys = true;
        else if(name == "Teapot.obj") glComponent->drawPianoKeys = false;
    }
}

void PluginEditor::selectShaderPreset (int preset)
{
    const auto& p = GLUtils::getShaderPresets()[preset];
    setShaderProgram(p.vertexShader, p.fragmentShader);
}

void  PluginEditor::setShaderProgram (const juce::String& vertexShader, const juce::String& fragmentShader)
{
    glComponent->newVertexShader = vertexShader;
    glComponent->newFragmentShader = fragmentShader;
}
//==============================================================================

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll(backgroundColor);
}

juce::Rectangle<int> PluginEditor::getSubdividedRegion(juce::Rectangle<int> region, int numer, int denom, SubdividedOrientation orientation)
{
    int x, y, height, width;
    if(orientation == Vertical)
    {
        x = region.getX();
        width = region.getWidth();
        height = region.getHeight() / denom;
        y = region.getY() + numer * height;
    }
    else
    {
        y = region.getY();
        height = region.getHeight();
        width = region.getWidth() / denom;
        x = region.getX() + numer * width;
    }
    return juce::Rectangle<int>(x, y, width, height);
}

void PluginEditor::resized()
{
    juce::Rectangle<int> r = getLocalBounds();
    float resizedKeybWidth = r.getWidth() - MARGIN * 2.0f, resizedKeybHeight = r.getHeight() - 5.0f;
    float keybWidth = resizedKeybWidth > MAX_KEYB_WIDTH ? MAX_KEYB_WIDTH : resizedKeybWidth;
    float keybHeight = resizedKeybHeight > MAX_KEYB_HEIGHT ? MAX_KEYB_HEIGHT : resizedKeybHeight;
    midiKeyboardComponent.setBounds (MARGIN, MARGIN, (int)keybWidth, (int)keybHeight );
    
    auto areaBelowKeyboard = r.removeFromBottom(r.getHeight() - (int)(keybHeight + MARGIN));
    
    int leftToolbarWidth = r.getWidth() / 7;
    
    auto glArea = areaBelowKeyboard.removeFromRight(r.getWidth() - leftToolbarWidth);
    glComponent->setBounds(glArea);
    
    auto leftButtonToolbarArea = areaBelowKeyboard.removeFromLeft(glArea.getWidth());
    auto radioObjSelectorRegion = leftButtonToolbarArea.removeFromTop((BUTTON_HEIGHT * 2 + MARGIN*3));
    radioObjSelectorRegion.translate(0, MARGIN);
    radioButtonsObjSelector->setBounds (radioObjSelectorRegion);//MARGIN, keybHeight + MARGIN, 220, 140);
    togglePianoKeyObj->setBounds(getSubdividedRegion(radioObjSelectorRegion, 1, 3, Vertical));//
    toggleTeapotObj->setBounds (getSubdividedRegion(radioObjSelectorRegion, 2, 3, Vertical));
    
    int sliderRegionHeight = leftButtonToolbarArea.getHeight() / 4 + BUTTON_HEIGHT;
    auto sliderRegion1 = leftButtonToolbarArea.removeFromTop(sliderRegionHeight);
    zoomSlider.setBounds(sliderRegion1.removeFromBottom(sliderRegion1.getHeight() - BUTTON_HEIGHT));
    zoomSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, BUTTON_WIDTH, 20);
    
    toggleDrawControlMesh->setBounds(leftButtonToolbarArea.removeFromTop(radioObjSelectorRegion.getHeight()));
    
    shaderPresetBox.setBounds(leftButtonToolbarArea.removeFromTop(BUTTON_HEIGHT * 3));
    
}

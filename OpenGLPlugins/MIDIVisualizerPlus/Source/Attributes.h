/*
 ==============================================================================
 
 Ross Hoyt
 Attributes.h
 This class manages the attributes that the shaders use
 ==============================================================================
 */

#pragma once
#include "Vertex.h"
#include <juce_opengl/juce_opengl.h>

//==============================================================================
struct Attributes
{
    Attributes (juce::OpenGLContext& openGLContext, juce::OpenGLShaderProgram& shader)
    {
        position      = std::unique_ptr<juce::OpenGLShaderProgram::Attribute> (createAttribute (openGLContext, shader, "position"));
        normal        = std::unique_ptr<juce::OpenGLShaderProgram::Attribute> (createAttribute (openGLContext, shader, "normal"));
        sourceColour  = std::unique_ptr<juce::OpenGLShaderProgram::Attribute> (createAttribute (openGLContext, shader, "sourceColour"));
        texureCoordIn = std::unique_ptr<juce::OpenGLShaderProgram::Attribute> (createAttribute (openGLContext, shader, "texureCoordIn"));
    }
    
    void enable (juce::OpenGLContext& openGLContext)
    {
        if (position != nullptr)
        {
            openGLContext.extensions.glVertexAttribPointer (position->attributeID, 3, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, sizeof (Vertex), 0);
            openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
        }
        
        if (normal != nullptr)
        {
            openGLContext.extensions.glVertexAttribPointer (normal->attributeID, 3, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
            openGLContext.extensions.glEnableVertexAttribArray (normal->attributeID);
        }
        
        if (sourceColour != nullptr)
        {
            openGLContext.extensions.glVertexAttribPointer (sourceColour->attributeID, 4, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 6));
            openGLContext.extensions.glEnableVertexAttribArray (sourceColour->attributeID);
        }
        
        if (texureCoordIn != nullptr)
        {
            openGLContext.extensions.glVertexAttribPointer (texureCoordIn->attributeID, 2, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
            openGLContext.extensions.glEnableVertexAttribArray (texureCoordIn->attributeID);
        }
    }
    
    void disable (juce::OpenGLContext& openGLContext)
    {
        if (position != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
        if (normal != nullptr)         openGLContext.extensions.glDisableVertexAttribArray (normal->attributeID);
        if (sourceColour != nullptr)   openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
        if (texureCoordIn != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (texureCoordIn->attributeID);
    }
    
    std::unique_ptr<juce::OpenGLShaderProgram::Attribute> position, normal, sourceColour, texureCoordIn;
    
private:
    static juce::OpenGLShaderProgram::Attribute* createAttribute (juce::OpenGLContext& openGLContext,
                                                                  juce::OpenGLShaderProgram& shader,
                                                            const char* attributeName)
    {
        if (openGLContext.extensions.glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
            return nullptr;
        
        return new juce::OpenGLShaderProgram::Attribute (shader, attributeName);
    }
};

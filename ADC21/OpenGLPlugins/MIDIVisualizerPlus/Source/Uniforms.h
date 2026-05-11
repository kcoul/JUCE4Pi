/*
 ==============================================================================
 
 Ross Hoyt
 Uniforms.h
 ==============================================================================
 */

#pragma once

//==============================================================================
// Class that manages the uniform values that the shaders use.
struct Uniforms
{
    Uniforms (juce::OpenGLContext& openGLContext, juce::OpenGLShaderProgram& shader)
    {
        projectionMatrix = std::unique_ptr<juce::OpenGLShaderProgram::Uniform> (createUniform (openGLContext, shader, "projectionMatrix"));
        viewMatrix       = std::unique_ptr<juce::OpenGLShaderProgram::Uniform> (createUniform (openGLContext, shader, "viewMatrix"));
        texture          = std::unique_ptr<juce::OpenGLShaderProgram::Uniform> (createUniform (openGLContext, shader, "demoTexture"));
        lightPosition    = std::unique_ptr<juce::OpenGLShaderProgram::Uniform> (createUniform (openGLContext, shader, "lightPosition"));
    }
    
    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, texture, lightPosition;
    
private:
    static juce::OpenGLShaderProgram::Uniform* createUniform (juce::OpenGLContext& openGLContext,
                                                              juce::OpenGLShaderProgram& shader,
                                                        const char* uniformName)
    {
        if (openGLContext.extensions.glGetUniformLocation (shader.getProgramID(), uniformName) < 0)
            return nullptr;
        
        return new juce::OpenGLShaderProgram::Uniform (shader, uniformName);
    }
};

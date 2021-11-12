/*
  ==============================================================================

    Ross Hoyt
    ShaderPreset.h
    Each Shader is stored with a Name (for dispay in the selector box),
    a vertexShader, and a Fragment Shader.
  ==============================================================================
*/

#pragma once

struct ShaderPreset
{
    const char* name;
    const char* vertexShader;
    const char* fragmentShader;
};

/*
 ==============================================================================
 
 Ross Hoyt
 Shape.h
 
 ==============================================================================
 */

#pragma once
#include "Attributes.h"
#include "WavefrontObjParser.h"
#include "Vertex.h"

//==============================================================================
/** This loads a 3D model from an OBJ file and converts it into some vertex buffers
 that we can draw.
 */
struct Shape
{
    Shape (juce::OpenGLContext& openGLContext, const juce::String& objFileContent, bool tryParseMtl)
    {
        if (shapeFile.load (objFileContent, tryParseMtl).wasOk())
            for (int i = 0; i < shapeFile.shapes.size(); ++i)
                vertexBuffers.add (new VertexBuffer (openGLContext, *shapeFile.shapes.getUnchecked(i)));
        
    }
    
    void draw (juce::OpenGLContext& openGLContext, Attributes& attributes)
    {
        for (int i = 0; i < vertexBuffers.size(); ++i)
        {
            VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
            vertexBuffer.bind();
            
            attributes.enable (openGLContext);
            
            juce::gl::glDrawElements (juce::gl::GL_TRIANGLES, vertexBuffer.numIndices, juce::gl::GL_UNSIGNED_INT, 0);
            
            attributes.disable (openGLContext);
        }
    }
    
    void drawControlMesh(juce::OpenGLContext& openGLContext, Attributes& attributes)
    {
        juce::gl::glLineWidth(1);
        
        for (int i = 0; i < vertexBuffers.size(); ++i)
        {
            VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
            vertexBuffer.bind();
            
            attributes.enable (openGLContext);
            
            juce::gl::glDrawArrays(juce::gl::GL_LINE_LOOP, vertexBuffer.indexBuffer, vertexBuffer.numIndices);
            
            attributes.disable (openGLContext);
            
        }
        
        juce::gl::glFlush();
    }
    
private:
    struct VertexBuffer
    {
        VertexBuffer (juce::OpenGLContext& context, WavefrontObjFile::Shape& shape) : openGLContext (context)
        {
            numIndices = shape.mesh.indices.size();
            
            openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
            openGLContext.extensions.glBindBuffer (juce::gl::GL_ARRAY_BUFFER, vertexBuffer);
            
            juce::Array<Vertex> vertices;
            createVertexListFromMesh (shape.mesh, vertices, juce::Colours::green);
            
            openGLContext.extensions.glBufferData (juce::gl::GL_ARRAY_BUFFER, vertices.size() * (int) sizeof (Vertex),
                                                   vertices.getRawDataPointer(), juce::gl::GL_STATIC_DRAW);
            
            openGLContext.extensions.glGenBuffers (1, &indexBuffer);
            openGLContext.extensions.glBindBuffer (juce::gl::GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            openGLContext.extensions.glBufferData (juce::gl::GL_ELEMENT_ARRAY_BUFFER, numIndices * (int) sizeof (juce::uint32),
                                                   shape.mesh.indices.getRawDataPointer(), juce::gl::GL_STATIC_DRAW);
        }
        
        ~VertexBuffer()
        {
            openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
            openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
        }
        
        void bind()
        {
            openGLContext.extensions.glBindBuffer (juce::gl::GL_ARRAY_BUFFER, vertexBuffer);
            openGLContext.extensions.glBindBuffer (juce::gl::GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        }
        
        GLuint vertexBuffer, indexBuffer;
        int numIndices;
        juce::OpenGLContext& openGLContext;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
    };
    
    WavefrontObjFile shapeFile;
    juce::OwnedArray<VertexBuffer> vertexBuffers;
    
    static void createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, juce::Array<Vertex>& list, juce::Colour colour)
    {
        const float scale = 0.2f;
        WavefrontObjFile::TextureCoord defaultTexCoord = { 0.5f, 0.5f };
        WavefrontObjFile::Vertex defaultNormal = { 0.5f, 0.5f, 0.5f };
        
        for (int i = 0; i < mesh.vertices.size(); ++i)
        {
            const WavefrontObjFile::Vertex& v = mesh.vertices.getReference (i);
            
            const WavefrontObjFile::Vertex& n
            = i < mesh.normals.size() ? mesh.normals.getReference (i) : defaultNormal;
            
            const WavefrontObjFile::TextureCoord& tc
            = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i) : defaultTexCoord;
            
            Vertex vert =
            {
                { scale * v.x, scale * v.y, scale * v.z, },
                { scale * n.x, scale * n.y, scale * n.z, },
                { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                { tc.x, tc.y }
            };
            
            list.add (vert);
        }
    }
};


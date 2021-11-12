/*
 ==============================================================================
 
 Ross Hoyt
 Vertex.h
 
 
 ==============================================================================
 */

#pragma once

/** 
Vertex data to be passed to the shaders.
Each Vertex has a 3D position, a colour and a 2D texture co-ordinate
*/
struct Vertex
{
    float position[3];
    float normal[3];
    float colour[4];
    float texCoord[2];
};

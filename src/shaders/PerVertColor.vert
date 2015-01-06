#version 330 core

// Vertex shader to render points with color value per vertex.

in vec4 pos;

uniform mat4 modelViewProjectionMatrix;

void main()
{
	gl_Position = modelViewProjectionMatrix * pos;
}
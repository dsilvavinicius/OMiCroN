#version 330 core

// Fragment shader to render points with color value per vertex.

in vec4 color;
out vec4 outColor;

void main()
{
	outColor = color;
}
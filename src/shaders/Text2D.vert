#version 430 core

in vec4 in_Position;
in vec2 in_TexCoords;
out vec2 texpos;

void main(void)
{
	gl_Position = in_Position;
	texpos = in_TexCoords;
}
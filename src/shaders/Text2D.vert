#version 430 core

in vec4 in_Position;
out vec2 texpos;

void main(void)
{
	gl_Position = vec4( in_Position.xy, 0, 1 );
	texpos = in_Position.zw;
}
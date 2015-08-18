#version 430 core

in vec2 texpos;
uniform sampler2D tex;
uniform vec4 color;
out vec4 fragColor;

void main(void) {
  fragColor = vec4( 1, 1, 1, texture2D( tex, texpos ).a ) * color;
  //fragColor = vec4( 0, 0, 0, 1 );
}
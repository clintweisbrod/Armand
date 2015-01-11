#version 330

layout (location = 0) out vec4 outFragColor;

//
// Inputs
//
in vec3 starColor;

//
// Uniforms
//
uniform float uAlpha;

void main()
{
	outFragColor = vec4(starColor, uAlpha);
}
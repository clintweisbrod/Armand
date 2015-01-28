#version 330

layout (location = 0) out vec4 outFragColor;

//
// Inputs
//
in vec4 starColor;

//
// Uniforms
//
uniform float uAlpha;

void main()
{
	outFragColor = vec4(starColor.rgb, starColor.a * uAlpha);
}
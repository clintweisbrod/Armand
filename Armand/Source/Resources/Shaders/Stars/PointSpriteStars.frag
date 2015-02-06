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
uniform sampler2D uTexture;

void main()
{
	// Discard the fragments that are essentially invisible
	if (starColor.a < 0.001)
		discard;
	else
	{
		vec4 texel = texture(uTexture, gl_PointCoord);
		outFragColor = vec4(starColor.rgb, starColor.a * uAlpha) * texel;
	}
}
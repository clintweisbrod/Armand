#version 330

layout (location = 0) out vec4 outFragColor;

//
// Inputs
//
in vec3 lightIntensity;
in vec2 texCoords;

//
// Uniforms
//
uniform sampler2D	uTexture;
uniform bool		uIsTexturing;

void main()
{
	if (uIsTexturing)
	{
		// Modulate the texture color with the lightIntensity
		vec4 texColor = texture2D(uTexture, texCoords);
		outFragColor.r = lightIntensity.r * texColor.r;
		outFragColor.g = lightIntensity.g * texColor.g;
		outFragColor.b = lightIntensity.b * texColor.b;
		outFragColor.a = 1.0;
	}
	else
		outFragColor = vec4(lightIntensity, 1.0);
}
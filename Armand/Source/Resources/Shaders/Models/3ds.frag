#version 400

layout (location = 0) out vec4 outFragColor;

//
// Inputs
//
in vec3 gLightIntensity;
in vec2 gTexCoord;

//
// Uniforms
//
uniform sampler2D	uTexture;
uniform bool		uIsTexturing;

void main()
{
	if (uIsTexturing)
	{
		// Modulate the texture color with the gLightIntensity
		vec4 texColor = texture2D(uTexture, gTexCoord);
		outFragColor.r = gLightIntensity.r * texColor.r;
		outFragColor.g = gLightIntensity.g * texColor.g;
		outFragColor.b = gLightIntensity.b * texColor.b;
		outFragColor.a = 1.0;
	}
	else
		outFragColor = vec4(gLightIntensity, 1.0);
}
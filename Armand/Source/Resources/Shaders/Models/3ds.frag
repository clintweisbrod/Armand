#version 330

in vec3 lightIntensity;
in vec2 texCoords;

layout (location = 0) out vec4 outFragColor;

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
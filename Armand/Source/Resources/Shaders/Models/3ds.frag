#version 330

in vec3 LightIntensity;
in vec2 TexCoords;

layout (location = 0) out vec4 FragColor;

uniform sampler2D	uTexture;
uniform bool		uIsTexturing;

void main()
{
//	gl_FragColor = gl_Color;
//	gl_FragColor = gl_FrontMaterial.diffuse;
	
	if (uIsTexturing)
		FragColor = texture2D(uTexture, TexCoords);
	else
		FragColor = vec4(LightIntensity, 1.0);
}
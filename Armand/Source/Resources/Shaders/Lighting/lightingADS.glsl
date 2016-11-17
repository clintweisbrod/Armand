#version 400

//
// Uniforms
//
uniform float 	uAlpha;
uniform mat3	uNormalMatrix;	// Transforms vertex notmals to eye coordinates.
struct LightInfo
{
	vec3 position;	// Light position in eye coordinates.
	vec3 ambient;	// Ambient light intensity
	vec3 diffuse;	// Diffuse light intensity
	vec3 specular;	// Specular light intensity
};
uniform LightInfo uLight;

void computeLighting(in vec3 inVertexPositionInEyeCoordinates, in vec3 inNormalizedVertexPositionInEyeCoords,
					 in vec3 inNormal, in vec3 inAmbient, in vec3 inDiffuse, in vec3 inSpecular, in float inShininess,
					 out vec3 outLightIntensity)
{
	vec3 t = uNormalMatrix * inNormal;
	vec3 s = normalize(uLight.position - inVertexPositionInEyeCoordinates);
	vec3 v = -inNormalizedVertexPositionInEyeCoords;
	vec3 r = reflect(-s, t);
	vec3 ambient = uLight.ambient * inAmbient;
	float sDotN = max(dot(s, t), 0.0);
	vec3 diffuse = uLight.diffuse * inDiffuse * sDotN;
	vec3 spec = vec3(0.0);
	if (sDotN > 0.0)
		spec = uLight.specular * inSpecular * pow(max(dot(r, v), 0.0), inShininess);
	outLightIntensity = (ambient + diffuse + spec) * uAlpha;
}
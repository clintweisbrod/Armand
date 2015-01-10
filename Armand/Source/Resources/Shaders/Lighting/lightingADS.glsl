#version 330

// VAO definition
layout (location = 1) in vec3 vaoNormal;		// In local model coordinates
layout (location = 2) in vec3 vaoMaterialAmbient;
layout (location = 3) in vec3 vaoMaterialDiffuse;
layout (location = 4) in vec3 vaoMaterialSpecular;
layout (location = 5) in float vaoMaterialShininess;

//
// Globals
//
vec3 gVertexPositionInEyeCoords;
vec3 gNormalizedVertexPositionInEyeCoords;

//
// Outputs
//
out vec3 lightIntensity;

//
// Uniforms
//
uniform mat3	uNormalMatrix;	// Transforms vertex notmals to eye coordinates.
struct LightInfo
 {
	vec3 position;	// Light position in eye coordinates.
	vec3 ambient;	// Ambient light intensity
	vec3 diffuse;	// Diffuse light intensity
	vec3 specular;	// Specular light intensity
};
uniform LightInfo uLight;

void computeLighting()
{
	vec3 t = uNormalMatrix * vaoNormal;
	vec3 s = normalize(uLight.position - gVertexPositionInEyeCoords);
	vec3 v = -gNormalizedVertexPositionInEyeCoords;
	vec3 r = reflect(-s, t);
	vec3 ambient = uLight.ambient * vaoMaterialAmbient;
	float sDotN = max(dot(s, t), 0.0);
	vec3 diffuse = uLight.diffuse * vaoMaterialDiffuse * sDotN;
	vec3 spec = vec3(0.0);
	if (sDotN > 0.0)
		spec = uLight.specular * vaoMaterialSpecular * pow(max(dot(r, v), 0.0), vaoMaterialShininess);
	lightIntensity = ambient + diffuse + spec;
}
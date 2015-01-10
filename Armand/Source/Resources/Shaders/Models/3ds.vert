// See: http://paulbourke.net/dome/domegeom/

// Some explanation is in order here. To correctly render a 3DS model
// using fisheye projection, every vertex of the model must be transformed
// by the fisheye projection math. This is entirely impossible in software
// as 3DS models can typically have many thousands of vertices.
// Each vertex has a number of associated attributes besides position. The
// normal and material attributes allow us to perform lighting computations.
// The texture coordinates are used by the fragment shader to apply textures
// where necessary.
//
// Vertices entering this shader are in local 3DS model coordinates. These
// coordinates are transformed by uModelMatrix to arrive at "eye"
// coordinates. The assumption is that the viewer or "eye" is located at
// (0,0,0).
//
// In general, all vertex shaders performing fisheye projection will need
// to work within eye space. The general method of rendering ANYTHING in
// the Universe, depends on computing positions relative to the viewer.

#version 330

#include "/Projection/Fisheye.glsl"

// VAO definition
layout (location = 0) in vec3 vPosition;	// In local model coordinates
layout (location = 1) in vec3 vNormal;		// In local model coordinates
layout (location = 2) in vec3 vMaterialAmbient;
layout (location = 3) in vec3 vMaterialDiffuse;
layout (location = 4) in vec3 vMaterialSpecular;
layout (location = 5) in float vMaterialShininess;
layout (location = 6) in vec2 vTextureCoordinates;

//
// Outputs
//
out vec3 lightIntensity;
out vec2 texCoords;

//
// Uniforms
//
struct LightInfo
 {
	vec3 position;	// Light position in eye coordinates.
	vec3 ambient;	// Ambient light intensity
	vec3 diffuse;	// Diffuse light intensity
	vec3 specular;	// Specular light intensity
};
uniform LightInfo uLight;

uniform mat4	uModelMatrix;	// Transforms model coordinates to eye coordinates.
uniform mat4 	uProjectionMatrix;	// Transforms computed fisheye coordinates and depth value to clipping space.
uniform mat3	uNormalMatrix;	// Transforms vertex notmals to eye coordinates.

//
// Common function declarations
//
vec2 fisheyeProject(vec3 vPositionEyeNorm);
void setupClipPlane(vec3 vPositionEyeNorm);

void main()
{
	// Transform vertex in model coordinates to eye coordinates
	vec4 vPositionEye = uModelMatrix * vec4(vPosition, 1.0);
	
	// Sensible depth value is length of vPositionEye.xyz
	float depthValue = length(vPositionEye.xyz);
	
	// Compute: ||p-vp||	vp is assumed to be (0, 0, 0)
	vec3 vPositionEyeNorm = vPositionEye.xyz / depthValue;
	
	// Setup clipping for vertices that are behind the viewer
	setupClipPlane(vPositionEyeNorm);
	
	// Perform lighting calculations
	vec3 t = uNormalMatrix * vNormal;
	vec3 s = normalize(uLight.position - vPositionEye.xyz);
	vec3 v = -vPositionEyeNorm;
	vec3 r = reflect(-s, t);
	vec3 ambient = uLight.ambient * vMaterialAmbient;
	float sDotN = max(dot(s, t), 0.0);
	vec3 diffuse = uLight.diffuse * vMaterialDiffuse * sDotN;
	vec3 spec = vec3(0.0);
	if (sDotN > 0.0)
		spec = uLight.specular * vMaterialSpecular * pow(max(dot(r, v), 0.0), vMaterialShininess);
	lightIntensity = ambient + diffuse + spec;
	
	// Do fisheye projection
	vec2 point = fisheyeProject(vPositionEyeNorm);
	
	// Why does depthValue need to be negated???
	// I think this is because m22 in ortho matrix is negated
	gl_Position = uProjectionMatrix * vec4(point, -depthValue, 1.0);
	
	// Send the texture coordinates along to fragment shader
	texCoords = vTextureCoordinates;
}
#version 400

#include "/Projection/Fisheye.glsl"
#include "/Projection/FisheyeUtils.glsl"

const float kParsecsPerAU = 4.84813681e-6;
const float kNaturalLog10 = 2.3025850929;
const vec3	kWhiteLight = vec3(1.0, 1.0, 1.0);
const float kMinPointSize = 1.0;
const float kMaxPointSize = 5.0;

//
// VAO definition
//
layout (location = 0) in vec3 vaoPosition;	// In local model coordinates
layout (location = 1) in vec4 vaoColor;
layout (location = 2) in float vaoAbsMag;

//
// Outputs
//
out vec4 starColor;
out float gl_PointSize;

//
// Uniforms
//
uniform mat4	uModelViewMatrix;	// Transforms model coordinates to eye coordinates.
uniform float	uSaturation;
uniform float	uLimitingMagnitude;

//
// Function declarations defined in #include'd files.
//
void setupClipPlane(in vec3 inNormalizedVertexPositionInEyeCoords);
void fisheyeProject(in vec3 inVertexPositionInEyeCoordinates, out vec3 outNormalizedVertexPositionInEyeCoords,
					out vec4 outScreenPosition, out float outVertexEyeDistanceAU);

void main()
{
	// Do fisheye projection
	vec3 vertexPositionInEyeCoords = (uModelViewMatrix * vec4(vaoPosition, 1.0)).xyz;
	vec3 normalizedVertexPositionInEyeCoords;
	float vertexEyeDistanceAU;
	vec4 screenPosition;
	fisheyeProject(vertexPositionInEyeCoords, normalizedVertexPositionInEyeCoords, screenPosition, vertexEyeDistanceAU);
	gl_Position = screenPosition;
	setupClipPlane(normalizedVertexPositionInEyeCoords);

	// Mix-in white color according to uSaturation
	vec3 unsaturatedColor = mix(vaoColor.rgb, kWhiteLight, 1.0 - uSaturation);
	
	// Use vertexEyeDistanceAU and vaoAbsMag to compute apparent magnitude
	float parsecs = vertexEyeDistanceAU * kParsecsPerAU;
	float appMag = vaoAbsMag - 5.0 + 5.0 * log(parsecs) / kNaturalLog10;
	float magFactor = smoothstep(uLimitingMagnitude, -1.6, appMag);

	// Forward the star color along to the fragment shader
	starColor = vec4(unsaturatedColor, vaoColor.a * magFactor);	// Compute actual brightness (distance attenuation)

	// Set the point size
	gl_PointSize = mix(kMinPointSize, kMaxPointSize, magFactor);
}
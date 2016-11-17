#version 400

#include "/Projection/Fisheye.glsl"
#include "/Projection/FisheyeUtils.glsl"

//
// VAO definition
//
layout (location = 0) in vec3 vaoPosition;	// In local model coordinates
layout (location = 1) in vec4 vaoColor;
layout (location = 2) in float vaoSize;

//
// Outputs
//
out vec4 starColor;
out float gl_PointSize;

//
// Uniforms
//
uniform mat4	uModelViewMatrix;	// Transforms model coordinates to eye coordinates.

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

	// Set the point size
	gl_PointSize = vaoSize;

	// Set point color
	starColor = vaoColor;
}
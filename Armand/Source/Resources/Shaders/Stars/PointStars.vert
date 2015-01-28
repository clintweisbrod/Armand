#version 330

#include "/Projection/Fisheye.glsl"

const float kParsecsPerAU = 4.84813681e-6;
const float kNaturalLog10 = 2.3025850929;

//
// VAO definition
//
layout (location = 0) in vec3 vaoPosition;	// In local model coordinates
layout (location = 1) in float vaoSize;
layout (location = 2) in vec3 vaoColor;
layout (location = 3) in float vaoAbsMag;

//
// Globals
//
float gVertexEyeDistanceAU;	// Calculated by fisheyeProject()

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
void fisheyeProject(vec3 inVertexPositionInEyeCoordinates);

void main()
{
	// Do fisheye projection
	vec3 vertexPositionInEyeCoords = (uModelViewMatrix * vec4(vaoPosition, 1.0)).xyz;
	fisheyeProject(vertexPositionInEyeCoords);

	// Use gVertexEyeDistanceAU and vaoAbsMag to compute apparent magnitude
	float parsecs = gVertexEyeDistanceAU * kParsecsPerAU;
	float appMag = vaoAbsMag - 5.0 + 5.0 * log(parsecs) / kNaturalLog10;

	// Forward the star color along to the fragment shader
	starColor = vec4(vaoColor, smoothstep(6.0, -1.6, appMag));	// Compute actual brightness (distance attenuation)
//	starColor = vec4(vaoColor, 1.0);							// No distance attenuation
	
	// Set the point size
	gl_PointSize = vaoSize;
}
#version 330

#include "/Projection/Fisheye.glsl"

const float kParsecsPerAU = 4.84813681e-6;
const float kNaturalLog10 = 2.3025850929;
const vec3	kWhiteLight = vec3(1.0, 1.0, 1.0);

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
uniform float	uSaturation;
uniform bool	uUseMagnitude;

//
// Function declarations defined in #include'd files.
//
void fisheyeProject(vec3 inVertexPositionInEyeCoordinates);

void main()
{
	// Do fisheye projection
	vec3 vertexPositionInEyeCoords = (uModelViewMatrix * vec4(vaoPosition, 1.0)).xyz;
	fisheyeProject(vertexPositionInEyeCoords);

	// Mix-in white color according to uSaturation
	vec3 unsaturatedColor = mix(vaoColor, kWhiteLight, 1.0 - uSaturation);

	// Use gVertexEyeDistanceAU and vaoAbsMag to compute apparent magnitude
	if (uUseMagnitude)
	{
		float parsecs = gVertexEyeDistanceAU * kParsecsPerAU;
		float appMag = vaoAbsMag - 5.0 + 5.0 * log(parsecs) / kNaturalLog10;
		float magFactor = smoothstep(7.0, -1.6, appMag);

		// Forward the star color along to the fragment shader
		starColor = vec4(unsaturatedColor, magFactor);	// Compute actual brightness (distance attenuation)

		// Set the point size
		gl_PointSize = 1 + magFactor * 8;
	}
	else
	{
		starColor = vec4(unsaturatedColor, 1.0);		// No distance attenuation

		// Set the point size
		gl_PointSize = vaoSize;
	}
}
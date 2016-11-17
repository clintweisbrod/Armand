#version 400

#include "/Projection/Fisheye.glsl"
#include "/Projection/FisheyeUtils.glsl"
#include "/Lighting/lightingADS.glsl"

// Any geometry shader is fed as input, one primitive defined by the vertix information located
// in gl_in[0 - N-1], where N is the number of vertices that compose the input primitive as
// declared in the input layout statement below. In this case, the input primitive is a triangle
// so N=3.
// A geometry shader outputs individual vertices to produce new primitives as defined in the
// output layout statement below. In this particular case, we are outputting triangle strips but
// we only want to output one triangle for each invocation of this shader. We therefore call
// EndPrimitive() after emitting 3 vertices.

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 tePosition[3];
in vec3 teNormal[3];
in vec3 teMaterialAmbient[3];
in vec3 teMaterialDiffuse[3];
in vec3 teMaterialSpecular[3];
in float teMaterialShininess[3];
in vec2 teTexCoord[3];

out vec2 gTexCoord;
out vec3 gLightIntensity;

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
void computeLighting(in vec3 inVertexPositionInEyeCoordinates, in vec3 inNormalizedVertexPositionInEyeCoords,
					 in vec3 inNormal, in vec3 inAmbient, in vec3 inDiffuse, in vec3 inSpecular, in float inShininess,
					 out vec3 outLightIntensity);

void main()
{
	// For each vertex:
	// Transform tePosition from local model coordinates to eye coordinates,
	// do fisheye projection, perform lighting calculations, and output
	// texture coordinates to fragment shader.
	float outVertexEyeDistanceAU;
	vec3 vertexPositionInEyeCoords, normalizedVertexPositionInEyeCoords;
	vec4 screenPosition;
	
	vertexPositionInEyeCoords = (uModelViewMatrix * vec4(tePosition[0], 1.0)).xyz;
	fisheyeProject(vertexPositionInEyeCoords, normalizedVertexPositionInEyeCoords, screenPosition, outVertexEyeDistanceAU);
	gl_Position = screenPosition;
	setupClipPlane(normalizedVertexPositionInEyeCoords);
	computeLighting(vertexPositionInEyeCoords, normalizedVertexPositionInEyeCoords,
					teNormal[0], teMaterialAmbient[0], teMaterialDiffuse[0], teMaterialSpecular[0], teMaterialShininess[0],
					gLightIntensity);
	gTexCoord = teTexCoord[0];
	EmitVertex();

	vertexPositionInEyeCoords = (uModelViewMatrix * vec4(tePosition[1], 1.0)).xyz;
	fisheyeProject(vertexPositionInEyeCoords, normalizedVertexPositionInEyeCoords, screenPosition, outVertexEyeDistanceAU);
	gl_Position = screenPosition;
	setupClipPlane(normalizedVertexPositionInEyeCoords);
	computeLighting(vertexPositionInEyeCoords, normalizedVertexPositionInEyeCoords,
					teNormal[1], teMaterialAmbient[1], teMaterialDiffuse[1], teMaterialSpecular[1], teMaterialShininess[1],
					gLightIntensity);
	gTexCoord = teTexCoord[1];
	EmitVertex();

	vertexPositionInEyeCoords = (uModelViewMatrix * vec4(tePosition[2], 1.0)).xyz;
	fisheyeProject(vertexPositionInEyeCoords, normalizedVertexPositionInEyeCoords, screenPosition, outVertexEyeDistanceAU);
	gl_Position = screenPosition;
	setupClipPlane(normalizedVertexPositionInEyeCoords);
	computeLighting(vertexPositionInEyeCoords, normalizedVertexPositionInEyeCoords,
					teNormal[2], teMaterialAmbient[2], teMaterialDiffuse[2], teMaterialSpecular[2], teMaterialShininess[2],
					gLightIntensity);
	gTexCoord = teTexCoord[2];
	EmitVertex();

	// Calling EndPrimitive() causes individual triangles to be output
	EndPrimitive();
}
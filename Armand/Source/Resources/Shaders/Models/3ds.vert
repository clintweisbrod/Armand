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
// the Universe, depends on computing positions relative to the viewer, so
// this is nothing new.

#version 330

#include "/Projection/Fisheye.glsl"
#include "/Lighting/lightingADS.glsl"

//
// VAO definition
//
layout (location = 0) in vec3 vaoPosition;	// In local model coordinates
layout (location = 6) in vec2 vaoTextureCoordinates;

//
// Outputs
//
out vec2 texCoords;

//
// Uniforms
//
uniform mat4	uModelViewMatrix;	// Transforms model coordinates to eye coordinates.

//
// Function declarations defined in #include'd files.
//
void fisheyeProject(vec3 inVertexPositionInEyeCoordinates);
void computeLighting(vec3 inVertexPositionInEyeCoordinates);

void main()
{
	// Do fisheye projection
	vec3 vertexPositionInEyeCoords = (uModelViewMatrix * vec4(vaoPosition, 1.0)).xyz;
	fisheyeProject(vertexPositionInEyeCoords);
	
	// Perform lighting calculations
	computeLighting(vertexPositionInEyeCoords);
	
	// Send the texture coordinates along to fragment shader
	texCoords = vaoTextureCoordinates;
}
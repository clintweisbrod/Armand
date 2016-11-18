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

#version 400

//
// VAO definition
//
layout (location = 0) in vec3 vaoPosition;	// In local model coordinates
layout (location = 1) in vec3 vaoNormal;	// In local model coordinates
layout (location = 2) in vec3 vaoMaterialAmbient;
layout (location = 3) in vec3 vaoMaterialDiffuse;
layout (location = 4) in vec3 vaoMaterialSpecular;
layout (location = 5) in float vaoMaterialShininess;
layout (location = 6) in vec2 vaoTextureCoordinates;

//
// Outputs
//
out vec3 vNormal;
out vec3 vMaterialAmbient;
out vec3 vMaterialDiffuse;
out vec3 vMaterialSpecular;
out float vMaterialShininess;
out vec2 vTexCoord;

void main()
{
	gl_Position = vec4(vaoPosition, 1);	// Write to the built-in Position variable.
	vNormal = vaoNormal;
	vMaterialAmbient = vaoMaterialAmbient;
	vMaterialDiffuse = vaoMaterialDiffuse;
	vMaterialSpecular = vaoMaterialSpecular;
	vMaterialShininess = vaoMaterialShininess;
	vTexCoord = vaoTextureCoordinates;
}
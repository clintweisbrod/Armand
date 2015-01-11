#version 330

#include "/Projection/Fisheye.glsl"

//
// Outputs
//
out vec3 starColor;

//
// Function declarations defined in #include'd files.
//
void fisheyeProject();

void main()
{
	// Do fisheye projection
	fisheyeProject();	
}
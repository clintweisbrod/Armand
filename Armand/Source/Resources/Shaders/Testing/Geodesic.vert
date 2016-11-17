#version 400

layout (location = 0) in vec4 Position;
//out vec3 vPosition;	// This becomes an array of 3 values in the TCS because we made this
						// call; glPatchParameteri(GL_PATCH_VERTICES, 3) in the application code.

void main()
{
	gl_Position = Position;	// Write to the built-in Position variable.
							// The TCS will find this in gl_in[gl_InvocationID].gl_Position
//	vPosition = Position.xyz;
}
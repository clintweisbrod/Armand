#version 400

//
// Outputs
//
out float gl_ClipDistance[1];		// Need this to clip vertices behind viewer

//
// Uniforms
//
uniform float	uClipPlaneDistance;	// Is only dependent on uAperture and requires a cos() to calculate so
									// we calculate once on CPU rather than per-vertex.
uniform vec3	uViewDirection;		// This points to the center of the fisheye space.

void setupClipPlane(in vec3 inNormalizedVertexPositionInEyeCoords)
{
	vec4 clipPlane = vec4(uViewDirection, uClipPlaneDistance);
	gl_ClipDistance[0] = dot(vec4(inNormalizedVertexPositionInEyeCoords, 1.0), clipPlane);
}
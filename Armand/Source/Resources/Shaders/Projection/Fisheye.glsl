#version 330

//
// Globals
//
vec3 gVertexPositionInEyeCoords;
vec3 gNormalizedVertexPositionInEyeCoords;

// VAO definition
layout (location = 0) in vec3 vaoPosition;	// In local model coordinates

//
// Outputs
//
out float gl_ClipDistance[1];	// Need this to clip vertices behind viewer

//
// Uniforms
//
uniform float	uAperture;			// In radians. Typically a value near PI.
uniform float	uClipPlaneDistance;	// Is only dependent on uAperture and requires a cos().
uniform vec3	uViewDirection;		// This points to the center of the fisheye space.
uniform vec3	uUpDirection;		// The "up" direction relative to uViewDirection.
uniform vec3	uLeftDirection;		// The left direction relative to uViewDirection and uUpDirection.
uniform mat4	uModelMatrix;	// Transforms model coordinates to eye coordinates.
uniform mat4 	uProjectionMatrix;	// Transforms computed fisheye coordinates and depth value to clipping space.

void setupClipPlane()
{
	float halfAperture = uAperture * 0.5;
	vec4 clipPlane = vec4(uViewDirection, uClipPlaneDistance);
	gl_ClipDistance[0] = dot(vec4(gNormalizedVertexPositionInEyeCoords, 1.0), clipPlane);
}

void fisheyeProject()
{
	// Transform vertex in model coordinates to eye coordinates
	gVertexPositionInEyeCoords = (uModelMatrix * vec4(vaoPosition, 1.0)).xyz;
	
	// Sensible depth value is length of gVertexPositionInEyeCoords
	float depthValue = length(gVertexPositionInEyeCoords);
	
	// Compute: ||p-vp||	vp is assumed to be (0, 0, 0)
	gNormalizedVertexPositionInEyeCoords = gVertexPositionInEyeCoords / depthValue;
	
	// Setup clipping for vertices that are behind the viewer
	setupClipPlane();
	
	// Perform fisheye transformation
	vec2 point = vec2(0.0, 0.0);

	// Compute angular distance between view direction (center of projection) and vertex position
	// and scale by aperture.
	float halfAperture = uAperture * 0.5;
	float fisheyeAngleFactor = acos(dot(uViewDirection, gNormalizedVertexPositionInEyeCoords)) / halfAperture;
	if (fisheyeAngleFactor > 0.0)
	{
		// Compute normalized proportions of up and left relative to center of projection
		vec2 xyComponents = vec2(dot(gNormalizedVertexPositionInEyeCoords, uLeftDirection),
								 dot(gNormalizedVertexPositionInEyeCoords, uUpDirection));
		xyComponents = normalize(xyComponents);
		
		// Use the normalized up and left components and the angular distance
		point.x = -fisheyeAngleFactor * xyComponents.x;
		point.y = fisheyeAngleFactor * xyComponents.y;
	}
	
	// Why does depthValue need to be negated???
	// I think this is because m22 in ortho matrix is negated
	gl_Position = uProjectionMatrix * vec4(point, -depthValue, 1.0);
}

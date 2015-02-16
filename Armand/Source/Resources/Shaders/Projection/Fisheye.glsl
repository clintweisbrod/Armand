#version 330

//
// Globals
//
vec3 gNormalizedVertexPositionInEyeCoords;
float gVertexEyeDistanceAU;

//
// Outputs
//
out float gl_ClipDistance[1];		// Need this to clip vertices behind viewer

//
// Uniforms
//
uniform float	uHalfAperture;		// In radians. Typically a value near PI/2.
uniform float	uClipPlaneDistance;	// Is only dependent on uAperture and requires a cos() to calculate so
									// we calculate once on CPU rather than per-vertex.

									// The following 3 vec3 uniforms are in eye coordinates.
uniform vec3	uViewDirection;		// This points to the center of the fisheye space.
uniform vec3	uUpDirection;		// The "up" direction relative to uViewDirection.
uniform vec3	uLeftDirection;		// The left direction relative to uViewDirection and uUpDirection.

uniform mat4 	uProjectionMatrix;	// Transforms computed fisheye coordinates and depth value to clipping space.

void setupClipPlane()
{
	vec4 clipPlane = vec4(uViewDirection, uClipPlaneDistance);
	gl_ClipDistance[0] = dot(vec4(gNormalizedVertexPositionInEyeCoords, 1.0), clipPlane);
}

//
// The end goal of this method is to transform inVertexPositionInEyeCoordinates to gl_Position
// which is in homogeneous coordinates. This requires computing the angular distance between
// uViewDirection and inVertexPositionInEyeCoordinates, and then transforming that angle (based
// on uAperture) to 2D screen coordinates. Besides this, this method also computes the global
// variables; gNormalizedVertexPositionInEyeCoords and gVertexEyeDistanceAU.
// gNormalizedVertexPositionInEyeCoords is used externally by lightingADS.glsl.
// gVertexEyeDistanceAU is used externally by PointStars.vert.
//
void fisheyeProject(vec3 inVertexPositionInEyeCoordinates)
{
	// Sensible depth value is length of inVertexPositionInEyeCoordinates
	gVertexEyeDistanceAU = length(inVertexPositionInEyeCoordinates);
	
	// Need normalized version of inVertexPositionInEyeCoordinates.
	gNormalizedVertexPositionInEyeCoords = inVertexPositionInEyeCoordinates / gVertexEyeDistanceAU;
	
	// Setup clipping for vertices that are behind the viewer
	setupClipPlane();
	
	// Perform fisheye transformation
	vec2 point = vec2(0.0, 0.0);

	// Compute angular distance between view direction (center of projection) and vertex position
	// and scale by aperture.
	float fisheyeAngleFactor = acos(dot(uViewDirection, gNormalizedVertexPositionInEyeCoords)) / uHalfAperture;
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
	
	// Why does gVertexEyeDistanceAU need to be negated???
	// I think this is because m22 in ortho matrix is negated.
	// Compute the homogeneous gl_Position output variable.
	gl_Position = uProjectionMatrix * vec4(point, -gVertexEyeDistanceAU, 1.0);
}

#version 400

//
// Uniforms
//
uniform float	uHalfAperture;		// In radians. Typically a value near PI/2.
									// The following 3 vec3 uniforms are in eye coordinates.
uniform vec3	uViewDirection;		// This points to the center of the fisheye space.
uniform vec3	uUpDirection;		// The "up" direction relative to uViewDirection.
uniform vec3	uLeftDirection;		// The left direction relative to uViewDirection and uUpDirection.

uniform mat4 	uProjectionMatrix;	// Transforms computed fisheye coordinates and depth value to clipping space.

//
// The end goal of this method is to transform inVertexPositionInEyeCoordinates to gl_Position
// which is in homogeneous coordinates. This requires computing the angular distance between
// uViewDirection and inVertexPositionInEyeCoordinates, and then transforming that angle (based
// on uAperture) to 2D screen coordinates. Besides this, this method also computes the global
// variables; gNormalizedVertexPositionInEyeCoords and gVertexEyeDistanceAU.
// gNormalizedVertexPositionInEyeCoords is used externally by lightingADS.glsl.
// gVertexEyeDistanceAU is used externally by PointStars.vert.
//
void fisheyeProject(in vec3 inVertexPositionInEyeCoordinates, out vec3 outNormalizedVertexPositionInEyeCoords,
					out vec4 outScreenPosition, out float outVertexEyeDistanceAU)
{
	// Sensible depth value is length of inVertexPositionInEyeCoordinates
	outVertexEyeDistanceAU = length(inVertexPositionInEyeCoordinates);
	
	// Need normalized version of inVertexPositionInEyeCoordinates.
	outNormalizedVertexPositionInEyeCoords = inVertexPositionInEyeCoordinates / outVertexEyeDistanceAU;
	
	// Setup clipping for vertices that are behind the viewer
//	setupClipPlane(outNormalizedVertexPositionInEyeCoords);
	
	// Perform fisheye transformation
	vec2 point = vec2(0.0, 0.0);

	// Compute angular distance between view direction (center of projection) and vertex position
	// and scale by aperture.
	float fisheyeAngleFactor = acos(dot(uViewDirection, outNormalizedVertexPositionInEyeCoords)) / uHalfAperture;
	if (fisheyeAngleFactor > 0.0)
	{
		// Compute normalized proportions of up and left relative to center of projection
		vec2 xyComponents = vec2(dot(outNormalizedVertexPositionInEyeCoords, uLeftDirection),
								 dot(outNormalizedVertexPositionInEyeCoords, uUpDirection));
		xyComponents = normalize(xyComponents);
		
		// Use the normalized up and left components and the angular distance
		point.x = -fisheyeAngleFactor * xyComponents.x;
		point.y = fisheyeAngleFactor * xyComponents.y;
	}
	
	// Why does outVertexEyeDistanceAU need to be negated???
	// I think this is because m22 in ortho matrix is negated.
	// Compute the homogeneous gl_Position output variable.
	outScreenPosition = uProjectionMatrix * vec4(point, -outVertexEyeDistanceAU, 1.0);
//	gl_Position = uProjectionMatrix * vec4(point, -outVertexEyeDistanceAU, 1.0);
}

#version 330

//
// Outputs
//
out float gl_ClipDistance[1];	// Need this to clip vertices behind viewer

//
// Uniforms
//
uniform float	uAperture;		// In radians. Typically a value near PI.
uniform float	uClipPlaneDistance;	// Is only dependent on uAperture and requires a cos().
uniform vec3	uViewDirection;	// This points to the center of the fisheye space.
uniform vec3	uUpDirection;
uniform vec3	uLeftDirection;

vec2 fisheyeProject(vec3 vPositionEyeNorm)
{
	vec2 result = vec2(0.0, 0.0);

	// Compute angular distance between view direction (center of projection) and vertex position
	// and scale by aperture.
	float halfAperture = uAperture * 0.5;
	float fisheyeAngleFactor = acos(dot(uViewDirection, vPositionEyeNorm)) / halfAperture;
	if (fisheyeAngleFactor > 0.0)
	{
		// Compute normalized proportions of up and left relative to center of projection
		vec2 xyComponents = vec2(dot(vPositionEyeNorm, uLeftDirection), dot(vPositionEyeNorm, uUpDirection));
		xyComponents = normalize(xyComponents);
		
		// Use the normalized up and left components and the angular distance
		result.x = -fisheyeAngleFactor * xyComponents.x;
		result.y = fisheyeAngleFactor * xyComponents.y;
	}
	
	return result;
}

void setupClipPlane(vec3 vPositionEyeNorm)
{
	float halfAperture = uAperture * 0.5;
	vec4 clipPlane = vec4(uViewDirection, uClipPlaneDistance);
	gl_ClipDistance[0] = dot(vec4(vPositionEyeNorm, 1.0), clipPlane);
}
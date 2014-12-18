uniform float	uAperture;
uniform vec3	uVD;	// View direction

uniform mat4	uView;
uniform mat4 	uProjection;

//attribute vec3 vertex;
//attribute vec4 color;

void main()
{
	// Take vertex in world coordinates and transform to eye coordinates
	vec4 eyePoint = uView * gl_Vertex;
	
	// Compute: ||p-vp||
	vec3 eyePointNorm = normalize(eyePoint.xyz);
	
	// Compute: acos(vd . ||p-vp||)
	float eyePointViewDirectionAngle = acos(dot(uVD, eyePointNorm));
	
	// Compute: x = acos(vd . ||p-vp||) (||p-vp|| . vr) / (pi/2)
	//			z = acos(vd . ||p-vp||) (||p-vp|| . vu) / (pi/2)
	
	vec2 xyComponents = vec2(eyePointNorm.x, eyePointNorm.y);
	xyComponents = normalize(xyComponents);
	float halfAperture = uAperture * 0.5;
	vec2 point;
//	point.x = eyePointViewDirectionAngle * dot(eyePointNorm, uVR) / halfAperture;
//	point.y = -eyePointViewDirectionAngle * dot(eyePointNorm, uVU) / halfAperture;
	point.x = eyePointViewDirectionAngle * xyComponents.x / halfAperture;
	point.y = -eyePointViewDirectionAngle * xyComponents.y / halfAperture;
	
	gl_Position = uProjection * vec4(point, 0.0, 1.0);
}
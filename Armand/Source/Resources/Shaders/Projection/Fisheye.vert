uniform float	uAperture;
uniform vec3	uVD;	// View direction
uniform vec3	uVU;	// View up
uniform vec3	uVR;	// View right
uniform vec3	uVP;	// Eye position

uniform mat4 	uProjection;

attribute vec3 vertex;
attribute vec4 color;

void main()
{
	// Take vertex and transform to point
	
	// Compute: ||p-vp||
//	vec3 eyePoint = vertex - uVP;
	vec3 eyePoint = uVP - vertex;
	vec3 eyePointNorm = normalize(eyePoint);
	
	// Compute: acos(vd . ||p-vp||)
	float eyePointViewDirectionAngle = acos(dot(uVD, eyePointNorm));
	
	// Compute: x = acos(vd . ||p-vp||) (||p-vp|| . vr) / (pi/2)
	//			z = acos(vd . ||p-vp||) (||p-vp|| . vu) / (pi/2)
	
	float halfAperture = uAperture * 0.5;
	vec2 point;
	point.x = eyePointViewDirectionAngle * dot(eyePointNorm, uVR) / halfAperture;
	point.y = eyePointViewDirectionAngle * dot(eyePointNorm, uVU) / halfAperture;
	
	gl_FrontColor = color;
	gl_Position = uProjection * vec4(point, 0.0, 1.0);
}
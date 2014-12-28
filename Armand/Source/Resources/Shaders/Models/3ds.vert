// See: http://paulbourke.net/dome/domegeom/

#version 330

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 MaterialAmbient;
layout (location = 3) in vec3 MaterialDiffuse;
layout (location = 4) in vec3 MaterialSpecular;
layout (location = 5) in float MaterialShininess;
layout (location = 6) in vec2 TextureCoordinates;

out vec3 LightIntensity;
out vec2 TexCoords;

uniform float	uAperture;
uniform vec3	uVD;	// View direction

uniform mat4	uView;
uniform mat4 	uProjection;

//attribute vec3 vertex;
//attribute vec4 color;

void main()
{
	// Take vertex in world coordinates and transform to eye coordinates
	vec4 eyePoint = uView * vec4(VertexPosition, 1.0);
	
	// Sensible depth value is uVD . eyePoint
	float depthValue = dot(uVD, eyePoint.xyz);
	
	// Compute: ||p-vp||
	vec3 eyePointNorm = normalize(eyePoint.xyz);
	
	vec2 point = vec2(0.0, 0.0);
	
	// Compute: acos(vd . ||p-vp||)
	float eyePointViewDirectionAngle = acos(dot(uVD, eyePointNorm));
	if (eyePointViewDirectionAngle > 0.0)
	{
		// Compute: x = acos(vd . ||p-vp||) (||p-vp|| . vr) / (pi/2)
		//			z = acos(vd . ||p-vp||) (||p-vp|| . vu) / (pi/2)
	
		vec2 xyComponents = vec2(eyePointNorm.x, eyePointNorm.y);
		xyComponents = normalize(xyComponents);
		float halfAperture = uAperture * 0.5;
		point.x = eyePointViewDirectionAngle * xyComponents.x / halfAperture;
		point.y = -eyePointViewDirectionAngle * xyComponents.y / halfAperture;
	}
	
	// Why does depthValue need to be negated???
	// I think this is because m22 in ortho matrix is negated
	gl_Position = uProjection * vec4(point, -depthValue, 1.0);
	
	LightIntensity = MaterialDiffuse;
	TexCoords = TextureCoordinates;
}
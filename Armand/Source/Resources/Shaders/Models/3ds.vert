// See: http://paulbourke.net/dome/domegeom/

#version 330

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vMaterialAmbient;
layout (location = 3) in vec3 vMaterialDiffuse;
layout (location = 4) in vec3 vMaterialSpecular;
layout (location = 5) in float vMaterialShininess;
layout (location = 6) in vec2 vTextureCoordinates;

out vec3 lightIntensity;
out vec2 texCoords;
out float gl_ClipDistance[1];	// Need this to clip vertices behind viewer

struct LightInfo
 {
	vec3 position;	// Light position in eye coords.
	vec3 ambient;	// Ambient light intensity
	vec3 diffuse;	// Diffuse light intensity
	vec3 specular;	// Specular light intensity
};
uniform LightInfo uLight;

uniform float	uAperture;
uniform vec3	uViewDirection;	// In eye coordinates

uniform mat4	uViewMatrix;
uniform mat4 	uProjectionMatrix;
uniform mat3	uNormalMatrix;

void main()
{
	// Transform vertex in world coordinates to eye coordinates
	vec4 eyePoint = uViewMatrix * vec4(vPosition, 1.0);
	
	// Compute: ||p-vp||
	vec3 eyePointNorm = normalize(eyePoint.xyz);
	
	// Setup clipping for vertices that are behind the viewer
	float halfAperture = uAperture * 0.5;
	vec4 clipPlane = vec4(0.0, 0.0, 1.0, -cos(halfAperture));
	gl_ClipDistance[0] = dot(vec4(eyePointNorm, 1.0), clipPlane); 
	
	// Perform lighting calculations
	vec3 t = uNormalMatrix * vNormal;
	vec3 s = normalize(uLight.position - eyePoint.xyz);
	vec3 v = -eyePointNorm;
	vec3 r = reflect(-s, t);
	vec3 ambient = uLight.ambient * vMaterialAmbient;
	float sDotN = max(dot(s, t), 0.0);
	vec3 diffuse = uLight.diffuse * vMaterialDiffuse * sDotN;
	vec3 spec = vec3(0.0);
	if (sDotN > 0.0)
		spec = uLight.specular * vMaterialSpecular * pow(max(dot(r, v), 0.0), vMaterialShininess);
	lightIntensity = ambient + diffuse + spec;

	// Sensible depth value is uViewDirection . eyePoint
	float depthValue = dot(uViewDirection, eyePoint.xyz);
	
	// Compute: acos(vd . ||p-vp||)
	float vDotE = dot(uViewDirection, eyePointNorm);
	float eyePointViewDirectionAngle = acos(vDotE);
	vec2 point = vec2(0.0, 0.0);
	if (eyePointViewDirectionAngle > 0.0)
	{
		// Compute: x = acos(vd . ||p-vp||) (||p-vp|| . vr) / (pi/2)
		//			z = acos(vd . ||p-vp||) (||p-vp|| . vu) / (pi/2)
	
		vec2 xyComponents = vec2(eyePointNorm.x, eyePointNorm.y);
		xyComponents = normalize(xyComponents);
		point.x = eyePointViewDirectionAngle * xyComponents.x / halfAperture;
		point.y = -eyePointViewDirectionAngle * xyComponents.y / halfAperture;
	}
	
	// Why does depthValue need to be negated???
	// I think this is because m22 in ortho matrix is negated
	gl_Position = uProjectionMatrix * vec4(point, -depthValue, 1.0);
	
	// Send the texture coordinates along to fragment shader
	texCoords = vTextureCoordinates;
}
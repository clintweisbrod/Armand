#version 400

out vec4 FragColor;

// These values get interpolated for the fragment
in vec3 gFacetNormal;
in vec3 gTriDistance;
in vec3 gPatchDistance;

uniform vec3 LightPosition;
uniform vec3 DiffuseMaterial;
uniform vec3 AmbientMaterial;

float amplify(float d, float scale, float offset)
{
	d = scale * d + offset;
	d = clamp(d, 0, 1);
	d = 1 - exp2(-2*d*d);
	
	return d;
}

void main()
{
	// Do some basic lighting calculations.
	vec3 N = normalize(gFacetNormal);
	vec3 L = LightPosition;
	float df = abs(dot(N, L));
	vec3 color = AmbientMaterial + df * DiffuseMaterial;

	// Modulate the color to effect both thin and thick black lines to
	// show the boundaries of both the output patch triangle and the resulting
	// triangles from the tesselation, respectively. IOW, gTriDistance is used
	// to control the thin lines, while gPatchDistance is used to control the
	// thick lines.
	float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);
	float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);
	color = amplify(d1, 40, -0.5) * amplify(d2, 120, -0.5) * color;
//	color = vec3(gTriDistance.x, gTriDistance.y, gTriDistance.z);
//	color = vec3(gPatchDistance.x, gPatchDistance.y, gPatchDistance.z);

	// Output the fragment color
	FragColor = vec4(color, 1.0);
}
#include "stdafx.h"
#include "vecmath.h"
#include "ttmath/ttmathbig.h"

// This function converts a 128-bit integer vector to floating point equivalent.
void Vec3i128toVec3d(Vec3i128& in, Vec3d& out)
{
	ttmath::Big<1, 2> temp;

	// Convert the x component
	temp.FromInt(in.x);
	temp.ToDouble(out.x);

	// Convert the y component
	temp.FromInt(in.y);
	temp.ToDouble(out.y);

	// Convert the z component
	temp.FromInt(in.z);
	temp.ToDouble(out.z);
}
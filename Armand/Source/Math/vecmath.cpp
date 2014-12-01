#include "stdafx.h"
#include "vecmath.h"
#include "ttmath/ttmathbig.h"

/*
Given that the observable Universe is a sphere with radius on the order of 50 billion
light years, a light year is 9.46e15 metres, and there are 1000 millimetres in a metre.
We have a radius of roughly 4.7e29 millimetres. The number of bits required to represent
such a number in integer format is:

bits = log(4.7e29) / log(2) = 98.6, rounded up to 99 bits, +1 for signed.
= 100 bits.

128 bits is therefore more than adequate to represent any object's position in the Universe
with millimetre precision. In fact, the extra 28 bits allows us to position objects down
to the millimeter in a universe 360 million times larger than our own.
*/

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
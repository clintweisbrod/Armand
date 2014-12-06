// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// vecmath.cpp
//
// Vector math library.
//
// THIS SOFTWARE IS PROVIDED BY CLINT WEISBROD "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL CLINT WEISBROD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------

#include "stdafx.h"
#include "vecmath.h"
#include "ttmath/ttmathbig.h"

// A pretty fundamental question about rendering things on a large scale like the
// observable universe is the limited accuracy available in floating point numbers.
// The problem with floating point is that it can only represent a finite set of values
// within the infinite set of real numbers. For very large and very small numbers, the
// precision becomes hoplessly inadequate. In a universe roughly 50 billion light years
// in all directions, using floating point to represent the positions of all objects
// will quickly become inadequate. Since we will be needing to subtract two very large
// numbers in order to compute positions relative to the viewer, round-off errors will
// swamp the calculations, making the results useless.
//
// So, floating point is not an option. That leaves us with integers. Since we're
// running on a 64-bit platform for the forseeable future, we can natively represent
// signed integers with 63-bits. This allows us to represent numbers up to about 9.2e18.
// What kind of spatial precision does this buy us?
//
// Given that the observable Universe is a sphere with radius on the order of 50 billion
// light years and a light year is 9.46e15 metres, we have a dimensional requirement of
// roughly 4.7e26 metres. Dividing this value by our maximum 63-bit integer size gives us
// a precision of 51413 km. No good. We want millimeter precision (or better). We have no
// choice but to require more than 63 bits.
//
// How many bits do we need to use to represent positions in the observable universe with
// millimeters?
//
// bits = log(4.7e29) / log(2) = 98.6, rounded up to 99 bits, +1 for signed, = 100 bits.
//
// We will therefore use two unsigned 64-bit integers to represent positions. Since we
// want calculations to be as fast as possible, there is no point in introducing extra
// overhead to manage exactly 100 bits, so we use 128-bit integers.
//
// 128 bits is more than adequate to represent the position of an object in the Universe
// with millimetre precision. In fact, the extra 28 bits allows us to position objects
// down to the millimeter in a universe 360 million times larger than our own. Or looked
// at another way, within our observable universe, the extra 28 bits gives us a precision
// of roughly 2.8 nanometers!


// This function converts a 128-bit integer vector to floating point equivalent. Why do
// we need this? After all the above talk of useless precision in floating point, why do
// we want to convert 128-bit integers to floating point representation? All the usual
// graphics pipeline computations need to occur in floating point. To transform a 128-bit
// position into screen cooridinates, we must subtract the viewer's position from it to
// obtain a relative position. This subtraction operation needs to be very accurate. In
// general, the subtraction result will be (relatively speaking) a small value that
// floating point math will handle with sufficient precision.
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
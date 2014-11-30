// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// mathlib.h
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

#pragma once

#include <cmath>
#include <stdlib.h>

#define PI 3.14159265358979323846

template<class T> class Math
{
public:
    static inline void sincos(T, T&, T&);
    static inline T frand();
    static inline T sfrand();
    static inline T lerp(T t, T a, T b);
    static inline T clamp(T t);
	static inline void constrain(T& ioVal, T inMin, T inMax);

	static T lerp2(T inX1, T inX2, T inY1, T inY2, T inX);
	static T lerp2NoConstrain(T inX1, T inX2, T inY1, T inY2, T inX);

private:
    // This class is static and should not be instantiated
    Math() {};
};


typedef Math<int> Mathi;
typedef Math<float> Mathf;
typedef Math<double> Mathd;


template<class T> T degToRad(T d)
{
    return d / 180 * static_cast<T>(PI);
}

template<class T> T radToDeg(T r)
{
    return r * 180 / static_cast<T>(PI);
}

template<class T> T abs(T x)
{
    return (x < 0) ? -x : x;
}

template<class T> T square(T x)
{
    return x * x;
}

template<class T> T cube(T x)
{
    return x * x * x;
}

template<class T> T clamp(T x)
{
    if (x < 0)
        return 0;
    else if (x > 1)
        return 1;
    else
        return x;
}

template<class T> T sign(T x)
{
    if (x < 0)
        return -1;
    else if (x > 0)
        return 1;
    else
        return 0;
}

// This function is like fmod except that it always returns
// a positive value in the range [ 0, y )
template<class T> T pfmod(T x, T y)
{
    T quotient = std::floor(std::abs(x / y));
    if (x < 0.0)
        return x + (quotient + 1) * y;
    else
        return x - quotient * y;
}

template<class T> T circleArea(T r)
{
    return (T) PI * r * r;
}

template<class T> T sphereArea(T r)
{
    return 4 * (T) PI * r * r;
}

template<class T> void Math<T>::sincos(T angle, T& s, T& c)
{
    s = (T) sin(angle);
    c = (T) cos(angle);
}


// return a random float in [0, 1]
template<class T> T Math<T>::frand()
{
    return (T) (rand() & 0x7fff) / (T) 32767;
}


// return a random float in [-1, 1]
template<class T> T Math<T>::sfrand()
{
    return (T) (rand() & 0x7fff) / (T) 32767 * 2 - 1;
}


template<class T> T Math<T>::lerp(T t, T a, T b)
{
    return a + t * (b - a);
}


// return t clamped to [0, 1]
template<class T> T Math<T>::clamp(T t)
{
    if (t < 0)
        return 0;
    else if (t > 1)
        return 1;
    else
        return t;
}

template<class T> void Math<T>::constrain(T& ioVal, T inMin, T inMax)
{
	if (ioVal < inMin)
		ioVal = inMin;
	if (ioVal > inMax)
		ioVal = inMax;
}

template<class T> T Math<T>::lerp2(T inX1, T inX2, T inY1, T inY2, T inX)
{
	if (fabs((double)inX2 - (double)inX1) < 1e-6)
		return inY1;

	T result = (T)(((double)(inY2 - inY1) / (double)(inX2 - inX1))*(double)inX + ((double)(inX2*inY1 - inX1*inY2) / (double)(inX2 - inX1)));

	// Constrain result between inY1 and inY2
	T minVal = fmin((double)inY1, (double)inY2);
	T maxVal = fmax((double)inY1, (double)inY2);
	if (result < minVal)
		result = minVal;
	if (result > maxVal)
		result = maxVal;

	return result;
}

template<class T> T Math<T>::lerp2NoConstrain(T inX1, T inX2, T inY1, T inY2, T inX)
{
	if (fabs((double)inX2 - (double)inX1) < 1e-6)
		return inY1;

	return (T)(((double)(inY2 - inY1) / (double)(inX2 - inX1))*(double)inX + ((double)(inX2*inY1 - inX1*inY2) / (double)(inX2 - inX1)));
}

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

#include "constants.h"

template<class T> class Math
{
public:
	static inline T sin(T);
	static inline T cos(T);
	static inline T tan(T);
	static inline void sincos(T, T&, T&);
	static inline T asin(T);
	static inline T acos(T);
	static inline T atan2(T, T);
	static inline T sqrt(T);
	static inline T exp(T);
	static inline T log(T);
	static inline T fabs(T);

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
typedef Math<float_t> Mathf;
typedef Math<double_t> Mathd;

// There are many templated methods that need to make heavy math calls. What I don't
// want, for example, is to have a Mat3f instance calling the double_t version
// of sin() or cos() when the faster sinf() and cosf() methods should be called.
// Therefore, I'm providing overloaded methods for each math function that the 
// corresponding templated methods will call.
inline double_t sinT(double_t angle)
{
	return sin(angle);
}
inline float_t sinT(float_t angle)
{
	return sinf(angle);
}
inline double_t cosT(double_t angle)
{
	return cos(angle);
}
inline float_t cosT(float_t angle)
{
	return cosf(angle);
}
inline double_t tanT(double_t angle)
{
	return tan(angle);
}
inline float_t tanT(float_t angle)
{
	return tanf(angle);
}
inline void sincosT(double_t angle, double_t& s, double_t& c)
{
	s = sin(angle);
	c = cos(angle);
}
inline void sincosT(float_t angle, float_t& s, float_t& c)
{
	s = sinf(angle);
	c = cosf(angle);
}
inline double_t asinT(double_t x)
{
	return asin(x);
}
inline float_t asinT(float_t x)
{
	return asinf(x);
}
inline double_t acosT(double_t x)
{
	return acos(x);
}
inline float_t acosT(float_t x)
{
	return acosf(x);
}
inline double_t atanT(double_t x)
{
	return atan(x);
}
inline float_t atanT(float_t x)
{
	return atanf(x);
}
inline double_t atan2T(double_t x, double_t y)
{
	return atan2(x, y);
}
inline float_t atan2T(float_t x, float_t y)
{
	return atan2f(x, y);
}
inline double_t sqrtT(double_t x)
{
	return sqrt(x);
}
inline float_t sqrtT(float_t x)
{
	return sqrtf(x);
}
inline double_t expT(double_t x)
{
	return exp(x);
}
inline float_t expT(float_t x)
{
	return expf(x);
}
inline double_t logT(double_t x)
{
	return log(x);
}
inline float_t logT(float_t x)
{
	return logf(x);
}
inline double_t fabsT(double_t x)
{
	return fabs(x);
}
inline float_t fabsT(float_t x)
{
	return fabsf(x);
}


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

template<class T> T smoothStep(T e0, T e1, T x)
{
	T t = clamp((x - e0) / (e1 - e0));
	return t * t * (3 - 2 * t);
}

template<class T> T mix(T x, T y, T a)
{
	return x * (1 - a) + y * a;
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

template<class T> bool isPowerOf2(T inValue)
{
	return ((inValue & (inValue - 1)) == 0);
}

template<class T> T Math<T>::sin(T angle)
{
	return sinT(angle);
}

template<class T> T Math<T>::cos(T angle)
{
	return cosT(angle);
}

template<class T> T Math<T>::tan(T angle)
{
	return tanT(angle);
}

template<class T> void Math<T>::sincos(T angle, T& s, T& c)
{
	sincosT(angle, s, c);
}

template<class T> T Math<T>::acos(T x)
{
	return acosT(x);
}

template<class T> T Math<T>::sqrt(T x)
{
	return sqrtT(x);
}

template<class T> T Math<T>::exp(T x)
{
	return expT(x);
}

template<class T> T Math<T>::log(T x)
{
	return logT(x);
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
	if (fabs((double_t)inX2 - (double_t)inX1) < 1e-6)
		return inY1;

	T result = ((inY2 - inY1) / (inX2 - inX1)) * inX + ((inX2*inY1 - inX1*inY2) / (inX2 - inX1));

	// Constrain result between inY1 and inY2
	T minVal = fmin((double_t)inY1, (double_t)inY2);
	T maxVal = fmax((double_t)inY1, (double_t)inY2);
	if (result < minVal)
		result = minVal;
	if (result > maxVal)
		result = maxVal;

	return result;
}

template<class T> T Math<T>::lerp2NoConstrain(T inX1, T inX2, T inY1, T inY2, T inX)
{
	if (fabs((double_t)inX2 - (double_t)inX1) < 1e-6)
		return inY1;

	return ((inY2 - inY1) / (inX2 - inX1)) * inX + ((inX2*inY1 - inX1*inY2) / (inX2 - inX1));
}


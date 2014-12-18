// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// vecmath.h
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

#include "ttmath/ttmath.h"

template<class T> class Point2
{
public:
	inline Point2();
	inline Point2(T, T);

	T x, y;
};

template<class T> class Vector3;

template<class T> class Point3
{
public:
	inline Point3();
	inline Point3(const Point3&);
	inline Point3(T, T, T);
	inline Point3(T*);

	inline T& operator[](int) const;
	inline Point3& operator+=(const Vector3<T>&);
	inline Point3& operator-=(const Vector3<T>&);
	inline Point3& operator*=(T);

	inline T distanceTo(const Point3&) const;
	inline T distanceToSquared(const Point3&) const;
	inline T distanceFromOrigin() const;
	inline T distanceFromOriginSquared() const;

	T x, y, z;
};

template<class T> class Vector2
{
public:
	inline Vector2();
	inline Vector2(const Vector2<T>&);
	inline Vector2(T, T);

	inline Vector2& operator+=(const Vector2<T>&);
	inline Vector2& operator-=(const Vector2<T>&);
	inline Vector2& operator*=(T);
	inline Vector2& operator/=(T);
	inline Vector2 operator-() const;
	inline Vector2 operator+() const;

	inline void normalize();
	inline T length() const;
	inline T lengthSquared() const;

	// Use union to alias the members
	union
	{
		T data[2];
		struct
		{
			T x;
			T y;
		};
		struct
		{
			T s;
			T t;
		};
	};
};

template<class T> class Polar3;

template<class T> class Vector3
{
public:
	inline Vector3();
	inline Vector3(const Vector3<T>&);
	inline Vector3(const T, const T, const T);
	inline Vector3(const Polar3<T>&);
	inline Vector3(const string&);
	inline Vector3(T* v);

	static inline T dot(const Vector3<T>&, const Vector3<T>&);

	inline Vector3& operator+=(const Vector3<T>&);
	inline Vector3& operator-=(const Vector3<T>&);
	inline Vector3& operator*=(T);
	inline Vector3& operator/=(T);
	inline Vector3 operator-() const;
	inline Vector3 operator+() const;

	inline void normalize();
	inline T length() const;
	inline T lengthSquared() const;

	// Use union to alias the members
	union
	{
		T data[3];
		struct
		{
			T x;
			T y;
			T z;
		};
		struct
		{
			T r;
			T g;
			T b;
		};
	};
};

// I'm basically redefining a stripped down version of Vector3 here which will be used with ttmath 128-bit integers.
template<class T> class Vector3_ttmath
{
public:
	inline Vector3_ttmath();
	inline Vector3_ttmath(const Vector3_ttmath<T>&);
	inline Vector3_ttmath(const T, const T, const T);
	inline Vector3_ttmath(T* v);

	inline Vector3_ttmath& operator+=(const Vector3_ttmath<T>&);
	inline Vector3_ttmath& operator-=(const Vector3_ttmath<T>&);
	inline Vector3_ttmath& operator*=(T);
	inline Vector3_ttmath& operator/=(T);
	inline Vector3_ttmath operator-() const;
	inline Vector3_ttmath operator+() const;

	inline T length() const;
	inline T lengthSquared() const;

	T x, y, z;
};

template<class T> class Vector4
{
public:
	inline Vector4();
	inline Vector4(T, T, T, T);

	inline T& operator[](int) const;
	inline Vector4& operator+=(const Vector4&);
	inline Vector4& operator-=(const Vector4&);
	inline Vector4& operator*=(T);
	inline Vector4& operator/=(T);

	inline Vector4 operator-() const;
	inline Vector4 operator+() const;

	// Use union to alias the members
	union
	{
		T data[4];
		struct
		{
			T x;
			T y;
			T z;
			T w;
		};
		struct
		{
			T r;
			T g;
			T b;
			T a;
		};
		struct
		{
			T s;
			T t;
			T p;
			T q;
		};
	};
};

template<class T> class Polar3
{
public:
	inline Polar3();
	inline Polar3(const Polar3<T>&);
	inline Polar3(T, T, T);
	inline Polar3(const Vector3<T>&);

	T	fLongitude;		// aka azimuth
	T	fLatitude;		// aka altitude
	T	fRadius;
};

template<class T> class Matrix3
{
public:
	Matrix3();
	Matrix3(T*);
	Matrix3(const Matrix3<T>&);
	Matrix3(const Vector3<T>&, const Vector3<T>&, const Vector3<T>&);

	static Matrix3<T> identity();
	static Matrix3<T> xrotation(T);
	static Matrix3<T> yrotation(T);
	static Matrix3<T> zrotation(T);
	static Matrix3<T> scaling(const Vector3<T>&);
	static Matrix3<T> scaling(T);

	inline const Vector3<T>& operator[](int) const;
	inline Vector3<T> row(int) const;
	inline Vector3<T> column(int) const;

	inline Matrix3& operator*=(T);

	Matrix3<T> transpose() const;
	Matrix3<T> inverse() const;
	T determinant() const;

	union
	{
		T data[9];
		struct
		{
			T m00, m01, m02;	// Column 0
			T m10, m11, m12;	// Column 1
			T m20, m21, m22;	// Column 2
		};
	};

//	Vector3<T> r[3];
};

template<class T> class Matrix4
{
 public:
	Matrix4();
	Matrix4(T*);
	Matrix4(const Matrix4<T>& m);
	Matrix4(const Vector4<T>&, const Vector4<T>&, const Vector4<T>&, const Vector4<T>&);

	inline const Vector4<T>& operator[](int) const;
	inline Vector4<T> row(int) const;
	inline Vector4<T> column(int) const;

	static Matrix4<T> identity();
	static Matrix4<T> translation(const Point3<T>&);
	static Matrix4<T> translation(const Vector3<T>&);
	static Matrix4<T> rotation(const Vector3<T>&, T);
	static Matrix4<T> xrotation(T);
	static Matrix4<T> yrotation(T);
	static Matrix4<T> zrotation(T);
	static Matrix4<T> scaling(const Vector3<T>&);
	static Matrix4<T> scaling(T);
	static Matrix4<T> orthographic(	const T l, const T r,
									const T b, const T t,
									const T n, const T f);

	void translate(const Point3<T>&);

	Matrix4<T> transpose() const;
//	Matrix4<T> inverse() const;

	union
	{
		T data[16];
		struct
		{
			T m00, m01, m02, m03;	// Column 0
			T m10, m11, m12, m13;	// Column 1
			T m20, m21, m22, m23;	// Column 2
			T m30, m31, m32, m33;	// Column 3
		};
	};

    //Vector4<T> r[4];
};

typedef Vector3<float_t>	Vec3f;
typedef Vector3<double_t>	Vec3d;
typedef Point3<float_t>		Point3f;
typedef Point3<double_t>	Point3d;
typedef Vector2<int>		Vec2i;
typedef Vector2<float_t>	Vec2f;
typedef Point2<float_t>		Point2f;
typedef Vector4<float_t>	Vec4f;
typedef Vector4<double_t>	Vec4d;
typedef Matrix4<float_t>	Mat4f;
typedef Matrix4<double_t>	Mat4d;
typedef Matrix3<float_t>	Mat3f;
typedef Matrix3<double_t>	Mat3d;

typedef ttmath::Int<2>	Int128;					// 128-bit signed integer. // On x64 we need 2 values to represent 128 bits.
typedef Vector3_ttmath<Int128> Vec3i128;		// Vector using 128-bit signed integers components. See comments in vecmath.cpp.
void Vec3i128toVec3d(Vec3i128& in, Vec3d& out);	// Function converts a 128-bit integer vec to floating point equivalent.

///////////////////////////////////////////////////////////////////////

template<class T> Point2<T>::Point2() : x(0), y(0)
{
}

template<class T> Point2<T>::Point2(T _x, T _y) : x(_x), y(_y)
{
}

template<class T> bool operator==(const Point2<T>& a, const Point2<T>& b)
{
	return a.x == b.x && a.y == b.y;
}

template<class T> bool operator!=(const Point2<T>& a, const Point2<T>& b)
{
	return a.x != b.x || a.y != b.y;
}

///////////////////////////////////////////////////////////////////////

template<class T> Point3<T>::Point3() : x(0), y(0), z(0)
{
}

template<class T> Point3<T>::Point3(const Point3<T>& p) :
x(p.x), y(p.y), z(p.z)
{
}

template<class T> Point3<T>::Point3(T _x, T _y, T _z) : x(_x), y(_y), z(_z)
{
}

template<class T> Point3<T>::Point3(T* p) : x(p[0]), y(p[1]), z(p[2])
{
}

template<class T> T& Point3<T>::operator[](int n) const
{
	// Not portable--I'll write a new version when I try to compile on a
	// platform where it bombs.
	return ((T*) this)[n];
}

template<class T> Point3<T>& Point3<T>::operator+=(const Vector3<T>& v)
{
	x += v.x; y += v.y; z += v.z;
	return *this;
}

template<class T> Point3<T>& Point3<T>::operator-=(const Vector3<T>& v)
{
	x -= v.x; y -= v.y; z -= v.z;
	return *this;
}

template<class T> Point3<T>& Point3<T>::operator*=(T s)
{
	x *= s; y *= s; z *= s;
	return *this;
}

template<class T> bool operator==(const Point3<T>& a, const Point3<T>& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

template<class T> bool operator!=(const Point3<T>& a, const Point3<T>& b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

template<class T> Point3<T> operator+(const Point3<T>& p, const Vector3<T>& v)
{
	return Point3<T>(p.x + v.x, p.y + v.y, p.z + v.z);
}

template<class T> Point3<T> operator-(const Point3<T>& p, const Vector3<T>& v)
{
	return Point3<T>(p.x - v.x, p.y - v.y, p.z - v.z);
}

// Naughty naughty . . .  remove these since they aren't proper
// point methods--changing the implicit homogenous coordinate isn't
// allowed.
template<class T> Point3<T> operator*(const Point3<T>& p, T s)
{
	return Point3<T>(p.x * s, p.y * s, p.z * s);
}

template<class T> Point3<T> operator*(T s, const Point3<T>& p)
{
	return Point3<T>(p.x * s, p.y * s, p.z * s);
}

template<class T> T Point3<T>::distanceTo(const Point3& p) const
{
	return (T)sqrt((p.x - x) * (p.x - x) +
		(p.y - y) * (p.y - y) +
		(p.z - z) * (p.z - z));
}

template<class T> T Point3<T>::distanceToSquared(const Point3& p) const
{
	return ((p.x - x) * (p.x - x) +
		(p.y - y) * (p.y - y) +
		(p.z - z) * (p.z - z));
}

template<class T> T Point3<T>::distanceFromOrigin() const
{
	return (T)sqrt(x * x + y * y + z * z);
}

template<class T> T Point3<T>::distanceFromOriginSquared() const
{
	return x * x + y * y + z * z;
}

///////////////////////////////////////////////////////////////////////

template<class T> Vector2<T>::Vector2() : x(0), y(0)
{
}

template<class T> Vector2<T>::Vector2(const Vector2<T>& v) : x(v.x), y(v.y)
{
}

template<class T> Vector2<T>::Vector2(T _x, T _y) : x(_x), y(_y)
{
}

template<class T> Vector2<T>& Vector2<T>::operator+=(const Vector2<T>& a)
{
	x += a.x; y += a.y;
	return *this;
}

template<class T> Vector2<T>& Vector2<T>::operator-=(const Vector2<T>& a)
{
	x -= a.x; y -= a.y;
	return *this;
}

template<class T> Vector2<T>& Vector2<T>::operator*=(T s)
{
	x *= s; y *= s;
	return *this;
}

template<class T> Vector2<T>& Vector2<T>::operator/=(T s)
{
	x /= s; y /= s;
	return *this;
}

template<class T> Vector2<T> Vector2<T>::operator-() const
{
	return Vector2<T>(-x, -y);
}

template<class T> Vector2<T> Vector2<T>::operator+() const
{
	return *this;
}

template<class T> Vector2<T> operator+(const Vector2<T>& a, const Vector2<T>& b)
{
	return Vector2<T>(a.x + b.x, a.y + b.y);
}

template<class T> Vector2<T> operator-(const Vector2<T>& a, const Vector2<T>& b)
{
	return Vector2<T>(a.x - b.x, a.y - b.y);
}

template<class T> Vector2<T> operator*(T s, const Vector2<T>& v)
{
	return Vector2<T>(s * v.x, s * v.y);
}

template<class T> Vector2<T> operator*(const Vector2<T>& v, T s)
{
	return Vector2<T>(s * v.x, s * v.y);
}

template<class T> bool operator==(const Vector2<T>& a, const Vector2<T>& b)
{
	return a.x == b.x && a.y == b.y;
}

template<class T> bool operator!=(const Vector2<T>& a, const Vector2<T>& b)
{
	return a.x != b.x || a.y != b.y;
}

template<class T> Vector2<T> operator/(const Vector2<T>& v, T s)
{
	return Vector2<T>(v.x / s, v.y / s);
}

template<class T> void Vector2<T>::normalize()
{
	T s = 1 / (T)sqrt(x * x + y * y);
	x *= s;
	y *= s;
}

///////////////////////////////////////////////////////////////////////

template<class T> Vector3<T>::Vector3() : x(0), y(0), z(0)
{
}

template<class T> Vector3<T>::Vector3(const Vector3<T>& v) :
    x(v.x), y(v.y), z(v.z)
{
}

template<class T> Vector3<T>::Vector3(T _x, T _y, T _z) : x(_x), y(_y), z(_z)
{
}

template<class T> Vector3<T>::Vector3(T* v) : x(v[0]), y(v[1]), z(v[2])
{
}

template<class T> Vector3<T>::Vector3(const std::string& a)
{
	x = 0;
	y = 0;
	z = 0;

	if (a.empty())
		return;

	// Types could be a problem here
	char* dataStr = new char[a.length() + 1];
	if (dataStr)
	{
		strcpy_s(dataStr, a.length() + 1, a.c_str());

		const char* delims = "{}[](), \t";
		char* context = NULL;
		char* value = NULL;
		value = strtok_s(dataStr, delims, &context);
		if (value)
			x = (T)atof(value);
		value = strtok_s(NULL, delims, &context);
		if (value)
			y = (T)atof(value);
		value = strtok_s(NULL, delims, &context);
		if (value)
			z = (T)atof(value);

		delete[] dataStr;
	}
}

template<class T> Vector3<T>& Vector3<T>::operator+=(const Vector3<T>& a)
{
    x += a.x; y += a.y; z += a.z;
    return *this;
}

template<class T> Vector3<T>& Vector3<T>::operator-=(const Vector3<T>& a)
{
    x -= a.x; y -= a.y; z -= a.z;
    return *this;
}

template<class T> Vector3<T>& Vector3<T>::operator*=(T s)
{
    x *= s; y *= s; z *= s;
    return *this;
}

template<class T> Vector3<T>& Vector3<T>::operator/=(T s)
{
	x /= s; y /= s; z /= s;
	return *this;
}

template<class T> Vector3<T> Vector3<T>::operator-() const
{
    return Vector3<T>(-x, -y, -z);
}

template<class T> Vector3<T> Vector3<T>::operator+() const
{
    return *this;
}

template<class T> Vector3<T> operator+(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template<class T> Vector3<T> operator-(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template<class T> Vector3<T> operator*(T s, const Vector3<T>& v)
{
    return Vector3<T>(s * v.x, s * v.y, s * v.z);
}

template<class T> Vector3<T> operator*(const Vector3<T>& v, T s)
{
    return Vector3<T>(s * v.x, s * v.y, s * v.z);
}

// dot product
template<class T> T operator*(const Vector3<T>& a, const Vector3<T>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// cross product
template<class T> Vector3<T> operator^(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.y * b.z - a.z * b.y,
                      a.z * b.x - a.x * b.z,
                      a.x * b.y - a.y * b.x);
}

template<class T> bool operator==(const Vector3<T>& a, const Vector3<T>& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

template<class T> bool operator!=(const Vector3<T>& a, const Vector3<T>& b)
{
    return a.x != b.x || a.y != b.y || a.z != b.z;
}

template<class T> Vector3<T> operator/(const Vector3<T>& v, T s)
{
    return Vector3<T>(v.x / s, v.y / s, v.z / s);
}

template<class T> T dot(const Vector3<T>& a, const Vector3<T>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<class T> Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b)
{
    return Vector3<T>(a.y * b.z - a.z * b.y,
                      a.z * b.x - a.x * b.z,
                      a.x * b.y - a.y * b.x);
}

template<class T> T Vector3<T>::length() const
{
    return (T) sqrt(x * x + y * y + z * z);
}

template<class T> T Vector3<T>::lengthSquared() const
{
    return x * x + y * y + z * z;
}

template<class T> void Vector3<T>::normalize()
{
    T s = 1 / (T) sqrt(x * x + y * y + z * z);
    x *= s;
    y *= s;
    z *= s;
}

template<class T> Vector3<T> operator-(const Point3<T>& a, const Point3<T>& b)
{
    return Vector3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

///////////////////////////////////////////////////////////////////////
// Vector3_ttmath
///////////////////////////////////////////////////////////////////////

template<class T> Vector3_ttmath<T>::Vector3_ttmath() : x(0), y(0), z(0)
{
}

template<class T> Vector3_ttmath<T>::Vector3_ttmath(const Vector3_ttmath<T>& v) : x(v.x), y(v.y), z(v.z)
{
}

template<class T> Vector3_ttmath<T>::Vector3_ttmath(T _x, T _y, T _z) : x(_x), y(_y), z(_z)
{
}

template<class T> Vector3_ttmath<T>::Vector3_ttmath(T* v) : x(v[0]), y(v[1]), z(v[2])
{
}

template<class T> Vector3_ttmath<T>& Vector3_ttmath<T>::operator+=(const Vector3_ttmath<T>& a)
{
	x += a.x; y += a.y; z += a.z;
	return *this;
}

template<class T> Vector3_ttmath<T>& Vector3_ttmath<T>::operator-=(const Vector3_ttmath<T>& a)
{
	x -= a.x; y -= a.y; z -= a.z;
	return *this;
}

template<class T> Vector3_ttmath<T>& Vector3_ttmath<T>::operator*=(T s)
{
	x *= s; y *= s; z *= s;
	return *this;
}

template<class T> Vector3_ttmath<T>& Vector3_ttmath<T>::operator/=(T s)
{
	x /= s; y /= s; z /= s;
	return *this;
}

template<class T> Vector3_ttmath<T> Vector3_ttmath<T>::operator-() const
{
	return Vector3_ttmath<T>(-x, -y, -z);
}

template<class T> Vector3_ttmath<T> Vector3_ttmath<T>::operator+() const
{
	return *this;
}

template<class T> Vector3_ttmath<T> operator+(const Vector3_ttmath<T>& a, const Vector3_ttmath<T>& b)
{
	return Vector3_ttmath<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template<class T> Vector3_ttmath<T> operator-(const Vector3_ttmath<T>& a, const Vector3_ttmath<T>& b)
{
	return Vector3_ttmath<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template<class T> Vector3_ttmath<T> operator*(T s, const Vector3_ttmath<T>& v)
{
	return Vector3_ttmath<T>(s * v.x, s * v.y, s * v.z);
}

template<class T> Vector3_ttmath<T> operator*(const Vector3_ttmath<T>& v, T s)
{
	return Vector3_ttmath<T>(s * v.x, s * v.y, s * v.z);
}

// dot product
template<class T> T operator*(const Vector3_ttmath<T>& a, const Vector3_ttmath<T>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// cross product
template<class T> Vector3_ttmath<T> operator^(const Vector3_ttmath<T>& a, const Vector3_ttmath<T>& b)
{
	return Vector3_ttmath<T>(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

template<class T> bool operator==(const Vector3_ttmath<T>& a, const Vector3_ttmath<T>& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

template<class T> bool operator!=(const Vector3_ttmath<T>& a, const Vector3_ttmath<T>& b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

template<class T> Vector3_ttmath<T> operator/(const Vector3_ttmath<T>& v, T s)
{
	return Vector3<T>(v.x / s, v.y / s, v.z / s);
}

template<class T> T dot(const Vector3_ttmath<T>& a, const Vector3_ttmath<T>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<class T> Vector3_ttmath<T> cross(const Vector3_ttmath<T>& a, const Vector3_ttmath<T>& b)
{
	return Vector3<T>(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

template<class T> T Vector3_ttmath<T>::length() const
{
	return (T)sqrt(x * x + y * y + z * z);
}

template<class T> T Vector3_ttmath<T>::lengthSquared() const
{
	return x * x + y * y + z * z;
}

template<class T> Vector3_ttmath<T> operator-(const Point3<T>& a, const Point3<T>& b)
{
	return Vector3_ttmath<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

///////////////////////////////////////////////////////////////////////

template<class T> Vector4<T>::Vector4() : x(0), y(0), z(0), w(0)
{
}

template<class T> Vector4<T>::Vector4(T _x, T _y, T _z, T _w) :
    x(_x), y(_y), z(_z), w(_w)
{
}

template<class T> T& Vector4<T>::operator[](int n) const
{
    // Not portable--I'll write a new version when I try to compile on a
    // platform where it bombs.
    return ((T*) this)[n];
}

template<class T> bool operator==(const Vector4<T>& a, const Vector4<T>& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

template<class T> bool operator!=(const Vector4<T>& a, const Vector4<T>& b)
{
    return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

template<class T> Vector4<T>& Vector4<T>::operator+=(const Vector4<T>& a)
{
    x += a.x; y += a.y; z += a.z; w += a.w;
    return *this;
}

template<class T> Vector4<T>& Vector4<T>::operator-=(const Vector4<T>& a)
{
    x -= a.x; y -= a.y; z -= a.z; w -= a.w;
    return *this;
}

template<class T> Vector4<T>& Vector4<T>::operator*=(T s)
{
    x *= s; y *= s; z *= s; w *= s;
    return *this;
}

template<class T> Vector4<T>& Vector4<T>::operator/=(T s)
{
	x /= s; y /= s; z /= s; w /= s;
	return *this;
}

template<class T> Vector4<T> Vector4<T>::operator-() const
{
    return Vector4<T>(-x, -y, -z, -w);
}

template<class T> Vector4<T> Vector4<T>::operator+() const
{
    return *this;
}

template<class T> Vector4<T> operator+(const Vector4<T>& a, const Vector4<T>& b)
{
    return Vector4<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template<class T> Vector4<T> operator-(const Vector4<T>& a, const Vector4<T>& b)
{
    return Vector4<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

template<class T> Vector4<T> operator*(T s, const Vector4<T>& v)
{
    return Vector4<T>(s * v.x, s * v.y, s * v.z, s * v.w);
}

template<class T> Vector4<T> operator*(const Vector4<T>& v, T s)
{
    return Vector4<T>(s * v.x, s * v.y, s * v.z, s * v.w);
}

// dot product
template<class T> T operator*(const Vector4<T>& a, const Vector4<T>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template<class T> T dot(const Vector4<T>& a, const Vector4<T>& b)
{
    return a * b;
}

///////////////////////////////////////////////////////////////////////

template<class T> Polar3<T>::Polar3() : fRadius(1), fLongitude(0), fLatitude(0)
{
}

template<class T> Polar3<T>::Polar3(const Polar3<T>& a)
{
	fRadius = a.fRadius; fLongitude = a.fLongitude; fLatitude = a.fLatitude;
}

template<class T> Polar3<T>::Polar3(T inRadius, T inLongitude, T inLatitude)
{
	fRadius = inRadius; fLongitude = inLongitude; fLatitude = inLatitude;
}

template<class T> Polar3<T>::Polar3(const Vector3<T>& a)
{
	register T	theX, theY, theZ;

	theX = a.x;
	theY = a.y;
	theZ = a.z;
	fRadius = sqrt(theX*theX + theY*theY + theZ*theZ);
	if (theZ == 0.0 || fRadius == 0.0)
		fLatitude = 0.0;
	else
		fLatitude = asin(theZ / fRadius);
	fLongitude = atan2(theY, theX);
}

///////////////////////////////////////////////////////////////////////

template<class T> Matrix3<T>::Matrix3()
{
	m00 = m10 = m20 = 0;
	m01 = m11 = m21 = 0;
	m02 = m12 = m22 = 0;
}

template<class T> Matrix3<T>::Matrix3(T* inData)
{
	memcpy_s(data, sizeof(T)* 9, inData, sizeof(T)* 9);
}

template<class T> Matrix3<T>::Matrix3(const Matrix3<T>& m)
{
	m00 = m.m00; m10 = m.m10; m20 = m.m20;
	m01 = m.m01; m11 = m.m11; m21 = m.m21;
	m02 = m.m02; m12 = m.m12; m22 = m.m22;
}

template<class T> Matrix3<T>::Matrix3(const Vector3<T>& c0,
                                      const Vector3<T>& c1,
                                      const Vector3<T>& c2)
{
	m00 = c0.x; m10 = c1.x; m20 = c2.x;
	m01 = c0.y; m11 = c1.y; m21 = c2.y;
	m02 = c0.z; m12 = c1.z; m22 = c2.z;
}

template<class T> const Vector3<T>& Matrix3<T>::operator[](int n) const	// n refers to column
{
	return column(n);
}

template<class T> Vector3<T> Matrix3<T>::row(int n) const
{
	return Vector3<T>(data[n], data[n + 3], data[n + 6]);
}

template<class T> Vector3<T> Matrix3<T>::column(int n) const
{
	int b = 3 * n;
	return Vector3<T>(data[b], data[b + 1], data[b + 2]);
}

template<class T> Matrix3<T>& Matrix3<T>::operator*=(T s)
{
	m00 *= s; m10 *= s; m20 *= s;
	m01 *= s; m11 *= s; m21 *= s;
	m02 *= s; m12 *= s; m22 *= s;
    return *this;
}

// pre-multiply column vector by a 3x3 matrix
template<class T> Vector3<T> operator*(const Matrix3<T>& m, const Vector3<T>& v)
{
    return Vector3<T>(	m.m00 * v.x + m.m10 * v.y + m.m20 * v.z,
						m.m01 * v.x + m.m11 * v.y + m.m21 * v.z,
						m.m02 * v.x + m.m12 * v.y + m.m22 * v.z);
}

// post-multiply row vector by a 3x3 matrix
template<class T> Vector3<T> operator*(const Vector3<T>& v, const Matrix3<T>& m)
{
    return Vector3<T>(m.m00 * v.x + m.m01 * v.y + m.m02 * v.z,
                      m.m10 * v.x + m.m11 * v.y + m.m12 * v.z,
                      m.m20 * v.x + m.m21 * v.y + m.m22 * v.z);
}

// pre-multiply column point by a 3x3 matrix
template<class T> Point3<T> operator*(const Matrix3<T>& m, const Point3<T>& p)
{
	return Vector3<T>(	m.m00 * p.x + m.m10 * p.y + m.m20 * p.z,
						m.m01 * p.x + m.m11 * p.y + m.m21 * p.z,
						m.m02 * p.x + m.m12 * p.y + m.m22 * p.z);
}

// post-multiply row point by a 3x3 matrix
template<class T> Point3<T> operator*(const Point3<T>& p, const Matrix3<T>& m)
{
	return Vector3<T>(	m.m00 * p.x + m.m01 * p.y + m.m02 * p.z,
						m.m10 * p.x + m.m11 * p.y + m.m12 * p.z,
						m.m20 * p.x + m.m21 * p.y + m.m22 * p.z);
}

template<class T> Matrix3<T> operator*(const Matrix3<T>& a,
                                       const Matrix3<T>& b)
{
	return Matrix3<T>(	Vector3<T>(	a.m00 * b.m00 + a.m10 * b.m01 + a.m20 * b.m02,
									a.m01 * b.m00 + a.m11 * b.m01 + a.m21 * b.m02,
									a.m02 * b.m00 + a.m12 * b.m01 + a.m22 * b.m02),
						Vector3<T>(	a.m00 * b.m10 + a.m10 * b.m11 + a.m20 * b.m12,
									a.m01 * b.m10 + a.m11 * b.m11 + a.m21 * b.m12,
									a.m02 * b.m10 + a.m12 * b.m11 + a.m22 * b.m12),
						Vector3<T>(	a.m00 * b.m20 + a.m10 * b.m21 + a.m20 * b.m22,
									a.m01 * b.m20 + a.m11 * b.m21 + a.m21 * b.m22,
									a.m02 * b.m20 + a.m12 * b.m21 + a.m22 * b.m22));
}

template<class T> Matrix3<T> operator+(const Matrix3<T>& a,
                                       const Matrix3<T>& b)
{
	T sum[9];
	for (int i = 0; i < 9; i++)
		sum[i] = a.data[i] + b.data[i];

	return Matrix3<T>(sum);
}

template<class T> Matrix3<T> Matrix3<T>::identity()
{
	T d[] = { 1, 0, 0,
			  0, 1, 0,
			  0, 0, 1 };

	return Matrix3<T>(d);
}

template<class T> Matrix3<T> Matrix3<T>::transpose() const
{
	T d[] = { m00, m10, m20,
			  m01, m11, m21,
			  m02, m12, m22 };

	return Matrix3<T>(d);
}

template<class T> T det2x2(T a, T b, T c, T d)
{
    return a * d - b * c;
}

template<class T> T Matrix3<T>::determinant() const
{
	return (m00 * m11 * m22 + m10 * m21 * m02 + m20 * m01 * m12 - (m20 * m11 * m02 + m10 * m01 * m22 + m00 * m21 * m12));
}

template<class T> Matrix3<T> Matrix3<T>::inverse() const
{
	T d[9];

	// Just use Cramer's rule for now . . .
	d[0] = det2x2(m11, m21, m12, m22);
	d[3] = -det2x2(m01, m21, m02, m22);
	d[6] = det2x2(m01, m11, m02, m12);
	d[1] = -det2x2(m10, m20, m12, m22);
	d[4] = det2x2(m00, m20, m02, m22);
	d[7] = -det2x2(m00, m10, m02, m12);
	d[2] = det2x2(m10, m20, m11, m21);
	d[5] = -det2x2(m00, m20, m01, m21);
	d[8] = det2x2(m00, m10, m01, m11);

	Matrix3<T> result(d);
	result *= 1 / determinant();

	return result;
}

template<class T> Matrix3<T> Matrix3<T>::xrotation(T angle)
{
    T c = (T) cos(angle);
    T s = (T) sin(angle);

	T d[] = { 1, 0, 0,
			  0, c, -s,
			  0, s, c };

	Matrix3<T> result(d);
	return result.transpose();
}

template<class T> Matrix3<T> Matrix3<T>::yrotation(T angle)
{
    T c = (T) cos(angle);
    T s = (T) sin(angle);

	T d[] = { c, 0, s,
			  0, 1, 0,
			 -s, 0, c };

	Matrix3<T> result(d);
	return result.transpose();
}

template<class T> Matrix3<T> Matrix3<T>::zrotation(T angle)
{
    T c = (T) cos(angle);
    T s = (T) sin(angle);

	T d[] = { c, -s, 0,
			  s, c, 0,
			  0, 0, 1 };

	Matrix3<T> result(d);
	return result.transpose();
}

template<class T> Matrix3<T> Matrix3<T>::scaling(const Vector3<T>& scale)
{
	T d[] = { scale.x, 0, 0,
			  0, scale.y, 0,
			  0, 0, scale.z };

	return Matrix3<T>(d);
}

template<class T> Matrix3<T> Matrix3<T>::scaling(T scale)
{
    return scaling(Vector3<T>(scale, scale, scale));
}

///////////////////////////////////////////////////////////////////////

template<class T> Matrix4<T>::Matrix4()
{
	m00 = m10 = m20 = m30 = 0;
	m01 = m11 = m21 = m31 = 0;
	m02 = m12 = m22 = m32 = 0;
	m03 = m13 = m23 = m33 = 0;
}

template<class T> Matrix4<T>::Matrix4(T* inData)
{
	memcpy_s(data, sizeof(T)* 16, inData, sizeof(T)* 16);
}

template<class T> Matrix4<T>::Matrix4(const Matrix4<T>& m)
{
	m00 = m.m00; m10 = m.m10; m20 = m.m20; m30 = m.m30;
	m01 = m.m01; m11 = m.m11; m21 = m.m21; m31 = m.m31;
	m02 = m.m02; m12 = m.m12; m22 = m.m22; m32 = m.m32;
	m03 = m.m03; m13 = m.m13; m23 = m.m23; m33 = m.m33;
}

template<class T> Matrix4<T>::Matrix4(const Vector4<T>& c0, const Vector4<T>& c1, const Vector4<T>& c2, const Vector4<T>& c3)
{
	m00 = c0.x; m10 = c1.x; m20 = c2.x; m30 = c3.x;
	m01 = c0.y; m11 = c1.y; m21 = c2.y; m31 = c3.y;
	m02 = c0.z; m12 = c1.z; m22 = c2.z; m32 = c3.z;
	m03 = c0.w; m13 = c1.w; m23 = c2.w; m33 = c3.w;
}

template<class T> const Vector4<T>& Matrix4<T>::operator[](int n) const	// n refers to column
{
	return column(n);
}

template<class T> Vector4<T> Matrix4<T>::row(int n) const
{
	return Vector4<T>(data[n], data[n + 4], data[n + 8], data[n + 12]);
}

template<class T> Vector4<T> Matrix4<T>::column(int n) const
{
	int b = 4 * n;
	return Vector4<T>(data[b], data[b + 1], data[b + 2], data[b + 3]);
}

template<class T> Matrix4<T> Matrix4<T>::identity()
{
	T d[] = { 1, 0, 0, 0,
			  0, 1, 0, 0,
			  0, 0, 1, 0,
			  0, 0, 0, 1 };

	return Matrix4<T>(d);
}

template<class T> Matrix4<T> Matrix4<T>::translation(const Point3<T>& p)
{
	T d[] = { 1, 0, 0, 0,
			  0, 1, 0, 0,
			  0, 0, 1, 0,
			  p.x, p.y, p.z, 1 };
	Matrix4<T> result(d);

	return result.transpose();
}

template<class T> Matrix4<T> Matrix4<T>::translation(const Vector3<T>& v)
{
	T d[] = { 1, 0, 0, v.x,
			  0, 1, 0, v.y,
			  0, 0, 1, v.z,
			  0, 0, 0, 1 };
	Matrix4<T> result(d);

	return result.transpose();
}

template<class T> void Matrix4<T>::translate(const Point3<T>& p)
{
	m03 += p.x;
	m13 += p.y;
	m23 += p.z;
}

template<class T> Matrix4<T> Matrix4<T>::rotation(const Vector3<T>& axis, T angle)
{
    T c = (T) cos(angle);
    T s = (T) sin(angle);
    T t = 1 - c;

	T d[] = { t * axis.x * axis.x + c, t * axis.x * axis.y - s * axis.z, t * axis.x * axis.z + s * axis.y, 0,
			  t * axis.x * axis.y + s * axis.z, t * axis.y * axis.y + c, t * axis.y * axis.z - s * axis.x, 0,
			  t * axis.x * axis.z - s * axis.y, t * axis.y * axis.z + s * axis.x, t * axis.z * axis.z + c, 0,
			  0, 0, 0, 1 };
	Matrix4<T> result(d);

	return result.transpose();
}

template<class T> Matrix4<T> Matrix4<T>::xrotation(T angle)
{
	T c = (T)cos(angle);
	T s = (T)sin(angle);

	T d[] = { 1, 0, 0, 0,
			  0, c, -s, 0,
			  0, s, c, 0,
			  0, 0, 0, 1};
	Matrix4<T> result(d);

	return result.transpose();
}

template<class T> Matrix4<T> Matrix4<T>::yrotation(T angle)
{
	T c = (T)cos(angle);
	T s = (T)sin(angle);

	T d[] = { c, 0, s, 0,
			  0, 1, 0, 0,
			  -s, 0, c, 0,
			  0, 0, 0, 1 };
	Matrix4<T> result(d);

	return result.transpose();
}

template<class T> Matrix4<T> Matrix4<T>::zrotation(T angle)
{
	T c = (T)cos(angle);
	T s = (T)sin(angle);

	T d[] = { c, -s, 0, 0,
			  s, c, 0, 0,
			  0, 0, 1, 0,
			  0, 0, 0, 1 };
	Matrix4<T> result(d);

	return result.transpose();
}

template<class T> Matrix4<T> Matrix4<T>::scaling(const Vector3<T>& scale)
{
	T d[] = { scale.x, 0, 0, 0,
			  0, scale.y, 0, 0,
			  0, 0, scale.z, 0,
			  0, 0, 0, 1};

	return Matrix4<T>(d);
}

template<class T> Matrix4<T> Matrix4<T>::scaling(T scale)
{
    return scaling(Vector3<T>(scale, scale, scale));
}

template<class T> Matrix4<T> Matrix4<T>::orthographic(const T l, const T r,
													  const T b, const T t,
													  const T n, const T f)
{
	T tx = -(r + l) / (r - l);
	T ty = -(t + b) / (t - b);
	T tz = -(f + n) / (f - n);

	T d[] = { 2 / (r - l), 0, 0, tx,
			  0, 2 / (t - b), 0, ty,
			  0, 0, -2 / (f - n), tz,
			  0, 0, 0, 1};

	return Matrix4<T>(d);
}

// multiply column vector by a 4x4 matrix
template<class T> Vector3<T> operator*(const Matrix4<T>& m, const Vector3<T>& v)
{
	return Vector3<T>(	m.m00 * v.x + m.m10 * v.y + m.m20 * v.z,
						m.m01 * v.x + m.m11 * v.y + m.m21 * v.z,
						m.m02 * v.x + m.m12 * v.y + m.m22 * v.z);
}

// multiply row vector by a 4x4 matrix
template<class T> Vector3<T> operator*(const Vector3<T>& v, const Matrix4<T>& m)
{
    return Vector3<T>(m.m00 * v.x + m.m01 * v.y + m.m02 * v.z,
                      m.m10 * v.x + m.m11 * v.y + m.m12 * v.z,
                      m.m20 * v.x + m.m21 * v.y + m.m22 * v.z);
}

// multiply column point by a 4x4 matrix; no projection is performed
template<class T> Point3<T> operator*(const Matrix4<T>& m, const Point3<T>& p)
{
	return Point3<T>(m.m00 * p.x + m.m10 * p.y + m.m20 * p.z + m.m30,
					 m.m01 * p.x + m.m11 * p.y + m.m21 * p.z + m.m31,
					 m.m02 * p.x + m.m12 * p.y + m.m22 * p.z + m.m32);
}

// multiply row point by a 4x4 matrix; no projection is performed
template<class T> Point3<T> operator*(const Point3<T>& p, const Matrix4<T>& m)
{
	return Point3<T>(m.m00 * p.x + m.m01 * p.y + m.m02 * p.z + m.m03,
					 m.m10 * p.x + m.m11 * p.y + m.m12 * p.z + m.m13,
					 m.m20 * p.x + m.m21 * p.y + m.m22 * p.z + m.m23);
}

// multiply column vector by a 4x4 matrix
template<class T> Vector4<T> operator*(const Matrix4<T>& m, const Vector4<T>& v)
{
	return Vector4<T>(m.m00 * v.x + m.m10 * v.y + m.m20 * v.z + m.m30 * v.w,
					  m.m01 * v.x + m.m11 * v.y + m.m21 * v.z + m.m31 * v.w,
					  m.m02 * v.x + m.m12 * v.y + m.m22 * v.z + m.m32 * v.w,
					  m.m03 * v.x + m.m13 * v.y + m.m23 * v.z + m.m33 * v.w);
}

// multiply row vector by a 4x4 matrix
template<class T> Vector4<T> operator*(const Vector4<T>& v, const Matrix4<T>& m)
{
	return Vector4<T>(m.m00 * v.x + m.m01 * v.y + m.m02 * v.z + m.m03 * v.w,
					  m.m10 * v.x + m.m11 * v.y + m.m12 * v.z + m.m13 * v.w,
					  m.m20 * v.x + m.m21 * v.y + m.m22 * v.z + m.m23 * v.w,
					  m.m20 * v.x + m.m21 * v.y + m.m22 * v.z + m.m33 * v.w);
}

template<class T> Matrix4<T> Matrix4<T>::transpose() const
{
	T d[] = { m00, m10, m20, m30,
			  m01, m11, m21, m31,
			  m02, m12, m22, m32,
			  m03, m13, m23, m33};

	return Matrix4<T>(d);
}

template<class T> Matrix4<T> operator*(const Matrix4<T>& a,
                                       const Matrix4<T>& b)
{
	return Matrix4<T>(	Vector4<T>(	a.m00 * b.m00 + a.m10 * b.m01 + a.m20 * b.m02 + a.m30 * b.m03,
									a.m01 * b.m00 + a.m11 * b.m01 + a.m21 * b.m02 + a.m31 * b.m03,
									a.m02 * b.m00 + a.m12 * b.m01 + a.m22 * b.m02 + a.m32 * b.m03,
									a.m03 * b.m00 + a.m13 * b.m01 + a.m23 * b.m02 + a.m33 * b.m03),
						Vector4<T>(	a.m00 * b.m10 + a.m10 * b.m11 + a.m20 * b.m12 + a.m30 * b.m13,
									a.m01 * b.m10 + a.m11 * b.m11 + a.m21 * b.m12 + a.m31 * b.m13,
									a.m02 * b.m10 + a.m12 * b.m11 + a.m22 * b.m12 + a.m32 * b.m13,
									a.m03 * b.m10 + a.m13 * b.m11 + a.m23 * b.m12 + a.m33 * b.m13),
						Vector4<T>(	a.m00 * b.m20 + a.m10 * b.m21 + a.m20 * b.m22 + a.m30 * b.m23,
									a.m01 * b.m20 + a.m11 * b.m21 + a.m21 * b.m22 + a.m31 * b.m23,
									a.m02 * b.m20 + a.m12 * b.m21 + a.m22 * b.m22 + a.m32 * b.m23,
									a.m03 * b.m20 + a.m13 * b.m21 + a.m23 * b.m22 + a.m33 * b.m23),
						Vector4<T>(	a.m00 * b.m30 + a.m10 * b.m31 + a.m20 * b.m32 + a.m30 * b.m33,
									a.m01 * b.m30 + a.m11 * b.m31 + a.m21 * b.m32 + a.m31 * b.m33,
									a.m02 * b.m30 + a.m12 * b.m31 + a.m22 * b.m32 + a.m32 * b.m33,
									a.m03 * b.m30 + a.m13 * b.m31 + a.m23 * b.m32 + a.m33 * b.m33));
}

template<class T> Matrix4<T> operator+(const Matrix4<T>& a, const Matrix4<T>& b)
{
	T sum[16];
	for (int i = 0; i < 16; i++)
		sum[i] = a.data[i] + b.data[i];

	return Matrix4<T>(sum);
}

/*
// Compute inverse using Gauss-Jordan elimination; caller is responsible
// for ensuring that the matrix isn't singular.
template<class T> Matrix4<T> Matrix4<T>::inverse() const
{
    Matrix4<T> a(*this);
    Matrix4<T> b(Matrix4<T>::identity());
    int i, j;
    int p;

    for (j = 0; j < 4; j++)
    {
        p = j;
        for (i = j + 1; i < 4; i++)
        {
            if (fabs(a.r[i][j]) > fabs(a.r[p][j]))
                p = i;
        }

        // Swap rows p and j
        Vector4<T> t = a.r[p];
        a.r[p] = a.r[j];
        a.r[j] = t;

        t = b.r[p];
        b.r[p] = b.r[j];
        b.r[j] = t;

        T s = a.r[j][j];  // if s == 0, the matrix is singular
        a.r[j] *= (1.0f / s);
        b.r[j] *= (1.0f / s);

        // Eliminate off-diagonal elements
        for (i = 0; i < 4; i++)
        {
            if (i != j)
            {
                b.r[i] -= a.r[i][j] * b.r[j];
                a.r[i] -= a.r[i][j] * a.r[j];
            }
        }
    }

    return b;
}
*/
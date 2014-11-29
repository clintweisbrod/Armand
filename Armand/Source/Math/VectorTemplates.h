//----------------------------------------------------------------------
//	Copyright © 1990-2005 by Simulation Curriculum Corp., All rights reserved.
//
//	File:		VectorTemplates.h
//
//	Contains:	Templated vector data types.
//					
//	Authors:	Clint Weisbrod
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2005/03/07	CLW			5.0.2		first release
//	2005/12/13	CLW			5.8.0		Added mathematical operations
//
//----------------------------------------------------------------------

#pragma once

#include <gl/GL.h>


//----------------------------------------------------------------------
//	Template:	TVector2Template
//
//	Purpose:	Templated 2-component vector class.
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2005/03/07	CLW			5.0.0		first release
//	2005/12/13	CLW			5.8.0		Added mathematical operations
//
//----------------------------------------------------------------------

template<class T>
class TVector2Template
{
	public:
		inline TVector2Template();
		inline TVector2Template(const TVector2Template<T>&);
		inline TVector2Template(T, T);
		
		inline TVector2Template& operator+=(const TVector2Template<T>&);
		inline TVector2Template& operator-=(const TVector2Template<T>&);
		inline TVector2Template& operator*=(T);
		inline TVector2Template& operator/=(T);
		inline TVector2Template operator-() const;
		inline TVector2Template operator+() const;
    
		inline void Normalize();
		inline T Length() const;
		inline T LengthSquared() const;

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

template<class T> TVector2Template<T>::TVector2Template() : x(0), y(0)
{
}

template<class T> TVector2Template<T>::TVector2Template(const TVector2Template<T>& a)
{
	x = a.x; y = a.y;
}

template<class T> TVector2Template<T>::TVector2Template(T inX, T inY) : x(inX), y(inY)
{
}

template<class T> TVector2Template<T>& TVector2Template<T>::operator+=(const TVector2Template<T>& a)
{
	x += a.x; y += a.y;
	return *this;
}

template<class T> TVector2Template<T>& TVector2Template<T>::operator-=(const TVector2Template<T>& a)
{
	x -= a.x; y -= a.y;
	return *this;
}

template<class T> TVector2Template<T>& TVector2Template<T>::operator*=(T s)
{
	x *= s; y *= s;
	return *this;
}

template<class T> TVector2Template<T>& TVector2Template<T>::operator/=(T s)
{
	x /= s; y /= s;
	return *this;
}

template<class T> TVector2Template<T> TVector2Template<T>::operator-() const
{
	return TVector2Template<T>(-x, -y);
}

template<class T> TVector2Template<T> TVector2Template<T>::operator+() const
{
	return *this;
}

template<class T> TVector2Template<T> operator+(const TVector2Template<T>& a, const TVector2Template<T>& b)
{
    return TVector2Template<T>(a.x + b.x, a.y + b.y);
}

template<class T> TVector2Template<T> operator-(const TVector2Template<T>& a, const TVector2Template<T>& b)
{
    return TVector2Template<T>(a.x - b.x, a.y - b.y);
}

template<class T> TVector2Template<T> operator*(T s, const TVector2Template<T>& v)
{
    return TVector2Template<T>(s * v.x, s * v.y);
}

template<class T> TVector2Template<T> operator*(const TVector2Template<T>& v, T s)
{
    return TVector2Template<T>(s * v.x, s * v.y);
}

// dot product
template<class T> T operator*(const TVector2Template<T>& a, const TVector2Template<T>& b)
{
    return a.x * b.x + a.y * b.y;
}

// scalar division operator
template<class T> TVector2Template<T> operator/(const TVector2Template<T>& v, T s)
{
    T is = 1 / s;
    return TVector2Template<T>(is * v.x, is * v.y);
}

template<class T> void TVector2Template<T>::Normalize()
{
	T denom = (T) sqrt(x * x + y * y);
	if (denom == 0.0)
	{
		x = 0;
		y = 0;
	}
	else
	{
		T s = 1 / denom;
		x *= s;
		y *= s;
	}
}

template<class T> T TVector2Template<T>::Length() const
{
	return (T) sqrt(x * x + y * y);
}

template<class T> T TVector2Template<T>::LengthSquared() const
{
	return x * x + y * y;
}

//----------------------------------------------------------------------
//	Template:	TVector3Template
//
//	Purpose:	Templated 3-component vector class.
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2005/03/07	CLW			5.0.0		first release
//	2005/12/13	CLW			5.8.0		Added mathematical operations
//
//----------------------------------------------------------------------
template<class T> class TPolar3Template;
template<class T>
class TVector3Template
{
	public:
		inline TVector3Template();
		inline TVector3Template(const TVector3Template<T>&);
		inline TVector3Template(const T, const T, const T);
		inline TVector3Template(const TPolar3Template<T>&);
		inline TVector3Template(const string&);
		
		inline TVector3Template& operator+=(const TVector3Template<T>&);
		inline TVector3Template& operator-=(const TVector3Template<T>&);
		inline TVector3Template& operator*=(T);
		inline TVector3Template& operator/=(T);
		inline TVector3Template operator-() const;
		inline TVector3Template operator+() const;
		
		inline void Normalize();
		inline T Length() const;
		inline T LengthSquared() const;

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

template<class T> TVector3Template<T>::TVector3Template() : x(0), y(0), z(0)
{
}

template<class T> TVector3Template<T>::TVector3Template(const TVector3Template<T>& a)
{
	x = a.x; y = a.y; z = a.z;
}

template<class T> TVector3Template<T>::TVector3Template(const T inX, const T inY, const T inZ)
{
	x = inX; y = inY; z = inZ;
}

template<class T> TVector3Template<T>::TVector3Template(const TPolar3Template<T>& a)
{
	T cosLatitude = cos(a.fLatitude);
	x = a.fRadius * cosLatitude * cos(a.fLongitude);
	y = a.fRadius * cosLatitude * sin(a.fLongitude);
	z = a.fRadius * sin(a.fLatitude);
}

template<class T> TVector3Template<T>::TVector3Template(const string& a)
{
	if (a.length() == 0)
		return;

	// Types could be a problem here
	char* dataStr = new char[a.length() + 1];
	if (dataStr)
	{
		strcpy(dataStr, a.c_str());
		char* val = strtok(dataStr, "{}[](), \t");
		if (val)
			x = atof(val);
		val = strtok(NULL, "{}[](), \t");
		if (val)
			y = atof(val);
		val = strtok(NULL, "{}[](), \t");
		if (val)
			z = atof(val);
		
		delete [] dataStr;
	}
}

template<class T> TVector3Template<T>& TVector3Template<T>::operator+=(const TVector3Template<T>& a)
{
	x += a.x; y += a.y; z += a.z;
	return *this;
}

template<class T> TVector3Template<T>& TVector3Template<T>::operator-=(const TVector3Template<T>& a)
{
	x -= a.x; y -= a.y; z -= a.z;
	return *this;
}

template<class T> TVector3Template<T>& TVector3Template<T>::operator*=(T s)
{
	x *= s; y *= s; z *= s;
	return *this;
}

template<class T> TVector3Template<T>& TVector3Template<T>::operator/=(T s)
{
	x /= s; y /= s; z /= s;
	return *this;
}

template<class T> TVector3Template<T> TVector3Template<T>::operator-() const
{
	return TVector3Template<T>(-x, -y, -z);
}

template<class T> TVector3Template<T> TVector3Template<T>::operator+() const
{
	return *this;
}

template<class T> TVector3Template<T> operator+(const TVector3Template<T>& a, const TVector3Template<T>& b)
{
    return TVector3Template<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template<class T> TVector3Template<T> operator-(const TVector3Template<T>& a, const TVector3Template<T>& b)
{
    return TVector3Template<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template<class T> TVector3Template<T> operator*(T s, const TVector3Template<T>& v)
{
    return TVector3Template<T>(s * v.x, s * v.y, s * v.z);
}

template<class T> TVector3Template<T> operator*(const TVector3Template<T>& v, T s)
{
    return TVector3Template<T>(s * v.x, s * v.y, s * v.z);
}

// dot product
template<class T> T operator*(const TVector3Template<T>& a, const TVector3Template<T>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// cross product operator
template<class T> TVector3Template<T> operator^(const TVector3Template<T>& a, const TVector3Template<T>& b)
{
	return TVector3Template<T>(	a.y * b.z - a.z * b.y,
							a.z * b.x - a.x * b.z,
							a.x * b.y - a.y * b.x);
}
// scalar division operator
template<class T> TVector3Template<T> operator/(const TVector3Template<T>& v, T s)
{
    T is = 1 / s;
    return TVector3Template<T>(is * v.x, is * v.y, is * v.z);
}

template<class T> void TVector3Template<T>::Normalize()
{
	// DW added all of this 2005/12/20
//	Assert_(!isnan(x));
//	Assert_(!isnan(y));
//	Assert_(!isnan(z));
	
	T denom = (T) sqrt(x * x + y * y + z * z);
	if (denom == 0.0)
	{
		x = 0;
		y = 0;
		z = 0;
	}
	else
	{
		T s = 1 / denom;
		x *= s;
		y *= s;
		z *= s;
	}
}

template<class T> T TVector3Template<T>::Length() const
{
	return (T) sqrt(x * x + y * y + z * z);
}

template<class T> T TVector3Template<T>::LengthSquared() const
{
	return x * x + y * y + z * z;
}

//----------------------------------------------------------------------
//	Template:	TVector4Template
//
//	Purpose:	Templated 4-component vector class.
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2005/03/07	CLW			5.0.0		first release
//	2005/12/13	CLW			5.8.0		Templated
//
//----------------------------------------------------------------------
template<class T>
class TVector4Template
{
	public:
		inline TVector4Template();
		inline TVector4Template(T, T, T, T);

		inline T& operator[](int) const;
		inline TVector4Template& operator+=(const TVector4Template&);
		inline TVector4Template& operator-=(const TVector4Template&);
		inline TVector4Template& operator*=(T);

		inline TVector4Template operator-() const;
		inline TVector4Template operator+() const;

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

template<class T> TVector4Template<T>::TVector4Template() : x(0), y(0), z(0), w(0)
{
}

template<class T> TVector4Template<T>::TVector4Template(T inX, T inY, T inZ, T inW)
{
	x = inX; y = inY; z = inZ; w = inW;
}

template<class T> T& TVector4Template<T>::operator[](int n) const
{
	return ((T*)this)[n];
}

template<class T> bool operator==(const TVector4Template<T>& a, const TVector4Template<T>& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

template<class T> bool operator!=(const TVector4Template<T>& a, const TVector4Template<T>& b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

template<class T> TVector4Template<T>& TVector4Template<T>::operator+=(const TVector4Template<T>& a)
{
	x += a.x; y += a.y; z += a.z; w += a.w;
	return *this;
}

template<class T> TVector4Template<T>& TVector4Template<T>::operator-=(const TVector4Template<T>& a)
{
	x -= a.x; y -= a.y; z -= a.z; w -= a.w;
	return *this;
}

template<class T> TVector4Template<T>& TVector4Template<T>::operator*=(T s)
{
	x *= s; y *= s; z *= s; w *= s;
	return *this;
}

template<class T> TVector4Template<T> TVector4Template<T>::operator-() const
{
	return TVector4Template<T>(-x, -y, -z, -w);
}

template<class T> TVector4Template<T> TVector4Template<T>::operator+() const
{
	return *this;
}

template<class T> TVector4Template<T> operator+(const TVector4Template<T>& a, const TVector4Template<T>& b)
{
	return TVector4Template<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template<class T> TVector4Template<T> operator-(const TVector4Template<T>& a, const TVector4Template<T>& b)
{
	return TVector4Template<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

template<class T> TVector4Template<T> operator*(T s, const TVector4Template<T>& v)
{
	return TVector4Template<T>(s * v.x, s * v.y, s * v.z, s * v.w);
}

template<class T> TVector4Template<T> operator*(const TVector4Template<T>& v, T s)
{
	return TVector4Template<T>(s * v.x, s * v.y, s * v.z, s * v.w);
}

// dot product
template<class T> T operator*(const TVector4Template<T>& a, const TVector4Template<T>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template<class T> T dot(const TVector4Template<T>& a, const TVector4Template<T>& b)
{
	return a * b;
}


//----------------------------------------------------------------------
//	Template:	TMatrix2Template
//
//	Purpose:	Templated 2x2 matrix class. Simply data storage right now for GLSL shader support.
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2005/03/07	CLW			5.0.0		first release
//	2005/12/13	CLW			5.8.0		Added mathematical operations
//
//----------------------------------------------------------------------
template<class T>
class TMatrix2Template
{
	public:
		inline const TVector2Template<T>& operator[](int) const;
		
		TVector2Template<T> r[2];
};

template<class T> const TVector2Template<T>& TMatrix2Template<T>::operator[](int n) const
{
    return r[n];
}

//----------------------------------------------------------------------
//	Template:	TMatrix3Template
//
//	Purpose:	Templated 3x3 matrix class. Simply data storage right now for GLSL shader support.
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2005/03/07	CLW			5.0.0		first release
//	2005/12/13	CLW			5.8.0		Added mathematical operations
//
//----------------------------------------------------------------------
template<class T>
class TMatrix3Template
{
	public:
		TMatrix3Template();
		TMatrix3Template(const TVector3Template<T>&, const TVector3Template<T>&, const TVector3Template<T>&);
		TMatrix3Template(const TMatrix3Template<T>& m);

		static TMatrix3Template<T> identity();
		static TMatrix3Template<T> xrotation(T);
		static TMatrix3Template<T> yrotation(T);
		static TMatrix3Template<T> zrotation(T);
		static TMatrix3Template<T> scaling(const TVector3Template<T>&);
		static TMatrix3Template<T> scaling(T);
		
		inline const TVector3Template<T>& operator[](int) const;
		inline TVector3Template<T> row(int) const;
		inline TVector3Template<T> column(int) const;
		inline TMatrix3Template& operator*=(T);
		
		TMatrix3Template<T> transpose() const;
		TMatrix3Template<T> inverse() const;
		T determinant() const;
	
		TVector3Template<T> r[3];
};

template<class T> TMatrix3Template<T>::TMatrix3Template()
{
	r[0] = TVector3Template<T>(0, 0, 0);
	r[1] = TVector3Template<T>(0, 0, 0);
	r[2] = TVector3Template<T>(0, 0, 0);
}

template<class T> TMatrix3Template<T>::TMatrix3Template(const TVector3Template<T>& r0,
														const TVector3Template<T>& r1,
														const TVector3Template<T>& r2)
{
	r[0] = r0;
	r[1] = r1;
	r[2] = r2;
}

template<class T> TMatrix3Template<T>::TMatrix3Template(const TMatrix3Template<T>& m)
{
	r[0] = m.r[0];
	r[1] = m.r[1];
	r[2] = m.r[2];
}

template<class T> const TVector3Template<T>& TMatrix3Template<T>::operator[](int n) const
{
	return r[n];
}

template<class T> TVector3Template<T> TMatrix3Template<T>::row(int n) const
{
	return r[n];
}

template<class T> TVector3Template<T> TMatrix3Template<T>::column(int n) const
{
	return TVector3Template<T>(r[0][n], r[1][n], r[2][n]);
}

template<class T> TMatrix3Template<T>& TMatrix3Template<T>::operator*=(T s)
{
	r[0] *= s;
	r[1] *= s;
	r[2] *= s;
	return *this;
}

// pre-multiply column vector by a 3x3 matrix
template<class T> TVector3Template<T> operator*(const TMatrix3Template<T>& m, const TVector3Template<T>& v)
{
	return TVector3Template<T>(	m.r[0].x * v.x + m.r[0].y * v.y + m.r[0].z * v.z,
								m.r[1].x * v.x + m.r[1].y * v.y + m.r[1].z * v.z,
								m.r[2].x * v.x + m.r[2].y * v.y + m.r[2].z * v.z);
}

// post-multiply row vector by a 3x3 matrix
template<class T> TVector3Template<T> operator*(const TVector3Template<T>& v, const TMatrix3Template<T>& m)
{
	return TVector3Template<T>(	m.r[0].x * v.x + m.r[1].x * v.y + m.r[2].x * v.z,
								m.r[0].y * v.x + m.r[1].y * v.y + m.r[2].y * v.z,
								m.r[0].z * v.x + m.r[1].z * v.y + m.r[2].z * v.z);
}

template<class T> TMatrix3Template<T> operator*(const TMatrix3Template<T>& a,
                                       			const TMatrix3Template<T>& b)
{
#define MATMUL(R, C) (a[R].x * b[0].C + a[R].y * b[1].C + a[R].z * b[2].C)
	return TMatrix3Template<T>(	TVector3Template<T>(MATMUL(0, x), MATMUL(0, y), MATMUL(0, z)),
								TVector3Template<T>(MATMUL(1, x), MATMUL(1, y), MATMUL(1, z)),
								TVector3Template<T>(MATMUL(2, x), MATMUL(2, y), MATMUL(2, z)));
#undef MATMUL
}

template<class T> TMatrix3Template<T> operator+(const TMatrix3Template<T>& a,
												const TMatrix3Template<T>& b)
{
	return TMatrix3Template<T>(	a.r[0] + b.r[0],
								a.r[1] + b.r[1],
								a.r[2] + b.r[2]);
}

template<class T> TMatrix3Template<T> TMatrix3Template<T>::identity()
{
	return TMatrix3Template<T>(	TVector3Template<T>(1, 0, 0),
								TVector3Template<T>(0, 1, 0),
								TVector3Template<T>(0, 0, 1));
}

template<class T> TMatrix3Template<T> TMatrix3Template<T>::transpose() const
{
	return TMatrix3Template<T>(	TVector3Template<T>(r[0].x, r[1].x, r[2].x),
								TVector3Template<T>(r[0].y, r[1].y, r[2].y),
								TVector3Template<T>(r[0].z, r[1].z, r[2].z));
}

template<class T> T det2x2(T a, T b, T c, T d)
{
	return a * d - b * c;
}

template<class T> T TMatrix3Template<T>::determinant() const
{
	return (r[0].x * r[1].y * r[2].z +
			r[0].y * r[1].z * r[2].x +
			r[0].z * r[1].x * r[2].y -
			r[0].z * r[1].y * r[2].x -
			r[0].x * r[1].z * r[2].y -
			r[0].y * r[1].x * r[2].z);
}

template<class T> TMatrix3Template<T> TMatrix3Template<T>::inverse() const
{
	TMatrix3Template<T> adjoint;

	// Just use Cramer's rule for now . . .
	adjoint.r[0].x =  det2x2(r[1].y, r[1].z, r[2].y, r[2].z);
	adjoint.r[0].y = -det2x2(r[1].x, r[1].z, r[2].x, r[2].z);
	adjoint.r[0].z =  det2x2(r[1].x, r[1].y, r[2].x, r[2].y);
	adjoint.r[1].x = -det2x2(r[0].y, r[0].z, r[2].y, r[2].z);
	adjoint.r[1].y =  det2x2(r[0].x, r[0].z, r[2].x, r[2].z);
	adjoint.r[1].z = -det2x2(r[0].x, r[0].y, r[2].x, r[2].y);
	adjoint.r[2].x =  det2x2(r[0].y, r[0].z, r[1].y, r[1].z);
	adjoint.r[2].y = -det2x2(r[0].x, r[0].z, r[1].x, r[1].z);
	adjoint.r[2].z =  det2x2(r[0].x, r[0].y, r[1].x, r[1].y);
	adjoint *= 1 / determinant();

	return adjoint;
}

template<class T> TMatrix3Template<T> TMatrix3Template<T>::xrotation(T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);

	return TMatrix3Template<T>(	TVector3Template<T>(1, 0, 0),
								TVector3Template<T>(0, c, -s),
								TVector3Template<T>(0, s, c));
}

template<class T> TMatrix3Template<T> TMatrix3Template<T>::yrotation(T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);

	return TMatrix3Template<T>(	TVector3Template<T>(c, 0, s),
								TVector3Template<T>(0, 1, 0),
								TVector3Template<T>(-s, 0, c));
}

template<class T> TMatrix3Template<T> TMatrix3Template<T>::zrotation(T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);

	return TMatrix3Template<T>(	TVector3Template<T>(c, -s, 0),
								TVector3Template<T>(s, c, 0),
								TVector3Template<T>(0, 0, 1));
}

template<class T> TMatrix3Template<T> TMatrix3Template<T>::scaling(const TVector3Template<T>& scale)
{
	return TMatrix3Template<T>(	TVector3Template<T>(scale.x, 0, 0),
								TVector3Template<T>(0, scale.y, 0),
								TVector3Template<T>(0, 0, scale.z));
}

template<class T> TMatrix3Template<T> TMatrix3Template<T>::scaling(T scale)
{
	return scaling(TVector3Template<T>(scale, scale, scale));
}


//----------------------------------------------------------------------
//	Template:	TMatrix4Template
//
//	Purpose:	Templated 4x4 matrix class. Simply data storage right now for GLSL shader support.
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2005/03/07	CLW			5.0.0		first release
//	2005/12/13	CLW			5.8.0		Added mathematical operations
//
//----------------------------------------------------------------------
template<class T>
class TMatrix4Template
{
	public:
		TMatrix4Template();
		TMatrix4Template(const TVector4Template<T>&, const TVector4Template<T>&, const TVector4Template<T>&, const TVector4Template<T>&);
		TMatrix4Template(const TMatrix4Template<T>& m);

		static TMatrix4Template<T> identity();
		static TMatrix4Template<T> translation(const TVector3Template<T>&);
		static TMatrix4Template<T> xrotation(T);
		static TMatrix4Template<T> yrotation(T);
		static TMatrix4Template<T> zrotation(T);
		static TMatrix4Template<T> rotation(const TVector3Template<T>&, T);
		static TMatrix4Template<T> scaling(const TVector3Template<T>&);
		static TMatrix4Template<T> scaling(T);
		
		inline const TVector4Template<T>& operator[](int) const;
		inline TVector4Template<T> row(int) const;
		inline TVector4Template<T> column(int) const;
		inline TMatrix4Template& operator*=(T);
		
		TMatrix4Template<T> transpose() const;
		TMatrix4Template<T> inverse() const;

		TVector4Template<T> r[4];
};

template<class T> TMatrix4Template<T>::TMatrix4Template()
{
	r[0] = TVector4Template<T>(0, 0, 0, 0);
	r[1] = TVector4Template<T>(0, 0, 0, 0);
	r[2] = TVector4Template<T>(0, 0, 0, 0);
	r[3] = TVector4Template<T>(0, 0, 0, 0);
}

template<class T> TMatrix4Template<T>::TMatrix4Template(const TVector4Template<T>& v0,
														const TVector4Template<T>& v1,
														const TVector4Template<T>& v2,
														const TVector4Template<T>& v3)
{
	r[0] = v0;
	r[1] = v1;
	r[2] = v2;
	r[3] = v3;
}

template<class T> TMatrix4Template<T>::TMatrix4Template(const TMatrix4Template<T>& m)
{
	r[0] = m.r[0];
	r[1] = m.r[1];
	r[2] = m.r[2];
	r[3] = m.r[3];
}

template<class T> const TVector4Template<T>& TMatrix4Template<T>::operator[](int n) const
{
	return r[n];
}

template<class T> TVector4Template<T> TMatrix4Template<T>::row(int n) const
{
	return r[n];
}

template<class T> TVector4Template<T> TMatrix4Template<T>::column(int n) const
{
	return TVector4Template<T>(r[0][n], r[1][n], r[2][n], r[3][n]);
}

template<class T> TMatrix4Template<T> TMatrix4Template<T>::identity()
{
	return TMatrix4Template<T>(	TVector4Template<T>(1, 0, 0, 0),
								TVector4Template<T>(0, 1, 0, 0),
								TVector4Template<T>(0, 0, 1, 0),
								TVector4Template<T>(0, 0, 0, 1));
}

template<class T> TMatrix4Template<T> TMatrix4Template<T>::translation(const TVector3Template<T>& v)
{
	return TMatrix4Template<T>(	TVector4Template<T>(1, 0, 0, 0),
								TVector4Template<T>(0, 1, 0, 0),
								TVector4Template<T>(0, 0, 1, 0),
								TVector4Template<T>(v.x, v.y, v.z, 1));
}

template<class T> TMatrix4Template<T> TMatrix4Template<T>::rotation(const TVector3Template<T>& axis, T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);
	T t = 1 - c;

	return TMatrix4Template<T>(	TVector4Template<T>(	t * axis.x * axis.x + c,
														t * axis.x * axis.y - s * axis.z,
														t * axis.x * axis.z + s * axis.y,
														0),
								TVector4Template<T>(	t * axis.x * axis.y + s * axis.z,
														t * axis.y * axis.y + c,
														t * axis.y * axis.z - s * axis.x,
														0),
								TVector4Template<T>(	t * axis.x * axis.z - s * axis.y,
														t * axis.y * axis.z + s * axis.x,
														t * axis.z * axis.z + c,
														0),
								TVector4Template<T>(0, 0, 0, 1));
}

template<class T> TMatrix4Template<T> TMatrix4Template<T>::xrotation(T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);

	return TMatrix4Template<T>(	TVector4Template<T>(1, 0, 0, 0),
								TVector4Template<T>(0, c, -s, 0),
								TVector4Template<T>(0, s, c, 0),
								TVector4Template<T>(0, 0, 0, 1));
}

template<class T> TMatrix4Template<T> TMatrix4Template<T>::yrotation(T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);

	return TMatrix4Template<T>(	TVector4Template<T>(c, 0, s, 0),
								TVector4Template<T>(0, 1, 0, 0),
								TVector4Template<T>(-s, 0, c, 0),
								TVector4Template<T>(0, 0, 0, 1));
}

template<class T> TMatrix4Template<T> TMatrix4Template<T>::zrotation(T angle)
{
	T c = (T) cos(angle);
	T s = (T) sin(angle);

	return TMatrix4Template<T>(	TVector4Template<T>(c, -s, 0, 0),
								TVector4Template<T>(s, c, 0, 0),
								TVector4Template<T>(0, 0, 1, 0),
								TVector4Template<T>(0, 0, 0, 1));
}

template<class T> TMatrix4Template<T> TMatrix4Template<T>::scaling(const TVector3Template<T>& scale)
{
	return TMatrix4Template<T>(	TVector4Template<T>(scale.x, 0, 0, 0),
								TVector4Template<T>(0, scale.y, 0, 0),
								TVector4Template<T>(0, 0, scale.z, 0),
								TVector4Template<T>(0, 0, 0, 1));
}

template<class T> TMatrix4Template<T> TMatrix4Template<T>::scaling(T scale)
{
	return scaling(TVector3Template<T>(scale, scale, scale));
}

// multiply column vector by a 4x4 matrix
template<class T> TVector3Template<T> operator*(const TMatrix4Template<T>& m, const TVector3Template<T>& v)
{
	return TVector3Template<T>(	m.r[0].x * v.x + m.r[0].y * v.y + m.r[0].z * v.z,
								m.r[1].x * v.x + m.r[1].y * v.y + m.r[1].z * v.z,
								m.r[2].x * v.x + m.r[2].y * v.y + m.r[2].z * v.z);
}

// multiply row vector by a 4x4 matrix
template<class T> TVector3Template<T> operator*(const TVector3Template<T>& v, const TMatrix4Template<T>& m)
{
	return TVector3Template<T>(	m.r[0].x * v.x + m.r[1].x * v.y + m.r[2].x * v.z,
								m.r[0].y * v.x + m.r[1].y * v.y + m.r[2].y * v.z,
								m.r[0].z * v.x + m.r[1].z * v.y + m.r[2].z * v.z);
}

// multiply column vector by a 4x4 matrix
template<class T> TVector4Template<T> operator*(const TMatrix4Template<T>& m, const TVector4Template<T>& v)
{
	return TVector4Template<T>(	m.r[0].x * v.x + m.r[0].y * v.y + m.r[0].z * v.z + m.r[0].w * v.w,
								m.r[1].x * v.x + m.r[1].y * v.y + m.r[1].z * v.z + m.r[1].w * v.w,
								m.r[2].x * v.x + m.r[2].y * v.y + m.r[2].z * v.z + m.r[2].w * v.w,
								m.r[3].x * v.x + m.r[3].y * v.y + m.r[3].z * v.z + m.r[3].w * v.w);
}

// multiply row vector by a 4x4 matrix
template<class T> TVector4Template<T> operator*(const TVector4Template<T>& v, const TMatrix4Template<T>& m)
{
	return TVector4Template<T>(	m.r[0].x * v.x + m.r[1].x * v.y + m.r[2].x * v.z + m.r[3].x * v.w,
								m.r[0].y * v.x + m.r[1].y * v.y + m.r[2].y * v.z + m.r[3].y * v.w,
								m.r[0].z * v.x + m.r[1].z * v.y + m.r[2].z * v.z + m.r[3].z * v.w,
								m.r[0].w * v.x + m.r[1].w * v.y + m.r[2].w * v.z + m.r[3].w * v.w);
}

template<class T> TMatrix4Template<T> TMatrix4Template<T>::transpose() const
{
	return TMatrix4Template<T>(	TVector4Template<T>(r[0].x, r[1].x, r[2].x, r[3].x),
								TVector4Template<T>(r[0].y, r[1].y, r[2].y, r[3].y),
								TVector4Template<T>(r[0].z, r[1].z, r[2].z, r[3].z),
								TVector4Template<T>(r[0].w, r[1].w, r[2].w, r[3].w));
}

template<class T> TMatrix4Template<T> operator*(const TMatrix4Template<T>& a,
												const TMatrix4Template<T>& b)
{
#define MATMUL(R, C) (a[R].x * b[0].C + a[R].y * b[1].C + a[R].z * b[2].C + a[R].w * b[3].C)
	return TMatrix4Template<T>(	TVector4Template<T>(MATMUL(0, x), MATMUL(0, y), MATMUL(0, z), MATMUL(0, w)),
								TVector4Template<T>(MATMUL(1, x), MATMUL(1, y), MATMUL(1, z), MATMUL(1, w)),
								TVector4Template<T>(MATMUL(2, x), MATMUL(2, y), MATMUL(2, z), MATMUL(2, w)),
								TVector4Template<T>(MATMUL(3, x), MATMUL(3, y), MATMUL(3, z), MATMUL(3, w)));
#undef MATMUL
}

template<class T> TMatrix4Template<T> operator+(const TMatrix4Template<T>& a, const TMatrix4Template<T>& b)
{
	return TMatrix4Template<T>(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

// Compute inverse using Gauss-Jordan elimination; caller is responsible
// for ensuring that the matrix isn't singular.
template<class T> TMatrix4Template<T> TMatrix4Template<T>::inverse() const
{
	TMatrix4Template<T> a(*this);
	TMatrix4Template<T> b(TMatrix4Template<T>::identity());
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
		TVector4Template<T> t = a.r[p];
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

template<class T>
class TPolar3Template
{
	public:
		inline TPolar3Template();
		inline TPolar3Template(const TPolar3Template<T>&);
		inline TPolar3Template(T, T, T);
		inline TPolar3Template(const TVector3Template<T>&);
		
		T	fLongitude;		// aka azimuth
		T	fLatitude;		// aka altitude
		T	fRadius;
};

template<class T> TPolar3Template<T>::TPolar3Template() : fRadius(1), fLongitude(0), fLatitude(0)
{
}

template<class T> TPolar3Template<T>::TPolar3Template(const TPolar3Template<T>& a)
{
	fRadius = a.fRadius; fLongitude = a.fLongitude; fLatitude = a.fLatitude;
}

template<class T> TPolar3Template<T>::TPolar3Template(T inRadius, T inLongitude, T inLatitude)
{
	fRadius = inRadius; fLongitude = inLongitude; fLatitude = inLatitude;
}

template<class T> TPolar3Template<T>::TPolar3Template(const TVector3Template<T>& a)
{
	register T	theX, theY, theZ;
	
	theX = a.x;
	theY = a.y;
	theZ = a.z;
	fRadius = sqrt(theX*theX + theY*theY + theZ*theZ);
	if (theZ == 0.0 || fRadius == 0.0)
		fLatitude = 0.0;
	else
		fLatitude = asin(theZ/fRadius);
	fLongitude = atan2(theY, theX);	// TPolar not handle based so we don't care if FullArcTan moves mem
}

// Typedefs we need so far
typedef TVector2Template<GLfloat> TVector2f;
typedef TVector3Template<GLfloat> TVector3f;
typedef TVector4Template<GLfloat> TVector4f;
typedef TVector2Template<GLint> TVector2i;
typedef TVector3Template<GLint> TVector3i;
typedef TVector4Template<GLint> TVector4i;
typedef TVector2Template<GLdouble> TVector2d;
typedef TVector3Template<GLdouble> TVector3d;

typedef TMatrix2Template<GLfloat> TMatrix2f;
typedef TMatrix3Template<GLfloat> TMatrix3f;
typedef TMatrix4Template<GLfloat> TMatrix4f;

typedef TPolar3Template<GLfloat> TPolar3f;
typedef TPolar3Template<GLdouble> TPolar3d;

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// plane.h
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

#include "mathlib.h"
#include "vecmath.h"

template<class T> class Plane
{
 public:
    inline Plane();
    inline Plane(const Plane<T>&);
    inline Plane(const Vector3<T>&, T);
    inline Plane(const Vector3<T>&, const Point3<T>&);

    T distanceTo(const Point3<T>&) const;
    T distanceToSegment(const Point3<T>&, const Vector3<T>&) const;

    static Point3<T> intersection(const Plane<T>&,
                                  const Plane<T>&,
                                  const Plane<T>&);

 public:
    Vector3<T> normal;
    T d;
};


typedef Plane<float> Planef;
typedef Plane<double> Planed;


template<class T> Plane<T>::Plane() : normal(0, 0, 1), d(0)
{
}

template<class T> Plane<T>::Plane(const Plane<T>& p) :
    normal(p.normal), d(p.d)
{
}

template<class T> Plane<T>::Plane(const Vector3<T>& _normal, T _d) :
    normal(_normal), d(_d)
{
}

template<class T> Plane<T>::Plane(const Vector3<T>& _normal, const Point3<T>& _point) :
    normal(_normal)
{
    d = _normal.x * _point.x + _normal.y * _point.y + _normal.z * _point.z;
}

template<class T> T Plane<T>::distanceTo(const Point3<T>& p) const
{
    return normal.x * p.x + normal.y * p.y + normal.z * p.z + d;
}

// Distance between a plane and a segment defined by orig+dir*t, t <= 0 <= 1
template<class T> T Plane<T>::distanceToSegment(const Point3<T>& origin,
                                                const Vector3<T>& direction) const
{
    T u = (direction * normal);
    T dist;
    
    // Avoid divide by zero; near-zero values shouldn't cause problems
    if (u == 0)
    {
        // All points equidistant; we can just compute distance to origin
        dist = distanceTo(origin);
    }
    else
    {
        T t = -(d + Vector3<T>(origin.x, origin.y, origin.z) * normal) / u;
        if (t < 0)
        {
            dist = distanceTo(origin);
        }
        else if (t > 1)
        {
            dist = distanceTo(origin + direction);
        }
        else
        {
            // Segment intersects plane
            dist = 0.0;
        }
    }
    
    return dist;
}


template<class T> Plane<T> operator*(const Matrix3<T>& m, const Plane<T>& p)
{
    Vector3<T> v = m * p.normal;
    return Plane<T>(v, p.d);
}

template<class T> Plane<T> operator*(const Plane<T>& p, const Matrix3<T>& m)
{
    Vector3<T> v = p.normal * m;
    return Plane<T>(v, p.d);
}

template<class T> Plane<T> operator*(const Matrix4<T>& m, const Plane<T>& p)
{
    Vector4<T> v = m * Vector4<T>(p.normal.x, p.normal.y, p.normal.z, p.d);
    return Plane<T>(Vector3<T>(v.x, v.y, v.z), v.w);
}

template<class T> Plane<T> operator*(const Plane<T>& p, const Matrix4<T>& m)
{
    Vector4<T> v = Vector4<T>(p.normal.x, p.normal.y, p.normal.z, p.d) * m;
    return Plane<T>(Vector3<T>(v.x, v.y, v.z), v.w);
}

template<class T> Point3<T> Plane<T>::intersection(const Plane<T>& p0,
                                                   const Plane<T>& p1,
                                                   const Plane<T>& p2)
{
    T d = Matrix3<T>(p0.normal, p1.normal, p2.normal).determinant();

    Vector3<T> v = (p0.d * cross(p1.normal, p2.normal) +
                    p1.d * cross(p2.normal, p0.normal) +
                    p2.d * cross(p0.normal, p1.normal)) * (1.0f / d);
    return Point3<T>(v.x, v.y, v.z);
}

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// ray.h
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

#include "vecmath.h"

template<class T> class Ray3
{
 public:
    Ray3();
    Ray3(const Point3<T>&, const Vector3<T>&);
 
    Point3<T> point(T) const;
   
 public:
    Point3<T> origin;
    Vector3<T> direction;
};

typedef Ray3<float>   Ray3f;
typedef Ray3<double>  Ray3d;


template<class T> Ray3<T>::Ray3() :
    origin(0, 0, 0), direction(0, 0, -1)
{
}

template<class T> Ray3<T>::Ray3(const Point3<T>& _origin,
                                const Vector3<T>& _direction) :
    origin(_origin), direction(_direction)
{

}

template<class T> Point3<T> Ray3<T>::point(T t) const
{
    return origin + direction * t;
}

template<class T> Ray3<T> operator*(const Ray3<T>& r, const Matrix3<T>& m)
{
    return Ray3<T>(r.origin * m, r.direction * m);
}

template<class T> Ray3<T> operator*(const Ray3<T>& r, const Matrix4<T>& m)
{
    return Ray3<T>(r.origin * m, r.direction * m);
}

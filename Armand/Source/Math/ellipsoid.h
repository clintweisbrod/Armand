// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// ellipsoid.h
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

template<class T> class Ellipsoid
{
 public:
    Ellipsoid();
    Ellipsoid(const Vector3<T>&);
    Ellipsoid(const Point3<T>&, const Vector3<T>&);
    
    bool contains(const Point3<T>& p) const;
 
 public:
    Point3<T> center;
    Vector3<T> axes;
};

typedef Ellipsoid<float>   Ellipsoidf;
typedef Ellipsoid<double>  Ellipsoidd;


/*! Default Ellipsoid constructor. Create a unit sphere centered
 *  at the origin.
 */
template<class T> Ellipsoid<T>::Ellipsoid() :
    center(0, 0, 0), axes(1, 1, 1)
{
}

/*! Created an ellipsoid with the specified semiaxes, centered
 *  at the origin.
 */
template<class T> Ellipsoid<T>::Ellipsoid(const Vector3<T>& _axes) :
    center(0, 0, 0), axes(_axes)
{
}

/*! Create an ellipsoid with the specified center and semiaxes.
 */
template<class T> Ellipsoid<T>::Ellipsoid(const Point3<T>& _center,
                                          const Vector3<T>& _axes) :
    center(_center), axes(_axes)
{
}

/*! Test whether the point p lies inside the ellipsoid.
 */
template<class T> bool Ellipsoid<T>::contains(const Point3<T>& p) const
{
    Vector3<T> v = p - center;
    v = Vector3<T>(v.x / axes.x, v.y / axes.y, v.z / axes.z);
    return v * v <= (T) 1.0;
}

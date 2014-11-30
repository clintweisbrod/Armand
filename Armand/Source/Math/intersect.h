// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// intersect.h
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

#include "ray.h"
#include "sphere.h"
#include "ellipsoid.h"


template<class T> bool testIntersection(const Ray3<T>& ray,
                                        const Sphere<T>& sphere,
                                        T& distance)
{
    Vector3<T> diff = ray.origin - sphere.center;
    T s = (T) 1.0 / square(sphere.radius);
    T a = ray.direction * ray.direction * s;
    T b = ray.direction * diff * s;
    T c = diff * diff * s - (T) 1.0;
    T disc = b * b - a * c;
    if (disc < 0.0)
        return false;

    disc = (T) sqrt(disc);
    T sol0 = (-b + disc) / a;
    T sol1 = (-b - disc) / a;

    if (sol0 > 0)
    {
        if (sol0 < sol1 || sol1 < 0)
            distance = sol0;
        else
            distance = sol1;
        return true;
    }
    else if (sol1 > 0)
    {
        distance = sol1;
        return true;
    }
    else
    {
        return false;
    }
}


template<class T> bool testIntersection(const Ray3<T>& ray,
                                        const Sphere<T>& sphere,
                                        T& distanceToTester,
                                        T& cosAngleToCenter)
{
    if (testIntersection(ray, sphere, distanceToTester))
    {
        cosAngleToCenter    = (sphere.center - ray.origin)*ray.direction/(sphere.center - ray.origin).length();
        return true;
    }
    return false;
}


template<class T> bool testIntersection(const Ray3<T>& ray,
                                        const Ellipsoid<T>& e,
                                        T& distance)
{
    Vector3<T> diff = ray.origin - e.center;
    Vector3<T> s((T) 1.0 / square(e.axes.x),
                 (T) 1.0 / square(e.axes.y),
                 (T) 1.0 / square(e.axes.z));
    Vector3<T> sdir(ray.direction.x * s.x,
                    ray.direction.y * s.y,
                    ray.direction.z * s.z);
    Vector3<T> sdiff(diff.x * s.x, diff.y * s.y, diff.z * s.z);
    T a = ray.direction * sdir;
    T b = ray.direction * sdiff;
    T c = diff * sdiff - (T) 1.0;
    T disc = b * b - a * c;
    if (disc < 0.0)
        return false;

    disc = (T) sqrt(disc);
    T sol0 = (-b + disc) / a;
    T sol1 = (-b - disc) / a;

    if (sol0 > 0)
    {
        if (sol0 < sol1 || sol1 < 0)
            distance = sol0;
        else
            distance = sol1;
        return true;
    }
    else if (sol1 > 0)
    {
        distance = sol1;
        return true;
    }
    else
    {
        return false;
    }
}


template<class T> bool testIntersection(const Ray3<T>& ray,
                                        const Ellipsoid<T>& ellipsoid,
                                        T& distanceToTester,
                                        T& cosAngleToCenter)
{
    if (testIntersection(ray, ellipsoid, distanceToTester))
    {
        cosAngleToCenter  = (ellipsoid.center - ray.origin)*ray.direction/(ellipsoid.center - ray.origin).length();
        return true;
    }
    return false;
}

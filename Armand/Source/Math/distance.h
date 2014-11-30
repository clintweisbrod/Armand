// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// distance.h
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
#include "ray.h"
#include "sphere.h"
#include "ellipsoid.h"


template<class T> T distance(const Point3<T>& p, const Sphere<T>& s)
{
    return abs(s.center.distanceTo(p) - s.radius);
}

template<class T> T distance(const Point3<T>& p, const Ellipsoid<T>& e)
{
    return 0.0f;
}

template<class T> T distance(const Point3<T>& p, const Ray3<T>& r)
{
    T t = ((p - r.origin) * r.direction) / (r.direction * r.direction);
    if (t <= 0)
        return p.distanceTo(r.origin);
    else
        return p.distanceTo(r.point(t));
}

// Distance between a point and a segment defined by orig+dir*t, 0 <= t <= 1
template<class T> T distanceToSegment(const Point3<T>& p,
				      const Point3<T>& origin,
				      const Vector3<T>& direction)
{
    T t = ((p - origin) * direction) / (direction * direction);
    if (t <= 0)
        return p.distanceTo(origin);
    else if (t >= 1)
        return p.distanceTo(origin + direction);
    else
        return p.distanceTo(origin + direction * t);
}

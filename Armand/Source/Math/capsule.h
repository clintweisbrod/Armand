// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// capsule.h
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

template<class T> class Capsule
{
 public:
    Capsule();
    Capsule(const Vector3<T>& _axis, T _radius);
    Capsule(const Point3<T>& _origin, const Vector3<T>& _axis, T _radius);
 
 public:
    Point3<T> origin;
    Vector3<T> axis;
    T radius;
};

typedef Capsule<float>   Capsulef;
typedef Capsule<double>  Capsuled;


template<class T> Capsule<T>::Capsule() :
    origin(0, 0, 0), axis(0, 0, 0), radius(1)
{
}


template<class T> Capsule<T>::Capsule(const Vector3<T>& _axis,
				      T _radius) :
    origin(0, 0, 0), axis(_axis), radius(_radius)
{
}


template<class T> Capsule<T>::Capsule(const Point3<T>& _origin,
				      const Vector3<T>& _axis,
				      T _radius) :
    origin(_origin), axis(_axis), radius(_radius)
{
}

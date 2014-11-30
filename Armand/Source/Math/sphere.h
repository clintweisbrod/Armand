// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// sphere.h
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

template<class T> class Sphere
{
 public:
    Sphere();
    Sphere(T);
    Sphere(const Point3<T>&, T);
 
 public:
    Point3<T> center;
    T radius;
};

typedef Sphere<float>   Spheref;
typedef Sphere<double>  Sphered;


template<class T> Sphere<T>::Sphere() :
    center(0, 0, 0), radius(1)
{
}

template<class T> Sphere<T>::Sphere(T _radius) :
    center(0, 0, 0), radius(_radius)
{
}

template<class T> Sphere<T>::Sphere(const Point3<T>& _center, T _radius) :
    center(_center), radius(_radius)
{
}

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// Object.h
//
// Base class for all objects in Armand that have some position and orientation
// in universal coordinates.
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

#include "Math/quaternion.h"
#include "Math/vecmath.h"


class Object
{
public:
	Object();
	virtual ~Object();

	Vec3Big		getUniveralPositionAU() const { return mUniversalPositionAU; };
	void		setUniveralPositionAU(const Vec3Big& inPosition);

	void		setUniveralPositionLY(const Vec3d& inPosition);
	void		setUniveralPositionAU(const Vec3d& inPosition);
	void		setUniveralPositionKm(const Vec3d& inPosition);
	void		setUniveralPositionMetres(const Vec3d& inPosition);

	Quatf		getOrientation() { return mOrientation; };
	Mat4f		getOrientationMatrix() { return mOrientation.toMatrix4(); };

protected:
	Vec3Big		mUniversalPositionAU;
	Quatf		mOrientation;
};
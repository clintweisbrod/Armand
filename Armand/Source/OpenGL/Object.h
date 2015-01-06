#pragma once

#include "Math/quaternion.h"
#include "Math/vecmath.h"


class Object
{
public:
	Object();
	virtual ~Object();

	Vec3Big		getUniveralPosition() const { return mUniversalPosition; };
	void		setUniveralPosition(Vec3Big inPosition) { mUniversalPosition = inPosition; };

	Quatf		getOrientation() { return mOrientation; };
	Mat4f		getOrientationMatrix() { return mOrientation.toMatrix4(); };
protected:
	Vec3Big		mUniversalPosition;
	Quatf		mOrientation;
};
#pragma once

#include "Math/quaternion.h"
#include "Math/vecmath.h"


class Object
{
public:
	Object();
	virtual ~Object();

	Vec3Big		getUniveralPosition() const { return mUniversalPosition; };
	void		setUniveralPosition(const Vec3Big& inPosition);

	void		setUniveralPositionLY(const Vec3d& inPosition);
	void		setUniveralPositionAU(const Vec3d& inPosition);
	void		setUniveralPositionKm(const Vec3d& inPosition);
	void		setUniveralPositionMetres(const Vec3d& inPosition);

	Quatf		getOrientation() { return mOrientation; };
	Mat4f		getOrientationMatrix() { return mOrientation.toMatrix4(); };
protected:
	Vec3Big		mUniversalPosition;
	Quatf		mOrientation;
};
#include "stdafx.h"

#include "Camera.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::getViewerOrthoNormalBasis(Vec3f& ioViewDirection, Vec3f& ioUpDirection, Vec3f& ioLeftDirection) const
{
	const Vec3f kViewDirection(0, 0, 1);
	const Vec3f kUpDirection(0, 1, 0);
	const Vec3f kLeftDirection(1, 0, 0);

	Mat3f mat = mOrientation.toMatrix3();
	ioViewDirection = mat * kViewDirection;
	ioUpDirection = mat * kUpDirection;
	ioLeftDirection = mat * kLeftDirection;
}

Vec3f Camera::getCameraRelativePosition(Object& inObject) const
{
	return (Vec3f)(inObject.getUniveralPosition() - mUniversalPosition);

//	Vec3i128 vec128 = inObject.getUniveralPosition() - mUniversalPosition;
//	return Vec3i128toVec3f(vec128);
}

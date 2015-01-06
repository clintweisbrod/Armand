#include "stdafx.h"

#include "Camera.h"

// Establish correct orthonormal basis for camera view. We use the default right-handed OpenGL view with
// viewer looking down the -z axis and "up" in the +y direction.
Vec3f Camera::sDefaultViewDirection = Vec3f(0,0,-1);
Vec3f Camera::sDefaultUpDirection = Vec3f(0,1,0);
Vec3f Camera::sDefaultLeftDirection = sDefaultUpDirection ^ sDefaultViewDirection;

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::lookAt(Vec3f& inViewerDirection, Vec3f& inUpDirection)
{
	mOrientation = Quatf::lookAtFromOrgin(inViewerDirection, inUpDirection);
}

void Camera::getViewerOrthoNormalBasis(Vec3f& ioViewDirection, Vec3f& ioUpDirection, Vec3f& ioLeftDirection) const
{
	Mat3f mat = mOrientation.toMatrix3();
	ioViewDirection = mat * sDefaultViewDirection;
	ioUpDirection = mat * sDefaultUpDirection;
	ioLeftDirection = mat * sDefaultLeftDirection;
}

Vec3f Camera::getCameraRelativePosition(Object& inObject) const
{
	return (Vec3f)(inObject.getUniveralPosition() - mUniversalPosition);
}

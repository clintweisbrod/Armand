#include "stdafx.h"

#include "Objects/Camera.h"
#include "OpenGL/OpenGLWindow.h"

// Establish correct orthonormal basis for camera view. We use the default right-handed OpenGL view with
// viewer looking down the -z axis and "up" in the +y direction.
Vec3f Camera::sDefaultViewDirection = Vec3f(0,0,-1);
Vec3f Camera::sDefaultUpDirection = Vec3f(0,1,0);
Vec3f Camera::sDefaultLeftDirection = sDefaultUpDirection ^ sDefaultViewDirection;

Camera::Camera()
{
	mAperture = (float_t)kPi;
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

Vec3f Camera::getCameraRelativePosition(const Object* inObject) const
{
	return (Vec3f)(inObject->getUniveralPosition() - mUniversalPosition);
}

float_t Camera::getObjectPixelDiameter(float_t inObjectDistanceAU, float_t inObjectRadiusAU) const
{
	double_t angularDiameter = 2 * inObjectRadiusAU / inObjectDistanceAU;
	double_t fractionOfScene = angularDiameter / mAperture;
	int geometryRadius = gOpenGLWindow->getGeometryRadius();
	return (float_t)(fractionOfScene * geometryRadius);
}

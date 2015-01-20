// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// Camera.cpp
//
// Class defining location and orientation of viewer in Universal coordinates.
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

#include "stdafx.h"

#include "Objects/Camera.h"
#include "OpenGL/OpenGLWindow.h"

// Establish correct orthonormal basis for camera view. We use the default right-handed OpenGL view with
// viewer looking down the -z axis and "up" in the +y direction.
Vec3f Camera::sDefaultViewDirection = Vec3f(0,0,-1);
Vec3f Camera::sDefaultUpDirection = Vec3f(0,1,0);
Vec3f Camera::sDefaultLeftDirection = sDefaultUpDirection ^ sDefaultViewDirection;

Camera::Camera() : mSpeedAUPerSecond(0)
{
	setAperture((float_t)kPi);
}

Camera::~Camera()
{
}

void Camera::setAperture(float_t inAperture)
{
	mAperture = inAperture;

	// This is needed by Fisheye.glsl so best to cache it.
	mFisheyeClipPlaneDistance = -cosf(mAperture / 2);
}

void Camera::updateOrthoNormalBasis()
{
	// This normalize call is critical. Anytime we change mOrientation with a rotate() call,
	// mOrientation is multipled by another quaternion. Over time, mOrientation will gather
	// rounding errors and will produce non-normalized orthonormal vectors. This will lead 
	// to artifacts in the fisheye projection, specifically the center of the projection will
	// no longer contain geometry, it will always be black. Over time, this black central 
	// area will grow.
	mOrientation.normalize();

	Mat3f mat = mOrientation.toMatrix3();
	mLastViewDirection = mat * sDefaultViewDirection;
	mLastUpDirection = mat * sDefaultUpDirection;
	mLastLeftDirection = mat * sDefaultLeftDirection;
}

void Camera::lookAt(Vec3f& inViewerDirection, Vec3f& inUpDirection)
{
	mOrientation = Quatf::lookAt(inViewerDirection, inUpDirection);
	updateOrthoNormalBasis();
}

void Camera::changeSpeed(int inSense)
{
	const float_t kMinimumNonZeroSpeed = (float_t)kAuPerMetre * 0.01f;	// 1 cm/s :-)
	const float_t kAccelFactor = 0.05f;
//	float_t speedDelta = mSpeedAUPerSecond * kAccelFactor * inSense;
	float_t speedDelta = mSpeedAUPerSecond * kAccelFactor * inSense;
	if ((speedDelta == 0) && (inSense == 1))
		speedDelta = kMinimumNonZeroSpeed;
	mSpeedAUPerSecond += speedDelta;
}

void Camera::negateSpeed()
{
	mSpeedAUPerSecond *= -1;
}

void Camera::updatePosition()
{
	if (mSpeedAUPerSecond > 0)
	{
		double_t elapsedSeconds = mTimer.elapsedSeconds();
		float_t distanceTravelled = mSpeedAUPerSecond * (float_t)elapsedSeconds;
		Vec3f positionDelta = mLastViewDirection * distanceTravelled;
		mUniversalPositionAU += positionDelta;
	}
	mTimer.reset();
}

void Camera::rotateLeft()
{
	mOrientation.rotate(mLastUpDirection, degToRad(1.0f));
	updateOrthoNormalBasis();
}

void Camera::rotateRight()
{
	mOrientation.rotate(mLastUpDirection, degToRad(-1.0f));
	updateOrthoNormalBasis();
}

void Camera::rotateUp()
{
	mOrientation.rotate(mLastLeftDirection, degToRad(-1.0f));
	updateOrthoNormalBasis();
}

void Camera::rotateDown()
{
	mOrientation.rotate(mLastLeftDirection, degToRad(1.0f));
	updateOrthoNormalBasis();
}

void Camera::rollLeft()
{
	mOrientation.rotate(mLastViewDirection, degToRad(-1.0f));
	updateOrthoNormalBasis();
}

void Camera::rollRight()
{
	mOrientation.rotate(mLastViewDirection, degToRad(1.0f));
	updateOrthoNormalBasis();
}

void Camera::getViewerOrthoNormalBasis(Vec3f& ioViewDirection, Vec3f& ioUpDirection, Vec3f& ioLeftDirection) const
{
	ioViewDirection = mLastViewDirection;
	ioUpDirection = mLastUpDirection;
	ioLeftDirection = mLastLeftDirection;
}

Vec3f Camera::getCameraRelativePosition(const Object* inObject) const
{
	return (Vec3f)(inObject->getUniveralPositionAU() - mUniversalPositionAU);
}

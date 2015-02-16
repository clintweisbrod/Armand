// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// Camera.h
//
// Class defining location and orientation of viewer in universal coordinates.
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

#include "Object.h"
#include "Utilities/Timer.h"

class Camera : public Object
{
public:
	static Vec3f sDefaultViewDirection;
	static Vec3f sDefaultUpDirection;
	static Vec3f sDefaultLeftDirection;

	Camera();
	~Camera();

	void lookAt(Vec3f& inViewerDirection, Vec3f& inUpDirection);
	void update();

	Vec3f getCameraRelativePosition(const Object* inObject) const;
	void getViewerOrthoNormalBasis(Vec3f& ioViewDirection, Vec3f& ioUpDirection, Vec3f& ioLeftDirection) const;
//	float_t getAperture() const { return mAperture; };
	float_t getHalfAperture() const { return mHalfAperture; };
	float_t getFisheyeClipPlaneDistance() const { return mFisheyeClipPlaneDistance; };
	float_t getSpeed() const { return mSpeedAUPerSecond; };

	void setAperture(float_t inAperture);

	void changeSpeed(int inSense);
	void negateSpeed();

	void rotateLeftRight(float_t inAmount);
	void rotateUpDown(float_t inAmount);
	void rollLeftRight(float_t inAmount);

protected:
	void updateOrthoNormalBasis();

	float_t		mAperture;
	float_t		mHalfAperture;
	float_t		mSpeedAUPerSecond;
	float_t		mSpeedSign;	// +1 for forward, -1 for reverse
	Vec3f		mLastViewDirection;
	Vec3f		mLastUpDirection;
	Vec3f		mLastLeftDirection;
	Timer		mTimer;

	Vec3f		mRotationRate;

private:
	float_t		mFisheyeClipPlaneDistance;
};
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
	void updateOrthoNormalBasis();
	void updatePosition();

	Vec3f getCameraRelativePosition(const Object* inObject) const;
	void getViewerOrthoNormalBasis(Vec3f& ioViewDirection, Vec3f& ioUpDirection, Vec3f& ioLeftDirection) const;
	float_t getAperture() const { return mAperture; };
	float_t getFisheyeClipPlaneDistance() const { return mFisheyeClipPlaneDistance; };
	float_t getObjectPixelDiameter(float_t inObjectDistanceAU, float_t inObjectRadiusAU) const;
	float_t getSpeed() const { return mSpeedAUPerSecond; };

	void setAperture(float_t inAperture);

	void changeSpeed(int inSense);
	void negateSpeed();

	void rotateLeft();
	void rotateRight();
	void rotateUp();
	void rotateDown();
	void rollLeft();
	void rollRight();

protected:
	float_t		mAperture;
	float_t		mSpeedAUPerSecond;
	Vec3f		mLastViewDirection;
	Vec3f		mLastUpDirection;
	Vec3f		mLastLeftDirection;
	Timer		mTimer;

private:
	float_t		mFisheyeClipPlaneDistance;
};
#pragma once

#include "Object.h"

class Camera : public Object
{
public:
	static Vec3f sDefaultViewDirection;
	static Vec3f sDefaultUpDirection;
	static Vec3f sDefaultLeftDirection;

	Camera();
	~Camera();

	void lookAt(Vec3f& inViewerDirection, Vec3f& inUpDirection);
	Vec3f getCameraRelativePosition(Object& inObject) const;
	void getViewerOrthoNormalBasis(Vec3f& ioViewDirection, Vec3f& ioUpDirection, Vec3f& ioLeftDirection) const;
};
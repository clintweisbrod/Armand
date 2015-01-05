#pragma once

#include "Object.h"

class Camera : public Object
{
public:
	Camera();
	~Camera();

	void getViewerOrthoNormalBasis(Vec3f& ioViewDirection, Vec3f& ioUpDirection, Vec3f& ioLeftDirection) const;
	Vec3f getCameraRelativePosition(Object& inObject) const;
};
#pragma once

#include "Static3DPointSet.h"

class RandomPointsCube : public Static3DPointSet
{
public:
	RandomPointsCube();
	virtual ~RandomPointsCube();

	virtual bool render(Camera& inCamera, float inAlpha);
	virtual void renderAsPoint(Camera& inCamera, float inAlpha);
	virtual void renderFull(Camera& inCamera, float inAlpha);
	virtual void setGLStateForFullRender(float inAlpha) const;
	virtual void setGLStateForPoint(float inAlpha) const;

protected:
	virtual void loadData();
};
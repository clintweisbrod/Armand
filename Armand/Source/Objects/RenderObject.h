#include "Camera.h"

#pragma once

class RenderObject : public Object
{
public:
	RenderObject();
	virtual ~RenderObject();

	virtual bool isInView(Camera& inCamera) = 0;
	virtual bool shouldRenderAsPoint(Camera& inCamera) const = 0;
	virtual void render(Camera& inCamera, float inAlpha) = 0;
	virtual void renderAsPoint(Camera& inCamera, float inAlpha) = 0;
	virtual void renderFull(Camera& inCamera, float inAlpha) = 0;
	virtual void setGLStateForFullRender(float inAlpha) const;
	virtual void setGLStateForPoint(float inAlpha) const;

protected:
};
#include "Camera.h"

#pragma once

class RenderObject : public Object
{
public:
	RenderObject();
	virtual ~RenderObject();

	virtual bool isInView(Camera& inCamera) = 0;
	virtual bool shouldRenderAsPoint(Camera& inCamera) const = 0;
	virtual void setGLStateForFullRender() const = 0;
	virtual void render(Camera& inCamera) = 0;
	virtual void renderAsPoint(Camera& inCamera) = 0;
	virtual void renderFull(Camera& inCamera) = 0;
	virtual void setGLStateForPoint() const;

protected:
};
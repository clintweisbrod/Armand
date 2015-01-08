#include "RenderObject.h"
#include "Models/3DSModel.h"

#pragma once

class ModelObject : public RenderObject
{
public:
	ModelObject(const char* inModelFileName);
	virtual ~ModelObject();

	virtual bool isInView(Camera& inCamera);
	virtual	bool shouldRenderAsPoint(Camera& inCamera) const;
	virtual void setGLStateForFullRender() const;
	virtual void render(Camera& inCamera);
	virtual void renderAsPoint(Camera& inCamera);
	virtual void renderFull(Camera& inCamera);

protected:
	T3DSModel*	mModel;

	Vec3f		mLastViewerModelVector;
	Vec3f		mLastScaledViewerModelVector;
	float_t		mLastViewerDistance;
};
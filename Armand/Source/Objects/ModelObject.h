#include "RenderObject.h"
#include "Models/3DSModel.h"
#include "OpenGL/VertexBufferStructs.h"

#pragma once

class ModelObject : public RenderObject
{
public:
	ModelObject(const char* inModelFileName);
	virtual ~ModelObject();

	virtual bool isInView(Camera& inCamera);
	virtual	bool shouldRenderAsPoint(Camera& inCamera) const;
	virtual void render(Camera& inCamera, float inAlpha);
	virtual void renderAsPoint(Camera& inCamera, float inAlpha);
	virtual void renderFull(Camera& inCamera, float inAlpha);
	virtual void setGLStateForFullRender(float inAlpha) const;
	virtual void setGLStateForPoint(float inAlpha) const;

protected:
	T3DSModel*	mModel;

	GLuint		mPointVAO;
	GLuint		mPointVBO;
	GLuint		mPointShaderHandle;

	Vec3f		mLastViewerModelVector;
	Vec3f		mLastScaledViewerModelVector;
	float_t		mLastViewerDistance;

	PointStarVertex	mStarArray[1000000];
};
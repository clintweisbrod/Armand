#pragma once

#include <GL/glew.h>
#include "OpenGL/VertexBufferStructs.h"
#include "RenderObject.h"

class Static3DPointSet : public RenderObject
{
public:
	Static3DPointSet();
	virtual ~Static3DPointSet();

protected:
	bool setNumPoints(size_t inNumPoints);
	size_t getNumPoints() { return mNumPoints; };

	GLuint				mPointVAO;
	GLuint				mPointVBO;
	GLuint				mPointShaderHandle;

	size_t				mNumPoints;
	PointStarVertex*	mPointArray;
};

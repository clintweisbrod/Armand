#include "stdafx.h"

#include "Static3DPointSet.h"

Static3DPointSet::Static3DPointSet()
{
	mNumPoints = 0;
	mPointArray = NULL;
	mPointVAO = 0;
	mPointVBO = 0;
	mPointShaderHandle = 0;
}

Static3DPointSet::~Static3DPointSet()
{
	if (mPointArray)
	{
		delete[] mPointArray;
		mPointArray = NULL;
	}

	if (mPointVAO)
		glDeleteVertexArrays(1, &mPointVAO);
	if (mPointVBO)
		glDeleteBuffers(1, &mPointVBO);
}

bool Static3DPointSet::setNumPoints(size_t inNumPoints)
{
	try	{
		mPointArray = new PointStarVertex[inNumPoints];
		mNumPoints = inNumPoints;
	}
	catch (bad_alloc) {
		mPointArray = NULL;
		mNumPoints = 0;
	}

	return (mPointArray != NULL);
}
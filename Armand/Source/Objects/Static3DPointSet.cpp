// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// Static3DPointSet.cpp
//
// General solution for rendering tons of data as points.
//
// THIS SOFTWARE IS PROVIDED BY CLINT WEISBROD "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL CLINT WEISBROD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------

#include "stdafx.h"

#include "Static3DPointSet.h"
#include "OpenGL/ShaderFactory.h"

Static3DPointSet::Static3DPointSet(int inNumPoints)
{
	mPointBuffer = NULL;
	mPointsVBO = 0;
	mNumPoints = inNumPoints;
}

Static3DPointSet::~Static3DPointSet()
{
	releasePointArray();

	if (mPointsVAO)
	{
		delete mPointsVAO;
		mPointsVAO = 0;
	}

	if (mPointsVBO)
		glDeleteBuffers(1, &mPointsVBO);
}

void Static3DPointSet::allocatePointArray(GLsizei inNumPoints, GLsizei inPointStride)
{
	mNumPoints = inNumPoints;
	mPointStride = inPointStride;

	// Allocate buffer large enough to hold the number of points specified
	mPointBuffer = new uint8_t[mNumPoints * mPointStride];
	if (!mPointBuffer)
		THROW(Static3DPointSetException, "Unable to allocate %d PointStarVertex elements.", mNumPoints);
}

void Static3DPointSet::releasePointArray()
{
	if (mPointBuffer)
	{
		delete[] mPointBuffer;
		mPointBuffer = NULL;
	}
}

void Static3DPointSet::finalize()
{
	uint64_t avgColor[] = { 0, 0, 0, 0 };

	// Compute and set average color as well as bounding radius
	float_t maxRadiusSquared = 0;
	for (int n = 0; n < mNumPoints; ++n)
	{
		ColorPointVertex* colorPointVertex = (ColorPointVertex*)getVertex(n);

		// Sum the colors
		avgColor[0] += colorPointVertex->color[0];
		avgColor[1] += colorPointVertex->color[1];
		avgColor[2] += colorPointVertex->color[2];
		avgColor[3] += colorPointVertex->color[3];

		// Compute largest vec
		Vec3f vec(colorPointVertex->position);
		float_t radiusSquared = vec.lengthSquared();
		if (radiusSquared > maxRadiusSquared)
			maxRadiusSquared = radiusSquared;
	}

	// Set average color
	GLubyte thePointColor[4];
	thePointColor[0] = (GLubyte)(avgColor[0] / mNumPoints);
	thePointColor[1] = (GLubyte)(avgColor[1] / mNumPoints);
	thePointColor[2] = (GLubyte)(avgColor[2] / mNumPoints);
	thePointColor[3] = (GLubyte)(avgColor[3] / mNumPoints);
	setPointColor(thePointColor);

	// Set the bounding radius of the data set
	setBoundingRadiusAU(sqrtf(maxRadiusSquared));

	// Setup VAO/VBO
	setupVBO();
}

void* Static3DPointSet::getVertex(GLsizei index)
{
	void* result = mPointBuffer + (index * mPointStride);
	return result;
}

void Static3DPointSet::setupVAO()
{
	// Setup the points VBO
	if (mPointsVAO == NULL)
	{
		mPointsVAO = new VAOBuilder;

		// Add the arrays
		mPointsVAO->addArray("vaoPosition", 3, GL_FLOAT, GL_FALSE);
		mPointsVAO->addArray("vaoSize", 1, GL_FLOAT, GL_FALSE);
		mPointsVAO->addArray("vaoColor", 4, GL_UNSIGNED_BYTE, GL_TRUE);
	}
}

void Static3DPointSet::setupVBO()
{
	// Virtual call
	setupVAO();

	// Allocate VBOs
	glGenBuffers(1, &mPointsVBO);

	// Setup the VAO
	mPointsVAO->setupGPU(mPointsVBO);

	// Copy the buffer up to the VBO
	glBufferData(GL_ARRAY_BUFFER, mNumPoints * mPointStride, mPointBuffer, GL_STATIC_DRAW);

	glIsError();

	// Release the buffer in CPU memory
	releasePointArray();
}

bool Static3DPointSet::shouldRenderAsPoint(Camera& inCamera) const
{
	// If we're inside the data set, we don't render as point.
	if (mLastViewerDistanceAU <= mBoundingRadiusAU)
		return false;
	else
		return RenderObject::shouldRenderAsPoint(inCamera);
}

bool Static3DPointSet::canRenderFull()
{
	if (mNumPoints == 0)
		return false;

	return true;
}

void Static3DPointSet::setGLStateForFullRender(float inAlpha) const
{
	setGLStateForPoint(inAlpha);
}

bool Static3DPointSet::renderFull(Camera& inCamera, float inAlpha)
{
	mPointsVAO->bind();

	enablePointShader(inCamera, inAlpha);
	glDrawArrays(GL_POINTS, 0, mNumPoints);
	disableShader();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}
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
#include "OpenGL/GLUtils.h"
#include "OpenGL/ShaderFactory.h"

Static3DPointSet::Static3DPointSet(int inNumPoints)
{
	mPointArray = NULL;
	mPointsVAO = 0;
	mPointsVBO = 0;

	// Allocate buffer for points
	setNumPoints(inNumPoints);
}

Static3DPointSet::~Static3DPointSet()
{
	if (mPointArray)
	{
		delete[] mPointArray;
		mPointArray = NULL;
	}

	if (mPointsVAO)
		glDeleteVertexArrays(1, &mPointsVAO);
	if (mPointsVBO)
		glDeleteBuffers(1, &mPointsVBO);
}

void Static3DPointSet::setNumPoints(GLsizei inNumPoints)
{
	// Allocate buffer large enough to hold the number of points specified
	mPointArray = new PointStarVertex[inNumPoints];
	if (!mPointArray)
		THROW(Static3DPointSetException, "Unable to allocate %d PointStarVertex elements.", inNumPoints);

	mNumPoints = inNumPoints;

	return;
}

void Static3DPointSet::finalize()
{
	uint64_t avgColor[] = { 0, 0, 0, 0 };

	// Compute and set average color as well as bounding radius
	float_t maxRadiusSquared = 0;
	for (int n = 0; n < mNumPoints; ++n)
	{
		// Sum the colors
		avgColor[0] += mPointArray[n].color[0];
		avgColor[1] += mPointArray[n].color[1];
		avgColor[2] += mPointArray[n].color[2];
		avgColor[3] += mPointArray[n].color[3];

		// Compute largest vec
		Vec3f vec(mPointArray[n].position);
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

	// Setup the points VBO
	if (mPointsVAO == 0)
	{
		// Allocate VAOs
		glGenVertexArrays(1, &mPointsVAO);

		// Allocate VBOs
		glGenBuffers(1, &mPointsVBO);

		// Bind the VAO as the current used object
		glBindVertexArray(mPointsVAO);

		// Bind the VBO as being the active buffer and storing vertex attributes (coordinates)
		glBindBuffer(GL_ARRAY_BUFFER, mPointsVBO);

		GLuint offset = 0;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoPosition
		offset += (3 * sizeof(GLfloat));
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoPointSize
		offset += (1 * sizeof(GLfloat));
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoColor
		offset += (4 * sizeof(GLubyte));
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoAbsMag

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		// Copy the buffer up to the VBO
		glBufferData(GL_ARRAY_BUFFER, mNumPoints * sizeof(PointStarVertex), mPointArray, GL_STATIC_DRAW);

		// Release the buffer in CPU memory
		delete[] mPointArray;
		mPointArray = NULL;

		glIsError();
	}
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

	if (mPointsVAO == 0)
		return false;

	return true;
}

void Static3DPointSet::setGLStateForFullRender(float inAlpha) const
{
	setGLStateForPoint(inAlpha);
}

bool Static3DPointSet::renderFull(Camera& inCamera, float inAlpha)
{
	glBindVertexArray(mPointsVAO);

	enablePointShader(inCamera, inAlpha);
	glDrawArrays(GL_POINTS, 0, mNumPoints);
	disableShader();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}
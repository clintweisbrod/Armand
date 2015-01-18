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
#include "OpenGL/OpenGLWindow.h"

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

bool Static3DPointSet::setNumPoints(GLsizei inNumPoints)
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

void Static3DPointSet::setGLStateForFullRender(float inAlpha) const
{
	// Enable multisampling if we can
	if (gOpenGLWindow->hasMultisampleBuffer() && GLEW_ARB_multisample)
	{
		glEnable(GL_MULTISAMPLE);
		glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	}
	else
		glDisable(GL_MULTISAMPLE_ARB);

	// Disable alpha test
	glDisable(GL_ALPHA_TEST);

	// Disable depth testing
	glDisable(GL_DEPTH_TEST);

	// Disable surface culling
	glDisable(GL_CULL_FACE);

	// Disable texturing
	glTexturingOff();

	// Enable setting point size within vertex shader
	glEnable(GL_PROGRAM_POINT_SIZE);

	// Smooth shading
	glShadeModel(GL_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if (inAlpha < 1)
	{
		// Enable blend
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		// Disable blend
		glDisable(GL_BLEND);
	}
}

void Static3DPointSet::setGLStateForPoint(float inAlpha) const
{
	setGLStateForFullRender(inAlpha);
}

bool Static3DPointSet::render(Camera& inCamera, float inAlpha)
{
	if (!RenderObject::render(inCamera, inAlpha))
		return false;

	if (shouldRenderAsPoint(inCamera))	// Render as point
	{
		setGLStateForPoint(inAlpha);
		renderAsPoint(inCamera, inAlpha);
	}
	else	// Full render
	{
		if (mNumPoints == 0)
			return false;

		// Obtain the shader
		if (mPointShaderHandle == 0)
		{
			// This is a good time load the shader program
			ShaderProgram* shaderProgram = ShaderFactory::inst()->getShaderProgram("Stars/PointStars.vert", "Stars/PointStars.frag");
			if (shaderProgram)
				mPointShaderHandle = shaderProgram->getHandle();
		}
		if (mPointShaderHandle == 0)
			return false;

		// Setup the VBO
		if (mPointVAO == 0)
		{
			// Allocate VAOs
			glGenVertexArrays(1, &mPointVAO);

			// Allocate VBOs
			glGenBuffers(1, &mPointVBO);

			// Bind the VAO as the current used object
			glBindVertexArray(mPointVAO);

			// Bind the VBO as being the active buffer and storing vertex attributes (coordinates)
			glBindBuffer(GL_ARRAY_BUFFER, mPointVBO);

			GLuint offset = 0;
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoPosition
			offset += (3 * sizeof(GLfloat));
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoPointSize
			offset += (1 * sizeof(GLfloat));
			glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoColor

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);

			// Copy the buffer up to the VBO
			glBufferData(GL_ARRAY_BUFFER, mNumPoints * sizeof(PointStarVertex), mPointArray, GL_STATIC_DRAW);

			glCheckForError();
		}

		// Render the points
		setGLStateForFullRender(inAlpha);
		renderFull(inCamera, inAlpha);
	}

	return true;
}

void Static3DPointSet::renderAsPoint(Camera& inCamera, float inAlpha)
{
	// TODO:
}

void Static3DPointSet::renderFull(Camera& inCamera, float inAlpha)
{
	glBindVertexArray(mPointVAO);

	// Get camera orthonormal basis
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);

	Vec3f viewerVector = inCamera.getCameraRelativePosition(this);
	float_t viewerDistance = viewerVector.length();

	Mat4f modelViewMatrix = Mat4f::translation(viewerVector);

	float_t n = viewerDistance - mBoundingRadiusAU;
	float_t f = viewerDistance + mBoundingRadiusAU;
	Mat4f projectionMatrix = gOpenGLWindow->getProjectionMatrix(n, f);

	// Need this to affect clipping vertices behind viewer
	glEnable(GL_CLIP_DISTANCE0);

	glUseProgram(mPointShaderHandle);
	{
		glUniform1f(glGetUniformLocation(mPointShaderHandle, "uAlpha"), inAlpha);
		glUniform1f(glGetUniformLocation(mPointShaderHandle, "uAperture"), inCamera.getAperture());
		glUniform1f(glGetUniformLocation(mPointShaderHandle, "uClipPlaneDistance"), inCamera.getFisheyeClipPlaneDistance());
		glUniform3fv(glGetUniformLocation(mPointShaderHandle, "uViewDirection"), 1, viewDirection.data);
		glUniform3fv(glGetUniformLocation(mPointShaderHandle, "uUpDirection"), 1, upDirection.data);
		glUniform3fv(glGetUniformLocation(mPointShaderHandle, "uLeftDirection"), 1, leftDirection.data);

		glUniformMatrix4fv(glGetUniformLocation(mPointShaderHandle, "uModelViewMatrix"), 1, 0, modelViewMatrix.data);
		glUniformMatrix4fv(glGetUniformLocation(mPointShaderHandle, "uProjectionMatrix"), 1, 0, projectionMatrix.data);

		glDrawArrays(GL_POINTS, 0, mNumPoints);

		glUseProgram(0);
	}

	glDisable(GL_CLIP_DISTANCE0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
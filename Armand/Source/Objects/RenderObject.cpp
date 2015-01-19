// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// RenderObject.cpp
//
// Abstract base class for all objects in Armand that are rendered.
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

#include "RenderObject.h"
#include "OpenGL/GLUtils.h"
#include "OpenGL/OpenGLWindow.h"

GLuint RenderObject::sPointVAO = 0;
GLuint RenderObject::sPointVBO = 0;
GLuint RenderObject::sPointShaderHandle = 0;

RenderObject::RenderObject()
{
	mPoint.position[0] = 0;
	mPoint.position[1] = 0;
	mPoint.position[2] = 0;
	mPoint.size = 3;				// Default point size is 3
	memset(mPoint.color, 255, 4);	// Default point color is white

	init();
}

RenderObject::~RenderObject()
{
//	if (sPointVAO)
//		glDeleteVertexArrays(1, &sPointVAO);
//	if (sPointVBO)
//		glDeleteBuffers(1, &sPointVBO);
}

void RenderObject::init()
{
	// Obtain the shader
	if (sPointShaderHandle == 0)
	{
		// This is a good time load the shader program
		ShaderProgram* shaderProgram = ShaderFactory::inst()->getShaderProgram("Stars/PointStars.vert", "Stars/PointStars.frag");
		if (shaderProgram)
			sPointShaderHandle = shaderProgram->getHandle();
	}

	// Setup the single point VBO
	if (sPointVAO == 0)
	{
		// Allocate VAOs
		glGenVertexArrays(1, &sPointVAO);

		// Allocate VBOs
		glGenBuffers(1, &sPointVBO);

		// Bind the VAO as the current used object
		glBindVertexArray(sPointVAO);

		// Bind the VBO as being the active buffer and storing vertex attributes (coordinates)
		glBindBuffer(GL_ARRAY_BUFFER, sPointVBO);

		GLuint offset = 0;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoPosition
		offset += (3 * sizeof(GLfloat));
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoPointSize
		offset += (1 * sizeof(GLfloat));
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PointStarVertex), BUFFER_OFFSET(offset));	// vaoColor

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// Place one point in the VBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(PointStarVertex), &mPoint, GL_DYNAMIC_DRAW);

		glCheckForError();
	}
}

void RenderObject::setPointSize(GLfloat inSize)
{
	if (sPointVBO)
	{
		mPoint.size = inSize;
		glBindBuffer(GL_ARRAY_BUFFER, sPointVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(PointStarVertex), &mPoint);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void RenderObject::setPointColor(const GLubyte* inColor)
{
	if (sPointVBO)
	{
		memcpy(mPoint.color, inColor, 4);
		glBindBuffer(GL_ARRAY_BUFFER, sPointVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(PointStarVertex), &mPoint);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void RenderObject::preRender(Camera& inCamera)
{
	// Get viewer-model vector, viewer distance and cache them
	mLastViewerObjectVector = inCamera.getCameraRelativePosition(this);
	mLastViewerDistanceAU = mLastViewerObjectVector.length();
	mLastViewerObjectVectorNormalized = mLastViewerObjectVector / mLastViewerDistanceAU;
}

bool RenderObject::isInView(Camera& inCamera)
{
	// If the camera is inside the bounding radius, it's visibe.
	if (mLastViewerDistanceAU <= mBoundingRadiusAU)
		return true;

	// Get camera direction
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);
	float_t fisheyeAperture = inCamera.getAperture();
	double_t objectAngularRadius = atan(mBoundingRadiusAU / mLastViewerDistanceAU);
	double_t angleBetween = acosf(viewDirection * mLastViewerObjectVectorNormalized);
	if (angleBetween - objectAngularRadius > fisheyeAperture / 2)
	{
		// Model is not visible.
		return false;
	}
	else
		return true;
}

bool RenderObject::render(Camera& inCamera, float inAlpha)
{
	if (inAlpha <= 0)
		return false;

	preRender(inCamera);

	return isInView(inCamera);
}

bool RenderObject::shouldRenderAsPoint(Camera& inCamera) const
{
	// Decide if model is big enough (in pixels) to warrant rendering.
	float_t pixelDiameter = inCamera.getObjectPixelDiameter(mLastViewerDistanceAU, mBoundingRadiusAU);
	if (pixelDiameter < mPoint.size)
		return true;
	else
		return false;
}

void RenderObject::setGLStateForFullRender(float inAlpha) const
{
}

void RenderObject::setGLStateForPoint(float inAlpha) const
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

void RenderObject::enableShader(Camera& inCamera, float inAlpha)
{
	// Get camera orthonormal basis
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);

	Mat4f modelViewMatrix = Mat4f::translation(mLastViewerObjectVector);

	float_t depth = max(mBoundingRadiusAU, mLastViewerDistanceAU * 0.1f);
	float_t n = mLastViewerDistanceAU - depth;
	float_t f = mLastViewerDistanceAU + depth;
	Mat4f projectionMatrix = gOpenGLWindow->getProjectionMatrix(n, f);

	// Need this to affect clipping vertices behind viewer
	glEnable(GL_CLIP_DISTANCE0);

	glUseProgram(sPointShaderHandle);
	{
		glUniform1f(glGetUniformLocation(sPointShaderHandle, "uAlpha"), inAlpha);
		glUniform1f(glGetUniformLocation(sPointShaderHandle, "uAperture"), inCamera.getAperture());
		glUniform1f(glGetUniformLocation(sPointShaderHandle, "uClipPlaneDistance"), inCamera.getFisheyeClipPlaneDistance());
		glUniform3fv(glGetUniformLocation(sPointShaderHandle, "uViewDirection"), 1, viewDirection.data);
		glUniform3fv(glGetUniformLocation(sPointShaderHandle, "uUpDirection"), 1, upDirection.data);
		glUniform3fv(glGetUniformLocation(sPointShaderHandle, "uLeftDirection"), 1, leftDirection.data);

		glUniformMatrix4fv(glGetUniformLocation(sPointShaderHandle, "uModelViewMatrix"), 1, 0, modelViewMatrix.data);
		glUniformMatrix4fv(glGetUniformLocation(sPointShaderHandle, "uProjectionMatrix"), 1, 0, projectionMatrix.data);
	}
}

void RenderObject::disableShader()
{
	glUseProgram(0);
	glDisable(GL_CLIP_DISTANCE0);
}

bool RenderObject::renderAsPoint(Camera& inCamera, float inAlpha)
{
	if (sPointVAO == 0)
		return false;

	glBindVertexArray(sPointVAO);

	enableShader(inCamera, inAlpha);
	glDrawArrays(GL_POINTS, 0, 1);
	disableShader();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

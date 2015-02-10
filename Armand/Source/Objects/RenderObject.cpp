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

VAOBuilder* RenderObject::sPointVAO = NULL;
GLuint RenderObject::sPointVBO = 0;

RenderObject::RenderObject()
{
	mBoundingRadiusAU = (float_t)kAuPerMetre;	// Default to 1 metre

	// Initialize single point
	mPoint.position[0] = 0;
	mPoint.position[1] = 0;
	mPoint.position[2] = 0;
	mPoint.size = 3;				// Default point size is 3
	memset(mPoint.color, 255, sizeof(mPoint.color));	// Default point color is white

	// Obtain the shader
	ShaderProgram* shaderProgram = ShaderFactory::inst()->getShaderProgram("Stars/ColorPoints.vert", "Stars/ColorPoints.frag");
	if (shaderProgram)
		mPointShaderHandle = shaderProgram->getHandle();

	// Setup the single point VBO
	if (sPointVAO == NULL)
	{
		// Allocate VAO
		sPointVAO = new VAOBuilder;

		// Add the arrays
		sPointVAO->addArray("vaoPosition", 3, GL_FLOAT, GL_FALSE);
		sPointVAO->addArray("vaoColor", 3, GL_UNSIGNED_BYTE, GL_TRUE);
		sPointVAO->addArray("vaoSize", 1, GL_FLOAT, GL_FALSE);

		// Allocate VBOs
		glGenBuffers(1, &sPointVBO);

		// Setup the VAO
		sPointVAO->setupGPU(sPointVBO);

		// Place one point in the VBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(ColorPointVertex), &mPoint, GL_DYNAMIC_DRAW);

		glIsError();
	}
}

RenderObject::~RenderObject()
{
//	if (sPointVAO)
//		delete sPointVAO;
//	if (sPointVBO)
//		glDeleteBuffers(1, &sPointVBO);
}

void RenderObject::setPointSize(GLfloat inSize)
{
	if (sPointVBO)
	{
		mPoint.size = inSize;
		glBindBuffer(GL_ARRAY_BUFFER, sPointVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ColorPointVertex), &mPoint);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void RenderObject::setPointColor(const GLubyte* inColor)
{
	if (sPointVBO)
	{
		memcpy(mPoint.color, inColor, 4);
		glBindBuffer(GL_ARRAY_BUFFER, sPointVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ColorPointVertex), &mPoint);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

bool RenderObject::canRenderFull()
{
	return false;
}

bool RenderObject::isInView(Camera& inCamera)
{
	if (mLastViewerDistanceAU == 0)
		return false;

	// If the camera is inside the bounding radius, it's visibe.
	if (mLastViewerDistanceAU <= mBoundingRadiusAU)
		return true;

	// Get camera direction
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);

	// Compute angular radius.
	// Try to use small angle approximation when possible.
	const float_t kSmallAngleApprox = 0.244f;	// roughly 1% error at this value	
	float_t angularRadius = mBoundingRadiusAU / mLastViewerDistanceAU;
	if (angularRadius > kSmallAngleApprox)
		angularRadius = asinf(angularRadius);

	// Compute angle between viewDirection and viewer-object direction
	float_t angleBetween = acosf(viewDirection * mLastViewerObjectVectorNormalized);

	GLfloat halfAperture = inCamera.getAperture() / 2;
	if (angleBetween - angularRadius > halfAperture)
		return false;
	else
	{
		// Since object is in view, we will need to compute a few other values.
		mLastAngularDiameter = angularRadius * 2;
		mLastPixelDiameter = gOpenGLWindow->getGeometryRadius() * mLastAngularDiameter / halfAperture;

		return true;
	}
}

//
// This is called per-frame by RenderObjectList::renderObjects()
//
void RenderObject::preRender(Camera& inCamera)
{
	// Get viewer-model vector, viewer distance and cache them
	mLastViewerObjectVector = inCamera.getCameraRelativePosition(this);
	mLastViewerDistanceAU = mLastViewerObjectVector.length();
	if (mLastViewerDistanceAU > 0)
		mLastViewerObjectVectorNormalized = mLastViewerObjectVector / mLastViewerDistanceAU;
}

bool RenderObject::render(Camera& inCamera, float inAlpha)
{
	bool result = false;

	if (inAlpha <= 0)
		return false;

	if (shouldRenderAsPoint(inCamera))
	{
		setGLStateForPoint(inAlpha);
		result = renderAsPoint(inCamera, inAlpha);
	}
	else if (canRenderFull())
	{
		setGLStateForFullRender(inAlpha);
		result = renderFull(inCamera, inAlpha);
	}

	return result;
}

bool RenderObject::shouldRenderAsPoint(Camera& inCamera) const
{
	// Decide if object is big enough (in pixels) to warrant rendering.
	float_t kMaxPixelDiameter = 5.0f;
	if (mLastPixelDiameter < kMaxPixelDiameter)
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

	// Enable setting point size within vertex shader
	glEnable(GL_PROGRAM_POINT_SIZE);

	// Disable point sprites
	glDisable(GL_POINT_SPRITE);

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
//		glDisable(GL_BLEND);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void RenderObject::enablePointShader(Camera& inCamera, float inAlpha)
{
	glUseProgram(mPointShaderHandle);

	setPointShaderUniforms(inCamera, inAlpha);

	// Need this to affect clipping vertices outside of field of view
	glEnable(GL_CLIP_DISTANCE0);
}

void RenderObject::setPointShaderUniforms(Camera& inCamera, float inAlpha)
{
	// Get camera orthonormal basis
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);

	Mat4f modelViewMatrix = Mat4f::translation(mLastViewerObjectVector);

	float_t depth = max(mBoundingRadiusAU, mLastViewerDistanceAU * 0.1f);
	float_t n = mLastViewerDistanceAU - depth;
	float_t f = mLastViewerDistanceAU + depth;
	Mat4f projectionMatrix = gOpenGLWindow->getProjectionMatrix(n, f);

	glUniform1f(glGetUniformLocation(mPointShaderHandle, "uAlpha"), inAlpha);
	glUniform1f(glGetUniformLocation(mPointShaderHandle, "uAperture"), inCamera.getAperture());
	glUniform1f(glGetUniformLocation(mPointShaderHandle, "uClipPlaneDistance"), inCamera.getFisheyeClipPlaneDistance());
	glUniform3fv(glGetUniformLocation(mPointShaderHandle, "uViewDirection"), 1, viewDirection.data);
	glUniform3fv(glGetUniformLocation(mPointShaderHandle, "uUpDirection"), 1, upDirection.data);
	glUniform3fv(glGetUniformLocation(mPointShaderHandle, "uLeftDirection"), 1, leftDirection.data);

	glUniformMatrix4fv(glGetUniformLocation(mPointShaderHandle, "uModelViewMatrix"), 1, 0, modelViewMatrix.data);
	glUniformMatrix4fv(glGetUniformLocation(mPointShaderHandle, "uProjectionMatrix"), 1, 0, projectionMatrix.data);
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

	setPointSize(max(mLastPixelDiameter, 1));

	sPointVAO->bind();

	enablePointShader(inCamera, inAlpha);
	glDrawArrays(GL_POINTS, 0, 1);
	disableShader();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

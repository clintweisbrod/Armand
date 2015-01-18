// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// ModelObject.cpp
//
// Class defining location and orientation of a 3DS model.
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

#include <random>

#include "ModelObject.h"
#include "Models/3DSModelFactory.h"
#include "OpenGL/OpenGLWindow.h"
#include "OpenGL/GLUtils.h"

ModelObject::ModelObject(const char* inModelFileName)
{
	mPointVAO = 0;
	mPointVBO = 0;
	mPointShaderHandle = 0;

	// Get pointer to model
	mModel = T3DSModelFactory::inst()->get(inModelFileName);
	if (mModel)
		setBoundingRadiusAU(mModel->getPhysicalRadiusAU());
}

ModelObject::~ModelObject()
{
	if (mPointVAO)
		glDeleteVertexArrays(1, &mPointVAO);
	if (mPointVBO)
		glDeleteBuffers(1, &mPointVBO);
}

bool ModelObject::isInView(Camera& inCamera)
{
	if (mModel == NULL)
		return false;

	// viewerModelVector is in AU. We need to use mModelBoundingRadius and mPhysicalRadiusInAU
	// to correctly scale viewerModelVector so that things look correct. Compute modelUnitsPerAU.
	float_t modelBoundingRadius = mModel->getModelBoundingRadius();
	float_t modelUnitsPerAU = mModel->getModelUnitsPerAU();
	mLastScaledViewerModelVector = mLastViewerObjectVector * modelUnitsPerAU;

	// Get camera direction
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);
	GLfloat fisheyeAperture = inCamera.getAperture();
	float_t viewerModelLength = mLastViewerDistanceAU * modelUnitsPerAU;
	double_t modelAngularRadius = atan(modelBoundingRadius / viewerModelLength);
	double_t angleBetween = acosf(viewDirection * mLastViewerObjectVectorNormalized);
	if (angleBetween - modelAngularRadius > fisheyeAperture / 2)
	{
		// Model is not visible.
		return false;
	}
	else
		return true;
}

bool ModelObject::shouldRenderAsPoint(Camera& inCamera) const
{
	if (mModel == NULL)
		return false;
	else
		return RenderObject::shouldRenderAsPoint(inCamera);
}

void ModelObject::setGLStateForPoint(float inAlpha) const
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

	// Disable blend
	glDisable(GL_BLEND);
}

void ModelObject::setGLStateForFullRender(float inAlpha) const
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

	// Enable depth testing
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	// Enable surface culling
	glEnable(GL_CULL_FACE);

	// Smooth shading
	glShadeModel(GL_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glDisable(GL_BLEND);
}

bool ModelObject::render(Camera& inCamera, float inAlpha)
{
	if (!RenderObject::render(inCamera, inAlpha))
		return false;

	if (shouldRenderAsPoint(inCamera))
	{
		if (mPointShaderHandle == 0)
		{
			// This is a good time load the shader program
			ShaderProgram* shaderProgram = ShaderFactory::inst()->getShaderProgram("Stars/PointStars.vert",
																				   "Stars/PointStars.frag");
			if (shaderProgram)
				mPointShaderHandle = shaderProgram->getHandle();
		}
		if (mPointShaderHandle == 0)
			return false;

		setGLStateForPoint(inAlpha);
		renderAsPoint(inCamera, inAlpha);
	}
	else
	{
		if (mModel == NULL)
			return false;
		if (!mModel->getShaderHandle())
			return false;

		setGLStateForFullRender(inAlpha);
		renderFull(inCamera, inAlpha);
	}

	return true;
}

void ModelObject::renderAsPoint(Camera& inCamera, float inAlpha)
{
	// REVISIT: 
	// TODO: This is a lot of code to render ONE point! Furthermore, I shouldn't be
	// using GL_POINTS or GL_POINT_SPRITE because although the point vertex will be
	// transformed through the fisheye math, the actual rasterized shape of the point
	// will always be circular. This will cause the point to look egg-shaped. This
	// seems like a lot of complaining for one point, but we will need a generalized
	// method for rendering thousands of nice points when we get to star rendering.
/*
	PointStarVertex vertexInfo;
	vertexInfo.position[0] = 0;
	vertexInfo.position[1] = 0;
	vertexInfo.position[2] = 0;
	vertexInfo.size = 5;
	const GLfloat kWhiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glColor4fToColor4ub(kWhiteLight, vertexInfo.color);

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
		glBufferData(GL_ARRAY_BUFFER, sizeof(PointStarVertex), &vertexInfo, GL_STATIC_DRAW);

		glCheckForError();
	}

	glBindVertexArray(mPointVAO);

	// Get camera orthonormal basis
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);

	Mat4f modelViewMatrix = Mat4f::translation(mLastViewerObjectVector);

	float_t starFieldDepth = 2000000;
	float_t n = mLastViewerDistanceAU - starFieldDepth;
	float_t f = mLastViewerDistanceAU + starFieldDepth;
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

		glDrawArrays(GL_POINTS, 0, 1);

		glUseProgram(0);
	}

	glDisable(GL_CLIP_DISTANCE0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
*/
}

void ModelObject::renderFull(Camera& inCamera, float inAlpha)
{
	// Apply translation to model to position it in world coordinates
	Mat4f viewMatrix = Mat4f::translation(mLastScaledViewerModelVector);

	// Apply rotation to model
	static GLfloat rotationY = 0;
	static GLfloat dRotationY = 0.05f;
	Quatf modelOrientation = Quatf::yrotation(degToRad(rotationY));

	// Place light at eye for now.
	// Eventually, we'll pass a "light source" object instance to inCamera.getCameraRelativePosition() 
	Vec3f lightPositionEyeCoords = inCamera.getCameraRelativePosition(&inCamera);
	lightPositionEyeCoords *= mModel->getModelUnitsPerAU();

	// Eventually light color will come from "light source" object instance
	Vec3f lightColor(1, 1, 1);

	// Render the model
	mModel->render(inCamera, viewMatrix, modelOrientation, lightPositionEyeCoords, lightColor, inAlpha);

	rotationY += dRotationY;
	if (rotationY > 360)
		rotationY -= 360;

#if 0
	// Debugging shader
	Vec4f vPosition(5, 0, 0, 1);
	Vec4f vPositionEye = modelMatrix * vPosition;
	Vec3f vPositionEyeNorm(vPositionEye.x, vPositionEye.y, vPositionEye.z);
	float depthValue = vPositionEyeNorm.length();
	vPositionEyeNorm.normalize();

	Vec2f point;
	GLfloat eyePointViewDirectionAngle = acos(viewDirection * vPositionEyeNorm);
	if (eyePointViewDirectionAngle > 0)
	{
		Vec2f xyComponents(vPositionEyeNorm * leftDirection, vPositionEyeNorm * upDirection);
		xyComponents.normalize();
		GLfloat halfAperture = kFisheyeAperture * 0.5f;
		point.x = eyePointViewDirectionAngle * xyComponents.x / halfAperture;
		point.y = -eyePointViewDirectionAngle * xyComponents.y / halfAperture;
	}

	Vec4f gl_Position = projectionMatrix * Vec4f(point.x, point.y, -depthValue, 1.0f);
#endif

#if 0
	// Draw the model's coordinate axes
	if (mRenderCoordinateAxes)
	{
		GLdouble axisLength = mModelBoundingRadius;
		//		inOpenGL->StateSet(TOGLDrawer::kOpaque);
		//		inOpenGL->StateSetTexturingOff();
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(axisLength, 0.0, 0.0);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, axisLength, 0.0);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, 0.0, axisLength);
		glEnd();
	}
#endif
}

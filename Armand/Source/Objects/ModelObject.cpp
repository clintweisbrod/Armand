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
	// Get pointer to model but only load meta data for now
	mModel = T3DSModelFactory::inst()->get(inModelFileName, true);
	if (mModel)
		setBoundingRadiusAU(mModel->getPhysicalRadiusAU());
}

ModelObject::~ModelObject()
{
}

bool ModelObject::canRenderFull()
{
	// Make sure the model factory found the file this instance was created with
	if (mModel == NULL)
		return false;

	// Make sure the model shader was successfully loaded
	if (!mModel->getShaderHandle())
		return false;

	return true;
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

bool ModelObject::renderAsPoint(Camera& inCamera, float inAlpha)
{
	// Set color
	GLubyte theColor[] = { 0, 0, 255, 255 };
	this->setPointColor(theColor);

	return RenderObject::renderAsPoint(inCamera, inAlpha);
}

bool ModelObject::renderFull(Camera& inCamera, float inAlpha)
{
	bool result = false;

	// Make sure the model data is loaded
	if (!mModel->isLoaded())
		mModel->load();
	if (!mModel->isLoaded())
		return false;	// Error while loading model data

	// Apply translation to model to position it in world coordinates
	mLastScaledViewerModelVector = mLastViewerObjectVector * mModel->getModelUnitsPerAU();
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
	result = mModel->render(inCamera, viewMatrix, modelOrientation, lightPositionEyeCoords, lightColor, inAlpha);

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

	return result;
}

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

/*
GLuint vao;
static GLsizei IndexCount;
static const GLuint PositionSlot = 0;

static void CreateIcosahedron()
{
	const int Faces[] = {
		2, 1, 0,
		3, 2, 0,
		4, 3, 0,
		5, 4, 0,
		1, 5, 0,

		11, 6, 7,
		11, 7, 8,
		11, 8, 9,
		11, 9, 10,
		11, 10, 6,

		1, 2, 6,
		2, 3, 7,
		3, 4, 8,
		4, 5, 9,
		5, 1, 10,

		2, 7, 6,
		3, 8, 7,
		4, 9, 8,
		5, 10, 9,
		1, 6, 10 };

	const float Verts[] = {
		0.000f, 0.000f, 1.000f,
		0.894f, 0.000f, 0.447f,
		0.276f, 0.851f, 0.447f,
		-0.724f, 0.526f, 0.447f,
		-0.724f, -0.526f, 0.447f,
		0.276f, -0.851f, 0.447f,
		0.724f, 0.526f, -0.447f,
		-0.276f, 0.851f, -0.447f,
		-0.894f, 0.000f, -0.447f,
		-0.276f, -0.851f, -0.447f,
		0.724f, -0.526f, -0.447f,
		0.000f, 0.000f, -1.000f };

	IndexCount = sizeof(Faces) / sizeof(Faces[0]);

	// Create the VAO:
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create the VBO for positions:
	GLuint positions;
	GLsizei stride = 3 * sizeof(float);
	glGenBuffers(1, &positions);
	glBindBuffer(GL_ARRAY_BUFFER, positions);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Verts), Verts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(PositionSlot);
	glVertexAttribPointer(PositionSlot, 3, GL_FLOAT, GL_FALSE, stride, 0);

	// Create the VBO for indices:
	GLuint indices;
	glGenBuffers(1, &indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Faces), Faces, GL_STATIC_DRAW);

	glIsError();
}
*/

ModelObject::ModelObject(const char* inModelFileName)
{
	// Get pointer to model but only load meta data for now
	mModel = T3DSModelFactory::inst()->get(inModelFileName, true);
	if (mModel)
		setBoundingRadiusAU(mModel->getPhysicalRadiusAU());

//	CreateIcosahedron();
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
/*
	// This code is for rendering tessellated icosahedron. Was learning/testing OpenGL tessellation shaders.
	// Set up the projection matrix:
	Vec2i sceneSize;
	gRenderer->getSceneSize(sceneSize);
	float aspect = (float)sceneSize.x / (float)sceneSize.y;
	Mat4f projectionMatrix = Mat4f::perspective((float)(60 * kRadPerDegree), aspect, 1, 20);

	static float theta = 0;
	Vec3f eyePosition(0, 0, -3);
	Vec3f targetPosition(0, 0, 0);
	Vec3f upVector(0, 1, 0);
	Mat4f modelViewMatrix = Mat4f::lookAt(eyePosition, targetPosition, upVector);
	Mat4f rotationMatrix = Mat4f::rotationY((float)(theta * kRadPerDegree));
	modelViewMatrix = modelViewMatrix * rotationMatrix;

	Mat3f normalMatrix(modelViewMatrix.transpose());
	Vec4f lightPosition(0.25f, 0.25f, 1.0f, 0.0f);
	lightPosition = lightPosition * rotationMatrix;

	const float TessLevelInner = 10;
	const float TessLevelOuter = 10;

	theta += 0.001f;
	if (theta > 360)
		theta -= 360;

	GLuint handle = mModel->getShaderHandle();
	glUseProgram(handle);

	glUniformMatrix4fv(glGetUniformLocation(handle, "Projection"), 1, 0, projectionMatrix.data);
	glUniformMatrix4fv(glGetUniformLocation(handle, "Modelview"), 1, 0, modelViewMatrix.data);
	glUniformMatrix3fv(glGetUniformLocation(handle, "NormalMatrix"), 1, 0, normalMatrix.data);
	glUniform3fv(glGetUniformLocation(handle, "LightPosition"), 1, lightPosition.data);
	glUniform3f(glGetUniformLocation(handle, "AmbientMaterial"), 0.04f, 0.04f, 0.04f);
	glUniform3f(glGetUniformLocation(handle, "DiffuseMaterial"), 0.0f, 0.75f, 0.75f);
	glUniform1f(glGetUniformLocation(handle, "TessLevelInner"), TessLevelInner);
	glUniform1f(glGetUniformLocation(handle, "TessLevelOuter"), TessLevelOuter);

	// This call defines that each patch received by the TCS will consist of 3 vertices.
	glPatchParameteri(GL_PATCH_VERTICES, 3);

	// Draw the patches
	glDrawElements(GL_PATCHES, IndexCount, GL_UNSIGNED_INT, 0);

	glUseProgram(0);

	return true;
*/
	bool result = false;

	// Make sure the model data is loaded
	if (!mModel->isLoaded())
	{
		Timer t;
		mModel->load();
		*DebugStream::inst() << "Model load time: " << t.elapsedSeconds() << " ms." << endl;
	}
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
	GLfloat eyePointViewDirectionAngle = acosT(viewDirection * vPositionEyeNorm);
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

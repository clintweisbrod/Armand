#include "stdafx.h"

#include "ModelObject.h"
#include "Models/3DSModelFactory.h"
#include "OpenGL/OpenGLWindow.h"
#include "OpenGL/GLUtils.h"

ModelObject::ModelObject(const char* inModelFileName)
{
	mModel = T3DSModelFactory::inst()->get(inModelFileName);
//	mModel = T3DSModelFactory::inst()->get(inModelFileName, true);

	mPointVAO = 0;
	mPointVBO = 0;
	mPointShaderHandle = 0;
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

	// This is the rudiments of a viewer model coordinate system. We're positioning a viewer
	// and a model in Cartesian space, computing a viewer-model vector, applying a rotation
	// to the model, and a rotation to the viewer, and then letting the shader do the hard
	// work of fisheye projecting the resulting image.
	// We have to perform the viewer-model vector computation in software as in general,
	// all objects (including the viewer) will maintain 128-bit integer Cartesian positions.
	// We will perform the vector subtraction in 128-bit integer math, and then cast to
	// Vec3f.

	// Get viewer-model vector, viewer distance and cache
	mLastViewerModelVector = inCamera.getCameraRelativePosition(this);
	mLastViewerDistance = mLastViewerModelVector.length();

	// viewerModelVector is in AU. We need to use mModelBoundingRadius and mPhysicalRadiusInAU
	// to correctly scale viewerModelVector so that things look correct. Compute modelUnitsPerAU.
	float_t modelBoundingRadius = mModel->getModelBoundingRadius();
	float_t modelUnitsPerAU = mModel->getModelUnitsPerAU();
	mLastScaledViewerModelVector = mLastViewerModelVector * modelUnitsPerAU;

	// Get camera direction
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);
	GLfloat fisheyeAperture = inCamera.getAperture();
	float_t viewerModelLength = mLastViewerDistance * modelUnitsPerAU;
	double_t modelAngularRadius = atan(modelBoundingRadius / viewerModelLength);
	Vec3f viewerModelVectorNorm = mLastViewerModelVector / mLastViewerDistance;
	double_t angleBetween = acosf(viewDirection * viewerModelVectorNorm);
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

	// Decide if model is big enough (in pixels) to warrant rendering.
	const float_t kMinPixelDiameter = 5;
	float_t physicalRadiusInAU = mModel->getPhysicalRadius();
	float_t pixelDiameter = inCamera.getObjectPixelDiameter(mLastViewerDistance, physicalRadiusInAU);
	if (pixelDiameter < kMinPixelDiameter)
		return true;
	else
		return false;
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

void ModelObject::render(Camera& inCamera, float inAlpha)
{
	if (!isInView(inCamera))
		return;

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

		setGLStateForPoint(inAlpha);
		renderAsPoint(inCamera, inAlpha);
	}
	else
	{
		if (mModel == NULL)
			return;
		if (!mModel->getShaderHandle())
			return;

		setGLStateForFullRender(inAlpha);
		renderFull(inCamera, inAlpha);
	}
}

void ModelObject::renderAsPoint(Camera& inCamera, float inAlpha)
{
	// TODO: This is a lot of code to render ONE point! Furthermore, I shouldn't be
	// using GL_POINTS or GL_POINT_SPRITE because although the point vertex will be
	// transformed through the fisheye math, the actual rasterized shape of the point
	// will always be circular. This will cause the point to look egg-shaped. This
	// seems like a lot of complaining for one point, but we will need a generalized
	// method for rendering thousands of nice points when we get to star rendering.
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
	}

	glBindVertexArray(mPointVAO);

	PointStarVertex vertexInfo;
	vertexInfo.position[0] = mLastViewerModelVector.x;
	vertexInfo.position[1] = mLastViewerModelVector.y;
	vertexInfo.position[2] = mLastViewerModelVector.z;
	vertexInfo.size = 5;	// not working
	const GLfloat kWhiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glColor4fToColor4ub(kWhiteLight, vertexInfo.color);

	glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(PointStarVertex), &vertexInfo, GL_DYNAMIC_DRAW);
	glCheckForError();

	// Get camera orthonormal basis
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);

	Mat4f modelViewMatrix = Mat4f::identity();

	float_t nfDelta = mLastViewerDistance * 0.01f;
	float_t n = mLastViewerDistance - nfDelta;
	float_t f = mLastViewerDistance + nfDelta;
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

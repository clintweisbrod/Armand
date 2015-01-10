#include "stdafx.h"

#include "ModelObject.h"
#include "Models/3DSModelFactory.h"
#include "OpenGL/OpenGLWindow.h"

ModelObject::ModelObject(const char* inModelFileName)
{
	mModel = T3DSModelFactory::inst()->get(inModelFileName);
//	mModel = T3DSModelFactory::inst()->get(inModelFileName, true);

	mPointVAO = 0;
	mPointShaderHandle = 0;
}

ModelObject::~ModelObject()
{
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
}

void ModelObject::renderFull(Camera& inCamera, float inAlpha)
{
	// Apply translation to model to position it in world coordinates
	Mat4f modelTranslation = Mat4f::translation(mLastScaledViewerModelVector);

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
	mModel->render(inCamera, modelTranslation, modelOrientation, lightPositionEyeCoords, lightColor, inAlpha);

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

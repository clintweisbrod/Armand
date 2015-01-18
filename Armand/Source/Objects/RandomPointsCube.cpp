#include "stdafx.h"

#include <random>
#include "RandomPointsCube.h"
#include "OpenGL/GLUtils.h"
#include "OpenGL/ShaderFactory.h"
#include "OpenGL/OpenGLWindow.h"

RandomPointsCube::RandomPointsCube()
{
	loadData();
}

RandomPointsCube::~RandomPointsCube()
{
}

void RandomPointsCube::loadData()
{
	const int kNumSamples = 1000000;
	const double_t kBoundingRadiusAU = 1000000.0;

	if (setNumPoints(kNumSamples))
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> posDis(-kBoundingRadiusAU, kBoundingRadiusAU);
		std::uniform_real_distribution<> sizeDis(0.1, 3.0);
		std::uniform_real_distribution<> colorDis(0.5, 1.0);
		for (int n = 0; n < kNumSamples; ++n)
		{
			mPointArray[n].position[0] = (GLfloat)posDis(gen);
			mPointArray[n].position[1] = (GLfloat)posDis(gen);
			mPointArray[n].position[2] = (GLfloat)posDis(gen);
			mPointArray[n].size = (GLfloat)sizeDis(gen);
			mPointArray[n].color[0] = (GLubyte)(colorDis(gen) * 255.0);
			mPointArray[n].color[1] = (GLubyte)(colorDis(gen) * 255.0);
			mPointArray[n].color[2] = (GLubyte)(colorDis(gen) * 255.0);
			mPointArray[n].color[3] = 255;
		}
	}

	setUniveralPositionAU(Vec3d(0.0, 0.0, 0.0));
	setBoundingRadiusAU((float_t)kBoundingRadiusAU);
}

void RandomPointsCube::setGLStateForFullRender(float inAlpha) const
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

void RandomPointsCube::setGLStateForPoint(float inAlpha) const
{
	setGLStateForFullRender(inAlpha);
}

bool RandomPointsCube::render(Camera& inCamera, float inAlpha)
{
	if (!RenderObject::render(inCamera, inAlpha))
		return false;

	if (shouldRenderAsPoint(inCamera))
	{
		setGLStateForPoint(inAlpha);
		renderAsPoint(inCamera, inAlpha);
	}
	else
	{
		setGLStateForFullRender(inAlpha);
		renderFull(inCamera, inAlpha);
	}

	return true;
}

void RandomPointsCube::renderAsPoint(Camera& inCamera, float inAlpha)
{
}

void RandomPointsCube::renderFull(Camera& inCamera, float inAlpha)
{
	if (getNumPoints() == 0)
		return;

	if (mPointShaderHandle == 0)
	{
		// This is a good time load the shader program
		ShaderProgram* shaderProgram = ShaderFactory::inst()->getShaderProgram(	"Stars/PointStars.vert",
																				"Stars/PointStars.frag");
		if (shaderProgram)
			mPointShaderHandle = shaderProgram->getHandle();
	}
	if (mPointShaderHandle == 0)
		return;

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
		glBufferData(GL_ARRAY_BUFFER, getNumPoints() * sizeof(PointStarVertex), mPointArray, GL_STATIC_DRAW);

		glCheckForError();
	}

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

		//		glDrawArrays(GL_POINTS, 0, 1);
		glDrawArrays(GL_POINTS, 0, 1000000);

		glUseProgram(0);
	}

	glDisable(GL_CLIP_DISTANCE0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

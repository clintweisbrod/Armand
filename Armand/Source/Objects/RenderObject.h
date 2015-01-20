// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// RenderObject.h
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

#include <GL/glew.h>
#include "Camera.h"
#include "OpenGL/VertexBufferStructs.h"

#pragma once

class RenderObject : public Object
{
public:
	RenderObject();
	virtual ~RenderObject();

	void init();
	void setBoundingRadiusAU(float_t inBoundingRadiusAU) { mBoundingRadiusAU = inBoundingRadiusAU; };
	void preRender(Camera& inCamera);
	bool render(Camera& inCamera, float inAlpha);

	float_t	getLastViewerDistanceAU() const { return mLastViewerDistanceAU; };
	float_t getLastAngularDiameter() const { return mLastAngularDiameter; };
	float_t getLastPixelDiameter() const { return mLastPixelDiameter; };

	virtual bool canRenderFull();
	virtual bool isInView(Camera& inCamera);
	virtual bool shouldRenderAsPoint(Camera& inCamera) const;
	virtual bool renderAsPoint(Camera& inCamera, float inAlpha);
	virtual bool renderFull(Camera& inCamera, float inAlpha);
	virtual void setGLStateForFullRender(float inAlpha) const;
	virtual void setGLStateForPoint(float inAlpha) const;

protected:
	Vec3f	mLastViewerObjectVector;
	Vec3f	mLastViewerObjectVectorNormalized;
	float_t	mLastViewerDistanceAU;
	float_t mLastAngularDiameter;
	float_t mLastPixelDiameter;

	float_t	mBoundingRadiusAU;

	//
	// Render as point
	//
	void	setPointSize(GLfloat inSize);
	void	setPointColor(const GLubyte* inColor);
	void	enableShader(Camera& inCamera, float inAlpha);
	void	disableShader();

	PointStarVertex	mPoint;

	// These are static so that multiple instances of RenderObject can share
	static GLuint	sPointVAO;
	static GLuint	sPointVBO;
	static GLuint	sPointShaderHandle;
};
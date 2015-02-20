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
#include "OpenGL/VAOBuilder.h"
#include "OpenGL/VertexBufferStructs.h"

#pragma once

class RenderObjectList;

class RenderObject : public Object
{
public:
	RenderObject();
	virtual ~RenderObject();

	void setBoundingRadiusAU(float_t inBoundingRadiusAU) { mBoundingRadiusAU = inBoundingRadiusAU; };
	virtual void preRender(Camera& inCamera, RenderObjectList& ioRenderList);
	bool render(Camera& inCamera, float inAlpha);
	virtual void postRender();

	float_t	getLastViewerDistanceAU() const { return mLastViewerDistanceAU; };
	float_t getLastAngularDiameter() const { return mLastAngularDiameter; };
	float_t getLastPixelDiameter() const { return mLastPixelDiameter; };

	virtual bool canRenderFull();
	virtual bool isInView(Camera& inCamera);
	virtual bool shouldRenderAsPoint(Camera& inCamera) const;
	virtual bool renderAsPoint(Camera& inCamera, float inAlpha);
	virtual bool renderFull(Camera& inCamera, float inAlpha) = 0;
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
	void	enablePointShader(Camera& inCamera, float inAlpha);

	virtual void	setPointShaderUniforms(Camera& inCamera, float inAlpha);

	void	disableShader();

	ColorPointVertex	mPoint;
	GLuint				mPointShaderHandle;

	// These are static so that multiple instances of RenderObject can share
	static VAOBuilder*	sPointVAO;
	static GLuint		sPointVBO;
};
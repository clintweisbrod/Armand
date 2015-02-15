// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// Renderer.h
//
// Class encapsulating high-level OpenGL render activity.
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

#pragma once

#include "Objects/Camera.h"
#include "Objects/Rendering/RenderObjectList.h"
#include "Utilities/Timer.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

	void	init(HDC inDC, HGLRC inRC);
	void	render();

	Camera& getCamera() { return mCamera; };
	Mat4f	getProjectionMatrix(float_t inNear, float_t inFar);

	void	getSceneSize(Vec2i& ioSceneSize) { ioSceneSize = mSceneSize; };
	void	setSceneSize(Vec2i inSceneSize) { mSceneSize = inSceneSize; };
	int		getGeometryRadius() { return mGeometryRadius; };
	void	setGeometryRadius(int inGeometryRadius) { mGeometryRadius = inGeometryRadius; };

	void	onResize(Vec2i inNewSize);

protected:
	void	preRender();
	void	postRender();

	// Viewer state
	Camera	mCamera;

	RenderObjectList	mRenderList;

	// Fisheye projection boundary
	v2f		mFisheyeBoundaryVertices[360];

private:
	HDC		mDC;					// Private GDI device context
	HGLRC	mRC;					// Permanent rendering context

	Vec2i	mSceneSize;				// Current size of windows's client area
	int		mGeometryRadius;

	Timer	mTimer;
};

extern Renderer* gRenderer;

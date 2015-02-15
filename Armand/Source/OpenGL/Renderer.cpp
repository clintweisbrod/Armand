// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// Renderer.cpp
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

#include "stdafx.h"
#include "Renderer.h"
#include "ShaderFactory.h"
#include "Fonts/FontFactory.h"
#include "Models/3DSModelFactory.h"
#include "Utilities/StringUtils.h"

#include "Objects/ModelObject.h"
#include "Objects/RandomPointsCube.h"
#include "Objects/Stars/HYGDatabase.h"

Renderer::Renderer()
{
	gRenderer = this;
}

Renderer::~Renderer()
{
	// Release the FontFactory
	FontFactory::destroy();

	// Release the ShaderFactory
	ShaderFactory::destroy();

	// Release the model factory
	T3DSModelFactory::destroy();
}

// This method is called once on app startup.
// Good place to initialize any global OpenGL resources.
void Renderer::init(HDC inDC, HGLRC inRC)
{
	mDC = inDC;
	mRC = inRC;

	// Setup camera
	mCamera.setAperture(degToRad(180.0f));			// 180 degree fisheye
	mCamera.setUniveralPositionKm(Vec3d(0, 0, 5));	// Located at origin in our universal coordinate system
	mCamera.lookAt(Vec3f(0, 0, -1), Vec3f(0, 1, 0));	// Looking down -z axis with +y axis up.

	// Make sure all our factory instances are created
	ShaderFactory* sf = ShaderFactory::inst();
	FontFactory* ff = FontFactory::inst();
	T3DSModelFactory* mf = T3DSModelFactory::inst();
	if (!ff || !sf || !mf)
		return;
	///*
	glShadeModel(GL_SMOOTH);
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	//	glEnable(GL_DEPTH_TEST);
	//	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//*/
	//	theTexture = new Texture("Data/TestImage.dds");
	//	if (theTexture->getImageBufferOK())
	//		theTexture->sendToGPU();
}

void Renderer::onResize(Vec2i inNewSize)
{
	mSceneSize = inNewSize;

	// Reset the current viewport
	glViewport(0, 0, mSceneSize.x, mSceneSize.y);

	// Construct ortho projection matrix that has origin in center of viewport, such that smaller
	// dimension of viewport runs to +-1 and larger dimension is proportionally greater than 1.
	float h = 1, v = 1;
	if (mSceneSize.x > mSceneSize.y)
		h = (float)mSceneSize.x / (float)mSceneSize.y;
	else
		v = (float)mSceneSize.y / (float)mSceneSize.x;
	mGeometryRadius = min(mSceneSize.x, mSceneSize.y) / 2;

	// Set the fixed pipeline projection to the same as described above
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-h, h, -v, v, -1, 1);

	FontFactory::inst()->sceneSizeChanged(mSceneSize);

	// Compute the fisheye boundary vertices
	for (int i = 0; i < 360; i++)
	{
		float_t angle = degToRad((float_t)i);
		mFisheyeBoundaryVertices[i].x = cosf(angle);
		mFisheyeBoundaryVertices[i].y = sinf(angle);
	}
}

Mat4f Renderer::getProjectionMatrix(float_t inNear, float_t inFar)
{
	float h = 1, v = 1;
	if (mSceneSize.x > mSceneSize.y)
		h = (float)mSceneSize.x / (float)mSceneSize.y;
	else
		v = (float)mSceneSize.y / (float)mSceneSize.x;

	return Mat4f::orthographic(-h, h, -v, v, inNear, inFar);
}

void Renderer::preRender()
{
	mCamera.update();
}

void Renderer::render()
{
	preRender();

	// Clear screen and modelview matrix
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear screen and depth buffer

	mRenderList.clear();

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	// Draw boundary of projection area
	glColor3f(1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	glMatrixMode(GL_MODELVIEW);							// Select the modelview matrix
	glLoadIdentity();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2,							// number of coordinates per vertex (x,y)
		GL_FLOAT,					// they are floats
		sizeof(v2f),				// stride
		mFisheyeBoundaryVertices);	// the array pointer
	glDrawArrays(GL_LINE_LOOP, 0, 360);
	glDisableClientState(GL_VERTEX_ARRAY);

	/*
	// Testing texture loading
	glMatrixMode(GL_PROJECTION);						// Select the projection matrix
	glLoadIdentity();									// Reset the projection matrix
	glOrtho(0.0, mSceneSize.x, mSceneSize.y, 0.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);							// Select the modelview matrix
	glLoadIdentity();									// Reset the modelview matrix

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, theTexture->getTextureID());
	Vec2i dimensions = theTexture->getDimensions();
	Vec2f texCoords = theTexture->getTexCoords();
	Vec2i screenLocationTL = (mSceneSize - dimensions) / 2;
	Vec2i screenLocationBR = screenLocationTL + dimensions;
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);						glVertex2i(screenLocationTL.x, screenLocationTL.y);
	glTexCoord2f(0, texCoords.t);			glVertex2i(screenLocationTL.x, screenLocationBR.y);
	glTexCoord2f(texCoords.s, texCoords.t); glVertex2i(screenLocationBR.x, screenLocationBR.y);
	glTexCoord2f(texCoords.s, 0);			glVertex2i(screenLocationBR.x, screenLocationTL.y);
	glEnd();
	*/



	///*
	// Testing 3DS model loading and fisheye projection shader
	static ModelObject model("Apollo_3rdStage.3ds");
	model.setUniveralPositionKm(Vec3d(0.0, 0.0, 10));
	mRenderList.addObject(mCamera, model);

	//	T3DSModelFactory::inst()->RemoveAll();
	//*/
	static RandomPointsCube dataCube(1000000);
	mRenderList.addObject(mCamera, dataCube);

	static HYGDatabase hygStars;
	mRenderList.addObject(mCamera, hygStars);

	// Render the objects in the render list
	mRenderList.renderObjects(mCamera);

	// FontFactory testing
	string fontName("Verdana");
//	wstring text(L"\260 A quick brown fox jumped over the lazy dog. !@#$%^&*()-=+{}[];:'<>,.?/`~\"");

	FontRenderer* fontRenderer = FontFactory::inst()->getFontRenderer(fontName);

	// The code below renders the text string above 1000 times with random positions, sizes
	// and angles. On my 2010 Macbook Pro, framerate was 26.6 fps. Pretty damn good
	// under normal circumstances far fewer than 1000 text items will be rendered per frame
	// and each item will certainly be much smaller than the 72-char string we're using.
#if 0
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> xDis(0, mWindowSize.cx);
	std::uniform_int_distribution<> yDis(0, mWindowSize.cy);
	std::uniform_int_distribution<> sizeDis(10, 36);
	std::uniform_int_distribution<> angleDis(0, 359);
	Vec2f position;
	int fontSize;
	float angle;
	for (int n = 0; n < 50; ++n)
	{
		position.x = (GLfloat)xDis(gen);
		position.y = (GLfloat)yDis(gen);
		fontSize = sizeDis(gen);
		angle = (float)angleDis(gen);
		fontRenderer->render(text, fontSize, position, Vec4f(1.0f, 1.0f, 1.0f, 1.0f), angle);
	}
#endif

	glEnable(GL_BLEND);
	wstring speedStr = getNiceSpeedString(mCamera.getSpeed(), 2);
	wchar_t infoBuffer[256];
	wstring text;
	swprintf(infoBuffer, 256, L"Speed: %s", speedStr.c_str());
	text = infoBuffer;
	fontRenderer->renderSpherical(text, 20, Vec2f(degToRad(-40.0f), degToRad(5.0f)), Vec4f(1.0f, 1.0f, 1.0f, 1.0f), true);

	Vec3f cameraPos = (Vec3f)mCamera.getUniveralPositionAU();
	float_t cameraDistanceFromOrigin = cameraPos.length();
	wstring distanceStr = getNiceDistanceString(cameraDistanceFromOrigin, 2);
	swprintf(infoBuffer, 256, L"Origin: %s", distanceStr.c_str());
	text = infoBuffer;
	fontRenderer->renderSpherical(text, 20, Vec2f(degToRad(40.0f), degToRad(5.0f)), Vec4f(1.0f, 1.0f, 1.0f, 1.0f));

	HYGDataVecP_t nearestStars;
//	mTimer.reset();
	hygStars.getNearestToPosition((Vec3f)mCamera.getUniveralPositionAU(), nearestStars);
//	LOG(INFO) << "getNearestToPosition(): " << mTimer.elapsedMilliseconds() << " ms.";
	if (!nearestStars.empty())
	{
		swprintf(infoBuffer, 256, L"Nearest Star: %s", wstringFromString(nearestStars[0]->mIdentifier).c_str());
		text = infoBuffer;
		fontRenderer->renderSpherical(text, 20, Vec2f(degToRad(-20.0f), degToRad(5.0f)), Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	}

	postRender();
}

void Renderer::postRender()
{
	SwapBuffers(mDC);	// Swap buffers (double buffering)
}

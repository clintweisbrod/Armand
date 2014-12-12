// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// OpenGLWindow.h
//
// Class encapsulating a window containing an OpenGL context.
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

#include <GL/glew.h>
#include <GL/wglew.h>

#include "Math/vecmath.h"
#include "OpenGL/Textures/Texture.h"

class OpenGLWindow
{
	public:
		OpenGLWindow();
		~OpenGLWindow();

		// Creation / destruction
		bool			create(HINSTANCE inInstance, WNDPROC inWndProc, WORD inMenuID,
							   TCHAR* inTitle, int inWidth, int inHeight, int inBitsPerPixel, bool inFullscreenFlag);
		void			destroy();
		HINSTANCE		getAppInstance() { return mhInstance; };
		HWND			getHWND() { return mhWnd; };

		// Informational
		bool			getIsCreated() { return mCreated; };
		void			getSceneSize(Vec2i& ioSceneSize) { ioSceneSize = mSceneSize; };

		// OpenGL
		void			drawScene();
		void			resizeScene(GLsizei inWidth, GLsizei inHeight);
		
		// User input
		void			mouseEvent(WORD inXPos, WORD inYPos, bool inCtrlDown, bool inShiftDown, bool inLeftDown, bool inMiddleDown, bool inRightDown);
		void			mouseWheelEvent(double inWheelDelta);
		void			keyboardKeyDown(WPARAM inKey);
		void			keyboardKeyUp(WPARAM inKey);
		bool*			getKeys() { return mKeys; };
		
		// Harness state
		void			setClearColor(const GLfloat inRed, const GLfloat inGreen, const GLfloat inBlue);

	protected:
		static bool		sEnabledGLExtensions;

		bool			createWindow(HINSTANCE inInstance, WNDPROC inWndProc, WORD inMenuID,
									 TCHAR* inTitle, int inWidth, int inHeight, int inBitsPerPixel, bool inFullscreenFlag);
		bool			setupOpenGLForWindow(GLuint inPixelFormat, PIXELFORMATDESCRIPTOR* inPFD);
		GLuint			selectBestPixelFormatUsingWGL(HDC hDC);
		double			getCurrentSeconds() const;
		void			handleKeys();

		// OpenGL
		void			initGL();
		void			preRender();
		void			render();
		void			postRender();

		bool			mCreated;
		bool			mGLInitialized;
		bool			mHasMultisampleBuffer;

		HINSTANCE		mhInstance;		// Holds the instance of the application
		HWND			mhWnd;			// Holds our window handle
		HDC				mhDC;			// Private GDI device context
		HGLRC			mhRC;			// Permanent rendering context
		wstring			mWindowTitle;
		bool			mFullscreen;
		Vec2i			mSceneSize;
		int				mCmdShow;

		// Frame rate determination
		double			mFrameStartTime;
		LARGE_INTEGER	mTicksPerSecond;
		unsigned int	mFrameCount;
		double			mAverageRenderedFrameRate;	// In microseconds

		// Keyboard input
		bool			mKeys[256];		// Array used for the keyboard routine
		double			mLastKeyboardResponseSeconds;

		// Mouse input
		double			mLastMouseMoveSeconds;
		Vec2i			mLastMousePosition;

		Texture*		theTexture;

		Vec3f			mClearColor;
};
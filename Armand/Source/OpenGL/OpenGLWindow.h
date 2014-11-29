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

// TODO: Move these some place more logical
#define			kPiDefine				3.14159265358979323846	// pi base unit used to calculate others
const double	kPi						= kPiDefine;
const double	kOneOverPi				= 1.0/kPiDefine;
const double	kTwicePi				= 2.0*kPiDefine;
const double	kOneOverTwicePi			= 1.0/(2.0*kPiDefine);
const double	kOneOverFourPi			= 1.0/(4.0*kPiDefine);
const double	kHalfPi					= kPiDefine/2.0;
const double	kPiBy4					= kPiDefine/4.0;
const double	k3PiBy2					= 3.0*kPiDefine/2.0;
const double	kRadPerDegree			= kPiDefine/180.0;
const double	kDegPerRadian			= 180.0/kPiDefine;

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
		void			getWindowSize(SIZE& ioWindowSize) { ioWindowSize = mWindowSize; };

		// OpenGL
		void			render();
		void			resizeGLScene(GLsizei inWidth, GLsizei inHeight);
		
		// User input
		void			mouseEvent(WORD inXPos, WORD inYPos, bool inCtrlDown, bool inShiftDown, bool inLeftDown, bool inMiddleDown, bool inRightDown);
		void			mouseWheelEvent(double inWheelDelta);
		void			keyboardKeyDown(WPARAM inKey);
		void			keyboardKeyUp(WPARAM inKey);
		bool*			getKeys() { return mKeys; };
		
		// Viewer state
		void			getGazeAngles(double& ioAzimuth, double& ioAltitude) const;
		TVector3d		getGazeVector() const { return mGazeVector; };
		TVector3d		getStrafeVector() const;
		TVector3d		getViewerLocation() const { return mViewerLocation; };
		void			setViewerLocation(const TVector3d inViewerLocation) { mViewerLocation = inViewerLocation; };
		void			setViewerDirection(const TVector3d inViewerDirection);

		// Harness state
		void			showCoordinateAxes(bool inShow) { mShowCoordinateAxes = inShow; };
		void			setClearColor(const GLfloat inRed, const GLfloat inGreen, const GLfloat inBlue);

	protected:
		static bool		sEnabledGLExtensions;

		bool			createWindow(HINSTANCE inInstance, WNDPROC inWndProc, WORD inMenuID,
									 TCHAR* inTitle, int inWidth, int inHeight, int inBitsPerPixel, bool inFullscreenFlag);
		bool			setupOpenGLForWindow(GLuint inPixelFormat, PIXELFORMATDESCRIPTOR* inPFD);
		GLuint			selectBestPixelFormatUsingWGL(HDC hDC);
		void			initGL();
		void			renderCoordinateAxes() const;
		double			getCurrentSeconds() const;
		void			handleKeys();
		void			DecelerateFunction(TVector2d& ioVector, const double inBrakingFactor);

		bool			mCreated;
		bool			mGLInitialized;
		bool			mHasMultisampleBuffer;

		HINSTANCE		mhInstance;		// Holds the instance of the application
		HWND			mhWnd;			// Holds our window handle
		HDC				mhDC;			// Private GDI device context
		HGLRC			mhRC;			// Permanent rendering context
		wstring			mWindowTitle;
		bool			mFullscreen;
		SIZE			mWindowSize;
		int				mCmdShow;

		// Frame rate determination
		double			mFrameStartTime;
		LARGE_INTEGER	mTicksPerSecond;
		unsigned int	mFrameCount;
		double			mAverageRenderedFrameRate;	// In microseconds

		// Keyboard input
		bool			mKeys[256];		// Array used for the keyboard routine
		double			mLastKeyboardResponseSeconds;
		TVector2d		mGazeSpeed;
		TVector2d		mViewerSpeed;

		// Mouse input
		double			mLastMouseMoveSeconds;
		TVector2i		mLastMousePosition;
		TPolar3d		mGazePolar;
		TVector3d		mGazeVector;
		TPolar3d		mLightPolar;
		TVector3f		mLightVector;

		TVector3d		mViewerLocation;

		bool			mShowCoordinateAxes;
		TVector3f		mClearColor;
};
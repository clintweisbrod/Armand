// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// OpenGLWindow.cpp
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

#include "stdafx.h"

#include "OpenGLWindow.h"

bool OpenGLWindow::sEnabledGLExtensions = false;

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Class construction/destruction
//----------------------------------------------------------------------
//----------------------------------------------------------------------

OpenGLWindow::OpenGLWindow() : mCreated(false),
							   mGLInitialized(false),
							   mHasMultisampleBuffer(false),
							   mhInstance(NULL),
							   mhWnd(NULL),
							   mhDC(NULL),
							   mhRC(NULL),
							   mFullscreen(false),
							   mCmdShow(SW_SHOWMAXIMIZED),
							   mFrameCount(0),
							   mAverageRenderedFrameRate(1.0/60.0),
							   mLastKeyboardResponseSeconds(0.0),
							   mLastMouseMoveSeconds(0.0)
{
	// Get the high resolution counter's accuracy
	QueryPerformanceFrequency(&mTicksPerSecond);

	// Clear all keys
	memset(mKeys, 0, sizeof(mKeys));

	// Window styles for both windowed and fullscreen modes
	mWindowedStyle = WS_OVERLAPPEDWINDOW;
	mWindowedExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	mFullScreenStyle = WS_POPUP;
	mFullScreenExStyle = WS_EX_APPWINDOW;

	mIgnoreResizeEvents = false;

//	mCmdShow = SW_SHOW;
}

OpenGLWindow::~OpenGLWindow()
{
	// Clean up the resources for this window
	destroy();
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// OpenGL/Window creation-related methods
//----------------------------------------------------------------------
//----------------------------------------------------------------------

bool OpenGLWindow::create(HINSTANCE inInstance, WNDPROC inWndProc, WORD inMenuID,
	TCHAR* inTitle, int inWidth, int inHeight, int inBitsPerPixel,
	bool inFullscreen)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,								// Version number
		PFD_DRAW_TO_WINDOW |			// Format must support window
		PFD_SUPPORT_OPENGL |			// Format must support OpenGL
		PFD_DOUBLEBUFFER,				// Must support double Buffering
		PFD_TYPE_RGBA,					// Request an RGBA format
		inBitsPerPixel,					// Select our color depth
		0, 0, 0, 0, 0, 0,				// Color bits ignored
		0,								// No alpha buffer
		0,								// Shift bit ignored
		0,								// No accumulation buffer
		0, 0, 0, 0,						// Accumulation bits ignored
		16,								// 16-bit Z-buffer (depth buffer)  
		0,								// No stencil buffer
		0,								// No auxiliary buffer
		PFD_MAIN_PLANE,					// Main drawing layer
		0,								// Reserved
		0, 0, 0							// Layer masks ignored
	};

	if (createWindow(inInstance, inWndProc, inMenuID, inTitle, inWidth, inHeight, inBitsPerPixel, inFullscreen))
	{
		// Did we get a device context?
		if (!(mhDC = GetDC(mhWnd)))
		{
			destroy();
			MessageBox(NULL, L"Can't create a GL device context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		// Did Windows find a matching pixel format?
		GLuint basicPixelFormat;
		if (!(basicPixelFormat = ChoosePixelFormat(mhDC, &pfd)))
		{
			destroy();
			MessageBox(NULL, L"Can't find a suitable pixelformat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		if (setupOpenGLForWindow(basicPixelFormat, &pfd))
		{
			// We only need to do this once
			if (!sEnabledGLExtensions)
			{
				GLenum err = glewInit();
				if (err == GLEW_OK)
					sEnabledGLExtensions = true;
				else
				{
					// Problem: glewInit failed, something is seriously wrong.
					char errMsg[256];
					sprintf(errMsg, "Error: %s\n", glewGetErrorString(err));
					fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
				}
			}

			// Now try to obtain a pixel format with extended capabilities
			GLuint uberPixelFormat = selectBestPixelFormatUsingWGL(mhDC);
			if ((uberPixelFormat > 0) && (uberPixelFormat != basicPixelFormat))
			{
				// Windows only allows a pixel format to be set once in a window,
				// therefore, we must destroy and re-create our window.
				destroy();
				if (createWindow(inInstance, inWndProc, inMenuID, inTitle, inWidth, inHeight, inBitsPerPixel, inFullscreen))
				{
					if (!setupOpenGLForWindow(uberPixelFormat, &pfd))
					{
						// Well, this is lame! We can't use the uber pixel format
						// Destroy and re-create our window using basic pixel format
						destroy();
						if (createWindow(inInstance, inWndProc, inMenuID, inTitle, inWidth, inHeight, inBitsPerPixel, inFullscreen))
						{
							if (!setupOpenGLForWindow(basicPixelFormat, &pfd))
								return false;
						}
						mHasMultisampleBuffer = false;
					}
				}
				else
					return false;
			}

			mGLInitialized = true;

			if (mHasMultisampleBuffer)
			{
				glEnable(GL_MULTISAMPLE_ARB);
				glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
			}

			// Initialize our newly created GL window
			mRenderer.init(mhDC, mhRC);

			// Enable/disable VSYNC
			if (wglSwapIntervalEXT)
//				wglSwapIntervalEXT(0);	// Disabling with zero for now so that I can see the maximum frame rate.
				wglSwapIntervalEXT(1);

			// Make the window visible
			if (!mFullscreen)
				ShowWindow(mhWnd, mCmdShow);

			// Slightly higher priority
			SetForegroundWindow(mhWnd);

			// Sets keyboard focus to the window
			SetFocus(mhWnd);

			// Reset frame count
			mFrameCount = 0;

			mCreated = true;
		}
	}

	return true;
}

void OpenGLWindow::destroy()
{
	if (mhRC)
	{
		// Release the DC And RC contexts
		if (!wglMakeCurrent(NULL, NULL))
			MessageBox(NULL, L"Release of DC and RC failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		// Delete the RC?
		if (!wglDeleteContext(mhRC))
			MessageBox(NULL, L"Release rendering context failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		mhRC = NULL;
	}

	// Release the DC
	if (mhDC && !ReleaseDC(mhWnd, mhDC))
	{
		MessageBox(NULL, L"Release device context failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		mhDC = NULL;
	}

	// Destroy the window
	if (mhWnd && !DestroyWindow(mhWnd))
	{
		MessageBox(NULL, L"Could not release window handle.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		mhWnd = NULL;
	}

	// Unregister class
	if (!UnregisterClass(L"OpenGL", mhInstance))
	{
		MessageBox(NULL, L"Could not unregister class.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		mhInstance = NULL;
	}

	mCreated = false;
}

void OpenGLWindow::setFullScreen(bool inFullScreen)
{
	if (inFullScreen)
	{
		// Remember position and size of window
		RECT rect;
		GetWindowRect(mhWnd, &rect);
		mLastWindowPosition = Vec2i(rect.left, rect.top);
		mLastWindowSize = Vec2i(rect.right - rect.left, rect.bottom - rect.top);
		mRenderer.getSceneSize(mLastWindowedSceneSize);

		// The following calls may cause spurious WM_SIZE messages we don't want to handle
		mIgnoreResizeEvents = true;

		// Remove menu
		SetMenu(mhWnd, NULL);

		// Set fullscreen style
		SetWindowLong(mhWnd, GWL_STYLE, mFullScreenStyle);
		SetWindowLong(mhWnd, GWL_EXSTYLE, mFullScreenExStyle);

		mIgnoreResizeEvents = false;

		// Modify window size and position
		Vec2i windowPosition(GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN));
		Vec2i sceneSize(GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));
		mRenderer.setSceneSize(sceneSize);
		mWindowSize = sceneSize;
		MoveWindow(mhWnd, windowPosition.x, windowPosition.y, mWindowSize.x, mWindowSize.y, TRUE);
		mFullscreen = true;

		// Make sure window is visible
		if (!IsWindowVisible(mhWnd))
			ShowWindow(mhWnd, SW_SHOW);
	}
	else
	{
		// The following calls may cause spurious WM_SIZE messages we don't want to handle
		mIgnoreResizeEvents = true;

		// Set windowed style
		SetWindowLong(mhWnd, GWL_STYLE, mWindowedStyle);
		SetWindowLong(mhWnd, GWL_EXSTYLE, mWindowedExStyle);

		// Replace menu
		SetMenu(mhWnd, mWindowMenu);

		mIgnoreResizeEvents = false;

		// Restore size and position
		Vec2i windowPosition = mLastWindowPosition;
		mRenderer.setSceneSize(mLastWindowedSceneSize);
		mWindowSize = mLastWindowSize;
		MoveWindow(mhWnd, windowPosition.x, windowPosition.y, mWindowSize.x, mWindowSize.y, TRUE);
		mFullscreen = false;

		// Make sure window is visible
		if (!IsWindowVisible(mhWnd))
			ShowWindow(mhWnd, SW_SHOW);
	}
}

bool OpenGLWindow::createWindow(HINSTANCE inInstance, WNDPROC inWndProc, WORD inMenuID,
	TCHAR* inTitle, int inWidth, int inHeight, int inBitsPerPixel,
	bool inFullscreen)
{
	mhInstance = inInstance;
	mWindowTitle = inTitle;
	mFullscreen = inFullscreen;

	Vec2i desktopSize(GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = inWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = mhInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = ((inMenuID > 0) && !mFullscreen) ? MAKEINTRESOURCE(inMenuID) : NULL;
	wcex.lpszClassName = L"OpenGL";
	wcex.hIconSm = NULL;

	// Attempt to register the window class
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, L"Failed to register the window class.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	// Setup window styles, window position and size based on screen mode
	DWORD dwStyle, dwExStyle;
	Vec2i windowPosition;
	if (mFullscreen)
	{
		// Compute window dimensions to use after we come out of fullscreen.
		// inWidth and inHeight specify scene size, not window size
		mLastWindowedSceneSize = Vec2i(inWidth, inHeight);

		// Adjust window to true requested size
		RECT windowRect = { 0, 0, mLastWindowedSceneSize.x, mLastWindowedSceneSize.y };
		AdjustWindowRectEx(&windowRect, mWindowedStyle, (wcex.lpszMenuName != NULL) ? TRUE : FALSE, mWindowedExStyle);
		mLastWindowSize.x = windowRect.right - windowRect.left;
		mLastWindowSize.y = windowRect.bottom - windowRect.top;

		// Center window in desktop
		mLastWindowPosition = (desktopSize - mLastWindowSize) / 2;

		// Set window position, window size and scene size to desktop
		windowPosition = Vec2i(GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN));
		mWindowSize = desktopSize;
		mRenderer.setSceneSize(desktopSize);
		mFullscreen = true;

		dwStyle = mFullScreenStyle;
		dwExStyle = mFullScreenExStyle;
	}
	else
	{
		dwStyle = mWindowedStyle;
		dwExStyle = mWindowedExStyle;

		// inWidth and inHeight specify scene size, not window size
		mRenderer.setSceneSize(Vec2i(inWidth, inHeight));

		// Adjust window to true requested size
		RECT windowRect = { 0, 0, inWidth, inHeight };
		AdjustWindowRectEx(&windowRect, dwStyle, (wcex.lpszMenuName != NULL) ? TRUE : FALSE, dwExStyle);
		mWindowSize.x = windowRect.right - windowRect.left;
		mWindowSize.y = windowRect.bottom - windowRect.top;

		// Center window in desktop
		windowPosition = (desktopSize - mWindowSize) / 2;

		mFullscreen = false;
	}

	mhWnd = CreateWindowEx(dwExStyle,					// Extended style for the window
							L"OpenGL",					// Class name
							inTitle,					// Window title
							dwStyle |					// Defined window style
							WS_CLIPSIBLINGS |			// Required window style
							WS_CLIPCHILDREN,			// Required window style
							windowPosition.x,			// Window horizontal position
							windowPosition.y,			// Window vertical position
							mWindowSize.x,				// Window width
							mWindowSize.y,				// Window height
							NULL,						// No parent window
							NULL,						// No menu
							mhInstance,					// Instance
							NULL);						// Dont pass anything to WM_CREATE

	// Create the window
	if (mhWnd == NULL)
	{
		destroy();
		MessageBox(NULL, L"Window creation error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	// We need this for switching back from fullscreen
	mWindowMenu = GetMenu(mhWnd);

	return true;
}

bool OpenGLWindow::setupOpenGLForWindow(GLuint inPixelFormat, PIXELFORMATDESCRIPTOR* inPFD)
{
	// Did we get a device context?
	if (!(mhDC = GetDC(mhWnd)))
	{
		destroy();
		MessageBox(NULL, L"Can't create a GL device context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	// Are we able to set the pixel format?
	if (!SetPixelFormat(mhDC, inPixelFormat, inPFD))
	{
		destroy();
		MessageBox(NULL, L"Can't set the pixelformat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	// Are we able to get a rendering context?
	if (!(mhRC = wglCreateContext(mhDC)))
	{
		destroy();
		MessageBox(NULL, L"Can't create a GL rendering context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	// Try to activate the rendering context
	if (!wglMakeCurrent(mhDC, mhRC))
	{
		destroy();
		MessageBox(NULL, L"Can't activate the GL rendering context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
//	The modern method of choosing an appropriate pixel format.
//---------------------------------------------------------------------------
GLuint OpenGLWindow::selectBestPixelFormatUsingWGL(HDC hDC)
{
	GLuint result = 0;

	int attributeList[] = { WGL_PIXEL_TYPE_ARB,		// 0
		WGL_DRAW_TO_WINDOW_ARB,	// 1
		WGL_SUPPORT_OPENGL_ARB,	// 2
		WGL_ACCELERATION_ARB,	// 3
		WGL_DOUBLE_BUFFER_ARB,	// 4
		WGL_COLOR_BITS_ARB,		// 5
		WGL_ALPHA_BITS_ARB,		// 6
		WGL_STENCIL_BITS_ARB,	// 7
		WGL_SAMPLE_BUFFERS_ARB,	// 8
		WGL_SAMPLES_ARB,		// 9
		WGL_DEPTH_BITS_ARB,		// 10
		0, };

	// Figure out how many attribs we've specified. Could hardcode a value, but that's lame because we will likely add
	// more items in the future.
	UINT numAttributes = 0;
	int i;
	for (i = 0; i < sizeof(attributeList); i++)
	{
		if (attributeList[i] == 0)
			break;
		numAttributes++;
	}

	const bool kUseMultisampling = true;
	const char *WGLExtensionString = wglGetExtensionsStringARB(hDC);
	GLboolean hasMultisample = GLEW_ARB_multisample;
	bool wantMultisample = (hasMultisample && kUseMultisampling);

	int maxPixelFormats = DescribePixelFormat(hDC, 1, 0, NULL);
	if (maxPixelFormats > 0)
	{
		UINT* pixelFormatScores = new UINT[maxPixelFormats + 1];
		if (pixelFormatScores)
		{
			// Zero-out the pixel format scores array
			memset(pixelFormatScores, 0, sizeof(UINT)* (maxPixelFormats + 1));

			int* attributeValues = new int[numAttributes];
			if (attributeValues)
			{
				// Enumerate all available pixel formats assigning score to each one
				for (int pfIndex = 1; pfIndex <= maxPixelFormats; pfIndex++)
				{
					if (wglGetPixelFormatAttribivARB(hDC, pfIndex, 0, numAttributes, attributeList, attributeValues))
					{
						// This stuff is mandatory. We skip all pixel formats that are missing any of these
						if (attributeValues[0] != WGL_TYPE_RGBA_ARB)			// Must have RGBA pixel data
							continue;
						if (attributeValues[1] == 0)							// Must have window support
							continue;
						if (attributeValues[2] == 0)							// Must support OpenGL
							continue;
						if (attributeValues[3] != WGL_FULL_ACCELERATION_ARB)	// Must have full hardware acceleration
							continue;
						if (attributeValues[4] == 0)							// Must have double buffer support
							continue;

						// This stuff is optional but we prioritize the attributes we want so that the most important attributes
						// contribute most to the score we compute.

						// Must have 16, 24, or 32-bit color buffer
						if (attributeValues[5] == 32)		// 32-bit color?
							pixelFormatScores[pfIndex] |= (1 << 31);
						else if (attributeValues[5] == 24)	// 24-bit color?
							pixelFormatScores[pfIndex] |= (1 << 30);
						else if (attributeValues[5] == 16)	// 16-bit color?
							pixelFormatScores[pfIndex] |= (1 << 29);

						// Alpha would be nice
						if (attributeValues[6] >= 8)		// Have we got at least 8-bit alpha?
							pixelFormatScores[pfIndex] |= (1 << 28);
						else if (attributeValues[6] == 1)	// Have we got 1-bit alpha?
							pixelFormatScores[pfIndex] |= (1 << 27);

						// Stencil would be good, but not mission-critical
						if (attributeValues[7] > 8)			// Have we got more than 8-bit stencil?
							pixelFormatScores[pfIndex] |= (1 << 26);
						else if (attributeValues[7] == 8)	// Have we got 8-bit stencil?
							pixelFormatScores[pfIndex] |= (1 << 25);

						// Multisampling support
						if (wantMultisample)
						{
							if (attributeValues[8] > 0)	// Have we got a multisample buffer?
								pixelFormatScores[pfIndex] |= (1 << 24);

							if (attributeValues[9] == 6)		// 6 samples per pixel
								pixelFormatScores[pfIndex] |= (1 << 23);
							else if (attributeValues[9] == 4)	// 4 samples per pixel
								pixelFormatScores[pfIndex] |= (1 << 22);
							else if (attributeValues[9] == 2)	// 2 samples per pixel
								pixelFormatScores[pfIndex] |= (1 << 21);
						}

						// We should have some depth buffer for 3DS model rendering, but it appears that this can cause
						// SN to break on some older model laptops. We will leave it out for now.
						if (attributeValues[10] == 32)		// Have we got 32-bit depth buffer?
							pixelFormatScores[pfIndex] |= (1 << 20);
						else if (attributeValues[10] == 24)	// Have we got 24-bit depth buffer?
							pixelFormatScores[pfIndex] |= (1 << 19);
						else if (attributeValues[10] == 16)	// Have we got 16-bit depth buffer?
							pixelFormatScores[pfIndex] |= (1 << 18);
						else if ((attributeValues[10] > 0) && (attributeValues[10] < 16))	// Have we got less than 16-bit depth buffer?
							pixelFormatScores[pfIndex] |= (1 << 17);
					}
				}

				delete[] attributeValues;
			}

			// Now find the index in pixelFormatScore array with largest value
			UINT bestScore = 0;
			for (i = 1; i <= maxPixelFormats; i++)
			{
				if (pixelFormatScores[i] > bestScore)
				{
					bestScore = pixelFormatScores[i];
					result = i;
				}
			}

			// Set fMultisampleBuffer
			mHasMultisampleBuffer = hasMultisample && wantMultisample && (pixelFormatScores[result] & (1 << 24));

			delete[] pixelFormatScores;
		}
	}

	return result;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Mouse and keyboard event handlers
//----------------------------------------------------------------------
//----------------------------------------------------------------------

void OpenGLWindow::mouseEvent(WORD inXPos, WORD inYPos, bool inCtrlDown, bool inShiftDown, bool inLeftDown, bool inMiddleDown, bool inRightDown)
{
	// We use two consecutive events to determine how much to adjust Euler angles
	const double kMaxMouseEventInterval = 0.25;
	double currentSeconds = Timer::seconds();
	double mouseEventInterval = currentSeconds - mLastMouseMoveSeconds;

	mLastMouseMoveSeconds = currentSeconds;
	mLastMousePosition.x = inXPos;
	mLastMousePosition.y = inYPos;
}

void OpenGLWindow::mouseWheelEvent(double inWheelDelta)
{
}

void OpenGLWindow::keyboardKeyDown(WPARAM inVirtualKeyCode)
{
	mKeys[inVirtualKeyCode] = true;
}

void OpenGLWindow::keyboardKeyUp(WPARAM inVirtualKeyCode)
{
	mKeys[inVirtualKeyCode] = false;
}

void OpenGLWindow::keyboardKeyPressed(WPARAM inVirtualKeyCode)
{
	switch (inVirtualKeyCode)
	{
	case 'Q':
		mRenderer.getCamera().negateSpeed();
		break;
	}
}

void OpenGLWindow::handleKeys()
{
	// Only process keys every 1/60 of a second. This eliminates response inconsistencies
	// due to varying frame rates.
	const double kKeyboardResponseInterval = 1.0/60.0;
	double timeSinceLastResponse = mFrameStartTime - mLastKeyboardResponseSeconds;
	if (timeSinceLastResponse > kKeyboardResponseInterval)
	{
		//
		// Process keys here
		//
		const float_t kRotationAmount = 0.002f;
		if (mKeys[VK_NUMPAD4] || mKeys[VK_LEFT])
			mRenderer.getCamera().rotateLeftRight(kRotationAmount);
		if (mKeys[VK_NUMPAD6] || mKeys[VK_RIGHT])
			mRenderer.getCamera().rotateLeftRight(-kRotationAmount);
		if (mKeys[VK_NUMPAD2] || mKeys[VK_DOWN])
			mRenderer.getCamera().rotateUpDown(-kRotationAmount);
		if (mKeys[VK_NUMPAD8] || mKeys[VK_UP])
			mRenderer.getCamera().rotateUpDown(kRotationAmount);
		if (mKeys[VK_NUMPAD7])
			mRenderer.getCamera().rollLeftRight(-kRotationAmount);
		if (mKeys[VK_NUMPAD9])
			mRenderer.getCamera().rollLeftRight(kRotationAmount);

		if (mKeys['A'])
			mRenderer.getCamera().changeSpeed(1);
		if (mKeys['Z'])
			mRenderer.getCamera().changeSpeed(-1);

		mLastKeyboardResponseSeconds = mFrameStartTime;
	}
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// OpenGL-related methods
//----------------------------------------------------------------------
//----------------------------------------------------------------------

void OpenGLWindow::resizeScene(Vec2i inNewSize)
{
	if (mIgnoreResizeEvents)
		return;

	if ((inNewSize.x == 0) || (inNewSize.y == 0))
		return;

	LOG(INFO) << "resizeScene(" << inNewSize.x << "," << inNewSize.y << ")";

	mRenderer.setSceneSize(inNewSize);

	if (!mGLInitialized)
		return;

	// Notify FontFactory of scene size change
	mRenderer.onResize(inNewSize);
}

void OpenGLWindow::drawScene()
{
	if (!mGLInitialized)
		return;

	// Remember current time
	mFrameStartTime = Timer::seconds();

	// Handle any keyboard input
	handleKeys();

	// Render the scene
	mRenderer.render();

	mFrameCount++;

	// Average the frame render time over 50 frames
	const double kNumberOfFramesToAverageOver = 50.0;
	double frameRenderTime = Timer::seconds() - mFrameStartTime;
	mAverageRenderedFrameRate -= 1.0 / kNumberOfFramesToAverageOver * mAverageRenderedFrameRate;
	mAverageRenderedFrameRate += 1.0 / kNumberOfFramesToAverageOver * frameRenderTime;

	// Report frames per second in window caption ever 60 frames, which with VSYNC enabled should give
	// an update frequency of 1 Hz.
	if (!mFullscreen && ((mFrameCount % 60) == 0))
	{
		double fps = 1.0 / mAverageRenderedFrameRate;
		wstringstream fpsStream;
		fpsStream << mWindowTitle << " FPS: " << fps;
		wstring fpsString = fpsStream.str();
		SetWindowText(mhWnd, fpsString.c_str());
	}
}

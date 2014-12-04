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

#include <random>
#include "OpenGLWindow.h"
#include "ShaderFactory.h"
#include "Fonts/FontFactory.h"

bool OpenGLWindow::sEnabledGLExtensions = false;

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

	memset(mKeys, 0, sizeof(mKeys));
	mLastMousePosition.x = 0;
	mLastMousePosition.y = 0;
}

OpenGLWindow::~OpenGLWindow()
{
	// Release the FontFactory
	FontFactory::destroy();

	// Release the ShaderFactory
	ShaderFactory::destroy();

	// Clean up the resources for this window
	destroy();
}

void OpenGLWindow::mouseEvent(WORD inXPos, WORD inYPos, bool inCtrlDown, bool inShiftDown, bool inLeftDown, bool inMiddleDown, bool inRightDown)
{
	// We use two consecutive events to determine how much to adjust Euler angles
	const double kMaxMouseEventInterval = 0.25;
	double currentSeconds = getCurrentSeconds();
	double mouseEventInterval = currentSeconds - mLastMouseMoveSeconds;

	mLastMouseMoveSeconds = currentSeconds;
	mLastMousePosition.x = inXPos;
	mLastMousePosition.y = inYPos;
}

void OpenGLWindow::mouseWheelEvent(double inWheelDelta)
{
}

void OpenGLWindow::keyboardKeyDown(WPARAM inKey)
{
	mKeys[inKey] = true;
}

void OpenGLWindow::keyboardKeyUp(WPARAM inKey)
{
	mKeys[inKey] = false;
}

void OpenGLWindow::handleKeys()
{
	// Only process keys every 1/100 of a second. This eliminates response inconsistencies
	// due to varying frame rates.
	const double kKeyboardResponseInterval = 1.0/100.0;
	double timeSinceLastResponse = mFrameStartTime - mLastKeyboardResponseSeconds;
	if (timeSinceLastResponse > kKeyboardResponseInterval)
	{
		//
		// Process keys here
		//

		mLastKeyboardResponseSeconds = mFrameStartTime;
	}
}

// ---------------------------------------------------------------------------
// TOpenGLDrawer::SelectBestPixelFormatUsingWGL					  [protected]
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	24/07/2007	CLW			6.0.7		
//	14/11/2007	CLW			6.2.1		Re-implemented for more reliable pixel format selection
//	
//	The modern method of choosing an appropriate pixel format.
// ---------------------------------------------------------------------------
GLuint OpenGLWindow::selectBestPixelFormatUsingWGL(HDC hDC)
{
	GLuint result = 0;

	int attributeList[] = {	WGL_PIXEL_TYPE_ARB,		// 0
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
							0,};
	
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
			memset(pixelFormatScores, 0, sizeof(UINT) * (maxPixelFormats + 1));

			int* attributeValues = new int[numAttributes];
			if (attributeValues)
			{
				// Enumerate all available pixel formats assigning score to each one
				for (int pfIndex = 1; pfIndex <= maxPixelFormats; pfIndex++)
				{
					if (wglGetPixelFormatAttribivARB(hDC, pfIndex, 0, numAttributes, attributeList, attributeValues))
					{
						// This stuff is mandatory. We skip all pixel formats that are missing any of these
						if (attributeValues[0] != WGL_TYPE_RGBA_ARB )			// Must have RGBA pixel data
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

				delete [] attributeValues;
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

			delete [] pixelFormatScores;
		}
	}

	return result;
}

bool OpenGLWindow::createWindow(HINSTANCE inInstance, WNDPROC inWndProc, WORD inMenuID,
								TCHAR* inTitle, int inWidth, int inHeight, int inBitsPerPixel, bool inFullscreenFlag)
{
	DWORD		dwExStyle;				// Window extended style
	DWORD		dwStyle;				// Window style
	RECT		windowRect;				// Grabs rectangle upper left / lower right values

	mhInstance = inInstance;
	mWindowTitle = inTitle;
	mFullscreen = inFullscreenFlag;	// Set the global fullscreen flag

	windowRect.left = 0;
	windowRect.right = (long)inWidth;
	windowRect.top = 0;
	windowRect.bottom = (long)inHeight;

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= inWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= mhInstance;
	wcex.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= ((inMenuID > 0) && !mFullscreen)? MAKEINTRESOURCE(inMenuID): NULL;
	wcex.lpszClassName	= L"OpenGL";
	wcex.hIconSm		= NULL;

	// Attempt to register the window class
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, L"Failed to register the window class.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	
	// Attempt fullscreen mode?
	if (mFullscreen)
	{
		// Make some assumptions about screen size in case we fail to determine the actual size
		windowRect.right = 1024;
		windowRect.bottom = 768;

		// Get the screen size that the console window is in. Need this for calibrating mouse motion
		HMONITOR hMonitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
		if (hMonitor)
		{
			MONITORINFO mi;
			mi.cbSize = sizeof(MONITORINFO);
			if (GetMonitorInfo(hMonitor, &mi))
			{
				windowRect.right = mi.rcMonitor.right - mi.rcMonitor.left;
				windowRect.bottom = mi.rcMonitor.bottom - mi.rcMonitor.top;
			}
		}

		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = windowRect.right;
		dmScreenSettings.dmPelsHeight = windowRect.bottom;
		dmScreenSettings.dmBitsPerPel = inBitsPerPixel;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try to set selected mode and get results.  NOTE: CDS_FULLSCREEN getsrRid of start bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, L"The requested fullscreen mode is not supported by\nyour video card. Use windowed mode instead?", L"OPENGL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				mFullscreen = FALSE;			// Windowed mode delected.
				mCmdShow = SW_SHOWMAXIMIZED;	// Maximize window
			}
			else
			{
				// Pop up a message box letting user know the program is closing.
				MessageBox(NULL, L"Program will now close.", L"ERROR", MB_OK | MB_ICONSTOP);
				return false;
			}
		}
	}

	// Are we still in fullscreen mode?
	if (mFullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(FALSE);	// Hide mouse pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;

		// Adjust window to true requested size
		AdjustWindowRectEx(&windowRect, dwStyle, (wcex.lpszMenuName != NULL)?TRUE:FALSE, dwExStyle);
	}

	mWindowSize.cx = windowRect.right - windowRect.left;
	mWindowSize.cy = windowRect.bottom - windowRect.top;

	mhWnd = CreateWindowEx(dwExStyle,					// Extended style for the window
						   L"OpenGL",					// Class name
						   inTitle,						// Window title
						   dwStyle |					// Defined tindow style
						   WS_CLIPSIBLINGS |			// Required window style
						   WS_CLIPCHILDREN,				// Required window style
						   0, 0,						// Window position
						   mWindowSize.cx,				// Window width
						   mWindowSize.cy,				// Window height
						   NULL,						// No parent window
						   NULL,						// No menu
						   mhInstance,					// Instance
						   NULL);						// Dont pass anything to WM_CREATE

	// Create the window
	if (mhWnd == NULL)
	{
		destroy();										// Reset The Display
		MessageBox(NULL, L"Window creation error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}

bool OpenGLWindow::setupOpenGLForWindow(GLuint inPixelFormat, PIXELFORMATDESCRIPTOR* inPFD)
{
	if (!(mhDC = GetDC(mhWnd)))							// Did we get a device context?
	{
		destroy();										// Reset the display
		MessageBox(NULL, L"Can't create a GL device context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if (!SetPixelFormat(mhDC, inPixelFormat, inPFD))	// Are we able to set the pixel format?
	{
		destroy();										// Reset the display
		MessageBox(NULL, L"Can't set the pixelformat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if (!(mhRC = wglCreateContext(mhDC)))				// Are we able to get a rendering context?
	{
		destroy();										// Reset the display
		MessageBox(NULL, L"Can't create a GL rendering context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if (!wglMakeCurrent(mhDC, mhRC))					// Try to activate the rendering context
	{
		destroy();										// Reset the display
		MessageBox(NULL, L"Can't activate the GL rendering context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}

bool OpenGLWindow::create(HINSTANCE inInstance, WNDPROC inWndProc, WORD inMenuID,
						  TCHAR* inTitle, int inWidth, int inHeight, int inBitsPerPixel, bool inFullscreenFlag)
{
	PIXELFORMATDESCRIPTOR pfd =							// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
		1,												// Version Number
		PFD_DRAW_TO_WINDOW |							// Format Must Support Window
		PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,								// Must Support Double Buffering
		PFD_TYPE_RGBA,									// Request An RGBA Format
		inBitsPerPixel,									// Select Our Color Depth
		0, 0, 0, 0, 0, 0,								// Color Bits Ignored
		0,												// No Alpha Buffer
		0,												// Shift Bit Ignored
		0,												// No Accumulation Buffer
		0, 0, 0, 0,										// Accumulation Bits Ignored
		16,												// 16Bit Z-Buffer (Depth Buffer)  
		0,												// No Stencil Buffer
		0,												// No Auxiliary Buffer
		PFD_MAIN_PLANE,									// Main Drawing Layer
		0,												// Reserved
		0, 0, 0											// Layer Masks Ignored
	};

	if (createWindow(inInstance, inWndProc, inMenuID, inTitle, inWidth, inHeight, inBitsPerPixel, false))
	{
		if (!(mhDC = GetDC(mhWnd)))							// Did we get a device context?
		{
			destroy();										// Reset the display
			MessageBox(NULL, L"Can't create a GL device context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		GLuint basicPixelFormat;
		if (!(basicPixelFormat = ChoosePixelFormat(mhDC, &pfd)))	// Did Windows find a matching pixel format?
		{
			destroy();										// Reset the display
			MessageBox(NULL, L"Can't find a suitable pixelformat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		if (setupOpenGLForWindow(basicPixelFormat, &pfd))
		{
			if (!sEnabledGLExtensions)							// We only need to do this once
			{
				GLenum err = glewInit();
				if (err == GLEW_OK)
					sEnabledGLExtensions = true;
				else
				{
					/* Problem: glewInit failed, something is seriously wrong. */
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
				if (createWindow(inInstance, inWndProc, inMenuID, inTitle, inWidth, inHeight, inBitsPerPixel, inFullscreenFlag))
				{
					if (!setupOpenGLForWindow(uberPixelFormat, &pfd))
					{
						// Well, this is lame! We can't use the uber pixel format
						// Destroy and re-create our window using basic pixel format
						destroy();
						if (createWindow(inInstance, inWndProc, inMenuID, inTitle, inWidth, inHeight, inBitsPerPixel, inFullscreenFlag))
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

			// Enable VSYNC
			if (wglSwapIntervalEXT)
				wglSwapIntervalEXT(1);

			ShowWindow(mhWnd, mCmdShow);						// Make the window visible
			SetForegroundWindow(mhWnd);							// Slightly higher priority
			SetFocus(mhWnd);									// Sets keyboard focus to the window
			resizeGLScene(mWindowSize.cx, mWindowSize.cy);

			initGL();											// Initialize our newly created GL window
			mFrameCount = 0;									// Reset frame count

			mCreated = true;
		}
	}

	return true;										// Success
}

void OpenGLWindow::initGL()								// All setup for OpenGL goes here
{
	glShadeModel(GL_SMOOTH);							// Enable smooth shading
	glClearColor(mClearColor.x, mClearColor.y, mClearColor.z, 1.0f);
	glClearDepth(1.0f);									// Depth buffer setup
	glEnable(GL_DEPTH_TEST);							// Enables depth testing
	glDepthFunc(GL_LEQUAL);								// The type of depth testing to do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really nice perspective calculations
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (mHasMultisampleBuffer)
	{
		glEnable(GL_MULTISAMPLE_ARB);
		glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	}

	mGLInitialized = true;
}

void OpenGLWindow::resizeGLScene(GLsizei inWidth, GLsizei inHeight)		// Resize and initialize the GL window
{
	if (inHeight == 0)									// Prevent a divide by zero by
		inHeight = 1;									// making height equal one

	mWindowSize.cx = inWidth;							// Remember width
	mWindowSize.cy = inHeight;							// Remember height

	glViewport(0, 0, mWindowSize.cx, mWindowSize.cy);	// Reset the current viewport

	glMatrixMode(GL_PROJECTION);						// Select the projection matrix
	glLoadIdentity();									// Reset the projection matrix

	// Calculate the aspect ratio of the window
	GLfloat aspectRatio = (GLfloat)mWindowSize.cx / (GLfloat)mWindowSize.cy;
	gluPerspective(45.0f, aspectRatio, 0.1f, 200.0f);

	glMatrixMode(GL_MODELVIEW);							// Select the modelview matrix
	glLoadIdentity();									// Reset the modelview matrix
}

void OpenGLWindow::destroy()							// Properly kill the window
{
	if (mFullscreen)									// Are we in fullscreen mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If so switch back to the desktop
		ShowCursor(TRUE);								// Show mouse pointer
	}

	if (mhRC)											// Do we have a rendering context?
	{
		if (!wglMakeCurrent(NULL, NULL))				// Are we able to release the DC And RC contexts?
			MessageBox(NULL, L"Release of DC and RC failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		if (!wglDeleteContext(mhRC))					// Are we able to delete the RC?
			MessageBox(NULL, L"Release rendering context failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		mhRC = NULL;									// Set RC to NULL
	}

	if (mhDC && !ReleaseDC(mhWnd, mhDC))				// Are we able to Release the DC?
	{
		MessageBox(NULL, L"Release device context failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		mhDC = NULL;									// Set DC to NULL
	}

	if (mhWnd && !DestroyWindow(mhWnd))					// Are we able to destroy the window?
	{
		MessageBox(NULL, L"Could not release window handle.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		mhWnd = NULL;									// Set ghWnd to NULL
	}

	if (!UnregisterClass(L"OpenGL", mhInstance))			// Are we able to unregister class
	{
		MessageBox(NULL, L"Could not unregister class.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		mhInstance = NULL;								// Set ghInstance To NULL
	}

	mCreated = false;
}

void OpenGLWindow::render()								// Here's where we do all the drawing
{
	if (!mGLInitialized)
		return;

	// Remember current time
	mFrameStartTime = getCurrentSeconds();

	// Handle any keyboard input
	handleKeys();

	// Clear screen and modelview matrix
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear screen and depth buffer

	// FontFactory testing
	string fontName("Verdana");
	wstring text(L"\260 A quick brown fox jumped over the lazy dog. !@#$%^&*()-=+{}[];:'<>,.?/`~");
	FontFactory* ff = FontFactory::inst();
	FontRenderer* fontRenderer = ff->getFontRenderer(fontName);

	// The code below renders the text string above 1000 times with random positions, sizes
	// and angles. On my 2010 Macbook Pro, framerate was 26.6 fps. Pretty damn good
	// under normal circumstances far fewer than 1000 text items will be rendered per frame
	// and each item will certainly be much smaller than the 72-char string we're using.
/*
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> xDis(0, mWindowSize.cx);
	std::uniform_int_distribution<> yDis(0, mWindowSize.cy);
	std::uniform_int_distribution<> sizeDis(10, 36);
	std::uniform_int_distribution<> angleDis(0, 359);
	TVector2f position;
	int fontSize;
	float angle;
	for (int n = 0; n < 50; ++n)
	{
	position.x = (GLfloat)xDis(gen);
	position.y = (GLfloat)yDis(gen);
	fontSize = sizeDis(gen);
	angle = (float)angleDis(gen);
	fontRenderer->render(text, fontSize, position, TVector4f(1.0f, 1.0f, 1.0f, 1.0f), angle);
	}
*/
	fontRenderer->render(text, 30, Vec2f(100, 100), Vec4f(1.0f, 1.0f, 1.0f, 1.0f), 30);

	SwapBuffers(mhDC);									// Swap buffers (double buffering)
	mFrameCount++;

	// Average the frame render time over 50 frames
	const double kNumberOfFramesToAverageOver = 50.0;
	double frameRenderTime = getCurrentSeconds() - mFrameStartTime;
	mAverageRenderedFrameRate -= 1.0 / kNumberOfFramesToAverageOver * mAverageRenderedFrameRate;
	mAverageRenderedFrameRate += 1.0 / kNumberOfFramesToAverageOver * frameRenderTime;

	// Report frames per second in window caption ever 60 frames, which with VSYNC enabled should give
	// an update frequency on 1 Hz.
	if (!mFullscreen && ((mFrameCount % 60) == 0))
	{
		double fps = 1.0 / mAverageRenderedFrameRate;
		wstringstream fpsStream;
		fpsStream << mWindowTitle << " FPS: " << fps;
		wstring fpsString = fpsStream.str();
		SetWindowText(mhWnd, fpsString.c_str());
	}
}

void OpenGLWindow::setClearColor(const GLfloat inRed, const GLfloat inGreen, const GLfloat inBlue)
{
	mClearColor = Vec3f(inRed, inGreen, inBlue);
	glClearColor(mClearColor.x, mClearColor.y, mClearColor.z, 1.0f);
}

double OpenGLWindow::getCurrentSeconds() const
{
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);
	double rawTime = (double)tick.QuadPart;
	double seconds = rawTime/mTicksPerSecond.QuadPart;
	
	return seconds;
}

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// Armand.cpp
//
// Defines the entry point for the application.
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
#include <algorithm>
#include <random>
#include "Main/Armand.h"
#include "OpenGL/OpenGLWindow.h"

#define MAX_LOADSTRING 100

// Global Variables:
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
OpenGLWindow* gOpenGLWindow;
Renderer* gRenderer;
bool gActive = true;		// Window active flag set to true by default

_INITIALIZE_EASYLOGGINGPP

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(	_In_ HINSTANCE hInstance,
						_In_opt_ HINSTANCE hPrevInstance,
						_In_ LPTSTR lpCmdLine,
						_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Determine the application folder
	File::initAppFolder();

	// Create "Logs" folder if necessary.
	if (!File::folderExists("Logs"))
		File::createFolder("Logs");

	// Load logging configuration from file
	el::Configurations conf("Resources/logging.cfg");
	el::Loggers::reconfigureLogger("default", conf);
	el::Loggers::reconfigureAllLoggers(conf);
	LOG(INFO) << "Armand is starting...";

	// Get the high resolution counter's accuracy
	QueryPerformanceFrequency(&Timer::sTicksPerSecond);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ARMAND, szWindowClass, MAX_LOADSTRING);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ARMAND));

	// Create an instance of OpenGLWindow
	bool fullscreen = false;	// Fullscreen flag set to normal mode by default
	Vec2i windowSize(640, 480);
	gOpenGLWindow = new OpenGLWindow();
	if (gOpenGLWindow)
	{
		gOpenGLWindow->create(hInstance, WndProc, IDI_ARMAND, szTitle, windowSize.x, windowSize.y, 24, fullscreen);
		if (!gOpenGLWindow->getIsCreated())
			return 0;
	}

	// Main message loop
	bool done = false;
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is there a message waiting?
		{
			if (msg.message == WM_QUIT)				// Have we received a quit message?
				done = true;						// If so, done=TRUE
			else									// If not, deal with window messages
			{
				TranslateMessage(&msg);				// Translate the message
				DispatchMessage(&msg);				// Dispatch the message
			}
		}
		else										// If there are no messages
		{
			// Draw the scene.  Watch for ESC key and quit messages from DrawGLScene()
			if (gActive)
			{
				if (gOpenGLWindow->getKeys()[VK_ESCAPE])	// Was there a quit received?
					done = true;							// ESC or DrawGLScene signalled a quit
				else
					gOpenGLWindow->drawScene();
			}

			if (gOpenGLWindow->getKeys()[VK_F7])	// Is F1 being pressed?
			{
				gOpenGLWindow->getKeys()[VK_F7] = false;	// If so, make key FALSE

				fullscreen = !fullscreen;			// Toggle fullscreen / windowed mode
				gOpenGLWindow->setFullScreen(fullscreen);
			}
		}
	}

	// Shutdown
	if (gOpenGLWindow)
		delete gOpenGLWindow;						// Kill the window

	return (int) msg.wParam;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_MOUSEWHEEL:
		{
			gOpenGLWindow->mouseWheelEvent((double)GET_WHEEL_DELTA_WPARAM(wParam) / (double)WHEEL_DELTA);
			return 0;
		}

		case WM_ACTIVATE:							// Watch for window activate message
		{
			gActive = (HIWORD(wParam) == 0);		// Check minimization state
/*
			// Set mouse cursor to middle of screen if re-activating
			if (gActive)
			{
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			POINT screenPoint;
			screenPoint.x = (clientRect.right - clientRect.left) / 2;
			screenPoint.y = (clientRect.bottom - clientRect.top) / 2;
			ClientToScreen(hWnd, &screenPoint);
			SetCursorPos(screenPoint.x, screenPoint.y);
			}
*/
			return 0;
		}

		case WM_SYSCOMMAND:						// Intercept system commands
		{
			switch (wParam)						// Check system calls
			{
			case SC_SCREENSAVE:					// Screensaver trying to start?
			case SC_MONITORPOWER:				// Monitor trying to enter powersave?
				return 0;						// Prevent from happening
			}
			break;
		}

		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
			switch (wmId)
			{
			case IDM_ABOUT:
				DialogBox(gOpenGLWindow->getAppInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;;
			case IDM_EXIT:
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}

		case WM_KEYDOWN:							// Is a key being held down?
		{
			gOpenGLWindow->keyboardKeyPressed(wParam);
			gOpenGLWindow->keyboardKeyDown(wParam);	// If so, report it
			return 0;
		}

		case WM_KEYUP:								// Has a key been released?
		{
			gOpenGLWindow->keyboardKeyUp(wParam);	// If so, report it
			return 0;
		}

		case WM_CLOSE:								// Did we receive a close message?
		{
			PostQuitMessage(0);						// Send a quit message
			return 0;
		}

		case WM_SIZE:
			gOpenGLWindow->resizeScene(Vec2i(LOWORD(lParam), HIWORD(lParam)));
			gOpenGLWindow->drawScene();
			return 0;

		case WM_MOUSEMOVE:
		{
			WORD xPos = LOWORD(lParam);
			WORD yPos = HIWORD(lParam);
			bool ctrlDown = ((wParam & MK_CONTROL) != 0);
			bool shiftDown = ((wParam & MK_SHIFT) != 0);
			bool leftDown = ((wParam & MK_LBUTTON) != 0);
			bool middleDown = ((wParam & MK_MBUTTON) != 0);
			bool rightDown = ((wParam & MK_RBUTTON) != 0);
			gOpenGLWindow->mouseEvent(xPos, yPos, ctrlDown, shiftDown, leftDown, middleDown, rightDown);
			return 0;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// Convenient STL declaration
#include <string>
#include <iostream>
#include <sstream>
#include <map>

// TODO: reference additional headers your program requires here

// Logging
#define _ELPP_STL_LOGGING
#define _ELPP_THREAD_SAFE
#include "Utilities/easylogging++.h"

using namespace std;		// Use the STL namespace; std

#include <GL/glew.h>
#include <GL/wglew.h>

//#include <gl/gl.h>			// Header file for the OpenGL32 library
//#include <gl/glu.h>			// Header file for the GLu32 library

#include "Math/VectorTemplates.h"


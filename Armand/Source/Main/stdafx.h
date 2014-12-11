// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// Convenient STL declaration
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <exception>

// TODO: reference additional headers your program requires here

// Logging
#define _ELPP_STL_LOGGING
#define _ELPP_THREAD_SAFE
#define _ELPP_DEFAULT_LOG_FILE "logs/Armand.log"
#define _ALWAYS_CLEAN_LOGS
#include "Utilities/easylogging++.h"

using namespace std;		// Use the STL namespace; std

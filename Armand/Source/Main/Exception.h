// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// Exception.h
//
// Makes defining and throwing std exception really convenient.
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <exception>
#include <sstream>

#define MAX_EXCEPTION_MESSAGE 1024

//
// Call this macro in header files to define new std exceptions.
//
#define GENERATE_EXCEPTION(type) \
class type : public std::exception \
{ \
public: \
	type() : exception() { memset(mMsg, 0, MAX_EXCEPTION_MESSAGE); } \
	type(const char* fmt, ...) { \
		va_list args; \
		va_start(args, fmt); \
		vsprintf_s(mMsg, MAX_EXCEPTION_MESSAGE, fmt, args); \
		va_end(args); \
		std::ostringstream s; \
		s << #type << ": " << mMsg; \
		strcpy_s(mMsg, MAX_EXCEPTION_MESSAGE, s.str().c_str()); \
	} \
	virtual const char* what() const throw() { \
		return mMsg; \
	} \
private: \
	char mMsg[MAX_EXCEPTION_MESSAGE]; \
};

//
// Call this macro to throw an exception, causing what() to return the exception message
// and function name and line number. Nice!
//
#define THROW(ExceptionClass, Msg, ...) throw ExceptionClass("%s(), line %d: " #Msg, __FUNCTION__, __LINE__, __VA_ARGS__)
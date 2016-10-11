// ----------------------------------------------------------------------------
// Copyright (C) 2016 Clint Weisbrod. All rights reserved.
//
// DebugStream.h
//
// A convenient singleton interface supporting inserter operator for writing to
// debug console.
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

#include <string>
#include <sstream>
#include <Windows.h>
#include "Singleton.h"

using namespace std;

class DebugStream : public wostringstream, public Singleton<DebugStream>
{
	friend class Singleton<DebugStream>;

	public:
		DebugStream& operator<<(const char rhs);
		DebugStream& operator<<(const wchar_t rhs);
		DebugStream& operator<<(const char* rhs);
		DebugStream& operator<<(const wchar_t* rhs);
		DebugStream& operator<<(string rhs);
		DebugStream& operator<<(wstring rhs);
		DebugStream& operator<<(std::ostream& (*manip)(std::ostream&));	// For handling output manipulators like endl
		DebugStream& operator<<(const int rhs);
		DebugStream& operator<<(const float_t rhs);
		DebugStream& operator<<(const double_t rhs);
};

// ----------------------------------------------------------------------------
// Copyright (C) 2016 Clint Weisbrod. All rights reserved.
//
// DebugStream.cpp
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

#include "stdafx.h"

DebugStream& DebugStream::operator<<(const char rhs)
{
	char buf[2];
	buf[0] = rhs;
	buf[1] = 0;
	OutputDebugStringA(buf);
	return *this;
}

DebugStream& DebugStream::operator<<(const wchar_t rhs)
{
	wchar_t buf[2];
	buf[0] = rhs;
	buf[1] = 0;
	OutputDebugStringW(buf);
	return *this;
}

DebugStream& DebugStream::operator<<(const char* rhs)
{
	OutputDebugStringA(rhs);
	return *this;
}

DebugStream& DebugStream::operator<<(const wchar_t* rhs)
{
	OutputDebugStringW(rhs);
	return *this;
}

DebugStream& DebugStream::operator<<(string rhs)
{
	OutputDebugStringA(rhs.c_str());
	return *this;
}

DebugStream& DebugStream::operator<<(wstring rhs)
{
	OutputDebugStringW(rhs.c_str());
	return *this;
}

DebugStream& DebugStream::operator<<(std::ostream& (*manip)(std::ostream&))
{
	ostringstream os;
	manip(os);
	OutputDebugStringA(os.str().c_str());
	return *this;
}

DebugStream& DebugStream::operator<<(const int rhs)
{
	char buf[64];
	OutputDebugStringA(_itoa(rhs, buf, 10));
	return *this;
}

DebugStream& DebugStream::operator<<(const float_t rhs)
{
	char buf[64];
	sprintf(buf, "%f", rhs);
	OutputDebugStringA(buf);
	return *this;
}

DebugStream& DebugStream::operator<<(const double_t rhs)
{
	char buf[64];
	sprintf(buf, "%lf", rhs);
	OutputDebugStringA(buf);
	return *this;
}


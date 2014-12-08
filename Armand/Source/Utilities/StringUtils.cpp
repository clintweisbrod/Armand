// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// StringUtils.cpp
//
// Some additonal string processing that std::basic_string doesn't have.
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

#include "StringUtils.h"

bool intFromString(string& inString, int& outInt)
{
	if (inString.empty())
		return false;

	// Why not just use atoi()? Because atoi() doesn't give any indication of failure.
	// So, lets just make sure we've got sane character to convert.
	const char* validChars = "0123456789-";
	string::size_type pos = inString.find_first_not_of(validChars);
	if (pos == string::npos)
	{
		outInt = atoi(inString.c_str());
		return true;
	}
	else
	{
		LOG(ERROR) << "intFromString() encountered unexpected input.";
		return false;
	}
}

bool doubleFromString(string& inString, double_t& outDouble)
{
	if (inString.empty())
		return false;

	// Why not just use atof()? Because atof() doesn't give any indication of failure.
	// So, lets just make sure we've got sane character to convert.
	const char* validChars = "0123456789-.eE";
	string::size_type pos = inString.find_first_not_of(validChars);
	if (pos == string::npos)
	{
		outDouble = atof(inString.c_str());
		return true;
	}
	else
	{
		LOG(ERROR) << "doubleFromString() encountered unexpected input.";
		return false;
	}
}
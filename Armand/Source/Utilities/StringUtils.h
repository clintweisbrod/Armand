// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// StringUtils.h
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

#pragma once

#include <algorithm>
#include <string>

using namespace std;

bool intFromString(string& inString, int& outInt);
bool doubleFromString(string& inString, double_t& outDouble);

template<class T>
void trimString(basic_string<T>& ioString)
{
	const T* spaceChrs = " \t\n\v\f\r";
	basic_string<T>::size_type startPos = ioString.find_first_not_of(spaceChrs);
	basic_string<T>::size_type endPos = ioString.find_last_not_of(spaceChrs);
	ioString = ioString.substr(startPos, endPos - startPos + 1);
}

template<class T>
void toLower(basic_string<T>& ioString)
{
	transform(ioString.begin(), ioString.end(), ioString.begin(), ::tolower);
}

template<class T>
void toUpper(basic_string<T>& ioString)
{
	transform(ioString.begin(), ioString.end(), ioString.begin(), ::toupper);
}

inline wstring wstringFromString(string& inString)
{
	return wstring(inString.begin(), inString.end());
}

inline string stringFromWstring(wstring& inString)
{
	return string(inString.begin(), inString.end());
}

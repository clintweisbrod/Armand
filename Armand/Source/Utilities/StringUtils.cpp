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
#include "Math/constants.h"

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

bool floatFromString(string& inString, float_t& outFloat)
{
	if (inString.empty())
		return false;

	double_t value;
	bool result = doubleFromString(inString, value);
	if (result)
		outFloat = (float_t)value;

	return result;
}

//
// Given a distance in AU, returns a wstring (suitable for FontRenderer::render*()) that formats
// the number with a reasonable unit of length.
//
wstring getNiceDistanceString(float_t inSpeedInAUPerSecond, int inNumDecimalPlaces)
{
	wchar_t formatStr[16];
	swprintf(formatStr, 16, L"%%.%df %%s", inNumDecimalPlaces);

	float_t distanceInMetres = inSpeedInAUPerSecond * (float_t)kMetresPerAu;

	float_t factor = 1;
	wstring unit;

	if (distanceInMetres > (float_t)kMetresPerLightYear * 1e6f)
	{
		factor = 1 / ((float_t)kMetresPerLightYear * 1e6f);
		unit = L"MLY";
	}
	else if (distanceInMetres > (float_t)kMetresPerLightYear * 0.01f)
	{
		factor = 1 / (float_t)kMetresPerLightYear;
		unit = L"LY";
	}
	else if (distanceInMetres > (float_t)kMetresPerAu)
	{
		factor = 1 / (float_t)kMetresPerAu;
		unit = L"AU";
	}
	else if (distanceInMetres > (float_t)kMetresPerKilometre)
	{
		factor = 1 / (float_t)kMetresPerKilometre;
		unit = L"km";
	}
	else if (distanceInMetres > 1)
	{
		factor = 1;
		unit = L"m";
	}
	else
	{
		factor = 100;
		unit = L"cm";
	}

	float_t dislayedDistance = distanceInMetres * factor;
	wchar_t buf[32];
	swprintf(buf, 32, formatStr, dislayedDistance, unit.c_str());

	return wstring(buf);
}

//
// Given a speed in AU/s, returns a wstring (suitable for FontRenderer::render*()) that formats
// the number with a reasonable unit of speed.
//
wstring getNiceSpeedString(float_t inSpeedInAUPerSecond, int inNumDecimalPlaces)
{
	// Besides the distance units used in getNiceDistanceString(), also incorporate "c", the
	// speed of light into the speed units.
	wstring result;
	float_t speedFractionOfLight = inSpeedInAUPerSecond * (float_t)(kMetresPerAu / kSpeedOfLightMetresPerSecond);
	float_t kUpperLimit = (float_t)(kMetresPerAu / kSpeedOfLightMetresPerSecond);	// 1 AU/s
	if ((speedFractionOfLight > 0.01f) && (speedFractionOfLight < kUpperLimit))
	{
		wchar_t formatStr[16];
		swprintf(formatStr, 16, L"%%.%df c", inNumDecimalPlaces);
		wchar_t buf[32];
		swprintf(buf, 32, formatStr, speedFractionOfLight);
		result = buf;
	}
	else
	{
		result = getNiceDistanceString(inSpeedInAUPerSecond, inNumDecimalPlaces);
		result = result.append(L"/s");
	}

	return result;
}
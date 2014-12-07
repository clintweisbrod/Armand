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
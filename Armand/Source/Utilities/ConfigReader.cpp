// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// ConfigReader.cpp
//
// Encapsulates reading config files.
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

#include "ConfigReader.h"
#include "StringUtils.h"

ConfigFileReader::ConfigFileReader(string& inRelativeFilePath)
{
	if (!inRelativeFilePath.empty())
	{
		mConfigFile = new File(inRelativeFilePath);
		if (mConfigFile)
		{
			// Read config value and parse values into mConfigValues map.
			char* rawConfigText = mConfigFile->readAsText();
			if (rawConfigText)
			{
				// rawConfigText contains a series of strings delimited by '\r' and 'n' characters
				int lineNum = 0;
				const char* lineDelims = "\r\n";
				char* lineContext = NULL;
				char* line;
				line = strtok_s(rawConfigText, lineDelims, &lineContext);
				while (line != NULL)
				{
					// Keep track of the line number we're processing in case we need to log
					// unexpected formatting errors.
					lineNum++;

					// Advance past any leading whitespace
					while (isspace(*line))
						line++;

					// Any lines beginning with '#' or '//' are considered comments and ignored
					if ((line[0] == '#') || ((line[0] == '/') && (line[1] == '/')))
					{
						// Next line
						line = strtok_s(NULL, lineDelims, &lineContext);
						continue;
					}

					// All configuration lines have valueName=value. Split line at '=' character.
					const char* valueDelims = "=";
					char* valueContext = NULL;
					char* valueName = strtok_s(line, valueDelims, &valueContext);
					char* value = strtok_s(NULL, valueDelims, &valueContext);

					// It's possible that valueName and/or value could be NULL if the line actually
					// doesn't contain an '=' character. This is an error condition because the meta
					// file contains unexpected formatting.
					if (!valueName || !value)
					{
						LOG(ERROR) << "Unexpected formatting at line " << lineNum << " in: " << mConfigFile->getFullPath();
						
						// Next line
						line = strtok_s(NULL, lineDelims, &lineContext);
						continue;
					}

					// We allow for comments at the end of a line so we gotta trim those if they exist
					string valueStr = value;
					string::size_type pos = valueStr.find_first_of('#');
					if (pos != string::npos)
						valueStr = valueStr.substr(0, pos);
					pos = valueStr.find_first_of("//");
					if (pos != string::npos)
						valueStr = valueStr.substr(0, pos);

					// Trim the valueName and value of leading and trailing whitespace
					string valueNameStr = valueName;
					trimString(valueNameStr);
					trimString(valueStr);

					// Add the values to the map
					if (!valueNameStr.empty() && !valueStr.empty())
						mConfigValues[valueNameStr] = valueStr;

					// Next line
					line = strtok_s(NULL, lineDelims, &lineContext);
				}

				delete rawConfigText;
			}
		}
	}
}

ConfigFileReader::~ConfigFileReader()
{
	if (mConfigFile)
	{
		delete mConfigFile;
		mConfigFile = NULL;
	}
}

bool ConfigFileReader::getConfigValue(const char* inValueName, string& ioValue)
{
	ConfigValueMap_t::iterator it = mConfigValues.find(inValueName);
	if (it != mConfigValues.end())
	{
		ioValue = it->second;
		return true;
	}
	else
		return false;
}

bool ConfigFileReader::getConfigValue(const char* inValueName, int& ioValue)
{
	bool result = false;

	ConfigValueMap_t::iterator it = mConfigValues.find(inValueName);
	if (it != mConfigValues.end())
		result = intFromString(it->second, ioValue);

	return result;
}

bool ConfigFileReader::getConfigValue(const char* inValueName, float_t& ioValue)
{
	bool result = false;

	ConfigValueMap_t::iterator it = mConfigValues.find(inValueName);
	if (it != mConfigValues.end())
		result = floatFromString(it->second, ioValue);

	return result;
}

bool ConfigFileReader::getConfigValue(const char* inValueName, Vec3f& ioValue)
{
	bool result = false;

	ConfigValueMap_t::iterator it = mConfigValues.find(inValueName);
	if (it != mConfigValues.end())
	{
		if (!it->second.empty())
		{
			try
			{
				ioValue = Vec3f(it->second);
				result = true;
			}
			catch (bad_alloc)
			{
				LOG(ERROR) << "Exception thrown calling Vec3 string constructor.";
				result = false;
			}
		}
	}

	return result;
}

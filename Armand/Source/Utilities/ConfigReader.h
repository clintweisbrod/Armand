// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// ConfigReader.h
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

#pragma once

#include <string>

#include "File.h"
#include "Math/vecmath.h"

using namespace std;

typedef map<string, string> ConfigValueMap_t;
class ConfigFileReader
{
public:
	ConfigFileReader() {}
	ConfigFileReader(string& inRelativeFilePath);
	virtual ~ConfigFileReader();

	bool hasValues() const { return !mConfigValues.empty(); }

	bool getConfigValue(const char* inValueName, string& ioValue);
	bool getConfigValue(const char* inValueName, int& ioValue);
	bool getConfigValue(const char* inValueName, float_t& ioValue);
	bool getConfigValue(const char* inValueName, Vec3f& ioValue);

private:
	File*				mConfigFile;
	ConfigValueMap_t	mConfigValues;
};

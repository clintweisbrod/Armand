// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// File.cpp
//
// Encapsulates the use of files.
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

#include "Shlwapi.h"
#include "File.h"
#include "StringUtils.h"

extern string gAppFolder;

bool File::folderExists(const char* inRelativePath)	// static
{
	string relativePath(inRelativePath);
	File folder(relativePath);
	wstring fullPath = wstringFromString(folder.getFullPath());
	BOOL exists = PathFileExists(fullPath.c_str());

	return (exists == TRUE);
}

bool File::createFolder(const char* inRelativePath)	// static
{
	string relativePath(inRelativePath);
	File folder(relativePath);
	wstring fullPath = wstringFromString(folder.getFullPath());
	BOOL created = CreateDirectory(fullPath.c_str(), NULL);

	return (created == TRUE);
}

File::File(string& inRelativeFilePath)
{
	mRelativePath = inRelativeFilePath;

	// Parse just the filename
	size_t slashPos = mRelativePath.rfind('/');
	if (slashPos != string::npos)
		mFileName = mRelativePath.substr(slashPos + 1);

	// Parse filename without extension
	if (!mFileName.empty())
	{
		size_t dotPos = mFileName.rfind('.');
		if (dotPos != string::npos)
			mFileNameNoExtension = mFileName.substr(0, dotPos);
	}

	// Construct full path to this file
	mFullPath = gAppFolder;
	mFullPath.append(1, '/');
	mFullPath.append(mRelativePath);
}

File::~File()
{
}

bool File::exists() const
{
	// Check if file exists
	bool result = false;

	FILE* fp = fopen(mRelativePath.c_str(), "rb");
	result = (fp != NULL);
	if (fp)
		fclose(fp);

	return result;
}

FILE* File::getCRTFileHandle(const char* inReadMode) const
{
	return fopen(mRelativePath.c_str(), inReadMode);
}

char* File::readAsText() const
{
	FILE* fp = fopen(mRelativePath.c_str(), "rb");
	if (!fp)
	{
		LOG(ERROR) << "Unable to open file: " << mFullPath;
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buffer = new char[size + 1];
	if (buffer)
	{
		fread(buffer, 1, size, fp);
		buffer[size] = 0;
		fclose(fp);
	}

	return buffer;
}

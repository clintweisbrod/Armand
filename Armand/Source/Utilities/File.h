// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// File.h
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

#pragma once

#include <string>

using namespace std;

class File
{
public:
	File() {}
	File(string& inRelativeFilePath);
	virtual ~File();

	char*	readAsText() const;
	FILE*	getCRTFileHandle(const char* inReadMode) const;
	bool	exists() const;
	string	getFileName() const { return mFileName;	}
	string	getRelativePath() const { return mRelativePath; }
	string	getFullPath() const { return mFullPath; }
	string	getFileNameWithoutExtension() const { return mFileNameNoExtension; }

private:
	string	mRelativePath;
	string	mFullPath;
	string	mFileName;
	string	mFileNameNoExtension;
};

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// ShaderFactory.cpp
//
// Encapsulates the use of GLSL shaders.
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

#include "ShaderFactory.h"
#include "Utilities/File.h"

ShaderProgram::ShaderProgram()
{
	mHandle = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
	if (mHandle)
	{
		glDeleteProgram(mHandle);
		mHandle = 0;
	}
}

ShaderFactory::~ShaderFactory()
{
	// Iterate through the map and delete each TGLSLProgram instance
	ShaderProgramMap_t::iterator it;
	for (it = mPrograms.begin(); it != mPrograms.end(); it++)
	{
		// Destroy the GLSL program instance
		if (it->second)
			delete it->second;
	}

	// Clear the map
	mPrograms.clear();
}

ShaderProgram* ShaderFactory::getShaderProgram(const char* inVertexShader, const char* inFragmentShader)
{
	ShaderProgram* result = NULL;

	// Generate key to lookup shader 
	string key = genKey(inVertexShader, inFragmentShader);

	// Have we attempted to load this shader already?
	BoolMap_t::iterator loadAttempt = mProgramLoadAttempts.find(key);
	if (loadAttempt != mProgramLoadAttempts.end())
	{
		// If we get here, load attempt has already been made.
		if (loadAttempt->second == false)
		{
			// If the load attempt failed the first time, we bail.
			return NULL;
		}
	}

	// Attempt to locate the shader program
	ShaderProgramMap_t::iterator it = mPrograms.find(key);

	// If shader program is not found, attempt to create it. 
	if (it == mPrograms.end())
	{
		ShaderProgram* newProgram = new ShaderProgram;
		if (newProgram && newProgram->getHandle())
		{
			ShaderHandleVec_t shaderHandles;

			// Load vertex shader and potentially any it includes
			if (!loadShaderObject(inVertexShader, GL_VERTEX_SHADER, shaderHandles))
			{
				mProgramLoadAttempts[key] = false;
				return NULL;
			}

			// Load fragment shader and potentially any it includes
			if (!loadShaderObject(inFragmentShader, GL_FRAGMENT_SHADER, shaderHandles))
			{
				mProgramLoadAttempts[key] = false;
				return NULL;
			}

			// Attach shaders to program and delete them. OpenGL flags them for deletion because
			// they are attached to a program. When the program is deleted, GL deletes the shaders.
			GLuint programHandle = newProgram->getHandle();
			for (ShaderHandleVec_t::iterator it = shaderHandles.begin(); it != shaderHandles.end(); it++)
			{
				glAttachShader(programHandle, *it);
				glDeleteShader(*it);
			}

			// Link the shaders
			glLinkProgram(programHandle);
			GLint linkStatus = GL_FALSE;
			glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
			if (linkStatus == GL_FALSE)
			{
				GLchar messages[256];
				glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
				LOG(ERROR) << "Shader link failure: " << key << ": " << messages;
				mProgramLoadAttempts[key] = false;
				delete newProgram;
			}
			else
			{
				mPrograms[key] = newProgram;
				mProgramLoadAttempts[key] = true;
				result = newProgram;
			}
		}
	}
	else
		result = it->second;

	return result;
}

void ShaderFactory::deleteShaderProgram(GLuint inProgramHandle)
{
	// Search mPrograms for a ShaderProgram instance having the given handle
	ShaderProgramMap_t::iterator it;
	for (it = mPrograms.begin(); it != mPrograms.end(); it++)
	{
		if (it->second && (it->second->getHandle() == inProgramHandle))
		{
			delete it->second;
			mPrograms.erase(it);
			break;
		}
	}
}

string ShaderFactory::genKey(const char* inVertexShader, const char* inFragmentShader)
{
	string result;

	stringstream s;
	s << inVertexShader << "_" << inFragmentShader;
	result = s.str();

	return result;
}

// Return false if:
//		- shader file doesn't exist
//		- any shader files it #includes do not exist
//		- any of the shaders can't be compiled
// Potentially recursive
bool ShaderFactory::loadShaderObject(const char* inShaderFilePath, const GLenum inType, ShaderHandleVec_t& ioShaderHandles)
{
	string shaderPath = File::getShadersFolder().append("/").append(inShaderFilePath);

	// Load the shader source.
	File shaderFile(shaderPath);
	if (!shaderFile.exists())
	{
		LOG(ERROR) << "Shader file: " << shaderPath << " does not exist.";
		return false;
	}

	bool result = true;	// Assume everything is cool
	char* src = shaderFile.readAsText();
	if (src)
	{
		// Now scan source for existence of #include statement. If any are found, make
		// recursive call to this method.
		// We need to remove sub-string(s) from src. This is easier using a std:string.
		string shaderSrc(src);
		string::size_type startOfInclude = 0;
		while ((startOfInclude = shaderSrc.find(INCLUDE_STRING, startOfInclude)) != string::npos)
		{
			if (isStringPositionInCommentBlock(startOfInclude, shaderSrc))
			{
				startOfInclude++;
				continue;
			}

			startOfInclude++;
			string::size_type endOfInclude = shaderSrc.find('\n', startOfInclude);
			if (endOfInclude == string::npos)
			{
				result = false;
				LOG(ERROR) << "Syntax error in " << shaderPath << ". #include statement must terminate with newline.";
				break;
			}

			// Parse file name between quotes
			string::size_type openQuote = shaderSrc.find('\"', startOfInclude);
			if (openQuote == string::npos)
			{
				result = false;
				LOG(ERROR) << "Syntax error in " << shaderPath << ". Expected opening quote after #include.";
				break;
			}

			string::size_type closeQuote = shaderSrc.find('\"', openQuote + 1);
			if (closeQuote == string::npos)
			{
				result = false;
				LOG(ERROR) << "Syntax error in " << shaderPath << ". Expected closing quote in #include statement.";
				break;
			}

			string includedFileName = shaderSrc.substr(openQuote + 1, closeQuote - openQuote - 1);

			// If included filename begins with '/', a full path starting from the "Resources/Shaders"
			// folder is assumed. If no '/' is specified at the beginning, a relative path (relative
			// to the folder where inShaderObject hails from) is assumed.
			string shaderName;
			string pathRelativeToShadersFolder;
			if (includedFileName[0] == '/')
			{
				includedFileName.erase(0, 1);	// Just remove the leading forward slash
				pathRelativeToShadersFolder = includedFileName;
			}
			else
			{
				// Determine the path relative to the "Shaders" folder where shaderPath lives
				string shaderFolderPath = File::getShadersFolder();
				pathRelativeToShadersFolder = shaderPath;
				pathRelativeToShadersFolder.erase(0, shaderFolderPath.length() + 1);	// +1 for '/'
				string::size_type lastSlash = pathRelativeToShadersFolder.find_last_of('/');
				if (lastSlash != string::npos)
				{
					pathRelativeToShadersFolder.erase(lastSlash + 1);
					pathRelativeToShadersFolder = pathRelativeToShadersFolder.append(includedFileName);
				}
			}

			// Recursive call
			result = loadShaderObject(pathRelativeToShadersFolder.c_str(), inType, ioShaderHandles);
			if (!result)
				break;
		}

		if (result)
		{
			// When we get here, any #include statements have been handled. We still need to call
			// glShaderSource() and glCompileShader(), but the latter will fail because of the
			// #include statements (which are not part of the GLSL spec). So, we must remove any
			// lines from shaderSrc use the #include statement.
			startOfInclude = 0;
			while ((startOfInclude = shaderSrc.find(INCLUDE_STRING, startOfInclude)) != string::npos)
			{
				if (isStringPositionInCommentBlock(startOfInclude, shaderSrc))
				{
					startOfInclude++;
					continue;
				}

				startOfInclude++;
				string::size_type endOfInclude = shaderSrc.find('\n', startOfInclude);
				shaderSrc.erase(startOfInclude, endOfInclude - startOfInclude + 1);
			}

			// When we get here, shaderSrc should be ready to compile.
			// Create a new shader object
			GLuint handle = glCreateShader(inType);
			if (handle == 0)
			{
				LOG(ERROR) << "glCreateShader() failed for " << shaderPath << ".";
				return false;
			}

			// Compile it
			GLchar* str = (GLchar*)shaderSrc.c_str();
			glShaderSource(handle, 1, &str, NULL);
			glCompileShader(handle);

			// Make sure it compiled successfully. Log any error.
			GLint compileStatus;
			glGetShaderiv(handle, GL_COMPILE_STATUS, &compileStatus);
			if (compileStatus == GL_FALSE)
			{
				GLchar messages[256];
				glGetShaderInfoLog(handle, sizeof(messages), 0, &messages[0]);
				glDeleteShader(handle);
				LOG(ERROR) << "Shader compile failure: " << shaderPath << ": " << messages;
				result = false;
			}

			if (result)
				ioShaderHandles.push_back(handle);
		}

		delete[] src;
	}

	return result;
}

bool ShaderFactory::isStringPositionInCommentBlock(string::size_type inPosition, string& inSource)
{
	// If inPosition is not at beginning of file, it MUST be preceeded by a '\n'
	if (inPosition != 0)
	{
		if (inSource[inPosition - 1] != '\n')
			return true;
	}

	// Make sure inPosition is not inside a comment block
	string::size_type openCommentPos = inSource.rfind("/*", inPosition);
	if (openCommentPos != string::npos)
	{
		string::size_type closeCommentPos = inSource.find("*/", inPosition);
		if (closeCommentPos != string::npos)
		{
			// This doesn't necessarily mean inPosition is in a comment block. We need to make sure
			// there is no close comment in between the found open comment and inPosition.
			string::size_type closeCommentPos2 = inSource.rfind("*/", inPosition);
			if ((closeCommentPos2 != string::npos) && (openCommentPos < closeCommentPos2))
				return true;
		}
	}

	return false;
}
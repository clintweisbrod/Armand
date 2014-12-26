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

ShaderObject::ShaderObject(const char* inShaderFilePath, const GLenum inType)
{
	mHandle = 0;
	mCompileStatus = GL_FALSE;

	string shaderPath = File::getShadersFolder().append("/").append(inShaderFilePath);

	// Create a new shader object and compile it.
	mHandle = glCreateShader(inType);
	if (mHandle == 0)
	{
		LOG(ERROR) << "glCreateShader() failed for " << shaderPath << ".";
		return;
	}

	// Load the shader source.
	File shaderFile(shaderPath);
	char* shaderSrc = shaderFile.readAsText();
	if (shaderSrc)
	{
		glShaderSource(mHandle, 1, &shaderSrc, 0);
		glCompileShader(mHandle);

		glGetShaderiv(mHandle, GL_COMPILE_STATUS, &mCompileStatus);
		if (mCompileStatus == GL_FALSE)
		{
			GLchar messages[256];
			glGetShaderInfoLog(mHandle, sizeof(messages), 0, &messages[0]);
			LOG(ERROR) << "Shader compile failure. " << shaderPath << ": " << messages;
		}

		delete[] shaderSrc;
	}
}

ShaderObject::~ShaderObject()
{
	if (mHandle)
	{
		glDeleteShader(mHandle);
		mHandle = 0;
	}
}

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
			ShaderObject* vertexShader = new ShaderObject(inVertexShader, GL_VERTEX_SHADER);
			if (vertexShader == NULL)
			{
				LOG(ERROR) << "Unable to allocate ShaderObject instance.";
				mProgramLoadAttempts[key] = false;
				return NULL;
			}
			if (!vertexShader->isCompiled())
			{
				mProgramLoadAttempts[key] = false;
				return NULL;
			}

			ShaderObject* fragmentShader = new ShaderObject(inFragmentShader, GL_FRAGMENT_SHADER);
			if (fragmentShader == NULL)
			{
				LOG(ERROR) << "Unable to allocate ShaderObject instance.";
				mProgramLoadAttempts[key] = false;
				return NULL;
			}
			if (!fragmentShader->isCompiled())
			{
				mProgramLoadAttempts[key] = false;
				return NULL;
			}

			// Attach shaders to program
			GLuint programHandle = newProgram->getHandle();
			glAttachShader(programHandle, vertexShader->getHandle());
			delete vertexShader;
			glAttachShader(programHandle, fragmentShader->getHandle());
			delete fragmentShader;

			// Link the shaders
			glLinkProgram(programHandle);
			GLint linkStatus = GL_FALSE;
			glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
			if (linkStatus == GL_FALSE)
			{
				GLchar messages[256];
				glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
				LOG(ERROR) << "Shader link failure. " << key << ": " << messages;
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
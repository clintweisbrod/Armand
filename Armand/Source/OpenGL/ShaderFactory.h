// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// ShaderFactory.h
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

#pragma once

#include <GL/glew.h>
#include <GL/wglew.h>

#include "Utilities/Singleton.h"

class ShaderObject
{
public:
	ShaderObject(const char* inShaderFilePath, const GLenum inType);
	virtual ~ShaderObject();

	bool isCompiled() const { return mCompileStatus == GL_TRUE; }
	GLuint getHandle() const { return mHandle; }

private:
	GLuint	mHandle;
	GLint	mCompileStatus;
};

class ShaderProgram
{
public:
	ShaderProgram();
	virtual ~ShaderProgram();

	GLuint getHandle() const { return mHandle; }

private:
	GLuint	mHandle;
};

typedef map<string, ShaderProgram*> ShaderProgramMap_t;
class ShaderFactory : public Singleton<ShaderFactory>
{
	friend class Singleton<ShaderFactory>;

public:
	ShaderProgram*	getShaderProgram(const char* inVertexShader, const char* inFragmentShader);
	void			deleteShaderProgram(GLuint inProgramHandle);

protected:
	virtual ~ShaderFactory();

private:
	string genKey(const char* inVertexShader, const char* inFragmentShader);
	ShaderProgramMap_t	mPrograms;
};
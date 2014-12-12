// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// GLUtilis.cpp
//
// Defines handy GL-related functions.
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

#include "GLUtils.h"

bool glCheckForError()
{
	bool result = false;

#if _DEBUG
	GLenum errorNumber = glGetError();

	// if the error is GL_INVALID_OPERATION we assume that we are calling glGetError at the wrong time...
	if (errorNumber == GL_INVALID_OPERATION)
		return false;

	result = (errorNumber != GL_NO_ERROR);
	if (result)
	{
		const int kMaxIterations = 10;
		int iterations = 0;
		while ((errorNumber != GL_NO_ERROR) && (iterations++ < kMaxIterations))
		{
			LOG(ERROR) << "OpenGL Error: " << errorNumber << " : " << gluErrorString(errorNumber);
			errorNumber = glGetError();
		}
	}
#endif

	return result;
}

void glTexturingOn(GLuint inTextureMode, GLuint inTextureID)
{
	GLuint enabledTextureMode, disabledTextureMode;
	if ((inTextureMode == GL_TEXTURE_RECTANGLE) && GLEW_ARB_texture_rectangle)
	{
		enabledTextureMode = GL_TEXTURE_RECTANGLE;
		disabledTextureMode = GL_TEXTURE_2D;
	}
	else
	{
		enabledTextureMode = GL_TEXTURE_2D;
		disabledTextureMode = GL_TEXTURE_RECTANGLE;
	}
	glDisable(disabledTextureMode);
	glEnable(enabledTextureMode);
	glBindTexture(enabledTextureMode, inTextureID);
}

void glTexturingOff()
{
	if (GLEW_ARB_texture_rectangle)
		glDisable(GL_TEXTURE_RECTANGLE);

	glDisable(GL_TEXTURE_2D);
}

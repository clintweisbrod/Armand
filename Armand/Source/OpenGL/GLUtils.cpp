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

GLuint glSignedTwosCompliment(GLfloat inValue, int inBits)
{
	if ((inValue < -1) || (inValue > 1))
		return 0;

	GLuint result = 0;

	result = (GLuint)(fabs(inValue) * pow(2, inBits - 1));
	if (inValue < 0)
	{
		result = ~result;
		result++;
	}

	GLuint bitMask = (GLuint)pow(2, inBits) - 1;
	result &= bitMask;

	return result;
}

GLuint glNorma3fToGL_INT_2_10_10_10_REV(Vec3f& inNormal)
{
	GLuint result = 0;

	GLuint x = glSignedTwosCompliment(inNormal.x, 10);
	GLuint y = glSignedTwosCompliment(inNormal.y, 10);
	GLuint z = glSignedTwosCompliment(inNormal.z, 10);

	result = 0;
	result |= (x << 20);
	result |= (y << 10);
	result |= (z << 0);

	return result;
}

void glColor4fToColor4ub(const GLfloat inFloatColor[4], GLubyte ioByteColor[4])
{
	ioByteColor[0] = (GLubyte)(inFloatColor[0] * 255);
	ioByteColor[1] = (GLubyte)(inFloatColor[1] * 255);
	ioByteColor[2] = (GLubyte)(inFloatColor[2] * 255);
	ioByteColor[3] = (GLubyte)(inFloatColor[3] * 255);
}

void glTexCoord2fToTexCoord2us(const GLfloat inFloatTexCoords[2], GLushort ioUShortTexCoords[2])
{
	ioUShortTexCoords[0] = (GLushort)(inFloatTexCoords[0] * 65535);
	ioUShortTexCoords[1] = (GLushort)(inFloatTexCoords[1] * 65535);
}

void glTexCoord2fToTexCoord2us(const Vec2f& inFloatTexCoords, GLushort ioUShortTexCoords[2])
{
	ioUShortTexCoords[0] = (GLushort)(inFloatTexCoords.s * 65535);
	ioUShortTexCoords[1] = (GLushort)(inFloatTexCoords.t * 65535);
}

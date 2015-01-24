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
#include "Math/mathlib.h"

bool glIsError()
{
#if _DEBUG
	const int kMaxIterations = 10;
	int i = 0;
	GLenum glErr = glGetError();
	bool result = (glErr != GL_NO_ERROR);
	while ((glErr != GL_NO_ERROR) && (i++ < kMaxIterations))
	{
		LOG(ERROR) << "OpenGL Error: " << glErr << " : " << gluErrorString(glErr);
		glErr = glGetError();

	};

	return result;
#else
	return false;
#endif
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

GLubyte glConvertGLFloatToGLubyte(const GLfloat in)
{
	return (GLubyte)(clamp(in) * 255);
}

GLushort glConvertGLFloatToGLushort(const GLfloat in)
{
	return (GLushort)(clamp(in) * 65535);
}

void glColor4fToColor4ub(const GLfloat inFloatColor[4], GLubyte ioByteColor[4])
{
	ioByteColor[0] = glConvertGLFloatToGLubyte(inFloatColor[0]);
	ioByteColor[1] = glConvertGLFloatToGLubyte(inFloatColor[1]);
	ioByteColor[2] = glConvertGLFloatToGLubyte(inFloatColor[2]);
	ioByteColor[3] = glConvertGLFloatToGLubyte(inFloatColor[3]);
}

void glTexCoord2fToTexCoord2us(const GLfloat inFloatTexCoords[2], GLushort ioUShortTexCoords[2])
{
	ioUShortTexCoords[0] = glConvertGLFloatToGLushort(inFloatTexCoords[0]);
	ioUShortTexCoords[1] = glConvertGLFloatToGLushort(inFloatTexCoords[1]);
}

void glTexCoord2fToTexCoord2us(const Vec2f& inFloatTexCoords, GLushort ioUShortTexCoords[2])
{
	ioUShortTexCoords[0] = glConvertGLFloatToGLushort(inFloatTexCoords.s);
	ioUShortTexCoords[1] = glConvertGLFloatToGLushort(inFloatTexCoords.t);
}

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// GLUtilis.h
//
// Declares handy GL-related functions.
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
#include "Math/vecmath.h"

#define BUFFER_OFFSET(offset) ((void*)(offset))

bool glCheckForError();
void glTexturingOn(GLuint inTextureMode, GLuint inTextureID);
void glTexturingOff();
GLuint glSignedTwosCompliment(GLfloat inValue, int inBits);
GLuint glNorma3fToGL_INT_2_10_10_10_REV(Vec3f& inNormal);
inline GLubyte glConvertGLFloatToGLubyte(const GLfloat in);
inline GLushort glConvertGLFloatToGLushort(const GLfloat in);
void glColor4fToColor4ub(const GLfloat inFloatColor[4], GLubyte ioByteColor[4]);
void glTexCoord2fToTexCoord2us(const GLfloat inFloatTexCoords[2], GLushort ioUShortTexCoords[2]);
void glTexCoord2fToTexCoord2us(const Vec2f& inFloatTexCoords, GLushort ioUShortTexCoords[2]);

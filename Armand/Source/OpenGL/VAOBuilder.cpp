// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// VAOBuilder.cpp
//
// Helper-class for constructing VAO definitions for interleaved array data.
//
// NOTE: For large amounts of data, interleaved format seems to be a
// little quicker.
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

#include "VAOBuilder.h"
#include "GLUtils.h"

VAOBuilder::VAOBuilder() : mVAOid(0)
{
	// Allocate VAO
	glGenVertexArrays(1, &mVAOid);
}

VAOBuilder::~VAOBuilder()
{
	// Delete VAO
	glDeleteVertexArrays(1, &mVAOid);
}

void VAOBuilder::addArray(string name, GLuint index, GLint size, GLenum type, GLboolean normalized)
{
	VAOInfo info = { name, size, type, normalized, 0, size };
	if (info.mNumElements == GL_BGRA)
		info.mNumElements = 1;
	info.mBytesPerElement = getBytesFromType(info.mType);
	mArrayInfo[index] = info;

	// Recalculate mStride
	mStride = 0;
	for (VBOArrayInfoMap_t::iterator it = mArrayInfo.begin(); it != mArrayInfo.end(); it++)
		mStride += (it->second.mNumElements * it->second.mBytesPerElement);
}

void VAOBuilder::setupGPU(GLuint inVBOid)
{
	// Bind the VAO as the current used object
	glBindVertexArray(mVAOid);

	// Bind the VBO as being the active buffer and storing vertex attributes
	glBindBuffer(GL_ARRAY_BUFFER, inVBOid);

	GLuint offset = 0;
	for (VBOArrayInfoMap_t::iterator it = mArrayInfo.begin(); it != mArrayInfo.end(); it++)
	{
		glVertexAttribPointer(it->first, it->second.mSize, it->second.mType, it->second.mNormalized,
							  mStride, BUFFER_OFFSET(offset));
		offset += (it->second.mNumElements * it->second.mBytesPerElement);

		glEnableVertexAttribArray(it->first);
	}
}

GLint VAOBuilder::getBytesFromType(GLenum inType)
{
	GLint result;

	switch (inType)
	{
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		result = sizeof(GLbyte);
		break;
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		result = sizeof(GLshort);
		break;
	case GL_INT:
	case GL_UNSIGNED_INT:
	case GL_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT_10F_11F_11F_REV:
		result = sizeof(GLint);
		break;
	case GL_HALF_FLOAT:
		result = sizeof(GLhalf);
		break;
	case GL_FLOAT:
		result = sizeof(GLfloat);
		break;
	case GL_DOUBLE:
		result = sizeof(GLdouble);
		break;
	case GL_FIXED:
		result = sizeof(GLfixed);
		break;
	default:
		LOG(ERROR) << "Unknown vertex data type specified.";
		result = 0;
	}

	return result;
}
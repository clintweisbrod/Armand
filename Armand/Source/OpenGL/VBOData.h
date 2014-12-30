// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// VBOData.h
//
// Helper-class for constructing tightly-packed VBO array data.
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


#pragma once

#define BUFFER_OFFSET(offset) ((void*)(offset))

#include <GL/glew.h>

#include <map>

using namespace std;

struct VBOArrayInfo
{
	GLint mSize;	// Typically, 1,2,3 or 4, but can also be GL_BGRA. We therefore can't use this for buffer size calculations.
	GLenum mType;
	GLboolean mNormalized;
	GLint mBytesPerElement;
	GLuint mBaseIndex;
	GLint mNumElements;	// Need this because mSize can be GL_BGRA
};

typedef map<GLuint, VBOArrayInfo> VBOArrayInfoMap_t;
class VBOData
{
public:
	VBOData();
	virtual ~VBOData();

	void addArray(GLuint index, GLint size, GLenum type, GLboolean normalized);
	void setNumVertices(GLuint inNumVertices);
	GLubyte* getArray(GLuint inIndex);
	void setupGPU();
	GLubyte* getData() { return mData; }

private:
	GLint getBytesFromType(GLenum inType);

	VBOArrayInfoMap_t	mArrayInfo;
	GLuint				mNumVertices;
	GLuint				mBufSize;
	GLubyte*			mData;
};

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// VAOBuilder.h
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


#pragma once

#include <GL/glew.h>

#include <vector>

using namespace std;

struct VAOInfo
{
	string mName;	// For debug purposes only
	GLint mSize;	// Typically, 1,2,3 or 4, but can also be GL_BGRA. We therefore can't use this for buffer size calculations.
	GLenum mType;
	GLboolean mNormalized;
	GLint mBytesPerElement;
	GLint mNumElements;	// Need this because mSize can be GL_BGRA
};

typedef vector<VAOInfo> VBOArrayInfoVec_t;
class VAOBuilder
{
public:
	VAOBuilder();
	~VAOBuilder();

	void addArray(string name, GLint size, GLenum type, GLboolean normalized);
	void setupGPU(GLuint inVBOid);
	void bind() const { glBindVertexArray(mVAOid); };

private:
	GLint getBytesFromType(GLenum inType);

	GLuint				mVAOid;
	VBOArrayInfoVec_t	mArrayInfo;
	GLsizei				mStride;
};

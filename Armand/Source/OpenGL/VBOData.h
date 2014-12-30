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

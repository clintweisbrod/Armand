#include "stdafx.h"

#include "VBOData.h"
#include "GLUtils.h"

VBOData::VBOData()
{
	mNumVertices = 0;
	mBufSize = 0;
	mData = NULL;
}

VBOData::~VBOData()
{
	if (mData)
		delete[] mData;
}

void VBOData::addArray(GLuint index, GLint size, GLenum type, GLboolean normalized)
{
	VBOArrayInfo info = {size, type, normalized, 0, 0, size};
	if (info.mNumElements == GL_BGRA)
		info.mNumElements = 1;
	info.mBytesPerElement = getBytesFromType(info.mType);
	mArrayInfo[index] = info;
}

void VBOData::setNumVertices(GLuint inNumVertices)
{
	mNumVertices = inNumVertices;

	VBOArrayInfoMap_t::iterator it;

	// First make sure all specified data types are valid
	bool typesValid = true;
	for (it = mArrayInfo.begin(); it != mArrayInfo.end(); it++)
	{
		if (it->second.mBytesPerElement == 0)
		{
			typesValid = false;
			break;
		}
	}
	if (!typesValid)
		return;

	// Allocate buffer exactly large enough for all the vertices
	mBufSize = 0;
	for (it = mArrayInfo.begin(); it != mArrayInfo.end(); it++)
	{
		it->second.mBaseIndex = mBufSize;
		mBufSize += (it->second.mNumElements * it->second.mBytesPerElement * mNumVertices);
	}
	if (mBufSize > 0)
		mData = new GLubyte[mBufSize];
}

GLubyte* VBOData::getArray(GLuint inIndex)
{
	if (inIndex >= mArrayInfo.size())
		return NULL;

	return mData + mArrayInfo[inIndex].mBaseIndex;
}

void VBOData::setupGPU()
{
	glBufferData(GL_ARRAY_BUFFER, mBufSize, mData, GL_STATIC_DRAW);
	glCheckForError();

	for (VBOArrayInfoMap_t::iterator it = mArrayInfo.begin(); it != mArrayInfo.end(); it++)
	{
		glVertexAttribPointer(it->first, it->second.mSize, it->second.mType, it->second.mNormalized,
							  0, BUFFER_OFFSET(it->second.mBaseIndex));
		glEnableVertexAttribArray(it->first);
	}
}

GLint VBOData::getBytesFromType(GLenum inType)
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
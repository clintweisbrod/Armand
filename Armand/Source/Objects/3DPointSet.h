// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// 3DPointSet.h
//
// General solution for rendering tons of data as points.
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
#include "Main/Exception.h"
#include "OpenGL/VertexBufferStructs.h"
#include "OpenGL/VAOBuilder.h"
#include "RenderObject.h"
#include "OpenGL/GLUtils.h"

GENERATE_EXCEPTION(Static3DPointSetException)

class T3DPointSet : public RenderObject
{
public:
	T3DPointSet() {};
	T3DPointSet(int inNumPoints);
	virtual ~T3DPointSet();

	virtual bool shouldRenderAsPoint(Camera& inCamera) const;
	virtual bool canRenderFull();
	virtual bool renderFull(Camera& inCamera, float inAlpha);
	virtual void setGLStateForFullRender(float inAlpha) const;

protected:
	virtual void loadData() = 0;
	void allocatePointArray(GLsizei inNumPoints, GLsizei inPointStride);
	void releasePointArray();
	void finalize();

	virtual void setupVAO();
	void setupVBO();
	void* getVertex(GLsizei index);

	VAOBuilder*			mPointsVAO;
	GLuint				mPointsVBO;

	GLsizei				mPointStride;
	GLsizei				mNumPoints;
	uint8_t*			mPointBuffer;
};

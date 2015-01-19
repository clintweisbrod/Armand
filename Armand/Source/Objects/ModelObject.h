// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// ModelObject.h
//
// Class defining location and orientation of a 3DS model.
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

#include "RenderObject.h"
#include "Models/3DSModel.h"
#include "OpenGL/VertexBufferStructs.h"

#pragma once

class ModelObject : public RenderObject
{
public:
	ModelObject(const char* inModelFileName);
	virtual ~ModelObject();

	virtual bool canRenderFull();
	virtual bool renderAsPoint(Camera& inCamera, float inAlpha);
	virtual bool renderFull(Camera& inCamera, float inAlpha);
	virtual void setGLStateForFullRender(float inAlpha) const;

protected:
	T3DSModel*	mModel;
	Vec3f		mLastScaledViewerModelVector;

	PointStarVertex	mStarArray[1000000];
};
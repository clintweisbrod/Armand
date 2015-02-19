// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// HYGDatabase.h
//
// See: http://www.astronexus.com/hyg
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

#include "Objects/3DPointSet.h"
#include "Main/Exception.h"
#include "Math/vecmath.h"
#include "OpenGL/Textures/Texture.h"

GENERATE_EXCEPTION(HYGDatabaseException)

struct HYGDataRecord
{
	string	mProperName;
	string	mIdentifier;
	Vec3f	mPosition;
	float_t	mAbsMag;
	float_t	mColorIndex;
	float_t mEyeDistanceSq;
	size_t	mVBOIndex;
};

typedef vector<HYGDataRecord> HYGDataVec_t;
typedef vector<HYGDataRecord*> HYGDataVecP_t;
class HYGDatabase : public T3DPointSet
{
public:
	HYGDatabase();
	virtual ~HYGDatabase();

	virtual void setGLStateForFullRender(float inAlpha) const;
	virtual void preRender(Camera& inCamera);

	HYGDataRecord* getNearestStar() const;

protected:
	virtual void loadData();
	virtual void setupVAO();
	virtual void setPointShaderUniforms(Camera& inCamera, float inAlpha);

	void bv2rgb(float_t bv, float_t &r, float_t &g, float_t &b);

	// Chunker
	void chunkData();
	int getChunkIndexFromArrayIndices(int i, int j, int k) const;
	int getChunkIndexFromPosition(Vec3f& inPosition) const;
	void getAdjacentChunkIndices(int inIndex, vector<int>& ioAdjacentIndices) const;
	void computeNearestToPosition(Vec3f& inPosition, size_t inNumStarsToReturn = 1);

private:
	HYGDataVec_t		mData;
	HYGDataVecP_t*		mChunkedData;
	Texture*			mPointTexture;
	float_t				mPointSaturation;

	int					mChunkDivisions;
	int					mChunkDivisionsSq;
	float_t				mChunkSize;

	HYGDataVecP_t		mNearestStarsToViewer;
};

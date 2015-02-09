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

#include "Objects/Static3DPointSet.h"
#include "Main/Exception.h"
#include "Math/vecmath.h"
#include "OpenGL/Textures/Texture.h"

GENERATE_EXCEPTION(HYGDatabaseException)

struct HYGDataRecord
{
	string	mProperName;
	string	mHIP;
	Vec3f	mPosition;
	float_t	mAbsMag;
	float_t	mColorIndex;
};

typedef vector<HYGDataRecord> HYGData;
class HYGDatabase : public Static3DPointSet
{
public:
	HYGDatabase();
	virtual ~HYGDatabase();

	virtual void setGLStateForFullRender(float inAlpha) const;

protected:
	virtual void loadData();
	virtual void setupVAO();
	virtual void setPointShaderUniforms(Camera& inCamera, float inAlpha);

	void bv2rgb(float_t &r, float_t &g, float_t &b, float_t bv);

private:
	HYGData		mData;
	Texture*	mPointTexture;
	float_t		mPointSaturation;
};

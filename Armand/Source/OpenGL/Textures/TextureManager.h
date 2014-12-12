// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// TextureManager.h
//
// Handles creation, loading and unloading of OpenGL texture objects.
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

#include "Utilities/Singleton.h"
#include "Utilities/Timer.h"
#include "Texture.h"

const int kMaxItemsToCollect = 1000;

class TextureManager : public Singleton<TextureManager>
{
	friend class Singleton<TextureManager>;

public:
	void		startGarbageCollecting(Texture* inTexture); // called for every real load
	void		stopGarbageCollecting(Texture* inTexture); // texture calls on texture destruct. Texture may or may not be in list.
	void 		doGarbageCollection();		// sorts list by age, then takes off older ones until it gets to the ram target. 

	double_t	getMaxTextureRAMAvailable() const { return mMaxRAMAvailableForTextures; }
	double_t	getVRAMDetected() const { return mRAMDetected; }
	double_t	getTotalTextureRAMLoaded() const { return mTotalRAMLoaded; }
	double_t	getTotalTextureRAMUnused() const { return mMaxRAMAvailableForTextures - mTotalRAMLoaded; }
	void		forceDumpTexture(Texture* inTexture);

protected:
	void		sortByAge();

	Texture* 	mTextures[kMaxItemsToCollect + 2]; // we might like to go to a TArray? in the future
	int			mNumTexturesTracked;
	double_t	mTotalRAMLoaded;
	double_t	mPermanentRAMLoaded;
	double_t 	mMaxRAMAvailableForTextures;
	double_t 	mRAMDetected;

	Timer		mTimer;
	double_t	mGarbageCollectionMilliseconds;

protected:
	TextureManager();
	virtual ~TextureManager();
};
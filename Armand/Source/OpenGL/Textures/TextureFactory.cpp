// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// TextureFactory.cpp
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

#include "stdafx.h"
#include "TextureFactory.h"

TextureFactory::TextureFactory()
{
	// have to 0 out all the entries:
	for (int count = 0; count < kMaxItemsToCollect; count++)
		mTextures[count] = NULL;

	mNumTexturesTracked = 0;
	mTotalRAMLoaded = 0;
	mPermanentRAMLoaded = 0;
	mMaxRAMAvailableForTextures = 512.0; // TODO: detect this amount
	mRAMDetected = mMaxRAMAvailableForTextures;
}

TextureFactory::~TextureFactory()
{
	// TODO: Do we unload all the textures we're tracking? I think so.
//	for (TextureVec_t::iterator it = mTextures.begin(); it != mTextures.end(); it++)
//		delete *it;
}

void TextureFactory::startGarbageCollecting(Texture* inTexture)
{
	if (inTexture == NULL)
		return;

	// if we are full, then we can't trackit.
	// the item will work fine, just no garbage collection on it
	if (mNumTexturesTracked == kMaxItemsToCollect)
		return;

	double_t megabytesThisTexture = inTexture->getVRAMUsed();

	// If we're adding a texture that is not subject to garbage collection (permanent), then
	// we simply increase fTotalRAMLoaded by the amount of the permanent texture.
	if (!inTexture->getGarbageCollect())
	{
		mTotalRAMLoaded += megabytesThisTexture;
		mPermanentRAMLoaded += megabytesThisTexture;
		return;
	}

	// We are called from the Texture loader call after the load, so we are counting on the passed texture to stay loaded.
	// so we do garbage collection if necc, in order to free up room then we add the item:
	// Putting this call before the garbage collection call allows us to guarantee that inTexture will not be dumped.
	mTotalRAMLoaded += megabytesThisTexture;
	if (mTotalRAMLoaded > mMaxRAMAvailableForTextures)
		doGarbageCollection();

	mTextures[mNumTexturesTracked] = inTexture;
	mNumTexturesTracked++;
}

void TextureFactory::stopGarbageCollecting(Texture* inTexture)
{
	// we have to look for this in the list, only remove if found. 
	// have to 0 out all the entries:
	for (int count = 0; count < mNumTexturesTracked; count++)
	{
		if (mTextures[count] == inTexture)
		{
			mNumTexturesTracked--;
			mTotalRAMLoaded -= inTexture->getVRAMUsed();
			// move the others up on the list, so there are no gaps:
			for (int moveIt = count; moveIt < mNumTexturesTracked; moveIt++)
				mTextures[moveIt] = mTextures[moveIt + 1];

			mTextures[mNumTexturesTracked] = NULL; // clean up the last item, neatness counts
			break;
		}
	}
}

//----------------------------------------------------------------------
//	TTextureGarbageCollector::DoGarbageCollection
//
//	Purpose:	called when we go over the limit.
//					
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	02/10/10	Tom			5.0.0		Last minute PC bug fix
//
//----------------------------------------------------------------------
void TextureFactory::doGarbageCollection()
{
	mTimer.reset();

	// we only collect when we are over ram budget..
	sortByAge();

	// go through list from bottom up, dropping items until the RAM is under control:
	// we also reset the total amount of RAM used by adding up each item
	double_t ramToLose = mTotalRAMLoaded + 2.0 - mMaxRAMAvailableForTextures; // always lose an extra 2 mb?
	double_t ramDumped = 0.0;
	double_t newTotal = 0.0;
	int numDumped = 0;
	for (int count = (mNumTexturesTracked - 1); count >= 0; count--)
	{
		Texture* currentTexture = mTextures[count];
		if (currentTexture)
		{
			double_t ramThisTexture = currentTexture->getVRAMUsed();
			if (ramToLose > 0)
			{
				ramDumped += ramThisTexture;
				ramToLose -= ramThisTexture;
				currentTexture->unLoad();
				mTextures[count] = NULL;
				numDumped++;
			}
			else
				newTotal += ramThisTexture;
		}
	}


	mNumTexturesTracked -= numDumped;
	mTotalRAMLoaded = mPermanentRAMLoaded + newTotal;

	mGarbageCollectionMilliseconds = mTimer.elapsedMilliseconds();
}

void TextureFactory::forceDumpTexture(Texture* inTexture)
{
	// Rifle through fTextures looking for inTexture
	int index = 0;
	Texture* targetTexture = NULL;
	while (index < mNumTexturesTracked)
	{
		if (inTexture == mTextures[index])
		{
			targetTexture = mTextures[index];
			break;
		}

		index++;
	}
	if (targetTexture)
	{
		// Adjust values and call DumpTexture on the texture object
		mTotalRAMLoaded -= targetTexture->getVRAMUsed();
		targetTexture->unLoad();
		mNumTexturesTracked--;

		// Now move all the remaining textures up to keep the array contiguous
		if (mNumTexturesTracked > 0)
		{
			while (index < mNumTexturesTracked)
			{
				mTextures[index] = mTextures[index + 1];
				index++;
			}
			mTextures[index] = NULL;
		}
	}
}

//----------------------------------------------------------------------
//	TTextureGarbageCollector::DoGarbageCollection
//
//	Purpose:	called when we go over the limit.
//					
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	02/10/10	Tom			5.0.0		Last minute PC bug fix
//
//----------------------------------------------------------------------
void TextureFactory::sortByAge()
{
	// this sorts the items in the list by age
	// younngest at the top. The one we touched last will have the lowest time value, and it goes at the bottom of the list.

	//Assumes that there are no NULL records!
	Texture* exchangeRecord;
	int i, j, gap;

	gap = 1;
	do
	{
		gap = 3 * gap + 1;
	} while (gap <= mNumTexturesTracked);
	for (gap /= 3; gap > 0; gap /= 3)
	{
		for (i = gap; i < mNumTexturesTracked; i++)
		{
			for (j = i - gap;
				j >= 0 && (mTextures[j]->getLastUsedSeconds() < mTextures[j + gap]->getLastUsedSeconds());
				j -= gap)
			{
				exchangeRecord = mTextures[j];
				mTextures[j] = mTextures[j + gap];
				mTextures[j + gap] = exchangeRecord;
			}
		}
	}
}

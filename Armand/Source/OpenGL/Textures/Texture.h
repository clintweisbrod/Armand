// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// Texture.h
//
// Uses Windows Imaging Components (WIC) to import common image files into
// an RGBA (actually BGRA) buffer that OpenGL can use for texturing.
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

#include <stdint.h>
#include <Wincodec.h>
#include "Utilities/File.h"
#include "Math/vecmath.h"

class Texture
{
public:
	Texture();
	virtual ~Texture();

	bool		load(File& inFile);
	bool		load(uint8_t* inEncodedBuffer, uint32_t inBufferBytes);

	uint8_t*	getImageBuffer() const { return mImageBuffer; }
	uint32_t	getImageBufferBytes() const { return mImageBufferBytes; }

private:
	HRESULT		getDecoder(File& inFile, IWICBitmapDecoder** ioDecoder);
	HRESULT		getDecoder(uint8_t* inEncodedBuffer, uint32_t inBufferBytes, IWICBitmapDecoder** ioDecoder);
	bool		loadNative(IWICBitmapDecoder* inDecoder);
	void		cleanup();

	uint8_t*	mImageBuffer;
	uint32_t	mImageBufferBytes;
	Vec2i		mDimensions;
};

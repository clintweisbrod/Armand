// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// Texture.cpp
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

#include "stdafx.h"

#include "Texture.h"
#include "WICImagingFactorySingleton.h"
#include "Utilities/StringUtils.h"

Texture::Texture() : mImageBuffer(NULL),
					 mImageBufferBytes(0),
					 mDimensions(0,0)
{
}

Texture::~Texture()
{
	cleanup();
}

void Texture::cleanup()
{
	if (mImageBuffer)
	{
		delete[] mImageBuffer;
		mImageBuffer = NULL;
	}
	mImageBufferBytes = 0;
	mDimensions = Vec2i(0, 0);
}

bool Texture::load(File& inFile)
{
	if (!inFile.exists())
		return false;

	bool result = false;

	// Release resources from any previous image load
	cleanup();

	IWICBitmapDecoder* pDecoder = NULL;
	HRESULT hr = getDecoder(inFile, &pDecoder);
	if (SUCCEEDED(hr))
		result = loadNative(pDecoder);

	// Release the image decoder
	if (pDecoder)
		pDecoder->Release();

	return result;
}

bool Texture::load(uint8_t* inEncodedBuffer, uint32_t inBufferBytes)
{
	if (inEncodedBuffer == NULL)
		return false;
	if (inBufferBytes == 0)
		return false;

	bool result = false;

	// Release resources from any previous image load
	cleanup();

	IWICBitmapDecoder* pDecoder = NULL;
	HRESULT hr = getDecoder(inEncodedBuffer, inBufferBytes, &pDecoder);
	if (SUCCEEDED(hr))
		result = loadNative(pDecoder);

	// Release the image decoder
	if (pDecoder)
		pDecoder->Release();

	return result;
}

HRESULT Texture::getDecoder(File& inFile, IWICBitmapDecoder** ioDecoder)
{
	HRESULT result = 0;

	WICImagingFactorySingleton* wicFactory = WICImagingFactorySingleton::inst();
	if (wicFactory && wicFactory->GetFactory())
	{
		// Get full path to file
		wstring fullPath = wstringFromString(inFile.getFullPath());

		// Create a decoder
		result = wicFactory->GetFactory()->CreateDecoderFromFilename(fullPath.c_str(),						// Image to be decoded
					NULL,							// Do not prefer a particular vendor
					GENERIC_READ,					// Desired read access to the file
					WICDecodeMetadataCacheOnDemand,// Cache metadata when needed
					ioDecoder);					// Pointer to the decoder
	}

	return result;
}

HRESULT Texture::getDecoder(uint8_t* inEncodedBuffer, uint32_t inBufferBytes, IWICBitmapDecoder** ioDecoder)
{
	HRESULT result = 0;

	WICImagingFactorySingleton* wicFactory = WICImagingFactorySingleton::inst();
	if (wicFactory && wicFactory->GetFactory())
	{
		IWICStream* pStream = NULL;
		result = wicFactory->GetFactory()->CreateStream(&pStream);
		if (SUCCEEDED(result))
		{
			result = pStream->InitializeFromMemory(inEncodedBuffer, inBufferBytes);
			if (SUCCEEDED(result))
			{
				// Create a decoder
				result = wicFactory->GetFactory()->CreateDecoderFromStream(pStream,						// Image to be decoded
					NULL,						// Do not prefer a particular vendor
					WICDecodeMetadataCacheOnDemand,// Cache metadata when needed
					ioDecoder);					// Pointer to the decoder
			}

			pStream->Release();
		}
	}

	return result;
}

bool Texture::loadNative(IWICBitmapDecoder* inDecoder)
{
	bool result = false;

	WICImagingFactorySingleton* wicFactory = WICImagingFactorySingleton::inst();
	if (wicFactory && wicFactory->GetFactory())
	{
		// Retrieve the first frame of the image from the decoder
		IWICBitmapFrameDecode *pFrame = NULL;
		HRESULT hr = inDecoder->GetFrame(0, &pFrame);

		// Make sure we have 32bppBGRA pixel format. If we don't, convert to it.
		IWICBitmapSource* bgraFrame = NULL;
		WICPixelFormatGUID pixelFormat;
		if (SUCCEEDED(hr))
			hr = pFrame->GetPixelFormat(&pixelFormat);
		if (SUCCEEDED(hr))
		{
			if (IsEqualGUID(pixelFormat, GUID_WICPixelFormat32bppBGRA))
				bgraFrame = pFrame;
			else
			{
				// Convert to 32bppBGRA
				WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, pFrame, &bgraFrame);
				pFrame->Release();
			}
			pFrame = NULL;
		}

		IWICBitmapLock *pILock = NULL;
		IWICBitmap *pIBitmap = NULL;
		if (bgraFrame)
		{
//			fNumChannels = 4;

			// Get image dimensions
			UINT width = 0, height = 0;
			hr = bgraFrame->GetSize(&width, &height);
			if (SUCCEEDED(hr))
			{
				mDimensions.x = width;
				mDimensions.y = height;
			}

			// Get the image buffer
			WICRect rcLock = { 0, 0, width, height };
			if (SUCCEEDED(hr))
			{
				// Create the bitmap from the image frame
				hr = wicFactory->GetFactory()->CreateBitmapFromSource(bgraFrame,				// Create a bitmap from the image frame
					WICBitmapCacheOnDemand,	// Cache metadata when needed
					&pIBitmap);				// Pointer to the bitmap
			}
			if (SUCCEEDED(hr))
			{
				// Obtain a bitmap lock for exclusive read
				// The lock is for a width x height rectangle starting at the top left of the bitmap.
				hr = pIBitmap->Lock(&rcLock, WICBitmapLockRead, &pILock);
			}
			UINT cbBufferSize = 0;
			BYTE *pv = NULL;
			if (SUCCEEDED(hr))
			{
				// Retrieve a pointer to the pixel data
				hr = pILock->GetDataPointer(&cbBufferSize, &pv);
			}
			if (SUCCEEDED(hr))
			{
				mImageBufferBytes = cbBufferSize;

				// Allocate out image buffer and copy the image data
				mImageBuffer = new uint8_t[mImageBufferBytes];
				if (mImageBuffer)
				{
					memcpy(mImageBuffer, pv, mImageBufferBytes);

					// Flip the image buffer vertically if requested.
//					if (IS_FLAG_SET(kSNImageFlagInvertVerticalOnLoad))
//						FlipVertical();

					result = true;
				}
			}
		}

		// Release the bitmap lock
		if (pILock)
			pILock->Release();

		// Release the bitmap
		if (pIBitmap)
			pIBitmap->Release();

		// Release the bitmap source object
		if (bgraFrame)
			bgraFrame->Release();
	}

	return result;
}

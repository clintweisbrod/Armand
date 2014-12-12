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
#include "TextureManager.h"
#include "WICImagingFactorySingleton.h"
#include "Utilities/StringUtils.h"
#include "Math/mathlib.h"
#include "OpenGL/GLUtils.h"

Texture::Texture()
{
	init();
}

Texture::Texture(const char* inFilePath) : Texture()
{
	load(File(inFilePath));
}

Texture::Texture(string& inFilePath) : Texture()
{
	load(File(inFilePath));
}

Texture::Texture(File& inFile) : Texture()
{
	load(inFile);
}

Texture::~Texture()
{
	cleanup();

	// Tell the TextureFactory to stop tracking this texture
	removeFromGPU(true);
}

void Texture::init()
{
	mSourceFile = NULL;
	mEncodedImageBuffer = NULL;
	mEncodedImageBufferBytes = 0;
	mImageBuffer = NULL;
	mImageBufferBytes = 0;
	mVRAMMegabytesUsed = 0.0;
	mDimensions = Vec2i(0, 0);
	mTexCoords = Vec2f(0, 0);
	mNumMipMaps = 0;
	mFlags = kTextureFlagLoadMipmaps;
	mLastTimeUsed = 0.0;
	mTextureID = 0;
	mClampingMode = GL_CLAMP;
	mInternalformat = 0;
}

void Texture::cleanup()
{
	if (mImageBuffer)
	{
		delete[] mImageBuffer;
		mImageBuffer = NULL;
	}
	if (mSourceFile)
	{
		delete mSourceFile;
		mSourceFile = NULL;
	}
	if (mEncodedImageBuffer != NULL)
	{
		delete[] mEncodedImageBuffer;
		mEncodedImageBuffer = NULL;
	}
}

bool Texture::load(File& inFile)
{
	if (!inFile.exists())
		return false;

	bool result = false;

	// If the file extension is "dds", the process of obtaining a buffer for the GPU is entirely different.
	if (inFile.getExtension() == "dds")
		result = loadDDS(inFile);
	else
	{
		IWICBitmapDecoder* pDecoder = NULL;
		HRESULT hr = getDecoder(inFile, &pDecoder);
		if (SUCCEEDED(hr))
			result = loadNative(pDecoder);

		// Release the image decoder
		if (pDecoder)
			pDecoder->Release();

		// Need this (as opposed to GL_RGBA) as some PC cards will default to 16 but rgba internals if you do not tell them too.
		mInternalformat = GL_RGBA8;
	}

	if (result)
	{
		SET_FLAG(kTextureFlagImageBufferOK);

		// Remember the source file in case this instance gets jettisoned by the TextureFactory, in which case we will
		// need to reload.
		if (mSourceFile == NULL)
		{
			mSourceFile = new File;
			*mSourceFile = inFile;
		}
	}
	else
		cleanup();

	// Indicate a load has been attempted
	SET_FLAG(kTextureFlagLoadAttempted);

	return result;
}

//
// NOTE: This method takes ownership of the supplied buffer!
bool Texture::load(uint8_t* inEncodedBuffer, size_t inBufferBytes)
{
	if (inEncodedBuffer == NULL)
		return false;
	if (inBufferBytes == 0)
		return false;

	bool result = false;

	IWICBitmapDecoder* pDecoder = NULL;
	HRESULT hr = getDecoder(inEncodedBuffer, inBufferBytes, &pDecoder);
	if (SUCCEEDED(hr))
		result = loadNative(pDecoder);

	// Release the image decoder
	if (pDecoder)
		pDecoder->Release();

	if (result)
	{
		SET_FLAG(kTextureFlagImageBufferOK);

		// Release resources from any previous image load
		if (mEncodedImageBuffer != NULL)
		{
			delete[] mEncodedImageBuffer;
			mEncodedImageBuffer = NULL;
		}

		// Remember the source image buffer in case this instance gets jettisoned by the TextureFactory, in which case we will
		// need to reload.
		if (mEncodedImageBuffer == NULL)
		{
			mEncodedImageBuffer = inEncodedBuffer;
			mEncodedImageBufferBytes = inBufferBytes;
		}
	}
	else
		cleanup();

	// Indicate a load has been attempted
	SET_FLAG(kTextureFlagLoadAttempted);

	return result;
}

bool Texture::sendToGPU()
{
	bool result = false;

	if (mInternalformat == GL_RGBA8)
		result = sendBufferToGPU_Native();
	else
		result = sendBufferToGPU_DDS();

	return result;
}

void Texture::removeFromGPU(bool inNotifyTextureManager)
{
	if (mTextureID != 0)
		glDeleteTextures(1, &mTextureID);
	mTextureID = 0;

	// We need this because this method can be called from the TextureManager when it dumps a texture.
	// We don't want to then tell the TextureManager the texture was dumped because it already is.
	if (inNotifyTextureManager)
		TextureManager::inst()->stopGarbageCollecting(this);

	CLEAR_FLAG(kTextureFlagGarbageCollect);
	CLEAR_FLAG(kTextureFlagBufferOnGPU);
}


GLuint Texture::getTextureID()
{
	GLuint result = 0;

	if (mTextureID != 0)
	{
		// Update last used member for garbage collection
		mLastTimeUsed = Timer::seconds();

		result = mTextureID;
	}
	else
	{
		// If this texture is being garbage collected, it may have been unloaded
		if (IS_FLAG_SET(kTextureFlagGarbageCollect))
		{
			// Synchronous texture load. We only attempt loading once.
			if (IS_FLAG_CLEAR(kTextureFlagLoadAttempted))
			{
				// We gotta load the image first
				if (reload())
					result = mTextureID;
			}
		}
	}

	return result;
}

bool Texture::reload()
{
	bool result = false;

	if (mSourceFile)
		result = load(*mSourceFile);
	else if (mEncodedImageBuffer && (mEncodedImageBufferBytes > 0))
		result = load(mEncodedImageBuffer, mEncodedImageBufferBytes);
	if (result)
		result = sendToGPU();

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

HRESULT Texture::getDecoder(uint8_t* inEncodedBuffer, size_t inBufferBytes, IWICBitmapDecoder** ioDecoder)
{
	HRESULT result = 0;

	WICImagingFactorySingleton* wicFactory = WICImagingFactorySingleton::inst();
	if (wicFactory && wicFactory->GetFactory())
	{
		IWICStream* pStream = NULL;
		result = wicFactory->GetFactory()->CreateStream(&pStream);
		if (SUCCEEDED(result))
		{
			result = pStream->InitializeFromMemory(inEncodedBuffer, (DWORD)inBufferBytes);
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
				// Allocate image buffer and copy the image data
				// If we already have a buffer allocated but not large enough, release it.
				if (mImageBuffer && (mImageBufferBytes < cbBufferSize))
				{
					delete[] mImageBuffer;
					mImageBuffer = NULL;
					mImageBufferBytes = 0;
				}
				if (mImageBuffer == NULL)
				{
					mImageBuffer = new uint8_t[cbBufferSize];
					mImageBufferBytes = cbBufferSize;
				}
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

bool Texture::loadDDS(File& inFile)
{
	bool result = false;

	FILE* fp = NULL;
	try
	{
		// Open file in binary mode
		fp = inFile.getCRTFileHandle("rb");
		if (fp == NULL)
		{
			LOG(ERROR) << "Failed to open the file: " << inFile.getFullPath();
			throw(result);
		}

		// The first 4 bytes of any DDS file are "DDS ". Verify this.
		const int kDDSHeaderIDLength = 4;
		char filecode[kDDSHeaderIDLength];
		size_t elementsRead = fread(filecode, 1, kDDSHeaderIDLength, fp);
		if (elementsRead != kDDSHeaderIDLength)
		{
			LOG(ERROR) << "Failed to read the file: " << inFile.getFullPath();
			throw(result);
		}

		if (strncmp(filecode, "DDS ", kDDSHeaderIDLength) != 0)
		{
			LOG(ERROR) << "File does not appear to be in DDS format: " << inFile.getFullPath();
			throw(result);
		}

		// Read the surface descriptor data
		DDS_SURFACE_DATA ddsd;
		elementsRead = fread(&ddsd, sizeof(DDS_SURFACE_DATA), 1, fp);
		if (elementsRead != 1)
		{
			LOG(ERROR) << "Error reading DDS surface descriptor data: " << inFile.getFullPath();
			throw(result);
		}

		// We want to swap the byte order in ddsd.fPixelFormatCode so that we can identify it later with
		// human-readable strings.
		uint32_t num = ddsd.fPixelFormatCode;
		uint32_t b0, b1, b2, b3;
		b0 = (num & 0x000000ff) << 24u;
		b1 = (num & 0x0000ff00) << 8u;
		b2 = (num & 0x00ff0000) >> 8u;
		b3 = (num & 0xff000000) >> 24u;
		ddsd.fPixelFormatCode = b0 | b1 | b2 | b3;

		/* The following fields exist but for our purposes, they are not needed.
		ddsd.fAlphaBitDepth = EndianU32_LtoN(ddsd.fAlphaBitDepth);
		ddsd.fPixelFormatFlags = EndianU32_LtoN(ddsd.fPixelFormatFlags);
		ddsd.fBitsPerPlane = EndianU32_LtoN(ddsd.fBitsPerPlane);
		ddsd.fRedMask = EndianU32_LtoN(ddsd.fRedMask);
		ddsd.fGreenMask = EndianU32_LtoN(ddsd.fGreenMask);
		ddsd.fBlueMask = EndianU32_LtoN(ddsd.fBlueMask);
		ddsd.fAlphaMask = EndianU32_LtoN(ddsd.fAlphaMask);
		*/

		// Make sure that if ddsd.fMipMapCount > 0, that we have the full set of mipmaps based on the
		// larger dimension. If we don't have a full set, we have to disable mipmap filtering.
		if (ddsd.fMipMapCount > 0)
		{
			// Compute number of mipmaps. We could do a floating point calculation using log() to determine
			// the expected mipmap count, but this loop is probably faster.
			uint32_t expectedMipmaps = 1;
			uint32_t maxDimension = max(ddsd.fWidth, ddsd.fHeight);
			while ((maxDimension >>= 1) > 0)
				expectedMipmaps++;

			if (expectedMipmaps != ddsd.fMipMapCount)
				CLEAR_FLAG(kTextureFlagLoadMipmaps);
		}

		// Set the detected size of the image
		mDimensions.x = ddsd.fWidth;
		mDimensions.y = ddsd.fHeight;

		// We may only be looking for the image dimensions
		if (IS_FLAG_SET(kTextureFlagQueryInfoOnly))
			return result;

		// DDS images are always POT texture
		CLEAR_FLAG(kTextureFlagMakeRectangleTexture);


		// Initialize the DDS_SURFACE_DATA struct
		switch (ddsd.fPixelFormatCode)
		{
			case 'DXT1':	// DXT1's compression ratio is 8:1
				mInternalformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				break;

			case 'DXT3':	// DXT3's compression ratio is 4:1
				mInternalformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				break;

			case 'DXT5':	// DXT5's compression ratio is 4:1
				mInternalformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;

			default:		// Unsupported dds format
			{
				LOG(ERROR) << "File does not appear to be compressed using DXT1, DXT3, or DXT5: " << inFile.getFullPath();
				throw(result);
			}
		}

		// The file apparently contains no pixel data
		if (ddsd.fLinearSize == 0)
		{
			LOG(ERROR) << "Error reading DDS file: " << inFile.getFullPath();
			throw(result);
		}

		// Decide how many mipmap levels to load. We do this here because if the image has mipmaps
		// included, but we've opted to ignore them, we don't need to read in the entire file.
		mNumMipMaps = ddsd.fMipMapCount;
		if ((mNumMipMaps == 0) /* || IS_FLAG_CLEAR(kSNImageFlagLoadMipmaps) */)
			mNumMipMaps = 1;	// Still have to load the first level of detail
		(mNumMipMaps > 1) ? SET_FLAG(kTextureFlagUsingMipmaps) : CLEAR_FLAG(kTextureFlagUsingMipmaps);

		// We need blockSize to compute number of bytes to read from file
		GLint blockSize = 16;
		if (mInternalformat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
			blockSize = 8;

		// Compute required buffer size to hold 1 or all mipmap images
		int i;
		size_t requiredBufferBytes = 0;
		int mapHeight = mDimensions.y;
		int mapWidth = mDimensions.x;
		for (i = 0; i < mNumMipMaps; ++i)
		{
			if (mapWidth == 0)
				mapWidth = 1;
			if (mapHeight == 0)
				mapHeight = 1;

			requiredBufferBytes += ((mapWidth + 3) / 4) * ((mapHeight + 3) / 4) * blockSize;

			// Half the image size for the next mip-map level...
			mapWidth /= 2;
			mapHeight /= 2;
		}

		// Allocate image buffer and load the image data
		// If we already have a buffer allocated but not large enough, release it.
		if (mImageBuffer && (mImageBufferBytes < requiredBufferBytes))
		{
			delete[] mImageBuffer;
			mImageBuffer = NULL;
			mImageBufferBytes = 0;
		}
		if (mImageBuffer == NULL)
		{
			mImageBuffer = new uint8_t[requiredBufferBytes];
			mImageBufferBytes = requiredBufferBytes;
		}
		if (mImageBuffer == NULL)
		{
			LOG(ERROR) << "Unable to allocate pixel buffer while reading DDS file: " << inFile.getFullPath();
			throw(result);
		}

		// Read the pixel data in
		elementsRead = fread(mImageBuffer, 1, mImageBufferBytes, fp);
		if (elementsRead != mImageBufferBytes)
		{
			LOG(ERROR) << "Unable to read all data from DDS file: " << inFile.getFullPath();
			throw(result);
		}

		result = true;
	}
	catch (...)
	{
		if (fp)
			fclose(fp);
	}

	return result;
}

bool Texture::sendBufferToGPU_Native()
{
	if (mImageBuffer == NULL)
		return 0;

	if (IS_FLAG_SET(kTextureFlagBufferOnGPU))
		return true;

	bool result = false;

	// Possibly create new texture object
	bool newTextureObject = false;
	if (mTextureID == 0)
	{
		newTextureObject = true;
		glGenTextures(1, &mTextureID);  // make up a texture name
	}

	// Enable the correct texture mode:
	// If rectangular texture is requested or either dimensions is not POT, we use GL_TEXTURE_RECTANGLE.
	// Otherwise, we use GL_TEXTURE_2D.
	GLuint enabledTextureMode, disabledTextureMode;
	if (IS_FLAG_SET(kTextureFlagMakeRectangleTexture) || !isPowerOf2(mDimensions.x) || !isPowerOf2(mDimensions.y))
	{
		enabledTextureMode = GL_TEXTURE_RECTANGLE;
		disabledTextureMode = GL_TEXTURE_2D;
		mTexCoords.s = (float_t)mDimensions.x;
		mTexCoords.t = (float_t)mDimensions.y;
		mClampingMode = GL_CLAMP_TO_EDGE;
	}
	else
	{
		enabledTextureMode = GL_TEXTURE_2D;
		disabledTextureMode = GL_TEXTURE_RECTANGLE;
		mTexCoords.s = 1;
		mTexCoords.t = 1;
	}
	glDisable(disabledTextureMode);
	glEnable(enabledTextureMode);
	glBindTexture(enabledTextureMode, mTextureID);

	// Pixels have no padding
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	// Check for any GL error conditions
	glCheckForError();

	// Specify internal format
	if (GLEW_ARB_texture_compression && IS_FLAG_SET(kTextureFlagUseCompression))
		mInternalformat = GL_COMPRESSED_RGBA;

	// Set clamping mode - Same for mipmapped or non-mipmapped
	glTexParameteri(enabledTextureMode, GL_TEXTURE_WRAP_S, mClampingMode);
	glTexParameteri(enabledTextureMode, GL_TEXTURE_WRAP_T, mClampingMode);

	// Do we load mipmaps?
	bool textureLoaded = false;
	bool mipmapsLoaded = false;
	if (IS_FLAG_SET(kTextureFlagLoadMipmaps) && newTextureObject && (enabledTextureMode == GL_TEXTURE_2D))
	{
		// if using MipMaps, then we use nearest for the minification filter.
		// this makes gl use smaller maps, saving memory
		//------------------------------------------------
		glTexParameteri(enabledTextureMode, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(enabledTextureMode, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

		if (GLEW_SGIS_generate_mipmap) // Generate mipmaps with hardware
		{
			LOG(INFO) << "Generating mipmaps via hardware for textureID=" << mTextureID;

			glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
			glTexParameteri(enabledTextureMode, GL_GENERATE_MIPMAP, GL_TRUE);
			glTexImage2D(enabledTextureMode, 0, mInternalformat, mDimensions.x, mDimensions.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, mImageBuffer);

			if (!glCheckForError())
			{
				mipmapsLoaded = true;
				textureLoaded = true;
			}
			else
				LOG(ERROR) << "Hardware mipmap generation failed.";

		}
		else	// Generate mipmaps with GLU - slower
		{
			LOG(INFO) << "Generating mipmaps via software for textureID=" << mTextureID;

			GLint success = gluBuild2DMipmaps(enabledTextureMode, mInternalformat, mDimensions.x, mDimensions.y, GL_BGRA, GL_UNSIGNED_BYTE, mImageBuffer);
			if (success != 0)
			{
				mipmapsLoaded = true;
				textureLoaded = true;
			}
			else
				LOG(ERROR) << "Software mipmap generation failed.";
		}
	}
	else	// Don't do mipmaps....
	{
		// For magnification and minification, we use a linear filter...
		glTexParameteri(enabledTextureMode, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(enabledTextureMode, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if (newTextureObject)
			glTexImage2D(enabledTextureMode, 0, mInternalformat, mDimensions.x, mDimensions.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, mImageBuffer);
		else
			glTexSubImage2D(enabledTextureMode, 0, 0, 0, mDimensions.x, mDimensions.y, GL_BGRA, GL_UNSIGNED_BYTE, mImageBuffer);

		if (!glCheckForError())
			textureLoaded = true;
	}

	// Check for any errors after attempting to get image up to card
	if (!textureLoaded)
	{
		LOG(ERROR) << "Error loading textureID=" << mTextureID;
		glDeleteTextures(1, &mTextureID);
		mTextureID = 0;
	}
	else
	{
		// The texture has been sent up to graphics card successfully...
		result = true;

		// ...so we can deallocate the raw image buffer. But only if the image was
		// not specified explicitly. If it was, we want to keep the buffer.
		if (IS_FLAG_CLEAR(kTextureFlagUsingExplicitBuffer))
		{
			delete[] mImageBuffer;
			mImageBuffer = NULL;
			mImageBufferBytes = 0;
		}

		// Indicate if we've generated mipmaps
		if (mipmapsLoaded)
			SET_FLAG(kTextureFlagUsingMipmaps);
		else
			CLEAR_FLAG(kTextureFlagUsingMipmaps);

		// Now determine how much VRAM has been used
		GLint bytesUsed = 0;
		if (mInternalformat == GL_COMPRESSED_RGBA)
		{
			GLint imageSize;

			// We must account for all mipmap textures
			if (IS_FLAG_SET(kTextureFlagLoadMipmaps))
			{
				GLint imageLevel = 0;
				while (true)
				{
					imageSize = 0;
					glGetTexLevelParameteriv(GL_TEXTURE_2D, imageLevel, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imageSize);
					if (imageSize > 0)
						bytesUsed += imageSize;
					else
						break;

					imageLevel++;
				}
			}
			else
			{
				// No mipmaps so just need to know level 0 image size
				imageSize = 0;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imageSize);
				if (imageSize > 0)
					bytesUsed = imageSize;
			}
		}
		else
		{
			// Calculate VRAM used based on image dimensions
			bytesUsed = (mDimensions.x * mDimensions.y * 4);
			if (IS_FLAG_SET(kTextureFlagLoadMipmaps))
			{
				int mipmapWidth = mDimensions.x;
				int mipmapHeight = mDimensions.y;
				while ((mipmapWidth > 1) || (mipmapHeight > 1))
				{
					// Calculate next smaller mipmap image size
					if (mipmapWidth > 1)
						mipmapWidth /= 2;
					if (mipmapHeight > 1)
						mipmapHeight /= 2;

					bytesUsed += (mipmapWidth * mipmapHeight * 4);
				}
			}
		}
		mVRAMMegabytesUsed = (double_t)bytesUsed / 1048576.0;

		// Start garbage collecting this texture
		SET_FLAG(kTextureFlagGarbageCollect);
		TextureManager::inst()->startGarbageCollecting(this);
		SET_FLAG(kTextureFlagBufferOnGPU);
	}

	return result;
}

bool Texture::sendBufferToGPU_DDS()
{
	if (mImageBuffer == NULL)
		return 0;

	if (IS_FLAG_SET(kTextureFlagBufferOnGPU))
		return true;

	bool result = false;

	// Check for any GL error conditions
	glCheckForError();

	// Generate a texture object
	glGenTextures(1, &mTextureID);

	// Enable GL_TEXTURE_2D mode
	glDisable(GL_TEXTURE_RECTANGLE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	// We use GL_TEXTURE_2D so we have normalized texture coordinates
	mTexCoords.s = 1;
	mTexCoords.t = 1;

	// Specify min and mag filters
	if (IS_FLAG_SET(kTextureFlagUsingMipmaps))
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set clamping mode - Same for mipmapped or non-mipmapped
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mClampingMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mClampingMode);

	// Now send the data up to the card
	bool textureLoaded = true;
	GLint blockSize = 16;
	if (mInternalformat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
		blockSize = 8;
	int i;
	int pixelOffset = 0;
	int mapHeight = mDimensions.y;
	int mapWidth = mDimensions.x;
	for (i = 0; i < mNumMipMaps; ++i)
	{
		if (mapWidth == 0)
			mapWidth = 1;
		if (mapHeight == 0)
			mapHeight = 1;
		int mapSize = ((mapWidth + 3) / 4) * ((mapHeight + 3) / 4) * blockSize;
		glCompressedTexImage2D(	GL_TEXTURE_2D,
								i,
								mInternalformat,
								mapWidth,
								mapHeight,
								0,
								mapSize,
								mImageBuffer + pixelOffset);

		// Check for any errors after attempting to get image up to card
		if (glCheckForError())
		{
			textureLoaded = false;
			break;
		}

		// Adjust the pixel offset
		pixelOffset += mapSize;

		// Half the image size for the next mip-map level...
		mapWidth /= 2;
		mapHeight /= 2;
	}

	if (!textureLoaded)
	{
		LOG(ERROR) << "Error loading textureID=" << mTextureID;
		glDeleteTextures(1, &mTextureID);
		mTextureID = 0;
	}
	else
	{
		// The texture has been sent up to graphics card successfully.
		result = true;

		// Get the texture VRAM used.
		GLint bytesUsed = 0;
		GLint compressed = GL_FALSE;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);
		if (compressed == GL_TRUE)
		{
			// We must account for all mipmap textures
			GLint imageSize;
			for (i = 0; i < mNumMipMaps; ++i)
			{
				glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imageSize);
				if (imageSize > 0)
					bytesUsed += imageSize;
			}
		}
		mVRAMMegabytesUsed = (double_t)bytesUsed / 1048576.0;

		// Start garbage collecting this texture
		SET_FLAG(kTextureFlagGarbageCollect);
		TextureManager::inst()->startGarbageCollecting(this);
		SET_FLAG(kTextureFlagBufferOnGPU);
	}

	// Deallocate buffer
	delete[] mImageBuffer;
	mImageBuffer = NULL;

	return result;
}

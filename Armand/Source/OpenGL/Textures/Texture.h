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
#include <GL/glew.h>
#include "Utilities/File.h"
#include "Math/vecmath.h"

// Bit-wise flag manipulators, for readability
#define IS_FLAG_SET(flag)		(mFlags & flag)
#define IS_FLAG_CLEAR(flag)	   !(mFlags & flag)
#define	SET_FLAG(flag)			(mFlags |= flag)
#define CLEAR_FLAG(flag)		(mFlags &= ~flag)

//----------------------------------------------------------------------
//	The following structure is needed to support loading
//	and saving of S3TC (or DDS) files. DDS is a Windows format originating
//  from DirectX but is actually a more widely used and established format
//  called S3TC. Leave it to Microsoft to reinvent yet another wheel!
//	Anyway, in order to load DDS files on Mac, we need some knowledge of
//	DirectX stuff. These structures serve as memory templates for the header
//	information at the front of the file.
//----------------------------------------------------------------------
struct DDS_SURFACE_DATA
{
	uint32_t		fSize;					// size of the DDSURFACEDESC structure
	uint32_t		fFlags;					// determines what fields are valid
	uint32_t		fHeight;				// height of surface to be created
	uint32_t		fWidth;					// width of input surface
	uint32_t		fLinearSize;
	uint32_t		fDepth;
	uint32_t		fMipMapCount;			// number of mip-map levels requested
	uint32_t		fAlphaBitDepth;
	uint8_t			fUnused1[44];
	uint32_t		fPixelFormatFlags;
	uint32_t		fPixelFormatCode;		// Four character code
	uint32_t		fBitsPerPlane;
	uint32_t		fRedMask;
	uint32_t		fGreenMask;
	uint32_t		fBlueMask;
	uint32_t		fAlphaMask;
	uint8_t			fUnused2[20];
};

class Texture
{
public:
	enum TextureFlags
	{
		kTextureFlagLoadMipmaps = 0x0001,			// Mipmap generation/loading is requested
		kTextureFlagUsingMipmaps = 0x0002,			// Set if texture is using mipmaps
		kTextureFlagUseCompression = 0x0004,		// Internal texture format is compressed using GL_COMPRESSED_RGBA if supported
		kTextureFlagSumColors = 0x0008,				// Tully needs this
		kTextureFlagComputeAlpha = 0x0010,			// Use RGB to compute alpha channel
		kTextureFlagMakeRectangleTexture = 0x0020,	// Use rectangle texture target if supported
		kTextureFlagGarbageCollect = 0x0040,		// for texture mem management
		kTextureFlagLoadAttempted = 0x0080,			// Set if image load was attempted
		kTextureFlagQueryInfoOnly = 0x0100,			// Set if we only want image info contained in header
		kTextureFlagInvertVerticalOnLoad = 0x0200,	// UI textures need to be inverted vertically.
		kTextureFlagInvertVerticalOnSave = 0x0400,	// UI textures need to be inverted vertically.
		kTextureFlagPreMultiplyAlpha = 0x0800,		// UI textures need premultiplied alpha
		kTextureFlagByteSwapPixelData = 0x1000,		// UI textures on Mac need byte swapping
		kTextureFlagAsyncLoad = 0x2000,				// Texture is loaded asynchronously
		kTextureFlagAsyncLoadInProgress = 0x4000,	// Asynchronous texture image buffer is being constructed
		kTextureFlagIncludeAlphaOnSave = 0x8000,	// Include alpha channel when saving
		kTextureFlagUsingExplicitBuffer = 0x10000,	// Using explicitly specified raw BRGA buffer
		kTextureFlagBufferOnGPU = 0x20000,			// Set if last loaded buffer was successfully sent to GPU
		kTextureFlagImageBufferOK = 0x40000,		// Set if image buffer was successfully loaded
	};

	// Constructors
	Texture();
	Texture(const char* inFilePath);
	Texture(string& inFilePath);
	Texture(File& inFile);
	virtual ~Texture();

	// These two load methods do not make any GL calls so in theory, they could be threaded.
	bool		load(File& inFile);
	bool		load(uint8_t* inEncodedBuffer, size_t inBufferBytes);
	
	bool		sendToGPU();
	void		removeFromGPU(bool inNotifyTextureManager);

	GLuint		getTextureID();
	double_t	getVRAMMegabytesUsed() const { return mVRAMMegabytesUsed; }
	double_t	getLastUsedSeconds() const { return mLastTimeUsed; }
	Vec2i&		getDimensions() { return mDimensions; }
	Vec2f&		getTexCoords() { return mTexCoords; }

	int			getImageBufferOK() const { return IS_FLAG_SET(kTextureFlagImageBufferOK); }
	int			getGarbageCollect() const { return IS_FLAG_SET(kTextureFlagGarbageCollect); };

private:
	void		init();
	bool		loadDDS(File& inFile);
	HRESULT		getDecoder(File& inFile, IWICBitmapDecoder** ioDecoder);
	HRESULT		getDecoder(uint8_t* inEncodedBuffer, size_t inBufferBytes, IWICBitmapDecoder** ioDecoder);
	bool		loadNative(IWICBitmapDecoder* inDecoder);
	bool		reload();
	void		cleanup();
	bool		sendBufferToGPU_Native();
	bool		sendBufferToGPU_DDS();

	File*		mSourceFile;			// This will be non-NULL if texture originated from local file
	uint8_t*	mEncodedImageBuffer;	// This will be non-NULL if texture originated from some remote source
	size_t		mEncodedImageBufferBytes;
	uint8_t*	mImageBuffer;
	size_t		mImageBufferBytes;
	double_t	mVRAMMegabytesUsed;
	Vec2i		mDimensions;
	Vec2f		mTexCoords;
	GLint		mNumMipMaps;
	int			mFlags;
	double_t	mLastTimeUsed;

	GLuint		mTextureID;
	GLuint		mClampingMode;

	// These are parameters named directly from argument names for glTexImage2D() and glCompressedTexImage2D() 
	GLint		mInternalformat;	// Third parameter to glTexImage2D() and glCompressedTexImage2D()
};

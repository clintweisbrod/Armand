// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// FontFactory.h
//
// Infrastructure leveraging freetype-gl library to expose font rendering
// within OpenGL context.
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

#include <freetype-gl/freetype-gl.h>
#include <freetype-gl/mat4.h>
#include <freetype-gl/shader.h>
#include <freetype-gl/vertex-buffer.h>

#include "Utilities/Singleton.h"

typedef map<int, texture_font_t*> FontMapType;
class FontRenderer
{
	public:
		FontRenderer(string& inFontName, GLuint inShader);
		virtual ~FontRenderer();

		bool render(wstring& inString, int inFontSize, TVector2f& inPen, TVector4f& inColor, float inRotationInDegrees = 0);

	private:
		string getSystemFontFile(const string &inFontName) const;

		string					mFontName;
		texture_atlas_t*		mAtlas;
		FontMapType				mFonts;
		vertex_buffer_t*		mVertexBuffer;
		int						mLargestFontSize;

		// Transformation matrices used to transform every vertex passing through shader
		mat4					mModelMatrix;
		mat4					mViewMatrix;
		mat4					mProjectionMatrix;
		GLuint					mShader;
};

typedef map<string, FontRenderer*> FontRendererMapType;
class FontFactory : public TSingleton<FontFactory>
{
	friend class TSingleton<FontFactory>;

	public:
		FontRenderer* getFontRenderer(string& inFaceName);

	protected:
		FontFactory();
		virtual ~FontFactory();

	private:
		GLuint				mShader;	// Used by all FontRenderer instances
		FontRendererMapType	mRenderers;
};

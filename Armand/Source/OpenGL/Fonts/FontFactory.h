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
#include <freetype-gl/shader.h>
#include <freetype-gl/vertex-buffer.h>

#include "Utilities/Singleton.h"
#include "Math/vecmath.h"

typedef map<int, texture_font_t*> FontMap_t;
class FontRenderer
{
	friend class FontFactory;

	public:
		FontRenderer(string& inFontName, GLuint inShader);
		virtual ~FontRenderer();

		bool render(wstring& inString, int inFontSize, Vec2f& inPen, Vec4f& inColor, float inRotationInDegrees = 0);
		bool renderSpherical(wstring& inString, int inFontSize, Vec2f& inPen, Vec4f& inColor);

	private:
		string getSystemFontFile(const string &inFontName) const;
		void setSceneSize(Vec2i& inSceneSize);
		texture_font_t* getFontTexture(int inFontSize);

		string					mFontName;
		texture_atlas_t*		mAtlas;
		FontMap_t				mFonts;
		vertex_buffer_t*		mVertexBuffer;
		int						mLargestFontSize;

		// Transformation matrices used to transform every vertex passing through shader
		Mat4f					mModelMatrix;
		Mat4f					mViewMatrix;
		Mat4f					mProjectionMatrix;
		GLuint					mShaderHandle;
};

typedef map<string, FontRenderer*> FontRendererMap_t;
class FontFactory : public Singleton<FontFactory>
{
	friend class Singleton<FontFactory>;

	public:
		FontRenderer* getFontRenderer(string& inFaceName);
		void sceneSizeChanged(Vec2i& inNewSceneSize);

	protected:
		FontFactory();
		virtual ~FontFactory();

	private:
		GLuint				mShaderHandle;	// Used by all FontRenderer instances
		FontRendererMap_t	mRenderers;
};

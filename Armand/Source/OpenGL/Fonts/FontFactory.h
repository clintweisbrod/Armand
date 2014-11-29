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
		void render_text(SIZE inWindowSize);

	protected:
		FontFactory();
		virtual ~FontFactory();

	private:
		GLuint				mShader;	// Used by all FontRenderer instances
		FontRendererMapType	mRenderers;
};

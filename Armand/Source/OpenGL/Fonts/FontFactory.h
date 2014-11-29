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
		FontRenderer(string& inFontName);
		virtual ~FontRenderer();

		bool render(wstring& inString, int inFontSize, TVector2f inPen, TVector4f inColor);

	private:
		string getSystemFontFile(const string &inFontName) const;

		string					mFontName;
		texture_atlas_t*		mAtlas;
		FontMapType				mFonts;
		vertex_buffer_t*		mVertexBuffer;
		int						mLargestFontSize;

		mat4					mModelMatrix;
		mat4					mViewMatrix;
		mat4					mProjectionMatrix;
};

typedef map<string, FontRenderer*> FontRendererMapType;
class FontFactory : public TSingleton<FontFactory>
{
	friend class TSingleton<FontFactory>;

	public:
		static GLuint		sShader;

		FontRenderer* getFontRenderer(string& inFaceName);
		void render_text(SIZE inWindowSize);

	protected:
		FontFactory();
		virtual ~FontFactory();

	private:

		FontRendererMapType	mRenderers;
};

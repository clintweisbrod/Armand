#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Utilities/Singleton.h"

class FontFactory : public TSingleton<FontFactory>
{
	friend class TSingleton<FontFactory>;

	public:
//		void renderString(string inString);
//		void render_text(const char *text, float x, float y, float sx, float sy);
		void render_text(SIZE inWindowSize);

	protected:
		FontFactory();
		virtual ~FontFactory();

	private:
		string getSystemFontFile(const string &faceName);

		FT_Library m_Ft;

		FT_Face face;
		FT_GlyphSlot g;
		GLuint tex;
};
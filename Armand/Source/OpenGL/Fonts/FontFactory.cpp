#include "stdafx.h"

#include "OpenGL/Fonts/FontFactory.h"
#include "OpenGL/OpenGLWindow.h"

extern OpenGLWindow* gOpenGLWindow;
GLuint FontFactory::sShader = 0;

const int kMinFontSize = 10;	// Font smaller than this are pretty difficult to read

typedef struct {
	float x, y, z;    // position
	float s, t;       // texture
	float r, g, b, a; // color
} vertex_t;

FontFactory::FontFactory()
{
	sShader = shader_load("shaders/v3f-t2f-c4f.vert", "shaders/v3f-t2f-c4f.frag");
}

FontFactory::~FontFactory()
{
	FontRendererMapType::iterator it;
	for (it = mRenderers.begin(); it != mRenderers.end(); it++)
	{
		if (it->second)
		{
			delete it->second;
			it->second = NULL;
		}
	}
}

FontRenderer* FontFactory::getFontRenderer(string& inFaceName)
{
	if (inFaceName.length() == 0)
		return NULL;

	FontRenderer* result = NULL;
	FontRendererMapType::iterator it = mRenderers.find(inFaceName);
	if (it == mRenderers.end())
	{
		// Create the renderer
		FontRenderer* newRenderer = new FontRenderer(inFaceName);
		if (newRenderer)
		{
			mRenderers[inFaceName] = newRenderer;
			result = newRenderer;
		}
	}
	else
		result = it->second;

	return result;
}

FontRenderer::FontRenderer(string& inFontName) : mAtlas(NULL),
												 mVertexBuffer(NULL),
												 mLargestFontSize(0)
{
	string fontFileName = getSystemFontFile(inFontName);
	if (fontFileName.length() == 0)
	{
		return;
	}

	mFontName = inFontName;
	mAtlas = texture_atlas_new(512, 512, 1);
	if (mAtlas == NULL)
	{
		return;
	}

	// TODO: Fill theGlyphs with all reasonable characters that might be used. 
	wchar_t* theGlyphs = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	// Populate 
	const char* filename = fontFileName.c_str();
	const int kMaxFontSize = 50;	// Highly unlikely we will get around to this font size.
	for (int i = kMinFontSize; i <= kMaxFontSize; i += 2)
	{
		texture_font_t* font = texture_font_new_from_file(mAtlas, (float)i, filename);
		if (texture_font_load_glyphs(font, theGlyphs) == 0)
			mFonts[i] = font;
		else
		{
			mLargestFontSize = i - 2;
			break;
		}
	}

	// Create a new vertex buffer
	mVertexBuffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");

	// Setup transform matrices
	mat4_set_identity(&mModelMatrix);
	mat4_set_identity(&mViewMatrix);
	SIZE windowSize;
	gOpenGLWindow->getWindowSize(windowSize);
	mat4_set_orthographic(&mProjectionMatrix, 0, (float)windowSize.cx, 0, (float)windowSize.cy, -1, 1);
}

FontRenderer::~FontRenderer()
{
	FontMapType::iterator it;
	for (it = mFonts.begin(); it != mFonts.end(); it++)
	{
		if (it->second)
		{
			texture_font_delete(it->second);
			it->second = NULL;
		}
	}

	if (mVertexBuffer)
	{
		vertex_buffer_delete(mVertexBuffer);
		mVertexBuffer = NULL;
	}
}

bool FontRenderer::render(wstring& inString, int inFontSize, TVector2f inPen, TVector4f inColor)
{
	int fontSize = inFontSize;

	// We only maintain fonts with size kMinFontSize, kMinFontSize+2, kMinFontSize+4, etc.
	constrain(fontSize, kMinFontSize, mLargestFontSize);

	FontMapType::iterator it = mFonts.find(fontSize);
	if (it == mFonts.end())
	{
		// If we don't find the font size go up one size if possible, otherwise go down one size.
		if (fontSize < mLargestFontSize - 1)
			fontSize++;
		else
			fontSize--;
		it = mFonts.find(fontSize);
	}
	if (it == mFonts.end())
	{
		LOG(ERROR) << "Font: " << mFontName << " " << inFontSize << " is not available.";
		return false;
	}

	// Clear the vertex buffer
	vertex_buffer_clear(mVertexBuffer);

	size_t numGlyphsRendered = 0;
	texture_font_t* font = it->second;
	float r = inColor.x, g = inColor.y, b = inColor.z, a = inColor.w;
	for (size_t i = 0; i < inString.length(); ++i)
	{
		texture_glyph_t* glyph = texture_font_get_glyph(font, inString[i]);
		if (glyph != NULL)
		{
			float kerning = 0.0f;
			if (i > 0)
			{
				kerning = texture_glyph_get_kerning(glyph, inString[i - 1]);
			}
			inPen.x += kerning;
			int x0 = (int)(inPen.x + glyph->offset_x);
			int y0 = (int)(inPen.y + glyph->offset_y);
			int x1 = (int)(x0 + glyph->width);
			int y1 = (int)(y0 - glyph->height);
			float s0 = glyph->s0;
			float t0 = glyph->t0;
			float s1 = glyph->s1;
			float t1 = glyph->t1;
			GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
			vertex_t vertices[4] = { { x0, y0, 0, s0, t0, r, g, b, a },
									 { x0, y1, 0, s0, t1, r, g, b, a },
									 { x1, y1, 0, s1, t1, r, g, b, a },
									 { x1, y0, 0, s1, t0, r, g, b, a } };
			vertex_buffer_push_back(mVertexBuffer, vertices, 4, indices, 6);
			inPen.x += glyph->advance_x;

			numGlyphsRendered++;
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mAtlas->id);

	glUseProgram(FontFactory::sShader);
	{
		glUniform1i(glGetUniformLocation(FontFactory::sShader, "texture"), 0);
		glUniformMatrix4fv(glGetUniformLocation(FontFactory::sShader, "model"), 1, 0, mModelMatrix.data);
		glUniformMatrix4fv(glGetUniformLocation(FontFactory::sShader, "view"), 1, 0, mViewMatrix.data);
		glUniformMatrix4fv(glGetUniformLocation(FontFactory::sShader, "projection"), 1, 0, mProjectionMatrix.data);
		vertex_buffer_render(mVertexBuffer, GL_TRIANGLES);
		glUseProgram(0);
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	return (numGlyphsRendered == inString.length());
}

string FontRenderer::getSystemFontFile(const string &inFontName) const
{
	static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
	HKEY hKey;
	LONG result;
	std::wstring wsFaceName(inFontName.begin(), inFontName.end());

	// Open Windows font registry key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fontRegistryPath, 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS)
		return "";

	DWORD maxValueNameSize, maxValueDataSize;
	result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
	if (result != ERROR_SUCCESS)
		return "";

	DWORD valueIndex = 0;
	LPWSTR valueName = new WCHAR[maxValueNameSize];
	LPBYTE valueData = new BYTE[maxValueDataSize];
	DWORD valueNameSize, valueDataSize, valueType;
	std::wstring wsFontFile;

	// Look for a matching font name
	do
	{
		wsFontFile.clear();
		valueDataSize = maxValueDataSize;
		valueNameSize = maxValueNameSize;
		result = RegEnumValue(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);
		valueIndex++;
		if (result != ERROR_SUCCESS || valueType != REG_SZ)
			continue;

		// Find a match
		wstring wsValueName(valueName, valueNameSize);
		if (_wcsnicmp(wsFaceName.c_str(), wsValueName.c_str(), wsFaceName.length()) == 0)
		{
			wsFontFile.assign((LPWSTR)valueData, valueDataSize);
			break;
		}
	} while (result != ERROR_NO_MORE_ITEMS);

	delete[] valueName;
	delete[] valueData;

	RegCloseKey(hKey);

	if (wsFontFile.empty())
		return "";

	// Build full font file path
	WCHAR winDir[MAX_PATH];
	GetWindowsDirectory(winDir, MAX_PATH);

	wstringstream ss;
	ss << winDir << "\\Fonts\\" << wsFontFile;
	wsFontFile = ss.str();

	return string(wsFontFile.begin(), wsFontFile.end());
}

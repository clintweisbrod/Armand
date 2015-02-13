// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// FontFactory.cpp
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

#include "stdafx.h"

#include "OpenGL/Fonts/FontFactory.h"
#include "OpenGL/Renderer.h"
#include "OpenGL/VertexBufferStructs.h"
#include "Math/mathlib.h"
#include "OpenGL/ShaderFactory.h"
#include "Utilities/StringUtils.h"

const int kMinFontSize = 10;	// Font smaller than this are pretty difficult to read

FontFactory::FontFactory()
{
	// Load the shader program we will use to draw fonts
	mShaderHandle = 0;
	ShaderProgram* shaderProg = ShaderFactory::inst()->getShaderProgram("FontFactory/v3f-t2f-c4f.vert",
																		"FontFactory/v3f-t2f-c4f.frag");
	if (shaderProg)
		mShaderHandle = shaderProg->getHandle();
}

FontFactory::~FontFactory()
{
	// Release all the FontRenderer instances
	FontRendererMap_t::iterator it;
	for (it = mRenderers.begin(); it != mRenderers.end(); it++)
	{
		if (it->second)
		{
			delete it->second;
			it->second = NULL;
		}
	}

	// Clean up the shader we've used.
	if (mShaderHandle)
		ShaderFactory::inst()->deleteShaderProgram(mShaderHandle);
}

FontRenderer* FontFactory::getFontRenderer(string& inFaceName)
{
	if (mShaderHandle == 0)
		return NULL;
	if (inFaceName.empty())
		return NULL;

	FontRenderer* result = NULL;
	FontRendererMap_t::iterator it = mRenderers.find(inFaceName);
	if (it == mRenderers.end())
	{
		// Create the renderer
		FontRenderer* newRenderer = new FontRenderer(inFaceName, mShaderHandle);
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

// This should be called anytime the scene size is changed
void FontFactory::sceneSizeChanged(Vec2i& inNewSceneSize)
{
	// Call setSceneSize() on each FontRenderer instance
	FontRendererMap_t::iterator it;
	for (it = mRenderers.begin(); it != mRenderers.end(); it++)
	{
		if (it->second)
			it->second->setSceneSize(inNewSceneSize);
	}
}

////////////////////////////

FontRenderer::FontRenderer(string& inFontName, GLuint inShader) : mAtlas(NULL),
																  mVertexBuffer(NULL),
																  mLargestFontSize(0),
																  mShaderHandle(inShader)
{
	// Locate system font file
	string fontFileName = getSystemFontFile(inFontName);
	if (fontFileName.empty())
	{
		LOG(ERROR) << "Unable to locate font: " << inFontName << ".";
		return;
	}

	// Build texture atlas
	mFontName = inFontName;
	mAtlas = texture_atlas_new(512, 512, 1);
	if (mAtlas == NULL)
	{
		LOG(ERROR) << "Unable to create texture font atlas for font: " << inFontName << ".";
		return;
	}

	// Fill theGlyphs with all reasonable characters that might be used.
	wstring glyphBuf;
	for (wchar_t i = 32; i <= 126; i++)
		glyphBuf.append(1, i);
	glyphBuf.append(1, 176);	// Degree symbol

	// Populate mFonts
	const char* filename = fontFileName.c_str();
	const int kMaxFontSize = 50;	// Highly unlikely we will get around to this font size.
	for (int i = kMinFontSize; i <= kMaxFontSize; i += 2)
	{
		texture_font_t* font = texture_font_new_from_file(mAtlas, (float)i, filename);
		if (texture_font_load_glyphs(font, glyphBuf.c_str()) == 0)
			mFonts[i] = font;
		else
		{
			mLargestFontSize = i - 2;
			LOG(INFO) << "Texture font atlas contains fonts up to size " << mLargestFontSize << ".";
			break;
		}
	}

	// Create a new vertex buffer
	mVertexBuffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");

	// Setup transform matrices
	Mat4f::setIdentity(mModelMatrix);
	Mat4f::setIdentity(mViewMatrix);
	Vec2i sceneSize;
	gRenderer->getSceneSize(sceneSize);
	setSceneSize(sceneSize);
}

FontRenderer::~FontRenderer()
{
	// Release font texture
	FontMap_t::iterator it;
	for (it = mFonts.begin(); it != mFonts.end(); it++)
	{
		if (it->second)
		{
			texture_font_delete(it->second);
			it->second = NULL;
		}
	}

	// Release vertex buffer
	if (mVertexBuffer)
	{
		vertex_buffer_delete(mVertexBuffer);
		mVertexBuffer = NULL;
	}
}

texture_font_t* FontRenderer::getFontTexture(int inFontSize)
{
	// We only maintain fonts with size kMinFontSize, kMinFontSize+2, kMinFontSize+4, etc.
	int fontSize = inFontSize;
	Mathi::constrain(fontSize, kMinFontSize, mLargestFontSize);

	FontMap_t::iterator it = mFonts.find(fontSize);
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
		return NULL;
	}

	return it->second;
}

size_t FontRenderer::buildLeftJustifiedLinear(wstring& inString, Vec4f& inColor, texture_font_t* inFont)
{
	size_t result = 0;

	// Generate new vertex buffer
	float_t xPos = 0;
	float_t r = inColor.r, g = inColor.g, b = inColor.b, a = inColor.a;
	for (size_t i = 0; i < inString.length(); ++i)
	{
		texture_glyph_t* glyph = texture_font_get_glyph(inFont, inString[i]);
		if (glyph != NULL)
		{
			// Get kerning
			float_t kerning = 0.0f;
			if (i > 0)
				kerning = texture_glyph_get_kerning(glyph, inString[i - 1]);
			xPos += kerning;

			// Compute location of glyph
			float_t x0 = xPos + glyph->offset_x;
			float_t x1 = x0 + glyph->width;
			float_t y0 = (float_t)glyph->offset_y;
			float_t y1 = y0 - glyph->height;

			// Texture coords
			float_t s0 = glyph->s0;
			float_t t0 = glyph->t0;
			float_t s1 = glyph->s1;
			float_t t1 = glyph->t1;
			GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
			v3f_t2f_c4f vertices[4] = { { x0, y0, 0, s0, t0, r, g, b, a },
										{ x0, y1, 0, s0, t1, r, g, b, a },
										{ x1, y1, 0, s1, t1, r, g, b, a },
										{ x1, y0, 0, s1, t0, r, g, b, a } };
			vertex_buffer_push_back(mVertexBuffer, vertices, 4, indices, 6);

			// Advance to position of next glyph
			xPos += glyph->advance_x;

			result++;
		}
	}

	return result;
}

size_t FontRenderer::buildRightJustifiedLinear(wstring& inString, Vec4f& inColor, texture_font_t* inFont)
{
	size_t result = 0;

	// Generate new vertex buffer
	float_t xPos = 0;
	float_t r = inColor.r, g = inColor.g, b = inColor.b, a = inColor.a;
	size_t strLen = inString.length();
	for (size_t i = 0; i < strLen; ++i)
	{
		size_t index = strLen - i - 1;
		texture_glyph_t* glyph = texture_font_get_glyph(inFont, inString[index]);
		if (glyph != NULL)
		{
			// Compute location of glyph
			float_t x0 = xPos - glyph->advance_x + (float_t)glyph->offset_x;
			float_t x1 = x0 + glyph->width;
			float_t y0 = (float_t)glyph->offset_y;
			float_t y1 = y0 - glyph->height;

			// Texture coords
			float_t s0 = glyph->s0;
			float_t t0 = glyph->t0;
			float_t s1 = glyph->s1;
			float_t t1 = glyph->t1;
			GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
			v3f_t2f_c4f vertices[4] = { { x0, y0, 0, s0, t0, r, g, b, a },
										{ x0, y1, 0, s0, t1, r, g, b, a },
										{ x1, y1, 0, s1, t1, r, g, b, a },
										{ x1, y0, 0, s1, t0, r, g, b, a } };
			vertex_buffer_push_back(mVertexBuffer, vertices, 4, indices, 6);

			// Get kerning
			float_t kerning = 0.0f;
			if (index > 0)
				kerning = texture_glyph_get_kerning(glyph, inString[index - 1]);

			// Advance to position of next glyph
			xPos -= (glyph->advance_x + kerning);

			result++;
		}
	}

	return result;
}

bool FontRenderer::render(wstring& inString, int inFontSize, Vec2f& inPen, Vec4f& inColor, float inRotationInDegrees, bool inRightJustify)
{
	if (!mShaderHandle)
		return false;

	texture_font_t* font = getFontTexture(inFontSize);
	if (!font)
		return false;

	// Clear the vertex buffer
	vertex_buffer_clear(mVertexBuffer);

	// Build the right set of vertices
	size_t numGlyphsRendered = 0;
	if (inRightJustify)
		numGlyphsRendered = buildRightJustifiedLinear(inString, inColor, font);
	else
		numGlyphsRendered = buildLeftJustifiedLinear(inString, inColor, font);

	// Apply rotation and translation transformations
	Mat4f::setRotationZ(mModelMatrix, degToRad(inRotationInDegrees));
	Mat4f::setTranslation(mViewMatrix, Vec3f(inPen.x, inPen.y, 0));

	// Enable blending and blend function
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable texturing and bind to atlas texture
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mAtlas->id);

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	// Activate the font shader
	glUseProgram(mShaderHandle);
	{
		glUniform1i(glGetUniformLocation(mShaderHandle, "texture"), 0);
		glUniformMatrix4fv(glGetUniformLocation(mShaderHandle, "model"), 1, 0, mModelMatrix.data);
		glUniformMatrix4fv(glGetUniformLocation(mShaderHandle, "view"), 1, 0, mViewMatrix.data);
		glUniformMatrix4fv(glGetUniformLocation(mShaderHandle, "projection"), 1, 0, mProjectionMatrix.data);
		vertex_buffer_render(mVertexBuffer, GL_TRIANGLES);

		// Deactivate the shader
		glUseProgram(0);
	}

	glPopClientAttrib();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	return (numGlyphsRendered == inString.length());
}

size_t FontRenderer::buildLeftJustifiedSpherical(wstring& inString, Vec4f& inColor, Vec2f& inPen, texture_font_t* inFont)
{
	size_t result = 0;

	float_t azimuth = inPen.x;
	float_t altitude = inPen.y;
	Vec2i sceneSize;
	gRenderer->getSceneSize(sceneSize);
	Point2f halfSceneSize((float_t)sceneSize.x / 2, (float_t)sceneSize.y / 2);
	float_t geometryRadius = (float_t)gRenderer->getGeometryRadius();
	float_t radial = geometryRadius * (float_t)((kHalfPi - altitude) / kHalfPi);
	float_t r = inColor.r, g = inColor.g, b = inColor.b, a = inColor.a;
	for (size_t i = 0; i < inString.length(); ++i)
	{
		texture_glyph_t* glyph = texture_font_get_glyph(inFont, inString[i]);
		if (glyph != NULL)
		{
			// Get kerning
			float_t kerning = 0.0f;
			if (i > 0)
				kerning = texture_glyph_get_kerning(glyph, inString[i - 1]);

			// Compute location of glyph
			float_t x0 = (float_t)glyph->offset_x;
			float_t x1 = x0 + glyph->width;
			float_t y0 = (float_t)glyph->offset_y;
			float_t y1 = y0 - glyph->height;

			// Rotate points by azimuth.
			Point2f p1(x0, y0);
			Point2f p2(x0, y1);
			Point2f p3(x1, y1);
			Point2f p4(x1, y0);
			Mat2f rotation = Mat2f::rotation(azimuth);
			p1 = rotation * p1;
			p2 = rotation * p2;
			p3 = rotation * p3;
			p4 = rotation * p4;

			// Translate glyph to center of screen
			p1 += halfSceneSize;
			p2 += halfSceneSize;
			p3 += halfSceneSize;
			p4 += halfSceneSize;

			// Now translate to correct location represented by azimuth and altitude
			Point2f t(radial * rotation.m10, -radial * rotation.m00);	// To avoid making two more trig calls
			p1 += t;
			p2 += t;
			p3 += t;
			p4 += t;

			// Texture coords
			float_t s0 = glyph->s0;
			float_t t0 = glyph->t0;
			float_t s1 = glyph->s1;
			float_t t1 = glyph->t1;
			GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
			v3f_t2f_c4f vertices[4] = { { p1.x, p1.y, 0, s0, t0, r, g, b, a },
										{ p2.x, p2.y, 0, s0, t1, r, g, b, a },
										{ p3.x, p3.y, 0, s1, t1, r, g, b, a },
										{ p4.x, p4.y, 0, s1, t0, r, g, b, a } };
			vertex_buffer_push_back(mVertexBuffer, vertices, 4, indices, 6);

			// Use glyph->advance_x and kerning to compute new value of azimuth
			azimuth += ((glyph->advance_x + kerning) / geometryRadius);
			azimuth = pfmod(azimuth, (float_t)kTwicePi);
			result++;
		}
	}

	return result;
}

size_t FontRenderer::buildRightJustifiedSpherical(wstring& inString, Vec4f& inColor, Vec2f& inPen, texture_font_t* inFont)
{
	size_t result = 0;

	float_t azimuth = inPen.x;
	float_t altitude = inPen.y;
	Vec2i sceneSize;
	gRenderer->getSceneSize(sceneSize);
	Point2f halfSceneSize((float_t)sceneSize.x / 2, (float_t)sceneSize.y / 2);
	float_t geometryRadius = (float_t)gRenderer->getGeometryRadius();
	float_t radial = geometryRadius * (float_t)((kHalfPi - altitude) / kHalfPi);
	float_t r = inColor.r, g = inColor.g, b = inColor.b, a = inColor.a;
	size_t strLen = inString.length();
	for (size_t i = 0; i < strLen; ++i)
	{
		size_t index = strLen - i - 1;
		texture_glyph_t* glyph = texture_font_get_glyph(inFont, inString[index]);
		if (glyph != NULL)
		{
			// Compute location of glyph
			float_t x0 = -glyph->advance_x + (float_t)glyph->offset_x;
			float_t x1 = x0 + glyph->width;
			float_t y0 = (float_t)glyph->offset_y;
			float_t y1 = y0 - glyph->height;

			// Rotate points by azimuth.
			Point2f p1(x0, y0);
			Point2f p2(x0, y1);
			Point2f p3(x1, y1);
			Point2f p4(x1, y0);
			Mat2f rotation = Mat2f::rotation(azimuth);
			p1 = rotation * p1;
			p2 = rotation * p2;
			p3 = rotation * p3;
			p4 = rotation * p4;

			// Translate glyph to center of screen
			p1 += halfSceneSize;
			p2 += halfSceneSize;
			p3 += halfSceneSize;
			p4 += halfSceneSize;

			// Now translate to correct location represented by azimuth and altitude
			Point2f t(radial * rotation.m10, -radial * rotation.m00);	// To avoid making two more trig calls
			p1 += t;
			p2 += t;
			p3 += t;
			p4 += t;

			// Texture coords
			float_t s0 = glyph->s0;
			float_t t0 = glyph->t0;
			float_t s1 = glyph->s1;
			float_t t1 = glyph->t1;
			GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
			v3f_t2f_c4f vertices[4] = { { p1.x, p1.y, 0, s0, t0, r, g, b, a },
										{ p2.x, p2.y, 0, s0, t1, r, g, b, a },
										{ p3.x, p3.y, 0, s1, t1, r, g, b, a },
										{ p4.x, p4.y, 0, s1, t0, r, g, b, a } };
			vertex_buffer_push_back(mVertexBuffer, vertices, 4, indices, 6);

			// Get kerning
			float_t kerning = 0.0f;
			if (index > 0)
				kerning = texture_glyph_get_kerning(glyph, inString[index - 1]);

			// Use glyph->advance_x and kerning to compute new value of azimuth
			azimuth -= ((glyph->advance_x + kerning) / geometryRadius);
			azimuth = pfmod(azimuth, (float_t)kTwicePi);
			result++;
		}
	}

	return result;
}

// TODO: As noted in comments, text is currently always rendered in increasing azimuth,
// starting from the pspecified pen location. It would be really helpful to be able to
// render the text from right to left (right-justification)

// inPen.x is interpreted as azimuth
// inPen.y is interpreted as altitude
// Azimuth is [0, 2pi). 0 is at bottom of screen (front of dome) and moves counterclockwise
// Altitude is [0, pi/2]. 0 is at dome edge. 
bool FontRenderer::renderSpherical(wstring& inString, int inFontSize, Vec2f& inPen, Vec4f& inColor, bool inRightJustify)
{
	if (!mShaderHandle)
		return false;

	texture_font_t* font = getFontTexture(inFontSize);
	if (!font)
		return false;

	// So, we have a projection matrix setup such that OpenGL screen coordinates prevail.
	// Origin located at bottom-left. We are provided with altitude and azimuth values.
	// Text will ALWAYS be rendered in increasing azimuth and constant altitude, starting
	// at the given azimuth and altitude. The trick to getting this right is computing the
	// amount of rotation that must be applied to each glyph so that they appear upright in
	// the dome. This is often referred to as "gravity" mode.

	// Clear the vertex buffer
	vertex_buffer_clear(mVertexBuffer);

	// Build the right set of vertices
	size_t numGlyphsRendered = 0;
	if (inRightJustify)
		numGlyphsRendered = buildRightJustifiedSpherical(inString, inColor, inPen, font);
	else
		numGlyphsRendered = buildLeftJustifiedSpherical(inString, inColor, inPen, font);

	// Rotation and translation matrices are simply identity since we've done all
	// the math above.
	Mat4f::setIdentity(mModelMatrix);
	Mat4f::setIdentity(mViewMatrix);

	// Enable blending and blend function
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable texturing and bind to atlas texture
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mAtlas->id);

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	// Activate the font shader
	glUseProgram(mShaderHandle);
	{
		glUniform1i(glGetUniformLocation(mShaderHandle, "texture"), 0);
		glUniformMatrix4fv(glGetUniformLocation(mShaderHandle, "model"), 1, 0, mModelMatrix.data);
		glUniformMatrix4fv(glGetUniformLocation(mShaderHandle, "view"), 1, 0, mViewMatrix.data);
		glUniformMatrix4fv(glGetUniformLocation(mShaderHandle, "projection"), 1, 0, mProjectionMatrix.data);
		vertex_buffer_render(mVertexBuffer, GL_TRIANGLES);

		// Deactivate the shader
		glUseProgram(0);
	}

	glPopClientAttrib();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	return (numGlyphsRendered == inString.length());
}

void FontRenderer::setSceneSize(Vec2i& inSceneSize)
{
	mProjectionMatrix = Mat4f::orthographic(0, (float)inSceneSize.x, 0, (float)inSceneSize.y, -1, 1);
}

string FontRenderer::getSystemFontFile(const string &inFontName) const
{
	static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
	HKEY hKey;
	LONG result;
	wstring wsFaceName(inFontName.begin(), inFontName.end());

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
	wstring wsFontFile;

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

	return stringFromWstring(wsFontFile);
}

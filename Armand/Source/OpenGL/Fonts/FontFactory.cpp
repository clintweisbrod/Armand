#include "stdafx.h"

#include <freetype-gl/freetype-gl.h>
#include <freetype-gl/mat4.h>
#include <freetype-gl/shader.h>
#include <freetype-gl/vertex-buffer.h>

#include "OpenGL/Fonts/FontFactory.h"

GLuint shader;
vertex_buffer_t *buffer;
mat4   model, view, projection;

typedef struct {
	float x, y, z;    // position
	float s, t;       // texture
	float r, g, b, a; // color
} vertex_t;


void add_text(vertex_buffer_t * buffer, texture_font_t * font, wchar_t * text, vec4 * color, vec2 * pen)
{
	size_t i;
	float r = color->red, g = color->green, b = color->blue, a = color->alpha;
	for (i = 0; i<wcslen(text); ++i)
	{
		texture_glyph_t *glyph = texture_font_get_glyph(font, text[i]);
		if (glyph != NULL)
		{
			float kerning = 0.0f;
			if (i > 0)
			{
				kerning = texture_glyph_get_kerning(glyph, text[i - 1]);
			}
			pen->x += kerning;
			int x0 = (int)(pen->x + glyph->offset_x);
			int y0 = (int)(pen->y + glyph->offset_y);
			int x1 = (int)(x0 + glyph->width);
			int y1 = (int)(y0 - glyph->height);
			float s0 = glyph->s0;
			float t0 = glyph->t0;
			float s1 = glyph->s1;
			float t1 = glyph->t1;
			GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
			vertex_t vertices[4] = {	{ x0, y0, 0, s0, t0, r, g, b, a },
										{ x0, y1, 0, s0, t1, r, g, b, a },
										{ x1, y1, 0, s1, t1, r, g, b, a },
										{ x1, y0, 0, s1, t0, r, g, b, a } };
			vertex_buffer_push_back(buffer, vertices, 4, indices, 6);
			pen->x += glyph->advance_x;
		}
	}
}

FontFactory::FontFactory()
{
	// Freetype library one-time initialization
	if (FT_Init_FreeType(&m_Ft))
	{
		LOG(ERROR) << "Failed to initialize Freetype library.";
		return;
	}

	size_t i;
	texture_font_t *font = 0;
	texture_atlas_t *atlas = texture_atlas_new(512, 512, 1);

	//	const char* filename = "fonts/Vera.ttf";
	string fontFileName = getSystemFontFile("Verdana");
	if (fontFileName.length() == 0)
		return;
	const char* filename = fontFileName.c_str();

	wchar_t *theText = L"A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";
	buffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");
	vec2 pen = { { 200, 800 } };
	vec4 white = { { 1, 1, 1, 1 } };
	for (i = 7; i < 27; ++i)
	{
		font = texture_font_new_from_file(atlas, i, filename);
		pen.x = 5;
		pen.y -= font->height;
		texture_font_load_glyphs(font, theText);
		add_text(buffer, font, theText, &white, &pen);
		texture_font_delete(font);
	}
	glBindTexture(GL_TEXTURE_2D, atlas->id);

	shader = shader_load("shaders/v3f-t2f-c4f.vert", "shaders/v3f-t2f-c4f.frag");
	mat4_set_identity(&projection);
	mat4_set_identity(&model);
	mat4_set_identity(&view);


/*
	string fontFileName = getSystemFontFile("Verdana");
	if (fontFileName.length() == 0)
		return;

	if (FT_New_Face(m_Ft, fontFileName.c_str(), 0, &face))
	{
		LOG(ERROR) << "Freetype library failed to open " << fontFileName;
		return;
	}
	g = face->glyph;
	FT_Set_Pixel_Sizes(face, 0, 30);
	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

//	glTexImage2D(
//		GL_TEXTURE_2D,
//		0,
//		GL_ALPHA,
//		g->bitmap.width,
//		g->bitmap.rows,
//		0,
//		GL_ALPHA,
//		GL_UNSIGNED_BYTE,
//		g->bitmap.buffer
//		);
*/
}

FontFactory::~FontFactory()
{
}




void FontFactory::render_text(SIZE inWindowSize)
//void FontFactory::render_text(const char *text, float x, float y, float sx, float sy)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
/*
	GLfloat x = 100;
	GLfloat y = 600;
	GLfloat size = 512;
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(x, y);
	glTexCoord2f(0, 1); glVertex2f(x, y - size);
	glTexCoord2f(1, 1); glVertex2f(x + size, y - size);
	glTexCoord2f(1, 0); glVertex2f(x + size, y);
	glEnd();
*/

	mat4_set_orthographic(&projection, 0, inWindowSize.cx, 0, inWindowSize.cy, -1, 1);

	glUseProgram(shader);
	{
		glUniform1i(glGetUniformLocation(shader, "texture"), 0);
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, 0, model.data);
		glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, 0, view.data);
		glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, 0, projection.data);
		vertex_buffer_render(buffer, GL_TRIANGLES);
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

/*
void FontFactory::render_text(const char *text, float x, float y, float sx, float sy)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	const char *p;

	for (p = text; *p; p++)
	{
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
			continue;

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_ALPHA,
			g->bitmap.width,
			g->bitmap.rows,
			0,
			GL_ALPHA,
			GL_UNSIGNED_BYTE,
			g->bitmap.buffer
			);

		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, 0); glVertex2f(x2, -y2);
		glTexCoord2f(1, 0); glVertex2f(x2 + w, -y2);
		glTexCoord2f(0, 1); glVertex2f(x2, -y2 - h);
		glTexCoord2f(1, 1); glVertex2f(x2 + w, -y2 - h);
		glEnd();

		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}

	glDisable(GL_TEXTURE_2D);
}

void FontFactory::renderString(string inString)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float x = 100;
	float y = 100;
	float sx = 1;
	float sy = 1;

	float x2 = x + g->bitmap_left * sx;
	float y2 = -y - g->bitmap_top * sy;
	float w = g->bitmap.width * sx;
	float h = g->bitmap.rows * sy;

	glEnable(GL_TEXTURE_2D);
	
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, 0); glVertex2f(x2,     -y2);
		glTexCoord2f(1, 0); glVertex2f(x2 + w, -y2);
		glTexCoord2f(0, 1); glVertex2f(x2,     -y2 - h);
		glTexCoord2f(1, 1); glVertex2f(x2 + w, -y2 - h);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}
*/

string FontFactory::getSystemFontFile(const string &faceName)
{
	static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
	HKEY hKey;
	LONG result;
	std::wstring wsFaceName(faceName.begin(), faceName.end());

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

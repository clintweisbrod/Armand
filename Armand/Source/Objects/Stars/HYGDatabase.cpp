// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// HYGDatabase.cpp
//
// See: http://www.astronexus.com/hyg
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

#include <fstream>
#include <sstream>
#include "HYGDatabase.h"
#include "Utilities/File.h"
#include "Utilities/StringUtils.h"
#include "Math/mathlib.h"
#include "OpenGL/ShaderFactory.h"

HYGDatabase::HYGDatabase()
{
	// Position the center of the data set at the origin.
	setUniveralPositionAU(Vec3d(0, 0, 0));

	// Modest amount of saturation for stars
	mPointSaturation = 0.75f;

	// Load point sprite texture
	mPointTexture = new Texture(File::getStarImagesFolder().append("/Default.png"));
	if (mPointTexture->getImageBufferOK())
		mPointTexture->sendToGPU();

	// Obtain the shader
	ShaderProgram* shaderProgram = ShaderFactory::inst()->getShaderProgram("Stars/PointStars.vert", "Stars/PointSpriteStars.frag");
	if (shaderProgram)
		mPointShaderHandle = shaderProgram->getHandle();

	// Load the data
	loadData();

	// Compute average color of points and setup VBO
	finalize();
}

HYGDatabase::~HYGDatabase()
{
}

void HYGDatabase::loadData()
{
	// Attempt to open data file
	string hygDataFilePath = File::getDataFolder().append("/hygdata_v3.csv");
	ifstream file(hygDataFilePath);
	if (!file.is_open())
		THROW(HYGDatabaseException, "Unable to open %s", hygDataFilePath.c_str());

	LOG(INFO) << "Parsing HYG database...";

	size_t kMaxStarsToParse = 1000;
	string line;
	getline(file, line);	// First line of database is hdr describing fields
	Vec3f sunPosition;
	while (getline(file, line))
	{
		// Parse the CSV line into a vector of strings
		istringstream lineStream(line);
		vector<string> lineValues;
		string value;
		while (getline(lineStream, value, ','))
			lineValues.push_back(value);

		// Just show me the dipper
//		if (!((lineValues[6] == "Alkaid") || (lineValues[6] == "Mizar") || (lineValues[6] == "Alioth") ||
//			(lineValues[6] == "Megrez") || (lineValues[6] == "Phad") || (lineValues[6] == "Merak") ||
//			(lineValues[6] == "Dubhe")))
//			continue;

		// Skip any records that have distance of "100000.0000". These stars have dubious parallax values.
		if (lineValues[9] == "100000.0000")
			continue;

		ostringstream s;
		if (!lineValues[1].empty())	// Do we have a HIP id?
			s << "HIP: " << lineValues[1];
		else if (!lineValues[2].empty())
			s << "HD: " << lineValues[2];
		else if (!lineValues[3].empty())
			s << "HR: " << lineValues[3];
		else if (!lineValues[4].empty())
			s << "Gliese: " << lineValues[4];
		else if (!lineValues[5].empty())
			s << "Bayer / Flamsteed: " << lineValues[5];
		else if (!lineValues[6].empty())
			s << "Proper Name: " << lineValues[6];
		string identifier = s.str();
		bool haveID = !identifier.empty();

		// Absolute magnitude
		float_t absMag;
		bool haveAbsMag = floatFromString(lineValues[14], absMag);

		// Color index
		float_t colorIndex = 0.5f;
		floatFromString(lineValues[16], colorIndex);

		// Position
		Vec3f position;
		bool havePosition = false;
		float_t x, y, z;
		if (floatFromString(lineValues[17], x) && floatFromString(lineValues[18], y) && floatFromString(lineValues[19], z))
		{
			position = Vec3f(x * (float_t)kAuPerParsec, y * (float_t)kAuPerParsec, z * (float_t)kAuPerParsec);
			if (lineValues[0] == "0")
				sunPosition = position;
			havePosition = true;
		}

		if (haveID && havePosition && haveAbsMag)
		{
			HYGDataRecord rec;

			rec.mProperName = lineValues[6];
			rec.mIdentifier = identifier;
			rec.mPosition = position - sunPosition;		// Translate position to Sun-centric
			rec.mAbsMag = absMag;
			rec.mColorIndex = colorIndex;

			mData.push_back(rec);

#if _DEBUG
			// We limit the number to parse in debug mode because Microsoft's implementation
			// of STL is so bloody slow!
			if (mData.size() == kMaxStarsToParse)
				break;
#endif
		}
	}

	LOG(INFO) << "Parsed " << mData.size() << " stars from HYG database.";

	// Possibly reduce storage requirements for all the data we've allocated
	mData.shrink_to_fit();

	// Allocate required buffer
	allocatePointArray((GLsizei)mData.size(), sizeof(ColorPointVertex));

	// Write data into render storage
	int n = 0;
	for (HYGData::iterator it = mData.begin(); it != mData.end(); it++)
	{
		ColorPointVertex* starVertex = (ColorPointVertex*)getVertex(n);

		starVertex->position[0] = it->mPosition.x;
		starVertex->position[1] = it->mPosition.y;
		starVertex->position[2] = it->mPosition.z;

		float_t r, g, b;
		bv2rgb(it->mColorIndex, r, g, b);
		starVertex->color[0] = (GLubyte)(255 * r);
		starVertex->color[1] = (GLubyte)(255 * g);
		starVertex->color[2] = (GLubyte)(255 * b);

		starVertex->absMag = it->mAbsMag;
		n++;
	}
}

//
// See: http://tiku.io/questions/764303/star-b-v-color-index-to-apparent-rgb-color
//
void HYGDatabase::bv2rgb(float_t bv, float_t &r, float_t &g, float_t &b)
{
	// RGB <0,1> <- BV <-0.4,+2.0>
	r = g = b = 0.0f;

	Mathf::constrain(bv, -0.4f, 2.0f);

	float_t t;
	if ((bv >= -0.4f) && (bv < 0.0f))
	{
		t = (bv + 0.4f) / (0.0f + 0.4f);
		r = 0.61f + (0.11f * t) + (0.1f * t * t);
	}
	else if ((bv >= 0.0f ) && (bv < 0.4f))
	{
		t = (bv - 0.0f) / (0.4f - 0.0f);
		r = 0.83f + (0.17f * t);
	}
	else if ((bv >= 0.4f) && (bv < 2.1f))
	{
		t = (bv - 0.4f) / (2.1f - 0.4f);
		r = 1.0f;
	}
	
	if ((bv >= -0.4f) && (bv < 0.0f))
	{
		t = (bv + 0.4f) / (0.0f + 0.4f);
		g = 0.7f + (0.07f * t) + (0.1f * t * t);
	}
	else if ((bv >= 0.0f) && (bv < 0.4f))
	{
		t = (bv - 0.0f) / (0.4f - 0.0f);
		g = 0.87f + (0.11f * t);
	}
	else if ((bv >= 0.4f) && (bv < 1.6f))
	{
		t = (bv - 0.4f) / (1.6f - 0.4f);
		g = 0.98f - (0.16f * t);
	}
	else if ((bv >= 1.6f) && (bv < 2.0f))
	{
		t = (bv - 1.6f) / (2.0f - 1.6f);
		g = 0.82f - (0.5f * t * t);
	}
	if ((bv >= -0.4f) && (bv < 0.4f))
	{
		t = (bv + 0.4f) / (0.4f + 0.4f);
		b = 1.0f;
	}
	else if ((bv >= 0.4f) && (bv < 1.5f))
	{
		t = (bv - 0.4f) / (1.5f - 0.4f);
		b = 1.0f - (0.47f * t) + (0.1f * t * t);
	}
	else if ((bv >= 1.5f) && (bv < 1.94f))
	{
		t = (bv - 1.5f) / (1.94f - 1.5f);
		b = 0.63f - (0.6f * t * t);
	}
}

void HYGDatabase::setupVAO()
{
	// Setup the points VBO
	if (mPointsVAO == NULL)
	{
		mPointsVAO = new VAOBuilder;

		// Add the arrays
		mPointsVAO->addArray("vaoPosition", 3, GL_FLOAT, GL_FALSE);
		mPointsVAO->addArray("vaoColor", 3, GL_UNSIGNED_BYTE, GL_TRUE);
		mPointsVAO->addArray("vaoAbsMag", 1, GL_FLOAT, GL_FALSE);
	}
}

void HYGDatabase::setGLStateForFullRender(float inAlpha) const
{
	setGLStateForPoint(inAlpha);

	// Enable point sprites
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glEnable(GL_POINT_SPRITE);
}

void HYGDatabase::setPointShaderUniforms(Camera& inCamera, float inAlpha)
{
	RenderObject::setPointShaderUniforms(inCamera, inAlpha);

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, mPointTexture->getTextureID());
	glUniform1i(glGetUniformLocation(mPointShaderHandle, "uTexture"), 0);
	glUniform1f(glGetUniformLocation(mPointShaderHandle, "uSaturation"), mPointSaturation);
}

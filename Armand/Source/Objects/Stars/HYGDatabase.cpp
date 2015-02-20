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
#include <limits>
#include "HYGDatabase.h"
#include "3DStar.h"
#include "Utilities/File.h"
#include "Utilities/StringUtils.h"
#include "Math/mathlib.h"
#include "OpenGL/ShaderFactory.h"

const float_t kLimitingStarMagnitude = 7.0f;	// Will eventually be a preference

HYGDatabase::HYGDatabase() : mChunkedData(NULL)
{
	mVBOUsage = GL_DYNAMIC_DRAW;

	// Position the center of the data set at the origin.
	setUniveralPositionAU(Vec3f(0, 0, 0));

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

	// Break data into chunks so that we can quickly determine which star is closest to viewer
	chunkData();
}

HYGDatabase::~HYGDatabase()
{
	if (mChunkedData)
		delete[] mChunkedData;
}

void HYGDatabase::loadData()
{
	// Attempt to open data file
	string hygDataFilePath = File::getDataFolder().append("/hygdata_v3.csv");
	ifstream file(hygDataFilePath);
	if (!file.is_open())
		THROW(HYGDatabaseException, "Unable to open %s", hygDataFilePath.c_str());

	LOG(INFO) << "Parsing HYG database...";

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
		if (!lineValues[6].empty())
			s << lineValues[6];
		else if (!lineValues[1].empty())	// Do we have a HIP id?
			s << "HIP: " << lineValues[1];
		else if (!lineValues[2].empty())
			s << "HD: " << lineValues[2];
		else if (!lineValues[3].empty())
			s << "HR: " << lineValues[3];
		else if (!lineValues[4].empty())
			s << "Gliese: " << lineValues[4];
		else if (!lineValues[5].empty())
			s << "Bayer / Flamsteed: " << lineValues[5];
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
			rec.mEyeDistanceSq = -1;
			rec.mVBOIndex = mData.size();

			mData.push_back(rec);

#if _DEBUG
			// We limit the number to parse in debug mode because Microsoft's implementation
			// of STL is so bloody slow!
			size_t kMaxStarsToParse = 1000;
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
	for (HYGDataVec_t::iterator it = mData.begin(); it != mData.end(); it++)
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
		starVertex->color[3] = 255;

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

// This method performs basic chunking of the star database. Here's the problem:
// Relative to the camera location, we need to know which star is nearest so that we can
// render the star as a 3D star when close enough. The star database has over a million
// entries so it would be insane to test the distances to every one of these stars from
// the camera per frame. Instead, we imagine the star database contained in a cube with
// sides of 2 * mBoundingRadius. We then equally subdivide this cube into many smaller
// cubes (or chunks) determined by fChunkDivisions. If fChunkDivisions were 3, we'd have
// 27 chunks, like a Rubik's cube. We then place each star in it's appropriate chunk. It's
// easy to compute which chunk the camera is in at any time. It's also easy to compute the
// six (or less) adjacent chunks. Given these chunks, we only need to find the nearest
// star among them, rather than the entire database. fChunkDivisions is chosen so that
// getNearestToPosition() runtime is minimized. fChunkDivisions = 20 seems to do it.
void HYGDatabase::chunkData()
{
	mChunkDivisions = 20;
	mChunkDivisionsSq = mChunkDivisions * mChunkDivisions;

	// Allocate the chunked data. Each chunk is a vector<HYGDataRecord*>, referring to
	// the HYGDataRecord items in mData.
	const int kNumChunks = mChunkDivisions * mChunkDivisionsSq;
	mChunkSize = 2 * mBoundingRadiusAU / mChunkDivisions;
	mChunkedData = new HYGDataVecP_t[kNumChunks];

	// Place each star in the correct chunk
	for (HYGDataVec_t::iterator it = mData.begin(); it != mData.end(); it++)
	{
		int index = getChunkIndexFromPosition(it->mPosition);
		mChunkedData[index].push_back(&(*it));
	}
}

int HYGDatabase::getChunkIndexFromArrayIndices(int i, int j, int k) const
{
	if ((i >= 0) && (i < mChunkDivisions) &&
		(j >= 0) && (j < mChunkDivisions) &&
		(k >= 0) && (k < mChunkDivisions))
		return (i + mChunkDivisions * (j + mChunkDivisions * k));
	else
		return -1;
}

int HYGDatabase::getChunkIndexFromPosition(Vec3f& inPosition) const
{
	int i = (int)((inPosition.x + mBoundingRadiusAU) / mChunkSize);
	int j = (int)((inPosition.y + mBoundingRadiusAU) / mChunkSize);
	int k = (int)((inPosition.z + mBoundingRadiusAU) / mChunkSize);

	return getChunkIndexFromArrayIndices(i, j, k);
}

// This method returns inNumStarsToReturn star records in ioNearestStars in ascending distance from inPosition.
void HYGDatabase::computeNearest(Camera& inCamera, size_t inNumStarsToReturn)
{
	mNearestStarsToViewer.clear();

	// Get chunk index containing the given position
	int index = getChunkIndexFromPosition((Vec3f)inCamera.getUniveralPositionAU());
	if (index == -1)
		return;

	// Get the chunk indices for chunks adjacent to the chunk containing the given position
	vector<int> nearestIndices;
	getAdjacentChunkIndices(index, nearestIndices);

	// Add the chunk index containing the given position 
	nearestIndices.push_back(index);

	// Accumulate all the records contained in each chunk
	HYGDataVecP_t nearestRecs;
	const float_t kMaxDistance = (std::numeric_limits<float_t>::max)();
	for (vector<int>::iterator it = nearestIndices.begin(); it != nearestIndices.end(); it++)
	{
		for (HYGDataVecP_t::iterator itData = mChunkedData[*it].begin(); itData != mChunkedData[*it].end(); itData++)
		{
			(*itData)->mEyeDistanceSq = kMaxDistance;
			nearestRecs.push_back(*itData);
		}
	}

	// nearestRecs now contains all data records in the chunk where inPosition is located, as well as
	// the data records from adjacent chunks.
	// Populate ioNearestStars with inNumStarsToReturn in ascending distance from inPosition.
	if (!nearestRecs.empty())
	{
		float_t nearestDistanceSq = (std::numeric_limits<float_t>::max)();

		Vec3f cameraPos = (Vec3f)inCamera.getUniveralPositionAU();
		for (HYGDataVecP_t::iterator it = nearestRecs.begin(); it != nearestRecs.end(); it++)
		{
			HYGDataRecord* rec = *it;

			// The inCamera.getCameraRelativePosition() calculation of diff is EXPENSIVE
			// but maybe unnecessary. As we get further from the origin of the dataset,
			// in order to compute the distance between the camera and the candidate star,
			// we must compute the difference between two nearly identical vectors. Unless,
			// we store the vectors as Vec3Big instances, we will run into precision
			// problems. However, we're only concerned with finding the closest star(s)
			// and not with exactly how far away they are, so it may be fine to leave
			// this calculation in Vec3f space.
//			Vec3f diff = inCamera.getCameraRelativePosition(rec->mPosition);
			Vec3f diff = rec->mPosition - cameraPos;
			rec->mEyeDistanceSq = diff.lengthSquared();

			if (mNearestStarsToViewer.size() == 0)
				mNearestStarsToViewer.push_back(rec);
			else if (mNearestStarsToViewer.size() < inNumStarsToReturn)
			{
				for (HYGDataVecP_t::iterator it2 = mNearestStarsToViewer.begin(); it2 != mNearestStarsToViewer.end(); it2++)
				{
					if (rec->mEyeDistanceSq < (*it2)->mEyeDistanceSq)
					{
						mNearestStarsToViewer.insert(it2, rec);
						break;
					}
				}
			}
			else if (rec->mEyeDistanceSq < mNearestStarsToViewer[inNumStarsToReturn - 1]->mEyeDistanceSq)
			{
				for (HYGDataVecP_t::iterator it2 = mNearestStarsToViewer.begin(); it2 != mNearestStarsToViewer.end(); it2++)
				{
					if (rec->mEyeDistanceSq < (*it2)->mEyeDistanceSq)
					{
						mNearestStarsToViewer.insert(it2, rec);
						break;
					}
				}
				mNearestStarsToViewer.pop_back();
			}
		}
	}
}

void HYGDatabase::getAdjacentChunkIndices(int inIndex, vector<int>& ioAdjacentIndices) const
{
	int k = inIndex / mChunkDivisionsSq;
	int j = (inIndex - k * mChunkDivisionsSq) / mChunkDivisions;
	int i = inIndex - k * mChunkDivisionsSq - j * mChunkDivisions;

	if (i > 0)
		ioAdjacentIndices.push_back(getChunkIndexFromArrayIndices(i - 1, j, k));
	if (i < mChunkDivisions - 1)
		ioAdjacentIndices.push_back(getChunkIndexFromArrayIndices(i + 1, j, k));
	if (j > 0)
		ioAdjacentIndices.push_back(getChunkIndexFromArrayIndices(i, j - 1, k));
	if (j < mChunkDivisions - 1)
		ioAdjacentIndices.push_back(getChunkIndexFromArrayIndices(i, j + 1, k));
	if (k > 0)
		ioAdjacentIndices.push_back(getChunkIndexFromArrayIndices(i, j, k - 1));
	if (k < mChunkDivisions - 1)
		ioAdjacentIndices.push_back(getChunkIndexFromArrayIndices(i, j, k + 1));
}

HYGDataRecord* HYGDatabase::getNearestStar() const
{
	HYGDataRecord* result = NULL;

	if (!mNearestStarsToViewer.empty())
		result = mNearestStarsToViewer[0];

	return result;
}

void HYGDatabase::setupVAO()
{
	// Setup the points VBO
	if (mPointsVAO == NULL)
	{
		mPointsVAO = new VAOBuilder;

		// Add the arrays
		mPointsVAO->addArray("vaoPosition", 3, GL_FLOAT, GL_FALSE);
		mPointsVAO->addArray("vaoColor", 4, GL_UNSIGNED_BYTE, GL_TRUE);
		mPointsVAO->addArray("vaoAbsMag", 1, GL_FLOAT, GL_FALSE);
	}
}

void HYGDatabase::preRender(Camera& inCamera, RenderObjectList& ioRenderList)
{
	RenderObject::preRender(inCamera, ioRenderList);

	// Deallocate any 3D star's we allocated from last frame 
	for (RenderObjectVecP_t::iterator it = mNearest3DStars.begin(); it != mNearest3DStars.end(); it++)
		delete *it;
	mNearest3DStars.clear();

	// Update list of stars nearest to camera.
	// We have to render the nearby ones as their own RenderObject instances.
	computeNearest(inCamera, 3);

	// Turn-off the nearby stars by setting their alpha value to zero
	if (!mNearestStarsToViewer.empty())
	{
		// Use the mVBOIndex member of each item in mNearestStarsToViewer to
		// modify the VBO data so that the star has a alpha value of zero.
		glBindBuffer(GL_ARRAY_BUFFER, mPointsVBO);
		void* gpuBuf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (gpuBuf)
		{
			for (HYGDataVecP_t::iterator it = mNearestStarsToViewer.begin(); it != mNearestStarsToViewer.end(); it++)
			{
				ColorPointVertex* vertex = (ColorPointVertex*)gpuBuf + (*it)->mVBOIndex;
				vertex->color[3] = 0;
			}
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// Add 3D stars to ioRenderList
	for (HYGDataVecP_t::iterator it = mNearestStarsToViewer.begin(); it != mNearestStarsToViewer.end(); it++)
	{
		RenderObject* new3DStar = new T3DStar;
		if (new3DStar)
		{
			new3DStar->setUniveralPositionAU((*it)->mPosition);

			ioRenderList.addObject(inCamera, new3DStar);
			mNearest3DStars.push_back(new3DStar);
		}
	}
}

void HYGDatabase::postRender()
{
	// Restore alpha of each item in mNearestStarsToViewer
	if (!mNearestStarsToViewer.empty())
	{
		glBindBuffer(GL_ARRAY_BUFFER, mPointsVBO);
		void* gpuBuf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (gpuBuf)
		{
			for (HYGDataVecP_t::iterator it = mNearestStarsToViewer.begin(); it != mNearestStarsToViewer.end(); it++)
			{
				ColorPointVertex* vertex = (ColorPointVertex*)gpuBuf + (*it)->mVBOIndex;
				vertex->color[3] = 255;
			}
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	glUniform1f(glGetUniformLocation(mPointShaderHandle, "uLimitingMagnitude"), kLimitingStarMagnitude);
}

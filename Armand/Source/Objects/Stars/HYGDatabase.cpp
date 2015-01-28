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

HYGDatabase::HYGDatabase()
{
	// Position the center of the cube in Universal coordinates
	setUniveralPositionAU(Vec3d(0, 0, 0));

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

		// Position
		bool havePosition = false;
		float_t x, y, z;
		if (floatFromString(lineValues[17], x) && floatFromString(lineValues[18], y) && floatFromString(lineValues[19], z))
			havePosition = true;

		// HIP
		bool haveHIP = !lineValues[1].empty();

		// Absolute magnitude
		float_t absMag;
		bool haveAbsMag = floatFromString(lineValues[14], absMag);

		if (haveHIP && havePosition && haveAbsMag)
		{
			HYGDataRecord rec;

			rec.mProperName = lineValues[6];
			rec.mHIP = lineValues[1];
			rec.mPosition = Vec3f(x * (float_t)kAuPerParsec, y * (float_t)kAuPerParsec, z * (float_t)kAuPerParsec);
			rec.mAbsMag = absMag;

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

	// Tell the base class how many points we have, this allocates the storage needed
	// to render the points.
	setNumPoints((GLsizei)mData.size());

	// Write data into render storage
	int n = 0;
	for (HYGData::iterator it = mData.begin(); it != mData.end(); it++)
	{
		mPointArray[n].position[0] = it->mPosition.x;
		mPointArray[n].position[1] = it->mPosition.y;
		mPointArray[n].position[2] = it->mPosition.z;
		mPointArray[n].size = 1;
		mPointArray[n].color[0] = 255;
		mPointArray[n].color[1] = 255;
		mPointArray[n].color[2] = 255;
		mPointArray[n].color[3] = 255;
		mPointArray[n].absMag = it->mAbsMag;
		n++;
	}
}
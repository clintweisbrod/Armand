// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// RandomPointsCube.cpp
//
// This is just a a bit of a load test for rendering tons of GL_POINTS. We will need
// this capability for rendering large data sets where the majority of the data is
// represented by a point in space.
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

#include <random>
#include "RandomPointsCube.h"

RandomPointsCube::RandomPointsCube()
{
	// Generate random data
	loadData();

	// Position the center of the cube in Universal coordinates
	setUniveralPositionLY(Vec3d(0.0, 0.0, -15000.0));
}

RandomPointsCube::~RandomPointsCube()
{
}

void RandomPointsCube::loadData()
{
	const int kNumSamples = 10000;
	const double_t kCubeDimensionAU = 1000000.0;

	if (setNumPoints(kNumSamples))
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> posDis(-kCubeDimensionAU, kCubeDimensionAU);
		std::uniform_real_distribution<> sizeDis(0.1, 3.0);
		std::uniform_real_distribution<> colorDis(0.5, 1.0);
		for (int n = 0; n < kNumSamples; ++n)
		{
			mPointArray[n].position[0] = (GLfloat)posDis(gen);
			mPointArray[n].position[1] = (GLfloat)posDis(gen);
			mPointArray[n].position[2] = (GLfloat)posDis(gen);
			mPointArray[n].size = (GLfloat)sizeDis(gen);
			mPointArray[n].color[0] = (GLubyte)(colorDis(gen) * 255.0);
			mPointArray[n].color[1] = (GLubyte)(colorDis(gen) * 255.0);
			mPointArray[n].color[2] = (GLubyte)(colorDis(gen) * 255.0);
			mPointArray[n].color[3] = 255;
		}
	}

	// Set the bounding radius of the data set
	setBoundingRadiusAU((float_t)(kRoot3 * kCubeDimensionAU));

	// finalize
	finalize();
}

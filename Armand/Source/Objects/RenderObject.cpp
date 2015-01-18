// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// RenderObject.cpp
//
// Abstract base class for all objects in Armand that are rendered.
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

#include "RenderObject.h"

RenderObject::RenderObject()
{
}

RenderObject::~RenderObject()
{
}

void RenderObject::preRender(Camera& inCamera)
{
	// Get viewer-model vector, viewer distance and cache them
	mLastViewerObjectVector = inCamera.getCameraRelativePosition(this);
	mLastViewerDistanceAU = mLastViewerObjectVector.length();
	mLastViewerObjectVectorNormalized = mLastViewerObjectVector / mLastViewerDistanceAU;
}

bool RenderObject::isInView(Camera& inCamera)
{
	// If the camera is inside the bounding radius, it's visibe.
	if (mLastViewerDistanceAU <= mBoundingRadiusAU)
		return true;

	// Get camera direction
	Vec3f viewDirection, upDirection, leftDirection;
	inCamera.getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);
	float_t fisheyeAperture = inCamera.getAperture();
	double_t objectAngularRadius = atan(mBoundingRadiusAU / mLastViewerDistanceAU);
	double_t angleBetween = acosf(viewDirection * mLastViewerObjectVectorNormalized);
	if (angleBetween - objectAngularRadius > fisheyeAperture / 2)
	{
		// Model is not visible.
		return false;
	}
	else
		return true;
}

bool RenderObject::render(Camera& inCamera, float inAlpha)
{
	if (inAlpha <= 0)
		return false;

	preRender(inCamera);

	if (!isInView(inCamera))
		return false;
	else
		return true;
}

bool RenderObject::shouldRenderAsPoint(Camera& inCamera) const
{
	// Decide if model is big enough (in pixels) to warrant rendering.
	const float_t kMinPixelDiameter = 5;
	float_t pixelDiameter = inCamera.getObjectPixelDiameter(mLastViewerDistanceAU, mBoundingRadiusAU);
	if (pixelDiameter < kMinPixelDiameter)
		return true;
	else
		return false;
}

void RenderObject::setGLStateForFullRender(float inAlpha) const
{
}

void RenderObject::setGLStateForPoint(float inAlpha) const
{
}
// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// RenderObjectList.cpp
//
// Performs painter algorithm, sorting RenderObject instance from furthest
// to closest.
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

#include "RenderObjectList.h"

bool EyeDistanceComparator::operator()(const RenderObject* inItem1, const RenderObject* inItem2) const
{
	return (inItem1->getLastViewerDistanceAU() > inItem2->getLastViewerDistanceAU());
}

void RenderObjectList::addObject(Camera& inCamera, RenderObject* inObject)
{
	// Don't add object to list if it is not in view
	inObject->preRender(inCamera, *this);
	if (!inObject->isInView(inCamera))
		return;

	// Don't add object to list if it is too small, unless it is self-illuminating OR
	// If the object's pixel diameter is less than zero, it means no calculations have been done to determin
	// if the object should be draw. In this case, we add it to the list.
	float_t kMinPixelDiameter = 0.1f;
	float_t lastPixelDiameter = inObject->getLastPixelDiameter();
	if ((lastPixelDiameter > 0) && (lastPixelDiameter < kMinPixelDiameter) && !inObject->isSelfIlluminating())
		return;

	push_back(inObject);
}

void RenderObjectList::renderObjects(Camera& inCamera)
{
	// If we've got nothing to render, we're done.
	if (empty())
		return;

	// Sort in descending eye distance
	if (size() > 1)
		std::stable_sort(begin(), end(), mEyeDistanceComparator);

	// Render each object
	for (iterator it = begin(); it != end(); it++)
		(*it)->render(inCamera, 1.0f);
}

void RenderObjectList::postRender()
{
	// Allow each object to perform post-frame cleanup.
	for (iterator it = begin(); it != end(); it++)
		(*it)->postRender();
}

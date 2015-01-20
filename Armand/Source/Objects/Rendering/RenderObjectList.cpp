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

void RenderObjectList::renderObjects(Camera& inCamera)
{
	// Compute the eye distance for each object
	for (iterator it = begin(); it != end(); it++)
		(*it)->preRender(inCamera);

	// Sort in descending eye distance
	std::stable_sort(begin(), end(), mEyeDistanceComparator);

	// Render each object
	for (iterator it = begin(); it != end(); it++)
		(*it)->render(inCamera, 1.0f);
}

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// RenderObjectList.h
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

#pragma once

#include <vector>
#include "Objects/Camera.h"
#include "Objects/RenderObject.h"

using namespace std;

class EyeDistanceComparator : public binary_function<const RenderObject&, const RenderObject&, bool>
{
public:
	bool operator()(const RenderObject* inItem1, const RenderObject* inItem2) const;
};

class RenderObjectList : public vector<RenderObject*>
{
public:
	void renderObjects(Camera& inCamera);
	void addObject(Camera& inCamera, RenderObject& inObject);

private:
	EyeDistanceComparator	mEyeDistanceComparator;
};
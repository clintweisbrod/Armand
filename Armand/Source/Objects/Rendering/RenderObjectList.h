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

private:
	EyeDistanceComparator	mEyeDistanceComparator;
};
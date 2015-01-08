// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// 3DSModelFactory.cpp
//
// Defines a factory class for creation and management of 3DS models.
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
#include "3DSModelFactory.h"

T3DSModelFactory::~T3DSModelFactory()
{
	// Blow all loaded models away.
	RemoveAll();
}

T3DSModel* T3DSModelFactory::get(const char* inModelFileName, bool inLoadMetaOnly)
{
	if (inModelFileName == NULL)
		return NULL;
	if (*inModelFileName == 0)
		return NULL;

	T3DSModelMapItem mapItem;
	mapItem.mModel = NULL;

	// Construct path to model file
	string modelPath = File::getModelsFolder().append("/").append(inModelFileName);

	// Build File instance for model. Note the constructor does no file I/O.
	File modelFile(modelPath);

	// Have we already instantiated this model?
	string modelName = modelFile.getFileNameWithoutExtension();
	T3DSModelMap::iterator it = mModelMap.find(modelName);
	if (it != mModelMap.end())
	{
		mapItem.mModel = it->second.mModel;

		// We may need to load the model data. This happens when we've previously asked only for meta data
		if (!inLoadMetaOnly && !mapItem.mModel->mModelDataLoaded)
			mapItem.mModel->load(modelFile, inLoadMetaOnly);
	}
	else
	{
		// We gotta create the model
		mapItem.mModel = new T3DSModel;
		if (mapItem.mModel)
		{
			if (mapItem.mModel->load(modelFile, inLoadMetaOnly))
			{
				// Add new reference for this model
				//				mapItem.fPlanetRefs[inReferrer] = 1;

				// Add the map item to the map
				mModelMap[modelName] = mapItem;
			}
			else
			{
				// If model fails to load, we drop it.
				delete mapItem.mModel;
				mapItem.mModel = NULL;

				LOG(ERROR) << "Model failed to load: " << modelFile.getFullPath();
			}
		}
	}

	return mapItem.mModel;
}

/*
//----------------------------------------------------------------------
//	T3DSModelFactory::GetInstance
//
//	Purpose:	Either constructs a new T3DSModel instance if the requested model
//				has not already been loaded, or returns the existing one.
//
//	Inputs:		inReferrer - TStaticPlanet instance requesting model
//				inModelFS - LFile identifying the 3DS model we require
//				inIsAUserModel - True if the requested model has been specified by user.
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2007/01/29	CLW			6.0.4
//
//----------------------------------------------------------------------
T3DSModel* T3DSModelFactory::GetInstance(TStaticPlanet* inReferrer, LFile& inModelFile, Boolean inLoadMetaOnly, Boolean inIsAUserModel)
{
T3DSModelMapItem mapItem;

// Get the pretty file name. This is the key in our map of 3DS models.
LStr255 prettyName;
inModelFile.GetPrettyFileName(prettyName);
string modelName = prettyName.GetSTLString();

// Have we already instantiated this model?
T3DSModelMap::iterator it = mModelMap.find(modelName);
if (it != mModelMap.end())
{
mapItem.mModel = it->second.mModel;

// We may need to load the model data. This happens when we've previously asked only for meta data
if (!inLoadMetaOnly && !mapItem.mModel->mModelDataLoaded)
mapItem.mModel->Load(inModelFile, inLoadMetaOnly, inIsAUserModel);

// Add new reference for this model (stl map handles duplicates)
mapItem.fPlanetRefs[inReferrer] = 1;
}
else
{
// We gotta create the model
mapItem.mModel = NEW T3DSModel;
if (mapItem.mModel)
{
if (mapItem.mModel->Load(inModelFile, inLoadMetaOnly, inIsAUserModel))
{
// Add new reference for this model
mapItem.fPlanetRefs[inReferrer] = 1;

// Add the map item to the map
mModelMap[modelName] = mapItem;
}
else
{
// If model fails to load, we drop it.
delete mapItem.mModel;
mapItem.mModel = NULL;

// Inform the user that there was an error loading the model.
UModalDialogs::AskUser(	kAlertStopAlert,
LStr255("STRx_AlertMessages"),
LStr255("STRx_AlertMessages", "ErrorLoading3DSModel"),
LStr255("STRx_AlertMessages", "ErrorLoading3DSModelExplanation"),
LStr255("Default"),
LStr255("Undefined"),
LStr255("Undefined"));
}
}
}

return mapItem.mModel;
}

T3DSModel* T3DSModelFactory::GetInstanceByName(TStaticPlanet* inReferrer, string inModelName)
{
T3DSModelMap::iterator it = mModelMap.find(inModelName);
if (it != mModelMap.end())
return it->second.mModel;
else
return NULL;
}

//----------------------------------------------------------------------
//	T3DSModelFactory::RemoveInstance
//
//	Purpose:	Deallocates and removes the given model from the factory.
//				Necessary to support user-specified models.
//
//	Inputs:		inReferrer - TStaticPlanet instance requesting removal
//				inModelName - the pretty filename of the model.
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2007/01/29	CLW			6.0.4
//
//----------------------------------------------------------------------
Boolean T3DSModelFactory::RemoveInstance(TStaticPlanet* inReferrer, string inModelName)
{
Boolean removed = false;
T3DSModelMap::iterator it = mModelMap.find(inModelName);
if (it != mModelMap.end())
{
// Remove reference to TStaticPlanet
TStaticPlanetMap::iterator itRef = it->second.fPlanetRefs.find(inReferrer);
if (itRef != it->second.fPlanetRefs.end())
{
it->second.fPlanetRefs.erase(itRef);
inReferrer->m3DSModel = NULL;
inReferrer->fAlreadyAttemptedToLoad3DSModel = false;
}

// If there are no references remaining, delete the model
if (it->second.fPlanetRefs.empty())
{
// Destroy the model instance
if (it->second.mModel)
delete it->second.mModel;

// Erase the item from the map
mModelMap.erase(it);
}

removed = true;
}

return removed;
}
*/

//----------------------------------------------------------------------
//	T3DSModelFactory::RemoveAll
//
//	Purpose:	Iterates through map of T3DSModel instances	and deallocates
//				them.
//
//	Date		Initials	Version		Comments
//  ----------	---------	----------	---------------------------
//	2007/06/18	CLW			6.0.8		
//
//----------------------------------------------------------------------
void T3DSModelFactory::RemoveAll()
{
	// Iterate through the map and delete each T3DSModel instance
	T3DSModelMap::iterator it;
	for (it = mModelMap.begin(); it != mModelMap.end(); it++)
	{
		// Destroy the model instance
		if (it->second.mModel)
			delete it->second.mModel;

		// Mark every referring TStaticPlanet*
		//		TStaticPlanetMap::iterator itRef;
		//		for (itRef = it->second.fPlanetRefs.begin(); itRef != it->second.fPlanetRefs.end(); itRef++)
		//		{
		//			itRef->first->fAlreadyAttemptedToLoad3DSModel = false;
		//			itRef->first->m3DSModel = NULL;
		//		}

		// Clear the reference list
		//		it->second.fPlanetRefs.clear();
	}

	// Clear the map
	mModelMap.clear();
}
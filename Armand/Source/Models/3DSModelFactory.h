// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// 3DSModelFactory.h
//
// Declares a factory class for creation and management of 3DS models.
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

#include <string>
#include <map>
#include "Utilities/Singleton.h"
#include "3DSModel.h"

using namespace std;

struct T3DSModelMapItem
{
	T3DSModel*			mModel;
	//	TStaticPlanetMap	fPlanetRefs;
};
typedef map<string, T3DSModelMapItem>	T3DSModelMap;

class T3DSModelFactory : public Singleton<T3DSModelFactory>
{
	friend class Singleton<T3DSModelFactory>;

public:
	T3DSModel*	get(const char* inModelFileName, bool inLoadMetaOnly = false);
	//		T3DSModel*	GetInstance(TStaticPlanet* inReferrer, LFile& inModelFile, Boolean inLoadMetaOnly = false, Boolean inIsAUserModel = false);
	//		T3DSModel*	GetInstanceByName(TStaticPlanet* inReferrer, string inModelName);
	//		Boolean		RemoveInstance(TStaticPlanet* inReferrer, string inModelName);
	void		RemoveAll();

private:
	virtual ~T3DSModelFactory();

	T3DSModelMap	mModelMap;
};
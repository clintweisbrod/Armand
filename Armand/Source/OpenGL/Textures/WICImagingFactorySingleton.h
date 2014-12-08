// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// WICImagingFactorySingleton.h
//
// Singleton that oalls CoInitialize() only once and obtains IWICImagingFactory
// instance.
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

#include <Wincodec.h>
#include "Utilities/Singleton.h"

class WICImagingFactorySingleton : public Singleton<WICImagingFactorySingleton>
{
	friend class Singleton<WICImagingFactorySingleton>;

public:
	IWICImagingFactory* GetFactory() { return mFactory; };

protected:
	WICImagingFactorySingleton()
	{
		// Create the COM imaging factory
		mFactory = NULL;

		// Initialize COM
		HRESULT hr = CoInitialize(NULL);
		if (SUCCEEDED(hr))
		{
			// Call CoInitializeSecurity()
			hr = CoInitializeSecurity(NULL,
				-1,							// COM authentication
				NULL,							// Authentication services
				NULL,							// Reserved
				RPC_C_AUTHN_LEVEL_DEFAULT,	// Default authentication
				RPC_C_IMP_LEVEL_IMPERSONATE,	// Default Impersonation
				NULL,							// Authentication info
				EOAC_NONE,					// Additional capabilities
				NULL);						// Reserved

			if (SUCCEEDED(hr))
			{
				hr = CoCreateInstance(CLSID_WICImagingFactory1,
									  NULL,
									  CLSCTX_INPROC_SERVER,
									  IID_PPV_ARGS(&mFactory));
				if (!SUCCEEDED(hr))
					mFactory = NULL;
			}
		}
	};

private:
	virtual ~WICImagingFactorySingleton()
	{
		// Release the IWICImagingFactory instance
		if (mFactory)
			mFactory->Release();

		// Shutdown COM
		CoUninitialize();
	};

	IWICImagingFactory* mFactory;
};
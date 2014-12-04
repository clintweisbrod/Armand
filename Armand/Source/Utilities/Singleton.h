// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// Singleton.h
//
// Base class for singleton class instances.
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

template <typename T> 
class TSingleton
{
	public:
		static T* inst()
		{
			if (fInstance == NULL)
				fInstance = new T;

			return fInstance;
		};

		static void destroy()
		{
			if (fInstance != NULL)
			{
				delete fInstance;
				fInstance = NULL;
			}
		};

	protected:
		// shield the constructor and destructor to prevent outside sources
		// from creating or destroying a TSingleton instance.

		//! Default constructor.
		TSingleton(){};

		//! Destructor.
		virtual ~TSingleton(){};

	private:
		//! Copy constructor.
		TSingleton(const TSingleton&){};

		static T* fInstance; //!< singleton class instance
};

//! static class member initialisation.
template <typename T> T* TSingleton<T>::fInstance = NULL;

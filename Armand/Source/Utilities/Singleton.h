#pragma once

template <typename T> 
class TSingleton
{
	public:
		static T* instance()
		{
			if (fInstance == NULL)
				fInstance = new T;

			return fInstance;
		};

		static void destroyInstance()
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

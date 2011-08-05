// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_SINGLETON_H
#define INCLUDED_SINGLETON_H

#ifndef INCLUDED_CASSERT
#define INCLUDED_CASSERT
#include <cassert>
#endif

namespace frozenbyte {
namespace exporter {

template <class T>
class TSingleton
{
	static T *instance;

public:
	TSingleton() 
	{
		assert(instance == 0);

		// Some pointer magic from 'Gems
		//	-> Cast dump pointer to both types and store relative address
		int pointerSuper = (int) (T*) 1;
		int pointerDerived = (int) (TSingleton<T> *) (T*) 1;
		int offset = pointerSuper - pointerDerived;
		
		// Use offset to get our instance address
		instance = (T*) (int)(this) + offset;
	}
	
	~TSingleton()
	{
		assert(instance);
		instance = 0;
	}

	static T* getSingleton()
	{
		assert(instance);
		return instance;
	}
};

// Initialize static instance
template<class T> T* TSingleton<T>::instance = 0;

} // end of namespace export
} // end of namespace frozenbyte

#endif


#ifndef SIMPLESTRINGPOOL_H
#define SIMPLESTRINGPOOL_H

#include <stdio.h>

namespace util
{
	class ISimpleStringPoolManager
	{
	public:
		virtual ~ISimpleStringPoolManager() {};

		// this gets called when the pool is about to resize itself - the manager should update all pointers that
		// point inside the pool to the new base pointer.
		// notice, however, that the pool base is ABOUT TO CHANGE and has not yet changed, so that you can
		// still use the isStringInPool() function to determine if a string pointer is inside the pool.
		virtual void poolBaseWillChange(const char *previousBasePtr, const char *newBasePtr) = 0;
		
		// this is just a "dummy" callback after the pool base has actually changed. assumably, no-one should be 
		// really specifically interested in this, but rather about the poolBaseWillChange callback.
		// (since at this time, the isStringInPool() will no-longer be correct for old string pointers)
		virtual void poolBaseHasChanged() = 0;
	};


	class SimpleStringPoolImpl;

	class SimpleStringPool
	{
		public:
			// the manager must handle the situations where the string pool gets dynamically resized,
			// if no manager is supplied (null), then the pool will not grow when full, but any more add calls rather fail
			// the fill holes determines whether the pool tries to fill in memory holes in the pool or not
			// (if the holes are not filled, then if the pool strings are often removed, the pool starts to become
			// very unefficient in terms of memory usage)
			SimpleStringPool(int initialSize, ISimpleStringPoolManager *manager, bool fillHoles);

			~SimpleStringPool();

			void clear();

			const char *addStringInPool(const char *str);

			void removeStringFromPool(const char *pooledStr);

			inline bool isStringInPool(const char *stringPtr) const
			{
				if (stringPtr >= stringPoolStart && stringPtr < stringPoolEnd)
					return true;
				else
					return false;
			}

			// delete the returned buffer once done with it.
			char *getStatusInfo();

		protected:
			ISimpleStringPoolManager *manager;
			
			int nextStringPoolPosition;
			char *stringPoolStart;
			char *stringPoolEnd;
			int allocedStringPoolSize;
			bool fillHoles;

			SimpleStringPoolImpl *impl; 
	};
}

#endif

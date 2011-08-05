
#ifndef SIMPLETRACKABLEUNIFIEDHANDLEOBJECTITERATOR_H
#define SIMPLETRACKABLEUNIFIEDHANDLEOBJECTITERATOR_H

#include <vector>
#include <assert.h>
#include "ITrackableUnifiedHandleObjectIterator.h"

namespace game
{
namespace tracking
{
	class SimpleTrackableUnifiedHandleObjectIterator : public ITrackableUnifiedHandleObjectIterator
	{
	public:
		SimpleTrackableUnifiedHandleObjectIterator()
			: listPos(0)
		{
		}

		virtual ~SimpleTrackableUnifiedHandleObjectIterator()
		{
		}

		virtual bool iterateAvailable() { return (listPos < (int)list.size()); }
		virtual UnifiedHandle iterateNext() { assert((int)list.size() > listPos); return list[listPos++]; }

	private:
		// TODO: optimize, move this out of the iterator...
		// possibly refer to the list using a smart pointer to allow the list to be deleted once iterator has been destroyed
		std::vector<UnifiedHandle> list;

		int listPos;

	public:
		// not intended for public access, but cannot use interface to define all friend classes. =/
		void addEntry(UnifiedHandle entry) { list.push_back(entry); }
	};
}
}

#endif

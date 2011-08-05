
#ifndef SIMPLETRACKABLEOBJECTITERATOR_H
#define SIMPLETRACKABLEOBJECTITERATOR_H

#include <vector>
#include <assert.h>
#include "ITrackableObjectIterator.h"

namespace game
{
namespace tracking
{

	// NOTE: this is a simple - and thus very ineffective - iterator implementation for trackable objects.

	class SimpleTrackableObjectIterator : public ITrackableObjectIterator
	{
	public:
		SimpleTrackableObjectIterator(std::vector<ITrackableObject *> list)
			: listPos(0)
		{
			this->list = list;
		}

		virtual ~SimpleTrackableObjectIterator()
		{
		}

		virtual bool iterateAvailable() { return (listPos < (int)list.size()); }
		virtual ITrackableObject *iterateNext() { assert((int)list.size() > listPos); return list[listPos++]; }

	private:
		std::vector<ITrackableObject *> list;
		int listPos;
	};
}
}

#endif

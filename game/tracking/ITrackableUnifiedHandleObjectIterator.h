
#ifndef ITRACKABLEUNIFIEDHANDLEOBJECTITERATOR_H
#define ITRACKABLEUNIFIEDHANDLEOBJECTITERATOR_H

#include "../unified_handle_type.h"

namespace game
{
namespace tracking
{

	class ITrackableUnifiedHandleObjectIterator
	{
	public:
		virtual ~ITrackableUnifiedHandleObjectIterator() { };

		virtual bool iterateAvailable() = 0;
		virtual UnifiedHandle iterateNext() = 0;
	};

}
}

#endif

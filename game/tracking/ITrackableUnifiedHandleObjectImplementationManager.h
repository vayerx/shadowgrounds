
#ifndef ITRACKABLEUNIFIEDHANDLEOBJECTIMPLEMENTATIONMANAGER_H
#define ITRACKABLEUNIFIEDHANDLEOBJECTIMPLEMENTATIONMANAGER_H

#include "ITrackableUnifiedHandleObjectIterator.h"
#include "../unified_handle_type.h"
#include "trackable_typeid.h"
#include <DatatypeDef.h>

namespace game
{
namespace tracking
{
	class ITrackableUnifiedHandleObjectImplementationManager
	{
	public:
		virtual bool doesTrackableUnifiedHandleObjectExist(UnifiedHandle unifiedHandle) const = 0;
		virtual VC3 getTrackableUnifiedHandlePosition(UnifiedHandle unifiedHandle) const = 0;
		virtual QUAT getTrackableUnifiedHandleRotation(UnifiedHandle unifiedHandle) const = 0;
		virtual VC3 getTrackableUnifiedHandleVelocity(UnifiedHandle unifiedHandle) const = 0;
		virtual ITrackableUnifiedHandleObjectIterator *getTrackableUnifiedHandleObjectsFromArea(const VC3 &position, float radius, TRACKABLE_TYPEID_DATATYPE typeMask) = 0;
		virtual ~ITrackableUnifiedHandleObjectImplementationManager() {};

		// NOTE: should have angular velocity too, but for now that is not so important.
	};
}
}

#endif


#ifndef ITRACKEROBJECTTYPE_H
#define ITRACKEROBJECTTYPE_H

#include "trackable_typeid.h"

namespace game
{
namespace tracking
{
	class ITrackerObject;

	class ITrackerObjectType
	{
	public:

		virtual ~ITrackerObjectType() {};

		virtual std::string getTrackerTypeName() const = 0;

		virtual bool doesGiveOwnershipToObjectTracker() const = 0;

		virtual void *getTypeId() const = 0;

		virtual int getTickInterval() const = 0;

		virtual bool doesAllowTickBalancing() const = 0;

		// the area within which the tracker is interested in trackables...
		virtual float getAreaOfInterestRadius() const = 0;

		// the type of trackables which the tracker is interested in (may be multi bit mask)
		virtual TRACKABLE_TYPEID_DATATYPE getTrackablesTypeOfInterest() const = 0;

		virtual ITrackerObject *createNewObjectInstance() = 0;
	};
}
}

#endif

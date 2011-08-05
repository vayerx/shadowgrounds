
#ifndef ITRACKABLEOBJECTFACTORY_H
#define ITRACKABLEOBJECTFACTORY_H

#include "trackable_typeid.h"
#include "ITrackableObject.h"
#include "ITrackableObjectIterator.h"

namespace game
{
namespace tracking
{
	class ITrackableObjectFactory
	{
	public:
		virtual TRACKABLE_TYPEID_DATATYPE getTrackableObjectType() = 0;

		//virtual ITrackableObject *createNewTrackableObjectInstance() = 0;

		virtual ITrackableObjectIterator *getTrackablesFromArea(const VC3 &globalPosition, float radius) = 0;

		virtual ~ITrackableObjectFactory() {};
	};

}
}


#endif


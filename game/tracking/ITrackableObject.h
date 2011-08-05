
#ifndef ITRACKABLEOBJECT_H
#define ITRACKABLEOBJECT_H

#include "trackable_typeid.h"
#include <DatatypeDef.h>

namespace game
{
namespace tracking
{


	//
	// WARNING: This interface is not finalized!!!
	// implementing this in interface requires the (addRef /) release methods to be properly designed/implemented
	// currently, will leak memory if any other classes implement this than TrackableUnifiedHandleObject
	// (which uses a static pool for the instances)
	//


	class ITrackableObject
	{
	protected:
		virtual ~ITrackableObject() { }

	public:
		// call this instead of delete
		virtual void release() = 0;

		virtual bool doesExist() const = 0;

		virtual void *getTypeId() const = 0;

		virtual VC3 getTrackableObjectPosition() const = 0;

		virtual QUAT getTrackableObjectRotation() const = 0;

		//virtual VC3 convertGlobalToLocalPosition(const VC3 &globalPosition) const = 0;

		//virtual VC3 convertLocalToGlobalPosition(const VC3 &localPosition) const = 0;

		//virtual VC3 keepInLocalBounds(const VC3 &localPosition) const = 0;

		virtual TRACKABLE_TYPEID_DATATYPE getTrackableTypes() const = 0;

		//virtual void addTrackableTypes(TRACKABLE_TYPEID_DATATYPE typeMask);

		//virtual void removeTrackableType(TRACKABLE_TYPEID_DATATYPE typeMask);
	};

}
}

#endif


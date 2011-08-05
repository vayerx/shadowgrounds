
#ifndef ITRACKEROBJECT_H
#define ITRACKEROBJECT_H

namespace game
{
namespace tracking
{
	class ITrackerObjectType;
	class ITrackableObjectIterator;
	class ITrackableObject;

	class ITrackerObject
	{
	public:
		virtual ~ITrackerObject() { }

		// should return the tracker object type (type of which this is an instance of)
		virtual ITrackerObjectType *getType() = 0;

		// the tracking system calls this every once a while based on the tracker type..
		virtual void tick() = 0;

		// the tracking system calls this if the tracker has lost the trackable it was attached to
		virtual void lostTracked() = 0;

		// a custom signal to the tracker...
		virtual void trackerSignal(int trackerSignalNumber) = 0;

		// the tracking system calls this if the tracker has been attached to a new trackable
		// (parameter is null if being detached)
		virtual void attachedToTrackable(ITrackableObject *trackable) = 0;

		// called when the tracker is being deleted. 
		// (only for tracker types that do not give ownership to the object tracker - assuming that is ok?)
		virtual void trackerDeleted() = 0;

		// should set the tracker's position (possibly only if not attached?)
		virtual void setTrackerPosition(const VC3 &position) = 0;

		// should return the tracker's position
		virtual VC3 getTrackerPosition() const = 0;

		// the tracking system calls this method as the trackable object moves... (when attached to trackable)
		// (every tick?)
		virtual void setTrackablePosition(const VC3 &globalPosition) = 0;

		virtual void setTrackableRotation(const QUAT &rotation) = 0;

		virtual void setTrackableVelocity(const VC3 &velocity) = 0;

		// the tracking system calls this when trackable iteration for this tracker is requested...
		virtual void iterateTrackables(ITrackableObjectIterator *iter) = 0;

	};


}
}

#endif

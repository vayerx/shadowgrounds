
#ifndef TRACKABLEUNIFIEDHANDLEOBJECT_H
#define TRACKABLEUNIFIEDHANDLEOBJECT_H

#include "ITrackableUnifiedHandleObjectImplementationManager.h"
#include "ITrackableObject.h"
#include "ObjectTracker.h"

// up to this many trackers will be efficiently handled when solving trackers attached to trackable...
#define TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE 3

namespace game
{
	class Game;

namespace tracking
{
	class TrackableUnifiedHandleObject : public ITrackableObject
	{
	private:
		virtual ~TrackableUnifiedHandleObject();

	public:
		// default constr. required for pool
		TrackableUnifiedHandleObject();

		TrackableUnifiedHandleObject(UnifiedHandle unifiedHandle);

		virtual bool doesExist() const;

		virtual void *getTypeId() const { return TrackableUnifiedHandleObject::typeId; }

		virtual void release();

		virtual VC3 getTrackableObjectPosition() const;

		virtual QUAT getTrackableObjectRotation() const;

		virtual TRACKABLE_TYPEID_DATATYPE getTrackableTypes() const;

		UnifiedHandle getUnifiedHandle() { return handle; }

		// static stuff...
		static void setGame(game::Game *game);

		static TrackableUnifiedHandleObject *getInstanceFromPool(int unifiedHandle);

		// "cache" for solving which trackers are tracking a trackable..
		UnifiedHandle getTrackedByForType(TrackerTypeNumber trackerTypeNumber);
		// (trackable that tracks any of the given types)
		UnifiedHandle getTrackedByForTrackableTypes(TRACKABLE_TYPEID_DATATYPE trackableTypes);

		// get tracker for trackable, with given index, returns UNIFIED_HANDLE_NONE, if no more available (at given index)
		// NOTE: indices may change if trackers are removed/added!
		UnifiedHandle getTrackedByWithIndex(int index);

		void addTrackedBy(UnifiedHandle tracker);
		void removeTrackedBy(UnifiedHandle tracker);
		//int getTrackedByCount();

		static void cleanupPool();
		
		static void *typeId;

		static std::string getStatusInfo();

	private:
		UnifiedHandle handle;
		int refCount;
		int trackedByCount;
		UnifiedHandle trackedBy[TRACKABLEUNIFIEDHANDLEOBJECT_TRACKED_BY_CACHE_SIZE];
	};
}
}

#endif

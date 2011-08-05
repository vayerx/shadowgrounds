
#ifndef OBJECTTRACKER_H
#define OBJECTTRACKER_H

#include "../unified_handle_type.h"
#include "trackable_typeid.h"

typedef int TrackerTypeNumber;

namespace ui
{
	class DebugTrackerVisualizer;
}

namespace game
{
namespace tracking
{
	class ITrackerObject;
	class ITrackerObjectType;
	class ITrackableObjectFactory;
	class ITrackableObject;

	// "what be this?"
	/*
	class ITrackableObjectProperties
	{
	public:
		int type;
		int typeParameter;
	};
	*/

	class ObjectTrackerImpl;

	class ObjectTracker
	{
	public:
		ObjectTracker();
		~ObjectTracker();

		// return assigned tracker type number or -1 if failed (due to too many tracker types).
		TrackerTypeNumber addTrackerType(ITrackerObjectType *trackerType);
		void removeTrackerType(ITrackerObjectType *trackerType);
		void removeTrackerType(TrackerTypeNumber trackerTypeNumber);
		void removeAllTrackerTypes();

		// returns assigned tracker type number or -1 if failed for given tracker type name
		TrackerTypeNumber getTrackerTypeNumberByName(const std::string &typeName) const;

		void addTrackableObjectFactory(ITrackableObjectFactory *trackableObjectFactory);

		// return unified handle for created tracker or 0 if failed (due to too many trackers / bad type number).
		UnifiedHandle createTracker(TrackerTypeNumber trackerTypeNumber);
		void deleteTracker(UnifiedHandle trackerUnifiedHandle);
		void deleteTracker(ITrackerObject *tracker);

		// (call this when the tracker is being deleted if the type does not give ownership to objecttracker...)
		void releaseTracker(ITrackerObject *tracker);

		void deleteAllTrackersOfType(ITrackerObjectType *trackerType);
		void deleteAllTrackersOfType(TrackerTypeNumber trackerTypeNumber);
		void deleteAllTrackers();

		ITrackerObject *getTrackerByUnifiedHandle(UnifiedHandle handle);

		// returns -1 if tracker does not exist.
		TrackerTypeNumber getTrackerTypeNumberForTracker(UnifiedHandle trackerHandle);

		void iterateTrackablesForTracker(ITrackerObject *tracker);
		void iterateTrackablesForTracker(UnifiedHandle trackerUnifiedHandle);

		void attachTrackerToTrackable(UnifiedHandle tracker, ITrackableObject *trackableObject);
		ITrackableObject *getTrackerAttachedTrackable(UnifiedHandle tracker);

		void signalToTrackerFromTrackable(UnifiedHandle trackable, int trackerSignalNumber);
		void signalToTrackerDirectly(UnifiedHandle trackerUnifiedHandle, int trackerSignalNumber);

		void run(int msec);

		std::string getStatusInfo();

	private:
		ObjectTrackerImpl *impl;

		// for visualization only (pretty ineffective for any other use)
		std::vector<ITrackerObject *> getAllTrackers();
		friend class ui::DebugTrackerVisualizer;
	};

}
}

#endif

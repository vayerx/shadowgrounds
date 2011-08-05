
// TODO

// THIS SHOULD KEEP A LIST (QUADTREE MAYBE?) OF LISTENED(TRACKED) AREAS AND OBJECTS
// TO BE TRACKED...

// THEN, FIRE AN EVENT WHEN AN OBJECT ENTERS(/LEAVES?) THE AREA.
// MAYBE JUST FIRE THE EVENT ONCE, AND THEN DELETE THAT TRACKING FROM THE LIST...

// THEREFORE, WHEN OUTSIDE AREA AND A TRACKER IS BEING ADDED, THE ENTRANCE
// WILL BE DETECTED BUT NO CPU WASTED FOR OTHERS...
// (SHOULD WORK OK FOR SCRIPTS AND AMBIENT TRIGGERS)
// (TRIGGER ONCE, AND AFTER THAT, JUST POLL EVERY ONCE A WHILE IF NEEDED)

// something like this...:
//
// unitPosition
// setAreaCenterToPosition
// getUnitVariable scriptrange
// setAreaRangeToValue
// getUnitVariable scriptclip
// setAreaClipToValue
// setPlayer 0
// 
// loop
//   if
//     hasAreaTriggered
//   then
//     breakLoop
//   endif
//   // after this call, entrance to area will cause skipMainScriptWait
//   areaListenToPlayerUnits
//   random 2000
//   addValue 1000
//   waitValue
// endLoop

#ifndef INCLUDED_CIRCLEAREATRACKER_H
#define INCLUDED_CIRCLEAREATRACKER_H

#include <boost/scoped_ptr.hpp>
#include <DatatypeDef.h>

namespace util {

class ITrackable;
class ITriggerListener;
class ClippedCircle;

class CircleAreaTracker
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	explicit CircleAreaTracker(const VC2 &size);
	~CircleAreaTracker();

	int addCircleTrigger(const ClippedCircle &circle, ITriggerListener *listener, void *triggerData);
	void addTrackable(int circleId, ITrackable *ptr);
	void removeCircleTrigger(int circleId);

	void update(int ms);
};

} // util

#endif

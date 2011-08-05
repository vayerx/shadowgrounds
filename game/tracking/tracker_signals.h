
#ifndef TRACKER_SIGNALS_H
#define TRACKER_SIGNALS_H

// (reserved)
#define TRACKER_SIGNAL_INVALID 0

// TODO:
// tracker, kill yourself
#define TRACKER_SIGNAL_KILL 1

// TODO:
// attached trackable has been destroyed - but continues to exist (unlike in case of lostTrackable event)
#define TRACKER_SIGNAL_TRACKABLE_DESTROYED 2

// TODO:
// attached trackable has warped (in other words, suddenly discontinuously moved to another location)
#define TRACKER_SIGNAL_WARPED 3

// attached trackable terrain object has done the hacky "break to self loop"
#define TRACKER_SIGNAL_TERRAIN_OBJECT_BREAK_SELF_LOOP 4


#ifdef PROJECT_CLAW_PROTO
// HACK: burning object breaks on unit?
#define TRACKER_SIGNAL_TERRAIN_OBJECT_HIT_UNIT 5
#endif



#endif


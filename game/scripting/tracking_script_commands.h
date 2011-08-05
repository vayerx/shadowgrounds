// from 2000

#define GS_CMD_BASE 2000

GS_CMD_SIMPLE(0, createScriptableTrackerType, STRING)

GS_CMD_SIMPLE(1, createTracker, STRING)
GS_CMD_SIMPLE(2, createTrackerAttachingToTrackableByValue, STRING)
GS_CMD_SIMPLE(3, doesTrackerExist, NONE)
GS_CMD_SIMPLE(4, doesTrackerAttachedTrackableExist, NONE)
GS_CMD_SIMPLE(5, attachTrackerToTrackableByValue, NONE)
GS_CMD_SIMPLE(6, setTrackerToPosition, NONE)
GS_CMD_SIMPLE(7, getTrackerPosition, NONE)
GS_CMD_SIMPLE(8, deleteTracker, NONE)
GS_CMD_SIMPLE(9, isTrackerAttached, NONE)
GS_CMD_SIMPLE(10, getTrackerAttachedTrackable, NONE)
GS_CMD_SIMPLE(11, detachTracker, NONE)
GS_CMD_SIMPLE(12, restoreTracker, NONE)

GS_CMD_SIMPLE(13, setTrackerProjectileBullet, STRING)

GS_CMD_SIMPLE(14, iterateTrackablesForTracker, NONE)

GS_CMD_SIMPLE(15, iterateNextTrackable, NONE)
GS_CMD_SIMPLE(16, getIteratedTrackableUnifiedHandle, NONE)
GS_CMD_SIMPLE(17, isTrackerType, STRING)
GS_CMD_SIMPLE(18, getScriptableTrackerVariable, STRING)
GS_CMD_SIMPLE(19, setScriptableTrackerVariable, STRING)

GS_CMD_SIMPLE(20, isUnifiedHandleObjectTrackedByValue, NONE)
GS_CMD_SIMPLE(21, isUnifiedHandleObjectTrackedByName, STRING)

GS_CMD_SIMPLE(22, getTrackedByForUnifiedHandleObjectByValue, NONE)
GS_CMD_SIMPLE(23, getTrackedByForUnifiedHandleObjectByName, STRING)

GS_CMD_SIMPLE(24, addScriptableTrackerTypeVariable, STRING)

GS_CMD_SIMPLE(25, dumpTrackingInfo, NONE)

GS_CMD_SIMPLE(26, deleteUnifiedHandleObjectTrackerByName, STRING)

GS_CMD_SIMPLE(27, isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes, INT)
GS_CMD_SIMPLE(28, getTrackedByForUnifiedHandleObjectHavingTrackerInterestedInTrackableTypes, INT)

GS_CMD_SIMPLE(29, getCurrentTrackerFloatDistanceToUnifiedHandleObject, NONE)

#undef GS_CMD_BASE

// up to 2049

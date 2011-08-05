
// Copyright(C) Jukka Kokkonen, 2007

// from 2200

#define GS_CMD_BASE 2200

GS_CMD_SIMPLE(0, directControlOnByName, STRING)
GS_CMD_SIMPLE(1, directControlOffByName, STRING)

GS_CMD_SIMPLE(2, directControlForwardOn, NONE)
GS_CMD_SIMPLE(3, directControlForwardOff, NONE)
GS_CMD_SIMPLE(4, directControlLeftOn, NONE)
GS_CMD_SIMPLE(5, directControlLeftOff, NONE)
GS_CMD_SIMPLE(6, directControlRightOn, NONE)
GS_CMD_SIMPLE(7, directControlRightOff, NONE)
GS_CMD_SIMPLE(8, directControlBackwardOn, NONE)
GS_CMD_SIMPLE(9, directControlBackwardOff, NONE)

GS_CMD_SIMPLE(10, directControlFireOn, NONE)
GS_CMD_SIMPLE(11, directControlFireOff, NONE)
GS_CMD_SIMPLE(12, directControlFireSecondaryOn, NONE)
GS_CMD_SIMPLE(13, directControlFireSecondaryOff, NONE)

GS_CMD_SIMPLE(14, directControlListenToEvent, STRING)
GS_CMD_SIMPLE(15, directControlDoNotListenToEvent, STRING)

GS_CMD_SIMPLE(16, directControlAutomaticallyDisableAction, STRING)
GS_CMD_SIMPLE(17, directControlDoNotAutomaticallyDisableAction, STRING)

GS_CMD_SIMPLE(18, directControlSetTimerMsec, INT)
GS_CMD_SIMPLE(19, directControlSetAimMode, STRING)
GS_CMD_SIMPLE(20, directControlSetAimPositionIgnoringMode, NONE)
GS_CMD_SIMPLE(21, directControlSetAimToPositionByMode, NONE)

#undef GS_CMD_BASE

// up to 2299

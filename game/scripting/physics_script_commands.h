
// from 2300

#define GS_CMD_BASE 2300

GS_CMD_SIMPLE(0, physSetGroupCont, STRING)
GS_CMD_SIMPLE(1, physSetGroupColl, STRING)

// the not-used system
GS_CMD_SIMPLE(2, jointAddStart, STRING)
GS_CMD_SIMPLE(3, jointAttachToUnifiedHandleObject, NONE)
GS_CMD_SIMPLE(4, jointSetParameterName, STRING)
GS_CMD_SIMPLE(5, jointSetParameterValue, FLOAT)
GS_CMD_SIMPLE(6, jointAddDone, STRING)

// ...
GS_CMD_SIMPLE(7, setJointObjectA, STRING)
GS_CMD_SIMPLE(8, setJointObjectB, STRING)
GS_CMD_SIMPLE(9, setJointTwistLimits, STRING)
GS_CMD_SIMPLE(10, setJointTwistLowSpring, STRING)
GS_CMD_SIMPLE(11, setJointTwistHighSpring, STRING)
GS_CMD_SIMPLE(12, setJointSwing1, STRING)
GS_CMD_SIMPLE(13, setJointSwing2, STRING)
GS_CMD_SIMPLE(14, setJointBreakForce, FLOAT)
GS_CMD_SIMPLE(15, setJointAxis, STRING)
GS_CMD_SIMPLE(16, setJointNormal, STRING)
GS_CMD_SIMPLE(17, setJointPosition, STRING)
GS_CMD_SIMPLE(18, setJointNormalAngle, FLOAT)
GS_CMD_SIMPLE(19, setJointId, STRING)
GS_CMD_SIMPLE(20, createJoint, NONE)

GS_CMD_SIMPLE(21, setJointObjectAToUnifiedHandle, NONE)
GS_CMD_SIMPLE(22, setJointObjectBToUnifiedHandle, NONE)
GS_CMD_SIMPLE(23, setJointPositionToPosition, NONE)

GS_CMD_SIMPLE(24, cancelJoint, NONE)
GS_CMD_SIMPLE(25, setJointParam, STRING)
GS_CMD_SIMPLE(26, setJointDeforming, STRING)

#undef GS_CMD_BASE

// up to 1299


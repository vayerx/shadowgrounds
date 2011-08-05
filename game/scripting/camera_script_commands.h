// Copyright 2002-2004 Frozenbyte Ltd.

// from 500

#define GS_CMD_BASE 500

GS_CMD(0, GS_CMD_CAMERAPOSITION, "cameraPosition", NONE)
GS_CMD(1, GS_CMD_SETCAMERAPOSITIONNEAR, "setCameraPositionNear", NONE)
GS_CMD(2, GS_CMD_SETCAMERAANGLE, "setCameraAngle", INT)
GS_CMD(3, GS_CMD_SETCAMERABETAANGLE, "setCameraBetaAngle", INT)
GS_CMD(4, GS_CMD_SETCAMERAMOVEMENTOFF, "setCameraMovementOff", STRING)
GS_CMD(5, GS_CMD_SETCAMERAMOVEMENTON, "setCameraMovementOn", STRING)
GS_CMD(6, GS_CMD_SETCAMERAZOOM, "setCameraZoom", INT)
GS_CMD(7, GS_CMD_SELECTCAMERA, "selectCamera", STRING)
GS_CMD(8, GS_CMD_SETCAMERAMODE, "setCameraMode", STRING)
GS_CMD(9, GS_CMD_SETCAMERATIMEFACTOR, "setCameraTimeFactor", STRING)
GS_CMD(10, GS_CMD_ISCAMERAZOOMLESSTHAN, "isCameraZoomLessThan", INT)
GS_CMD(11, GS_CMD_ISCAMERAZOOMGREATERTHAN, "isCameraZoomGreaterThan", INT)
GS_CMD(12, GS_CMD_SETCAMERAFOV, "setCameraFOV", INT)
GS_CMD(13, GS_CMD_COPYCAMERATO, "copyCameraTo", STRING)
GS_CMD(14, GS_CMD_ROTATECAMERATOWARDUNIT, "rotateCameraTowardUnit", NONE)
GS_CMD(15, GS_CMD_SETCAMERAHEIGHT, "setCameraHeight", INT)
GS_CMD(16, GS_CMD_SETCAMERAANGLETOVALUE, "setCameraAngleToValue", NONE)
GS_CMD(17, GS_CMD_SHAKECAMERANEARPOSITIONSHORT, "shakeCameraNearPositionShort", INT)
GS_CMD(18, GS_CMD_SHAKECAMERANEARPOSITIONMEDIUM, "shakeCameraNearPositionMedium", INT)
GS_CMD(19, GS_CMD_SHAKECAMERANEARPOSITIONLONG, "shakeCameraNearPositionLong", INT)
GS_CMD(20, GS_CMD_SETCAMERAPOSITION, "setCameraPosition", NONE)
GS_CMD(21, GS_CMD_SETCAMERAFLOATZOOM, "setCameraFloatZoom", STRING)
GS_CMD(22, GS_CMD_SETCAMERAINTERPOLATIONTYPE, "setCameraInterpolationType", STRING)

GS_CMD(23, GS_CMD_SETCAMERARANGE, "setCameraRange", INT)
GS_CMD(24, GS_CMD_RESTORECAMERARANGE, "restoreCameraRange", NONE)

GS_CMD(25, GS_CMD_LISTENERPOSITION, "listenerPosition", NONE)

GS_CMD(26, GS_CMD_SETCAMERATARGETDISTANCE, "setCameraTargetDistance", INT)
GS_CMD(27, GS_CMD_RESTORECAMERATARGETDISTANCE, "restoreCameraTargetDistance", NONE)

GS_CMD(28, GS_CMD_SETCAMERAANGLEFLOAT, "setCameraAngleFloat", FLOAT)
GS_CMD(29, GS_CMD_SETCAMERABETAANGLEFLOAT, "setCameraBetaAngleFloat", FLOAT)

GS_CMD(30, GS_CMD_GETCAMERAANGLE, "getCameraAngle", NONE)
GS_CMD(31, GS_CMD_GETCAMERABETAANGLE, "getCameraBetaAngle", NONE)

GS_CMD(32, GS_CMD_GETCAMERARANGE, "getCameraRange", NONE)

GS_CMD_SIMPLE(33, rotateCameraAroundPosition, NONE)

GS_CMD_SIMPLE(34, setCameraAutozoomIndoor, FLOAT)
GS_CMD_SIMPLE(35, setCameraAutozoomOutdoor, FLOAT)
GS_CMD_SIMPLE(36, saveCameraAutozoom, NONE)
GS_CMD_SIMPLE(37, loadCameraAutozoom, NONE)

GS_CMD_SIMPLE(38, setCameraFOVFloat, FLOAT)

GS_CMD_SIMPLE(39, setCameraUpVector, STRING)
GS_CMD_SIMPLE(40, setCameraNearClipValue, FLOAT)
GS_CMD_SIMPLE(41, setCameraNearClipDefault, NONE)

GS_CMD_SIMPLE(42, setCameraPositionOffsetXToFloatValue, NONE)
GS_CMD_SIMPLE(43, setCameraPositionOffsetYToFloatValue, NONE)
GS_CMD_SIMPLE(44, setCameraPositionOffsetZToFloatValue, NONE)

GS_CMD_SIMPLE(45, setCameraTargetOffsetXToFloatValue, NONE)
GS_CMD_SIMPLE(46, setCameraTargetOffsetYToFloatValue, NONE)
GS_CMD_SIMPLE(47, setCameraTargetOffsetZToFloatValue, NONE)

GS_CMD_SIMPLE(48, setCameraTargetOffsetAtPosition, NONE)
GS_CMD_SIMPLE(49, setCameraPositionOffsetAtPosition, NONE)

GS_CMD_SIMPLE(50, disableDirectCameraControls, NONE)
GS_CMD_SIMPLE(51, enableDirectCameraControls, NONE)

GS_CMD_SIMPLE(52, updateCamera, NONE)

GS_CMD_SIMPLE(53, setCameraAreaType, INT)
GS_CMD_SIMPLE(54, setCameraAreaCorner1ToPosition, NONE)
GS_CMD_SIMPLE(55, setCameraAreaCorner2ToPosition, NONE)
GS_CMD_SIMPLE(56, setCameraAreaCorner3ToPosition, NONE)
GS_CMD_SIMPLE(57, setCameraAreaCorner4ToPosition, NONE)
GS_CMD_SIMPLE(58, setCameraAreaFOV, FLOAT)
GS_CMD_SIMPLE(59, setCameraAreaAngle, FLOAT)
GS_CMD_SIMPLE(60, setCameraAreaBetaAngle, FLOAT)
GS_CMD_SIMPLE(61, setCameraAreaBank, FLOAT)
GS_CMD_SIMPLE(62, setCameraAreaOffsetX, FLOAT)
GS_CMD_SIMPLE(63, setCameraAreaOffsetY, FLOAT)
GS_CMD_SIMPLE(64, setCameraAreaOffsetZ, FLOAT)
GS_CMD_SIMPLE(65, setCameraAreaFollowX, FLOAT)
GS_CMD_SIMPLE(66, setCameraAreaFollowY, FLOAT)
GS_CMD_SIMPLE(67, setCameraAreaFollowZ, FLOAT)
GS_CMD_SIMPLE(68, setCameraAreaTargetToPosition, NONE)
GS_CMD_SIMPLE(69, setCameraAreaAnimation, STRING)
GS_CMD_SIMPLE(70, setCameraAreaGroup, INT)
GS_CMD_SIMPLE(71, addCameraArea, FLOAT)
GS_CMD_SIMPLE(72, setCameraAreaAngleToValue, NONE)
GS_CMD_SIMPLE(73, setCameraAreaName, STRING)
GS_CMD_SIMPLE(74, setCameraAreaCollision, INT)
GS_CMD_SIMPLE(75, setCameraAreaDistance, FLOAT)

GS_CMD_SIMPLE(76, actualCameraPosition, NONE)

GS_CMD_SIMPLE(77, setMapView, STRING)

GS_CMD_SIMPLE(78, moveCameraAngle, FLOAT)
GS_CMD_SIMPLE(79, moveCameraBetaAngle, FLOAT)
GS_CMD_SIMPLE(80, moveCameraBank, FLOAT)
GS_CMD_SIMPLE(81, moveCameraDistance, FLOAT)
GS_CMD_SIMPLE(82, moveCameraFOV, FLOAT)
GS_CMD_SIMPLE(83, clearCameraAreas, NONE)
GS_CMD_SIMPLE(84, isCameraSelected, STRING)

#undef GS_CMD_BASE

// up to 599

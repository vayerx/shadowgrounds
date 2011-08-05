// Copyright 2002-2004 Frozenbyte Ltd.

// from 1400

#define GS_CMD_BASE 1400

GS_CMD(0, GS_CMD_WAIT, "wait", INT)
GS_CMD(1, GS_CMD_RANDOM, "random", INT)
GS_CMD(2, GS_CMD_SETPLAYER, "setPlayer", INT)
GS_CMD(3, GS_CMD_MESSAGE, "message", STRING)
GS_CMD(4, GS_CMD_CHARACTERMESSAGE, "characterMessage", STRING)
GS_CMD(5, GS_CMD_WAITACCURATE, "waitAccurate", INT)
GS_CMD(6, GS_CMD_GETPLAYER, "getPlayer", NONE)
GS_CMD(7, GS_CMD_GETSINGLEPLAYER, "getSinglePlayer", NONE)
GS_CMD(8, GS_CMD_DISABLEVISIBILITYUPDATE, "disableVisibilityUpdate", NONE)
GS_CMD(9, GS_CMD_ENABLEVISIBILITYUPDATE, "enableVisibilityUpdate", NONE)
GS_CMD(10, GS_CMD_PAUSE, "pause", NONE)
GS_CMD(11, GS_CMD_UNPAUSE, "unpause", NONE)
GS_CMD(12, GS_CMD_CLEARMESSAGE, "clearMessage", NONE)
GS_CMD(13, GS_CMD_HINTMESSAGE, "hintMessage", STRING)
GS_CMD(14, GS_CMD_PRINTVALUE, "printValue", NONE)
GS_CMD(15, GS_CMD_DISABLECONTROLS, "disableControls", NONE)
GS_CMD(16, GS_CMD_ENABLECONTROLS, "enableControls", NONE)
GS_CMD(17, GS_CMD_DISABLEALLAI, "disableAllAI", NONE)
GS_CMD(18, GS_CMD_ENABLEALLAI, "enableAllAI", NONE)
GS_CMD(19, GS_CMD_WAITVALUE, "waitValue", NONE)
GS_CMD(20, GS_CMD_HUMANPLAYERSAMOUNT, "humanPlayersAmount", NONE)
GS_CMD(21, GS_CMD_ISCONTROLLERKEYDOWN, "isControllerKeyDown", STRING)
GS_CMD(22, GS_CMD_WASCONTROLLERKEYCLICKED, "wasControllerKeyClicked", STRING)
GS_CMD(23, GS_CMD_MAKELIGHTNING, "makeLightning", NONE)
GS_CMD(24, GS_CMD_BIND, "bind", STRING)
GS_CMD(25, GS_CMD_UNBIND, "unbind", STRING)
GS_CMD(26, GS_CMD_LISTBINDS, "listBinds", NONE)
GS_CMD(27, GS_CMD_LOADBINDS, "loadBinds", STRING)
GS_CMD(28, GS_CMD_PRELOADTEXTURE, "preloadTexture", STRING)
GS_CMD(29, GS_CMD_PRELOADTEMPORARYTEXTURE, "preloadTemporaryTexture", STRING)
GS_CMD(30, GS_CMD_CLEANTEMPORARYTEXTURECACHE, "cleanTemporaryTextureCache", NONE)
GS_CMD(31, GS_CMD_SETGLOBALTIMEFACTOR, "setGlobalTimeFactor", STRING)
GS_CMD(32, GS_CMD_SETPLAYERTOVALUE, "setPlayerToValue", NONE)
GS_CMD(33, GS_CMD_CENTERMESSAGE, "centerMessage", STRING)
GS_CMD(34, GS_CMD_QUIT, "quit", NONE)
GS_CMD(35, GS_CMD_ADDPARTICLESPAWNER, "addParticleSpawner", STRING)
GS_CMD(36, GS_CMD_MAKEHEIGHTAREABLOCKED, "makeHeightAreaBlocked", NONE)

GS_CMD(37, GS_CMD_RANDOMBELOWCURRENTVALUE, "randomBelowCurrentValue", NONE)
GS_CMD(38, GS_CMD_RANDOMOFFSETVALUE, "randomOffsetValue", INT)
GS_CMD(39, GS_CMD_RANDOMPOSITIVEOFFSETVALUE, "randomPositiveOffsetValue", INT)
GS_CMD(40, GS_CMD_RANDOMNEGATIVEOFFSETVALUE, "randomNegativeOffsetValue", INT)

GS_CMD(41, GS_CMD_ENABLEHOSTILEAI, "enableHostileAI", NONE)
GS_CMD(42, GS_CMD_DISABLEHOSTILEAI, "disableHostileAI", NONE)
GS_CMD(43, GS_CMD_DISABLEAIFORPLAYER, "disableAIForPlayer", INT)
GS_CMD(44, GS_CMD_ENABLEAIFORPLAYER, "enableAIForPlayer", INT)

GS_CMD(45, GS_CMD_RESTOREPARTTYPEORIGINALS, "restorePartTypeOriginals", NONE)

GS_CMD(46, GS_CMD_DISABLEPARTICLESPAWNERBYNAME, "disableParticleSpawnerByName", STRING)
GS_CMD(47, GS_CMD_ENABLEPARTICLESPAWNERBYNAME, "enableParticleSpawnerByName", STRING)

GS_CMD(48, GS_CMD_SPAWNPROJECTILE, "spawnProjectile", STRING)

GS_CMD(49, GS_CMD_SETCONTROLLERKEYDOWN, "setControllerKeyDown", STRING)
GS_CMD(50, GS_CMD_SETCONTROLLERKEYUP, "setControllerKeyUp", STRING)

GS_CMD(51, GS_CMD_STARTSCRIPTPROCESS, "startScriptProcess", STRING)
GS_CMD(52, GS_CMD_STARTSCRIPTPROCESSFORUNIT, "startScriptProcessForUnit", STRING)

GS_CMD(53, GS_CMD_FORCEBUILDINGROOFHIDE, "forceBuildingRoofHide", NONE)
GS_CMD(54, GS_CMD_FORCEBUILDINGROOFSHOW, "forceBuildingRoofShow", NONE)
GS_CMD(55, GS_CMD_ENDFORCEDBUILDINGROOF, "endForcedBuildingRoof", NONE)

GS_CMD(56, GS_CMD_HIDESKYMODEL, "hideSkyModel", NONE)
GS_CMD(57, GS_CMD_SHOWSKYMODEL, "showSkyModel", NONE)

GS_CMD(58, GS_CMD_SAVEPERMANENTVARIABLESTOTEMPORARY, "savePermanentVariablesToTemporary", NONE)
GS_CMD(59, GS_CMD_LOADPERMANENTVARIABLESFROMTEMPORARY, "loadPermanentVariablesFromTemporary", NONE)

GS_CMD(60, GS_CMD_SAVEGAME, "saveGame", STRING)
GS_CMD(61, GS_CMD_LOADGAME, "loadGame", STRING)

GS_CMD(62, GS_CMD_EXECUTETIPMESSAGE, "executeTipMessage", STRING)
GS_CMD(63, GS_CMD_PRIORITYEXECUTETIPMESSAGE, "priorityExecuteTipMessage", STRING)
GS_CMD(64, GS_CMD_STOPSCRIPTPROCESSBYID, "stopScriptProcessById", NONE)

GS_CMD(65, GS_CMD_PLAYLIPSYNC, "playLipsync", STRING)

GS_CMD(66, GS_CMD_SAVEGAMETYPE, "savegameType", STRING)
GS_CMD(67, GS_CMD_SAVEGAMEDESCRIPTION, "savegameDescription", STRING)
GS_CMD(68, GS_CMD_SAVEGAMETIME, "savegameTime", STRING)
GS_CMD(69, GS_CMD_SAVEGAMEVERSION, "savegameVersion", STRING)

GS_CMD(70, GS_CMD_WAITTICK, "waitTick", NONE)

GS_CMD(71, GS_CMD_FORCECONTROLLERKEYENABLED, "forceControllerKeyEnabled", STRING)

GS_CMD(72, GS_CMD_GETSCRIPTPROCESSID, "getScriptProcessId", NONE)
GS_CMD(73, GS_CMD_ISSCRIPTPROCESSOFVALUECINEMATIC, "isScriptProcessOfValueCinematic", NONE)


GS_CMD(74, GS_CMD_CLEARCENTERMESSAGE, "clearCenterMessage", NONE)
GS_CMD(75, GS_CMD_CLEARHINTMESSAGE, "clearHintMessage", NONE)
GS_CMD(76, GS_CMD_CLEAREXECUTETIPMESSAGE, "clearExecuteTipMessage", NONE)

GS_CMD_SIMPLE(77, enableMaterialScrollByName, STRING)
GS_CMD_SIMPLE(78, disableMaterialScrollByName, STRING)

GS_CMD_SIMPLE(79, addAlienRandomSpawnPoint, STRING)
GS_CMD_SIMPLE(80, resetAlienSpawner, NONE)
GS_CMD_SIMPLE(81, enableAlienSpawner, NONE)
GS_CMD_SIMPLE(82, disableAlienSpawner, NONE)
GS_CMD_SIMPLE(83, disableAlienSpawnerSpawnPoint, STRING)
GS_CMD_SIMPLE(84, enableAlienSpawnerSpawnPoint, STRING)
GS_CMD_SIMPLE(85, setAlienSpawnerSpawnRate, INT)
GS_CMD_SIMPLE(86, setAlienSpawnerNextSpawnDelay, INT)

GS_CMD_SIMPLE(87, setProfile, STRING)

GS_CMD_SIMPLE(88, statDeath, NONE)
GS_CMD_SIMPLE(89, statKill, STRING)
GS_CMD_SIMPLE(90, statPickup, STRING)
GS_CMD_SIMPLE(91, statMarker, STRING)

GS_CMD_SIMPLE(92, deleteAllProjectiles, NONE)
GS_CMD_SIMPLE(93, disablePathfindPortals, NONE)
GS_CMD_SIMPLE(94, enablePathfindPortals, NONE)

GS_CMD_SIMPLE(95, getLocaleStringLength, STRING)

GS_CMD_SIMPLE(96, characterMessageNoFace, STRING)

GS_CMD_SIMPLE(97, localizeSubtitleStringValue, NONE)
GS_CMD_SIMPLE(98, localizeGUIStringValue, NONE)
GS_CMD_SIMPLE(99, localizeSpeechStringValue, NONE)

// oops, was in string scripting.
//GS_CMD_SIMPLE(xxx, printStringValue, NONE)

GS_CMD_SIMPLE(100, printValueToConsole, NONE)
GS_CMD_SIMPLE(101, printStringValueToConsole, NONE)

GS_CMD_SIMPLE(102, resetRenderer, NONE)
GS_CMD_SIMPLE(103, reloadClawSettings, NONE)

GS_CMD_SIMPLE(104, saveGridOcclusionCulling, STRING)
GS_CMD_SIMPLE(105, loadGridOcclusionCulling, STRING)
GS_CMD_SIMPLE(106, applyGridOcclusionCulling, NONE)
GS_CMD_SIMPLE(107, previewGridOcclusionCullingByValue, NONE)

GS_CMD_SIMPLE(108, enableClaw, NONE)
GS_CMD_SIMPLE(109, disableClaw, NONE)
GS_CMD_SIMPLE(110, setClawToPosition, NONE)

GS_CMD_SIMPLE(111, spawnRandomAt, STRING)

GS_CMD_SIMPLE(112, chatConnect, NONE)
GS_CMD_SIMPLE(113, chatSend, STRING)
GS_CMD_SIMPLE(114, chatReceive, NONE)
GS_CMD_SIMPLE(115, consoleMiniQuery, STRING)
GS_CMD_SIMPLE(116, isScriptLoaded, STRING)
GS_CMD_SIMPLE(117, chatSendMessage, STRING)

GS_CMD_SIMPLE(118, openCombatSubWindow, STRING)
GS_CMD_SIMPLE(119, closeCombatSubWindow, STRING)

GS_CMD_SIMPLE(120, setElaborateHintStyle, STRING)
GS_CMD_SIMPLE(121, elaborateHint, STRING )

GS_CMD_SIMPLE(122, isUnifiedHandleValid, NONE)
GS_CMD_SIMPLE(123, isUnifiedHandleTerrainObject, NONE)
GS_CMD_SIMPLE(124, isUnifiedHandleUnit, NONE)
GS_CMD_SIMPLE(125, isUnifiedHandleTracker, NONE)
GS_CMD_SIMPLE(126, isUnifiedHandleLight, NONE)
GS_CMD_SIMPLE(127, isUnifiedHandleAmbientSound, NONE)
GS_CMD_SIMPLE(128, isUnifiedHandleItem, NONE)
GS_CMD_SIMPLE(129, isUnifiedHandleParticleSpawner, NONE)
GS_CMD_SIMPLE(130, doesUnifiedHandleObjectExist, NONE)

GS_CMD_SIMPLE(131, getUnifiedHandleObjectPosition, NONE)
GS_CMD_SIMPLE(132, setUnifiedHandleObjectPosition, NONE)
GS_CMD_SIMPLE(133, getUnifiedHandleObjectRotation, NONE)
GS_CMD_SIMPLE(134, setUnifiedHandleObjectRotation, NONE)
GS_CMD_SIMPLE(135, getUnifiedHandleObjectVelocity, NONE)
GS_CMD_SIMPLE(136, setUnifiedHandleObjectVelocity, NONE)
GS_CMD_SIMPLE(137, getUnifiedHandleObjectCenterPosition, NONE)
GS_CMD_SIMPLE(138, setUnifiedHandleObjectCenterPosition, NONE)

GS_CMD_SIMPLE(139, getTerrainObjectMetaValueString, STRING)

GS_CMD_SIMPLE(140, getUnifiedHandleObjectDistanceToPosition, NONE)
GS_CMD_SIMPLE(141, getUnifiedHandleObjectDistanceToPositionFloat, NONE)

GS_CMD_SIMPLE(142, physSetGroupCont, STRING)
GS_CMD_SIMPLE(143, physSetGroupColl, STRING)

GS_CMD_SIMPLE(144, doesReplacementForTerrainObjectExist, NONE)
GS_CMD_SIMPLE(145, getReplacementForTerrainObject, NONE)

GS_CMD_SIMPLE(146, unifiedHandleToValue, NONE)
GS_CMD_SIMPLE(147, valueToUnifiedHandle, NONE)

GS_CMD_SIMPLE(148, findClosestTerrainObjectOfMaterial, STRING)
GS_CMD_SIMPLE(149, getTerrainObjectPosition, NONE)

GS_CMD_SIMPLE(150, disableAllParticleSpawners, NONE)
GS_CMD_SIMPLE(151, deleteAllParticleSpawners, NONE)

GS_CMD_SIMPLE(152, hasTerrainObjectMetaValueString, STRING)

GS_CMD_SIMPLE(153, getTerrainObjectVariable, STRING)
GS_CMD_SIMPLE(154, setTerrainObjectVariable, STRING)

GS_CMD_SIMPLE(155, getControllerCursorScreenPositionX, NONE)
GS_CMD_SIMPLE(156, getControllerCursorScreenPositionY, NONE)

GS_CMD_SIMPLE(157, isControllerSceneSelectionAvailable, NONE)
GS_CMD_SIMPLE(158, getControllerSceneSelectionPosition, NONE)
GS_CMD_SIMPLE(159, getControllerSceneSelectionUnit, NONE)

GS_CMD_SIMPLE(160, changeTerrainObjectTo, STRING)

GS_CMD_SIMPLE(161, startScriptProcessForUnifiedHandle, STRING)

GS_CMD_SIMPLE(162, deleteTerrainObject, NONE)

GS_CMD_SIMPLE(163, setTerrainObjectDamageTextureFadeFactorToFloatValue, NONE)
GS_CMD_SIMPLE(164, setTerrainObjectByIdString, STRING)
GS_CMD_SIMPLE(165, getTerrainObjectIdString, NONE)

GS_CMD_SIMPLE(166, spawnProjectileWithShooter, STRING)
GS_CMD_SIMPLE(167, savegameStats, STRING)
GS_CMD_SIMPLE(168, statMarkTimeOfDeath, NONE)
GS_CMD_SIMPLE(169, getUnitPlayerNumber, NONE)
GS_CMD_SIMPLE(170, getShooter, NONE)

GS_CMD_SIMPLE(171, resetWorldFold, NONE)
GS_CMD_SIMPLE(172, addWorldFoldToPosition, FLOAT)
GS_CMD_SIMPLE(173, getWorldFoldNumberAtPosition, NONE)
GS_CMD_SIMPLE(174, moveWorldFoldToPosition, NONE)
GS_CMD_SIMPLE(175, setWorldFoldAngle, FLOAT)

GS_CMD_SIMPLE(176, setEngineMetaValueInt, STRING)
GS_CMD_SIMPLE(177, getEngineMetaValueInt, STRING)
GS_CMD_SIMPLE(178, defineEngineMetaValueInt, STRING)

GS_CMD_SIMPLE(179, getControllerSceneSelectionUnifiedHandle, NONE)
GS_CMD_SIMPLE(180, isRunningScriptProcessForUnit, STRING)

GS_CMD_SIMPLE(181, runScriptProcessImmediately, STRING)
GS_CMD_SIMPLE(182, runScriptProcessImmediatelyForUnit, STRING)
GS_CMD_SIMPLE(183, isUnitPlayerControlled, NONE)
GS_CMD_SIMPLE(184, openCombatSubWindowWithText, STRING)
GS_CMD_SIMPLE(185, doesFileExist, STRING)

GS_CMD_SIMPLE(186, swapCursors, STRING)
GS_CMD_SIMPLE(187, resetSwappedCursors, NONE)

#undef GS_CMD_BASE

// up to 1599

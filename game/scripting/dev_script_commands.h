// Copyright 2002-2004 Frozenbyte Ltd.

// from 700

#define GS_CMD_BASE 700

GS_CMD(0, GS_CMD_ERROR, "error", STRING)
GS_CMD(1, GS_CMD_RELOADSCRIPTFILE, "reloadScriptFile", STRING)
GS_CMD(2, GS_CMD_SCRIPTDUMP, "scriptDump", NONE)
GS_CMD(3, GS_CMD_FULLSCRIPTDUMP, "fullScriptDump", NONE)
GS_CMD(4, GS_CMD_DEVMESSAGE, "devMessage", STRING)
GS_CMD(5, GS_CMD_HIDECONSOLE, "hideConsole", NONE)
GS_CMD(6, GS_CMD_SHOWCONSOLE, "showConsole", NONE)
GS_CMD(7, GS_CMD_DEVSIDESWAP, "devSideSwap", INT)
GS_CMD(8, GS_CMD_DEVUNIT, "devUnit", NONE)
GS_CMD(9, GS_CMD_CLEARDEVUNIT, "clearDevUnit", NONE)
GS_CMD(10, GS_CMD_DUMPSTATUSINFO, "dumpStatusInfo", NONE)
GS_CMD(11, GS_CMD_DEVSIDESWAPTOVALUE, "devSideSwapToValue", NONE)
GS_CMD(12, GS_CMD_RELOADSTRINGVALUESCRIPTFILE, "reloadStringValueScriptFile", NONE)

GS_CMD(13, GS_CMD_RAYTRACEBLAST, "raytraceBlast", NONE)
GS_CMD(14, GS_CMD_DEVUNITSCRIPTDUMP, "devUnitScriptDump", NONE)
GS_CMD(15, GS_CMD_DEVUNITFULLSCRIPTDUMP, "devUnitFullScriptDump", NONE)

GS_CMD(16, GS_CMD_DEVEXIT, "devExit", NONE)
GS_CMD(17, GS_CMD_DEVASSERT, "devAssert", NONE)
GS_CMD(18, GS_CMD_DEVCRASH, "devCrash", NONE)

GS_CMD(19, GS_CMD_LISTGLOBALVARIABLES, "listGlobalVariables", NONE)

GS_CMD_SIMPLE(20, dumpGameSceneGraph, NONE)
GS_CMD_SIMPLE(21, dumpPhysicsInfo, NONE)
GS_CMD_SIMPLE(22, reloadParticleEffects, NONE)

GS_CMD_SIMPLE(23, dumpEffectsInfo, NONE)
GS_CMD_SIMPLE(24, reloadObjectDurabilities, NONE)

GS_CMD_SIMPLE(25, setHealthTextMultiplier, FLOAT)

GS_CMD_SIMPLE(26, openScoreWindow, NONE)
GS_CMD_SIMPLE(27, openMissionSelectionWindow, NONE)
GS_CMD_SIMPLE(28, missionSelectionWindowAddMissionButton, STRING)
GS_CMD_SIMPLE(29, openMainMenuWindow, INT)

GS_CMD_SIMPLE(30, devRunSingleCommand, STRING)

GS_CMD_SIMPLE(31, dumpScriptInfo, NONE)
GS_CMD_SIMPLE(32, devStopAllCustomScripts, NONE)
GS_CMD_SIMPLE(33, forceCursorVisibility, INT)

GS_CMD_SIMPLE(34, dumpMemoryInfo, NONE)

GS_CMD_SIMPLE(35, devPhysicsConnectToRemoteDebugger, STRING)

GS_CMD_SIMPLE(36, clearSelectionVisualizationForUnifiedHandle, NONE)
GS_CMD_SIMPLE(37, setSelectionVisualizationForUnifiedHandle, NONE)

GS_CMD_SIMPLE(38, reloadChangedScriptFiles, NONE)
GS_CMD_SIMPLE(40, restartApplicationSoft, NONE)
GS_CMD_SIMPLE(39, restartApplicationHard, NONE)

#undef GS_CMD_BASE

// up to 799

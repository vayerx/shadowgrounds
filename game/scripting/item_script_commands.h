// Copyright 2002-2004 Frozenbyte Ltd.

// from 1000

#define GS_CMD_BASE 1000

GS_CMD(0, GS_CMD_REMOVEITEM, "removeItem", NONE)
GS_CMD(1, GS_CMD_ADDITEM, "addItem", STRING)

GS_CMD(2, GS_CMD_REMOVECLOSESTITEMOFTYPE, "removeClosestItemOfType", STRING)
GS_CMD(3, GS_CMD_STOPBLINKINGFORCLOSESTITEMOFTYPE, "stopBlinkingForClosestItemOfType", STRING)
GS_CMD(4, GS_CMD_STARTBLINKINGFORCLOSESTITEMOFTYPE, "startBlinkingForClosestItemOfType", STRING)

GS_CMD(5, GS_CMD_DISABLEITEM, "disableItem", INT)
GS_CMD(6, GS_CMD_DISABLECLOSESTITEMOFTYPE, "disableClosestItemOfType", STRING)

GS_CMD(7, GS_CMD_SETPROGRESSLABEL, "setProgressLabel", STRING)
GS_CMD(8, GS_CMD_SETPROGRESSTOTALTIME, "setProgressTotalTime", INT)
GS_CMD(9, GS_CMD_SETPROGRESSTICKTIME, "setProgressTickTime", INT)
GS_CMD(10, GS_CMD_SETPROGRESSDONELABEL, "setProgressDoneLabel", STRING)
GS_CMD(11, GS_CMD_SETPROGRESSBARIMAGE, "setProgressBarImage", STRING)
GS_CMD(12, GS_CMD_SETPROGRESSBARBORDERIMAGE, "setProgressBarBorderImage", STRING)
GS_CMD(13, GS_CMD_GETPROGRESSDONETIME, "getProgressDoneTime", NONE)
GS_CMD(14, GS_CMD_STARTPROGRESS, "startProgress", NONE)
GS_CMD(15, GS_CMD_STOPPROGRESS, "stopProgress", NONE)
GS_CMD(16, GS_CMD_INTERRUPTPROGRESS, "interruptProgress", NONE)

GS_CMD(17, GS_CMD_SETPROGRESSINTERRUPTEDLABEL, "setProgressInterruptedLabel", STRING)

GS_CMD(18, GS_CMD_CHANGEITEMMODEL, "changeItemModel", STRING)
GS_CMD(19, GS_CMD_CHANGEITEMMODELFORCLOSESTITEMOFTYPETOSTRINGVALUE, "changeItemModelForClosestItemOfTypeToStringValue", STRING)

GS_CMD(20, GS_CMD_DELETELONGTIMEDISABLEDITEMS, "deleteLongTimeDisabledItems", NONE)
GS_CMD(21, GS_CMD_REENABLEALLTIMEDISABLEDITEMS, "reEnableAllTimeDisabledItems", NONE)

GS_CMD(22, GS_CMD_setItemHighlightStyle, "setItemHighlightStyle", INT)
GS_CMD(23, GS_CMD_setItemHighlightText, "setItemHighlightText", STRING)
GS_CMD(24, GS_CMD_setItemHighlightTextBySpecialString, "setItemHighlightTextBySpecialString", NONE )

GS_CMD(25, GS_CMD_openTerminalWindow, "openTerminalWindow", STRING)
GS_CMD(26, GS_CMD_closeTerminalWindow, "closeTerminalWindow", NONE)

GS_CMD(27, GS_CMD_setItemSpecialString, "setItemSpecialString", STRING)
GS_CMD(28, GS_CMD_setItemSpecialStringNull, "setItemSpecialStringNull", NONE)

GS_CMD(29, GS_CMD_openTerminalWindowByItemSpecialString, "openTerminalWindowByItemSpecialString", NONE)
GS_CMD(30, GS_CMD_openTerminalWindowByStringValue, "openTerminalWindowByStringValue", NONE)
GS_CMD(31, GS_CMD_getItemSpecialString, "getItemSpecialString", NONE)

GS_CMD(32, GS_CMD_setItemCustomTipText, "setItemCustomTipText", STRING)
GS_CMD(33, GS_CMD_setItemCustomTipTextNull, "setItemCustomTipTextNull", NONE)
GS_CMD(34, GS_CMD_getItemCustomTipText, "getItemCustomTipText", NONE)

GS_CMD(35, GS_CMD_setItemCustomHighlightText, "setItemCustomHighlightText", STRING)
GS_CMD(36, GS_CMD_setItemCustomHighlightTextNull, "setItemCustomHighlightTextNull", NONE)

GS_CMD_SIMPLE( 37, setProgressInterruptPercent, INT )
GS_CMD_SIMPLE( 38, createItemSpawner, STRING )
GS_CMD_SIMPLE( 39, activateItemSpawnerGroup, STRING )
GS_CMD_SIMPLE( 40, deactivateItemSpawnerGroup, STRING )
GS_CMD_SIMPLE( 41, itemPosition, NONE )

#undef GS_CMD_BASE

// up to 1099

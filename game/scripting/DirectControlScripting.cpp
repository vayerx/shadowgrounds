
// Copyright(C) Jukka Kokkonen, 2007

#include "precompiled.h"

#include "DirectControlScripting.h"

#include "scripting_macros_start.h"
#include "directcontrol_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>

#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../Game.h"
#include "../direct_controls.h"
#include "../ScriptableAIDirectControl.h"
#include "../IAIDirectControl.h"
#include "../../util/assert.h"
#include "GameScripting.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../../system/Logger.h"

#include "../../util/Debug_MemoryManager.h"


// WARNING: unsafe direct control scripting feature...
// define this to remove some type/parameter checks for the direct control scripting
// (will give a small performance increase, but will make the program crash if 
// a script error occurs in direct control scripts.)

//#define UNSAFE_DIRECT_CONTROL_SCRIPTING



namespace game
{
	void DirectControlScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game, bool *pause)
	{
		Unit *unit = gsd->unit;
		int intData = intFloat.i;

		switch(command)
		{
		case GS_CMD_directControlOnByName:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
					// TODO: optimize, should store the cached id to intData or something?
					int directControlId = getDirectControlIdForName(stringData);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					if (directControlId != DIRECT_CTRL_INVALID)
					{
						if (unit->isDirectControl()
							&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
							&& unit->getAIDirectControl() != NULL)
						{
							assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
							if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
							{
#endif
								ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
								sdc->enableAction(directControlId);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
							} else {
								sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
							}
						} else {
							sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
						}
					} else {
						sp->error("DirectControlScripting::process - directControlOnByName parameter invalid (expected direct control name).");
					}
#endif
				} else {
					sp->error("DirectControlScripting::process - Attempt to directControlOnByName for null unit.");
				}
			} else {
				sp->error("DirectControlScripting::process - directControlOnByName parameter missing.");
			}
			break;

		case GS_CMD_directControlOffByName:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
					// TODO: optimize, should store the cached id to intData or something?
					int directControlId = getDirectControlIdForName(stringData);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					if (directControlId != DIRECT_CTRL_INVALID)
					{
						if (unit->isDirectControl()
							&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
							&& unit->getAIDirectControl() != NULL)
						{
							assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
							if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
							{
#endif
								ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
								sdc->disableAction(directControlId);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
							} else {
								sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
							}
						} else {
							sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
						}
					} else {
						sp->error("DirectControlScripting::process - directControlOffByName parameter invalid (expected direct control name).");
					}
#endif
				} else {
					sp->error("DirectControlScripting::process - Attempt to directControlOffByName for null unit.");
				}
			} else {
				sp->error("DirectControlScripting::process - directControlOffByName parameter missing.");
			}
			break;

		case GS_CMD_directControlForwardOn:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->enableAction(DIRECT_CTRL_FORWARD);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlForwardOff:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->disableAction(DIRECT_CTRL_FORWARD);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlLeftOn:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->enableAction(DIRECT_CTRL_LEFT);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlLeftOff:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->disableAction(DIRECT_CTRL_LEFT);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlRightOn:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->enableAction(DIRECT_CTRL_RIGHT);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlRightOff:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->disableAction(DIRECT_CTRL_RIGHT);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlBackwardOn:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->enableAction(DIRECT_CTRL_BACKWARD);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlBackwardOff:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->disableAction(DIRECT_CTRL_BACKWARD);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlFireOn:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->enableAction(DIRECT_CTRL_FIRE);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlFireOff:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->disableAction(DIRECT_CTRL_FIRE);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlFireSecondaryOn:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->enableAction(DIRECT_CTRL_FIRE_SECONDARY);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlFireSecondaryOff:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->disableAction(DIRECT_CTRL_FIRE_SECONDARY);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControl..., unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControl..., unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControl... for null unit.");
			}
			break;

		case GS_CMD_directControlListenToEvent:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					ScriptableAIDirectControl::EVENT_MASK eventMask = ScriptableAIDirectControl::getEventMaskByName(stringData);
					if (eventMask != ScriptableAIDirectControl::EVENT_MASK_INVALID)
					{
						if (unit->isDirectControl()
							&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
							&& unit->getAIDirectControl() != NULL)
						{
							assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
							if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
							{
#endif
								ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
								sdc->enableEvent(eventMask);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
							} else {
								sp->error("DirectControlScripting::process - directControlListenToEvent, unit's \"directcontroltype\" is not \"scriptable\".");
							}
						} else {
							sp->error("DirectControlScripting::process - directControlListenToEvent, unit is not \"directcontrol\" type or not controlled by AI.");
						}
					} else {
						sp->error("DirectControlScripting::process - directControlListenToEvent parameter invalid (expected direct control event name).");
					}
#endif
				} else {
					sp->error("DirectControlScripting::process - Attempt to directControlListenToEvent for null unit.");
				}
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControlListenToEvent parameter missing.");
			}
			break;

		case GS_CMD_directControlDoNotListenToEvent:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					ScriptableAIDirectControl::EVENT_MASK eventMask = ScriptableAIDirectControl::getEventMaskByName(stringData);
					if (eventMask != ScriptableAIDirectControl::EVENT_MASK_INVALID)
					{
						if (unit->isDirectControl()
							&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
							&& unit->getAIDirectControl() != NULL)
						{
							assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
							if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
							{
#endif
								ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
								sdc->disableEvent(eventMask);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
							} else {
								sp->error("DirectControlScripting::process - directControlDoNotListenToEvent, unit's \"directcontroltype\" is not \"scriptable\".");
							}
						} else {
							sp->error("DirectControlScripting::process - directControlDoNotListenToEvent, unit is not \"directcontrol\" type or not controlled by AI.");
						}
					} else {
						sp->error("DirectControlScripting::process - directControlDoNotListenToEvent parameter invalid (expected direct control event name).");
					}
#endif
				} else {
					sp->error("DirectControlScripting::process - Attempt to directControlDoNotListenToEvent for null unit.");
				}
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControlDoNotListenToEvent parameter missing.");
			}
			break;

		case GS_CMD_directControlAutomaticallyDisableAction:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
					int directControlId = getDirectControlIdForName(stringData);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					if (directControlId != DIRECT_CTRL_INVALID)
					{
						if (unit->isDirectControl()
							&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
							&& unit->getAIDirectControl() != NULL)
						{
							assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
							if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
							{
#endif
								ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
								sdc->addActionToDisableAutomatically(directControlId);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
							} else {
								sp->error("DirectControlScripting::process - directControlAutomaticallyDisableAction, unit's \"directcontroltype\" is not \"scriptable\".");
							}
						} else {
							sp->error("DirectControlScripting::process - directControlAutomaticallyDisableAction, unit is not \"directcontrol\" type or not controlled by AI.");
						}
					} else {
						sp->error("DirectControlScripting::process - directControlAutomaticallyDisableAction parameter invalid (expected direct control name).");
					}
#endif
				} else {
					sp->error("DirectControlScripting::process - Attempt to directControlAutomaticallyDisableAction for null unit.");
				}
			} else {
				sp->error("DirectControlScripting::process - directControlAutomaticallyDisableAction parameter missing.");
			}
			break;

		case GS_CMD_directControlDoNotAutomaticallyDisableAction:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
					int directControlId = getDirectControlIdForName(stringData);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					if (directControlId != DIRECT_CTRL_INVALID)
					{
						if (unit->isDirectControl()
							&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
							&& unit->getAIDirectControl() != NULL)
						{
							assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
							if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
							{
#endif
								ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
								sdc->removeActionToDisableAutomatically(directControlId);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
							} else {
								sp->error("DirectControlScripting::process - directControlDoNotAutomaticallyDisableAction, unit's \"directcontroltype\" is not \"scriptable\".");
							}
						} else {
							sp->error("DirectControlScripting::process - directControlDoNotAutomaticallyDisableAction, unit is not \"directcontrol\" type or not controlled by AI.");
						}
					} else {
						sp->error("DirectControlScripting::process - directControlDoNotAutomaticallyDisableAction parameter invalid (expected direct control name).");
					}
#endif
				} else {
					sp->error("DirectControlScripting::process - Attempt to directControlDoNotAutomaticallyDisableAction for null unit.");
				}
			} else {
				sp->error("DirectControlScripting::process - directControlDoNotAutomaticallyDisableAction parameter missing.");
			}
			break;

		case GS_CMD_directControlSetTimerMsec:
			if (intData >= GAME_TICK_MSEC)
			{
				if (unit != NULL)
				{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					if (unit->isDirectControl()
						&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
						&& unit->getAIDirectControl() != NULL)
					{
						assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
						if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
						{
#endif
							ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
							sdc->setTimerEventParameters(intData);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
						} else {
							sp->error("DirectControlScripting::process - directControlSetTimerMsec, unit's \"directcontroltype\" is not \"scriptable\".");
						}
					} else {
						sp->error("DirectControlScripting::process - directControlSetTimerMsec, unit is not \"directcontrol\" type or not controlled by AI.");
					}
#endif
				} else {
					sp->error("DirectControlScripting::process - Attempt to directControlSetTimerMsec for null unit.");
				}
			} else {
				sp->error("DirectControlScripting::process - directControlSetTimerMsec parameter bad (value less than one tick in msec).");
			}
			break;

		case GS_CMD_directControlSetAimMode:
			if (stringData != NULL)
			{
				if (unit != NULL)
				{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					ScriptableAIDirectControl::AIM_MODE aimMode = ScriptableAIDirectControl::getAimModeByName(stringData);
					if (aimMode != ScriptableAIDirectControl::AIM_MODE_INVALID)
					{
						if (unit->isDirectControl()
							&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
							&& unit->getAIDirectControl() != NULL)
						{
							assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
							if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
							{
#endif
								ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
								sdc->setAimMode(aimMode);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
							} else {
								sp->error("DirectControlScripting::process - directControlSetAimMode, unit's \"directcontroltype\" is not \"scriptable\".");
							}
						} else {
							sp->error("DirectControlScripting::process - directControlSetAimMode, unit is not \"directcontrol\" type or not controlled by AI.");
						}
					} else {
						sp->error("DirectControlScripting::process - directControlSetAimMode parameter bad (expected offset_self, absolute, unit_target or target_unified_handle).");
					}
#endif
				} else {
					sp->error("DirectControlScripting::process - Attempt to directControlSetAimMode for null unit.");
				}
			} else {
				sp->error("DirectControlScripting::process - directControlSetAimMode parameter missing.");
			}
			break;

		case GS_CMD_directControlSetAimPositionIgnoringMode:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						sdc->setAimPosition(gsd->position);
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControlSetAimPositionIgnoringMode, unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControlSetAimPositionIgnoringMode, unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControlSetAimPositionIgnoringMode for null unit.");
			}
			break;

		case GS_CMD_directControlSetAimToPositionByMode:
			if (unit != NULL)
			{
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
				if (unit->isDirectControl()
					&& unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_AI
					&& unit->getAIDirectControl() != NULL)
				{
					assert(unit->getAIDirectControl()->getDirectControlName() != NULL);
					if (strcmp(unit->getAIDirectControl()->getDirectControlName(), "scriptable") == 0)
					{
#endif
						ScriptableAIDirectControl *sdc = (ScriptableAIDirectControl *)unit->getAIDirectControl();
						if (sdc->getAimMode() == ScriptableAIDirectControl::AIM_MODE_OFFSET_SELF)
						{
							VC3 offset = gsd->position - unit->getPosition();
							sdc->setAimPosition(offset);
						}
						else if (sdc->getAimMode() == ScriptableAIDirectControl::AIM_MODE_ABSOLUTE)
						{
							sdc->setAimPosition(gsd->position);
						}
						else if (sdc->getAimMode() == ScriptableAIDirectControl::AIM_MODE_OFFSET_UNIT_TARGET)
						{
							if (unit->targeting.hasTarget())
							{
								if (unit->targeting.getTargetUnit() != NULL)
								{
									VC3 offset = gsd->position - unit->targeting.getTargetUnit()->getPosition();
									sdc->setAimPosition(offset);
								} else {
									VC3 offset = gsd->position - unit->targeting.getTargetPosition();
									sdc->setAimPosition(offset);
								}
							} else {
								// no target, what should we do in this case?
								sp->warning("DirectControlScripting::process - directControlSetAimToPositionByMode, unit aim mode is \"offset_unit_target\" but unit has no target.");
							}
						}
						else if (sdc->getAimMode() == ScriptableAIDirectControl::AIM_MODE_OFFSET_TARGET_UNIFIED_HANDLE)
						{
							// TODO: ...
							assert(!"AIM_MODE_OFFSET_TARGET_UNIFIED_HANDLE");
						}
#ifndef UNSAFE_DIRECT_CONTROL_SCRIPTING
					} else {
						sp->error("DirectControlScripting::process - directControlSetAimToPositionByMode, unit's \"directcontroltype\" is not \"scriptable\".");
					}
				} else {
					sp->error("DirectControlScripting::process - directControlSetAimToPositionByMode, unit is not \"directcontrol\" type or not controlled by AI.");
				}
#endif
			} else {
				sp->error("DirectControlScripting::process - Attempt to directControlSetAimToPositionByMode for null unit.");
			}
			break;

		default:
			sp->error("DirectControlScripting::process - Unknown command.");
			assert(0);
		}
	}
}



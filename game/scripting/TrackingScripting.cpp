
#include "precompiled.h"

#include "TrackingScripting.h"

#include "scripting_macros_start.h"
#include "tracking_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>
#include <IStorm3D.h>

#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../GameMap.h"
#include "../UnitList.h"
#include "../scaledefs.h"

#include "../../system/Logger.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../DHLocaleManager.h"

#include "../tracking/ScriptableTrackerObjectType.h"
#include "../tracking/ScriptableTrackerObject.h"
#include "../tracking/ObjectTracker.h"
#include "../tracking/ITrackableObject.h"
#include "../tracking/ITrackerObject.h"
#include "../tracking/ITrackableObjectIterator.h"
#include "../tracking/TrackableUnifiedHandleObject.h"
#include "../UnifiedHandleManager.h"
#include "../unified_handle.h"

#include "../PartType.h"

#include "../../util/Debug_MemoryManager.h"

#include "GameScripting.h"


using namespace ui;
using namespace game::tracking;

// HACK: ...
game::Unit *hackhack_trackerunit = NULL;


// WARNING: unsafe tracker scripting feature...
// if this is defined, then the tracker script commands will not have proper 
// error checking, causing possible crashes if tracker scripting errors 
// occur. (this should however give a small performance increase thanks to being unsafe)
#define UNSAFE_TRACKER_SCRIPTING


namespace game
{
	// HACK: ...
	// TODO: cleanup these at exit.
	std::vector<tracking::ScriptableTrackerObjectType *> gs_scriptableTrackers;

	// the part type id (bullet type id) for a projectile tracker to be created...
	std::string gs_trackerProjectileBullet = "";

	UnifiedHandle gs_currentlyIteratedTrackableUnifiedHandle = UNIFIED_HANDLE_NONE;


	void TrackingScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game, bool *pause)
	{
		int intData = intFloat.i;
		switch(command)
		{
		case GS_CMD_createScriptableTrackerType:
			if (stringData != NULL)
			{				
				if (game->objectTracker->getTrackerTypeNumberByName(stringData) == -1)
				{
					tracking::ScriptableTrackerObjectType::setGameScripting(game->gameScripting);

					tracking::ScriptableTrackerObjectType *st = new tracking::ScriptableTrackerObjectType();
					util::Script *s = util::ScriptManager::getInstance()->getScript(stringData);
					if (s == NULL)
						sp->warning("TrackingScripting::process - createScriptableTrackerType, no script loaded with given name (creating the scriptable tracker type anyway, but initialization will be incomplete).");
					st->setScript(stringData);
					st->doSelfInit();
					gs_scriptableTrackers.push_back(st);
					*lastValue = game->objectTracker->addTrackerType(st);
				} else {
					sp->error("TrackingScripting::process - createScriptableTrackerType, another tracker type already exists with given name.");
					*lastValue = 0;
				}
			} else {
				sp->error("TrackingScripting::process - createScriptableTrackerType parameter missing.");
				*lastValue = 0;
			}
			break;

		case GS_CMD_createTracker:
			if (stringData != NULL)
			{
				const char *s = stringData;
				if (stringData[0] == '$' && stringData[1] == '\0')
					s = gsd->stringValue;

				if (s != NULL)
				{
					TrackerTypeNumber tnum = game->objectTracker->getTrackerTypeNumberByName(stringData);
					if (tnum != -1)
					{			
						gsd->unifiedHandle = game->objectTracker->createTracker(tnum);
						if (gsd->unifiedHandle != UNIFIED_HANDLE_NONE)
						{
							ITrackerObject *t = game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle);
							assert(t != NULL);
							t->setTrackerPosition(gsd->position);
							if (t->getType()->getTypeId() == ScriptableTrackerObjectType::typeId)
							{
								// WARNING: unsafe cast! (based on check above)
								((ScriptableTrackerObject *)t)->setUnifiedHandle(gsd->unifiedHandle);

								UnifiedHandle originalTrackerBeforeCall = ScriptableTrackerObject::trackerForCurrentlyRunningScript;
								ScriptableTrackerObject::trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;

								((ScriptableTrackerObject *)t)->initVariableValues();

								ScriptableTrackerObject::trackerForCurrentlyRunningScript = originalTrackerBeforeCall;
							}
						} else {
							sp->error("TrackingScripting::process - createTracker, failed to create a tracker (too many trackers or invalid tracker type parameter).");
						}
					} else {
						sp->error("TrackingScripting::process - createTracker, no tracker type of given name.");
						gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
					}
				} else {
					sp->error("TrackingScripting::process - createTracker, null string value.");
					gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				}
			} else {
				sp->error("TrackingScripting::process - createTracker parameter missing.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
			}
			break;

		case GS_CMD_createTrackerAttachingToTrackableByValue:
			if (stringData != NULL)
			{
				const char *s = stringData;
				if (stringData[0] == '$' && stringData[1] == '\0')
					s = gsd->stringValue;

				if (s != NULL)
				{
					TrackerTypeNumber tnum = game->objectTracker->getTrackerTypeNumberByName(s);
					if (tnum != -1)
					{			
						// HACK: ...
						hackhack_trackerunit = NULL;
						if (VALIDATE_UNIFIED_HANDLE_BITS(*lastValue)
							&& IS_UNIFIED_HANDLE_UNIT(*lastValue))
						{
							hackhack_trackerunit = game->units->getUnitById(*lastValue);
						}

						gsd->unifiedHandle = game->objectTracker->createTracker(tnum);
						if (gsd->unifiedHandle != UNIFIED_HANDLE_NONE)
						{
							UnifiedHandle originalTrackerBeforeCall = ScriptableTrackerObject::trackerForCurrentlyRunningScript;
							ScriptableTrackerObject::trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;

							ITrackerObject *t = game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle);
							assert(t != NULL);
							t->setTrackerPosition(gsd->position);
							if (t->getType()->getTypeId() == ScriptableTrackerObjectType::typeId)
							{
								// WARNING: unsafe cast! (based on check above)
								((ScriptableTrackerObject *)t)->setUnifiedHandle(gsd->unifiedHandle);
							}

							if (VALIDATE_UNIFIED_HANDLE_BITS(*lastValue))
							{
								ITrackableObject *trackable = TrackableUnifiedHandleObject::getInstanceFromPool(*lastValue);
								if (trackable != NULL)
								{
									game->objectTracker->attachTrackerToTrackable(gsd->unifiedHandle, trackable);
								} else {
									sp->error("TrackingScripting::process - createTrackerAttachingToTrackableByValue, failed to get trackable object by given value.");
								}
							} else {
								sp->error("TrackingScripting::process - createTrackerAttachingToTrackableByValue, value is not a valid unified handle.");
							}

							if (t->getType()->getTypeId() == ScriptableTrackerObjectType::typeId)
							{
								// WARNING: unsafe cast! (based on check above)
								((ScriptableTrackerObject *)t)->initVariableValues();
							}

							ScriptableTrackerObject::trackerForCurrentlyRunningScript = originalTrackerBeforeCall;

						} else {
							sp->error("TrackingScripting::process - createTrackerAttachingToTrackableByValue, failed to create a tracker (too many trackers or invalid tracker type parameter).");
						}
					} else {
						sp->error("TrackingScripting::process - createTrackerAttachingToTrackableByValue, no tracker type of given name.");
						gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
					}
				} else {
					sp->error("TrackingScripting::process - createTrackerAttachingToTrackableByValue, null string value parameter.");
					gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				}
			} else {
				sp->error("TrackingScripting::process - createTrackerAttachingToTrackableByValue parameter missing.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
			}
			break;

		case GS_CMD_doesTrackerExist:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
#endif
					if (game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle) != NULL)
						*lastValue = 1;
					else
						*lastValue = 0;
#ifndef UNSAFE_TRACKER_SCRIPTING
				} else {
					sp->error("TrackingScripting::process - doesTrackerExist, object with given unified handle is not a tracker.");
					*lastValue = 0;
				}
			} else {
				sp->error("TrackingScripting::process - doesTrackerExist, invalid unified handle.");
				*lastValue = 0;
			}
#endif
			break;

		case GS_CMD_doesTrackerAttachedTrackableExist:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
					if (game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle) != NULL)
					{
#endif
						{
							ITrackableObject *trackable = game->objectTracker->getTrackerAttachedTrackable(gsd->unifiedHandle);
							if (trackable != NULL)
							{						
								if (trackable->getTypeId() == TrackableUnifiedHandleObject::typeId)
								{
									// WARNING: unsafe cast, based on above type check.
									TrackableUnifiedHandleObject *unicasted = (TrackableUnifiedHandleObject *)trackable;
									UnifiedHandle tmp = unicasted->getUnifiedHandle();
									if (game->unifiedHandleManager->doesObjectExist(tmp))
										*lastValue = 1;
									else
										*lastValue = 0;
								} else {
									sp->warning("TrackingScripting::process - doesTrackerAttachedTrackableExist, attached trackable is not of unified handle type trackable.");
									*lastValue = 1;
								}
							} else {
								sp->warning("TrackingScripting::process - doesTrackerAttachedTrackableExist, tracker is not attached to any trackable.");
								*lastValue = 0;
							}
						}
#ifndef UNSAFE_TRACKER_SCRIPTING
					} else {
						sp->error("TrackingScripting::process - doesTrackerAttachedTrackableExist, tracker does not exist.");
						*lastValue = 0;
					}
				} else {
					sp->error("TrackingScripting::process - doesTrackerAttachedTrackableExist, object with given unified handle is not a tracker.");
					*lastValue = 0;
				}
			} else {
				sp->error("TrackingScripting::process - doesTrackerAttachedTrackableExist, invalid unified handle.");
				*lastValue = 0;
			}
#endif
			break;

		case GS_CMD_attachTrackerToTrackableByValue:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
					if (game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle) != NULL)
					{
						if (VALIDATE_UNIFIED_HANDLE_BITS(*lastValue))
						{
							ITrackableObject *trackable = TrackableUnifiedHandleObject::getInstanceFromPool(*lastValue);
							if (trackable != NULL)
							{
								UnifiedHandle originalTrackerBeforeCall = ScriptableTrackerObject::trackerForCurrentlyRunningScript;
								ScriptableTrackerObject::trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;

								game->objectTracker->attachTrackerToTrackable(gsd->unifiedHandle, trackable);

								ScriptableTrackerObject::trackerForCurrentlyRunningScript = originalTrackerBeforeCall;
							} else {
								sp->error("TrackingScripting::process - attachTrackerToTrackableByValue, failed to get trackable object by given value.");
							}
						} else {
							sp->error("TrackingScripting::process - attachTrackerToTrackableByValue, value is not a valid unified handle.");
						}
					} else {
						sp->error("TrackingScripting::process - attachTrackerToTrackableByValue, tracker does not exist.");
					}
				} else {
					sp->error("TrackingScripting::process - attachTrackerToTrackableByValue, object with given unified handle is not a tracker.");
				}
			} else {
				sp->error("TrackingScripting::process - attachTrackerToTrackableByValue, invalid unified handle.");
			}
			break;

		case GS_CMD_setTrackerToPosition:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
#endif
					{
						ITrackerObject *t = game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle);
						if (t != NULL)
						{
							t->setTrackerPosition(gsd->position);
						} else {
							sp->error("TrackingScripting::process - setTrackerToPosition, tracker does not exist.");
						}
					}
#ifndef UNSAFE_TRACKER_SCRIPTING
				} else {
					sp->error("TrackingScripting::process - setTrackerToPosition, object with given unified handle is not a tracker.");
				}
			} else {
				sp->error("TrackingScripting::process - setTrackerToPosition, invalid unified handle.");
			}
#endif
			break;

		case GS_CMD_getTrackerPosition:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
#endif
					{
						ITrackerObject *t = game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle);
						if (t != NULL)
						{
							gsd->position = t->getTrackerPosition();
						} else {
							sp->error("TrackingScripting::process - getTrackerPosition, tracker does not exist.");
							gsd->position = VC3(0,0,0);
						}
					}
#ifndef UNSAFE_TRACKER_SCRIPTING
				} else {
					sp->error("TrackingScripting::process - getTrackerPosition, object with given unified handle is not a tracker.");
					gsd->position = VC3(0,0,0);
				}
			} else {
				sp->error("TrackingScripting::process - getTrackerPosition, invalid unified handle.");
				gsd->position = VC3(0,0,0);
			}
#endif
			break;

		case GS_CMD_deleteTracker:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
					if (game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle) != NULL)
					{
						game->objectTracker->deleteTracker(gsd->unifiedHandle);
					} else {
						sp->error("TrackingScripting::process - deleteTracker, tracker does not exist.");
					}
				} else {
					sp->error("TrackingScripting::process - deleteTracker, object with given unified handle is not a tracker.");
				}
			} else {
				sp->error("TrackingScripting::process - deleteTracker, invalid unified handle.");
			}
			break;

		case GS_CMD_isTrackerAttached:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
					if (game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle) != NULL)
					{
#endif
						{
							ITrackableObject *trackable = game->objectTracker->getTrackerAttachedTrackable(gsd->unifiedHandle);
							if (trackable != NULL)
							{
								*lastValue = 1;
							} else {
								*lastValue = 0;
							}
						}
#ifndef UNSAFE_TRACKER_SCRIPTING
					} else {
						sp->error("TrackingScripting::process - isTrackerAttached, tracker does not exist.");
						*lastValue = 0;
					}
				} else {
					sp->error("TrackingScripting::process - isTrackerAttached, object with given unified handle is not a tracker.");
					*lastValue = 0;
				}
			} else {
				sp->error("TrackingScripting::process - isTrackerAttached, invalid unified handle.");
				*lastValue = 0;
			}
#endif
			break;

		case GS_CMD_getTrackerAttachedTrackable:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
					if (game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle) != NULL)
					{
#endif
						{
							ITrackableObject *trackable = game->objectTracker->getTrackerAttachedTrackable(gsd->unifiedHandle);
							if (trackable != NULL)
							{						
								if (trackable->getTypeId() == TrackableUnifiedHandleObject::typeId)
								{
									// WARNING: unsafe cast, based on above type check.
									TrackableUnifiedHandleObject *unicasted = (TrackableUnifiedHandleObject *)trackable;
									gsd->unifiedHandle = unicasted->getUnifiedHandle();
								} else {
									sp->warning("TrackingScripting::process - getTrackerAttachedTrackable, attached trackable is not of unified handle type trackable.");
									gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
								}
							} else {
								sp->warning("TrackingScripting::process - getTrackerAttachedTrackable, tracker is not attached to any trackable.");
								gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
							}
						}
#ifndef UNSAFE_TRACKER_SCRIPTING
					} else {
						sp->error("TrackingScripting::process - getTrackerAttachedTrackable, tracker does not exist.");
						gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
					}
				} else {
					sp->error("TrackingScripting::process - getTrackerAttachedTrackable, object with given unified handle is not a tracker.");
					gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				}
			} else {
				sp->error("TrackingScripting::process - getTrackerAttachedTrackable, invalid unified handle.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
			}
#endif
			break;

		case GS_CMD_detachTracker:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
					if (game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle) != NULL)
					{
						ITrackableObject *trackable = game->objectTracker->getTrackerAttachedTrackable(gsd->unifiedHandle);
						if (trackable != NULL)
						{						
							UnifiedHandle originalTrackerBeforeCall = ScriptableTrackerObject::trackerForCurrentlyRunningScript;
							ScriptableTrackerObject::trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;

							game->objectTracker->attachTrackerToTrackable(gsd->unifiedHandle, NULL);

							ScriptableTrackerObject::trackerForCurrentlyRunningScript = originalTrackerBeforeCall;
						} else {
							sp->warning("TrackingScripting::process - detachTracker, tracker is not attached to any trackable.");
						}
					} else {
						sp->error("TrackingScripting::process - detachTracker, tracker does not exist.");
					}
				} else {
					sp->error("TrackingScripting::process - detachTracker, object with given unified handle is not a tracker.");
				}
			} else {
				sp->error("TrackingScripting::process - detachTracker, invalid unified handle.");
			}
			break;

		case GS_CMD_getCurrentTrackerFloatDistanceToUnifiedHandleObject:
			{
				UnifiedHandle tracker = ScriptableTrackerObject::trackerForCurrentlyRunningScript;
				VC3 objPos = game->unifiedHandleManager->getObjectPosition(gsd->unifiedHandle);
				VC3 trackerPos = game->unifiedHandleManager->getObjectPosition(tracker);
				VC3 dV = objPos - trackerPos;
				gsd->floatValue = dV.GetLength();
			}
			break;

		case GS_CMD_restoreTracker:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (ScriptableTrackerObject::trackerForCurrentlyRunningScript == UNIFIED_HANDLE_NONE)
			{
				sp->error("TrackingScripting::process - restoreTracker called outside tracker script.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
			} else {
#endif
				gsd->unifiedHandle = ScriptableTrackerObject::trackerForCurrentlyRunningScript;

#ifndef UNSAFE_TRACKER_SCRIPTING
				if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
				{
					if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
					{
						// ok
					} else {
						sp->error("TrackingScripting::process - restoreTracker, restored tracker is not a tracker (internal error).");
						gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
					}
				} else {
					sp->error("TrackingScripting::process - restoreTracker, restored tracker handle is invalid (internal error).");
					gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				}
			}
#endif
			break;

		case GS_CMD_setTrackerProjectileBullet:
			if (stringData != NULL)
			{
				const char *s = stringData;
				if (stringData[0] == '$' && stringData[1] == '\0')
				{
					s = gsd->stringValue;
				}
				if (s != NULL)
				{
					if (PARTTYPE_ID_STRING_VALID(s))
					{
						PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(s));
						if (pt != NULL) 
						{ 
							PartType *bulletpt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"));

							if (bulletpt != NULL && pt->isInherited(bulletpt))
							{ 						
								gs_trackerProjectileBullet = s;
							} else {
								sp->error("UnitScripting::process - setTrackerProjectileBullet, given part type is non-bullet type.");
							}
						} else {
							sp->error("UnitScripting::process - setTrackerProjectileBullet, reference to unloaded part type.");
						}
					} else {
						sp->error("UnitScripting::process - setTrackerProjectileBullet, invalid part type id.");
					}
				} else {
					sp->error("TrackingScripting::process - setTrackerProjectileBullet, null string value.");
				}
			} else {
				sp->error("TrackingScripting::process - setTrackerProjectileBullet parameter missing.");
			}
			break;

		case GS_CMD_iterateNextTrackable:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (ScriptableTrackerObject::trackerForCurrentlyRunningScript != UNIFIED_HANDLE_NONE
				&& ScriptableTrackerObject::trackableIteratorForCurrentlyRunningScript != NULL)
			{
#endif
				if (ScriptableTrackerObject::trackableIteratorForCurrentlyRunningScript->iterateAvailable())
				{
					*lastValue = 1;

					ITrackableObject *trackable = ScriptableTrackerObject::trackableIteratorForCurrentlyRunningScript->iterateNext();
					if (trackable->getTypeId() == TrackableUnifiedHandleObject::typeId)
					{
						// WARNING: unsafe cast! (based on check above)
						gs_currentlyIteratedTrackableUnifiedHandle = ((TrackableUnifiedHandleObject *)trackable)->getUnifiedHandle();
					} else {
						gs_currentlyIteratedTrackableUnifiedHandle = UNIFIED_HANDLE_NONE;
					}
				} else {
					*lastValue = 0;
					gs_currentlyIteratedTrackableUnifiedHandle = UNIFIED_HANDLE_NONE;
				}
#ifndef UNSAFE_TRACKER_SCRIPTING
			} else {
				sp->error("TrackingScripting::process - iterateNextTrackable called outside tracker iterate script.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				*lastValue = 0;
			}
#endif
			break;

		case GS_CMD_getIteratedTrackableUnifiedHandle:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (ScriptableTrackerObject::trackerForCurrentlyRunningScript != UNIFIED_HANDLE_NONE
				&& ScriptableTrackerObject::trackableIteratorForCurrentlyRunningScript != NULL)
			{
#endif
				gsd->unifiedHandle = gs_currentlyIteratedTrackableUnifiedHandle;
				if (gsd->unifiedHandle != UNIFIED_HANDLE_NONE)
				{
					*lastValue = 1;
				} else {
					*lastValue = 0;
				}
#ifndef UNSAFE_TRACKER_SCRIPTING
			} else {
				sp->error("TrackingScripting::process - getIteratedTrackableUnifiedHandle called outside tracker iterate script.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				*lastValue = 0;
			}
#endif
			break;

		case GS_CMD_isTrackerType:
			// FIXME: this uses the original tracker, not current unified handle
			// should use the unified handle instead!???
			/*
			if (stringData != NULL)
			{
				if (ScriptableTrackerObject::trackerForCurrentlyRunningScript == UNIFIED_HANDLE_NONE)
				{
					sp->error("TrackingScripting::process - isTrackerType called outside tracker script.");
				} else {
					UnifiedHandle tmp = ScriptableTrackerObject::trackerForCurrentlyRunningScript;

					if (VALIDATE_UNIFIED_HANDLE_BITS(tmp))
					{
						if (IS_UNIFIED_HANDLE_TRACKER(tmp))
						{
							game->objectTracker->getTrackerByUnifiedHandle(tmp);
							// ok
							// TODO: check that tracker type name == stringData

						} else {
							sp->error("TrackingScripting::process - isTrackerType, restored tracker is not a tracker (internal error).");
						}
					} else {
						sp->error("TrackingScripting::process - isTrackerType, restored tracker handle is invalid (internal error).");
					}
				}
			} else {
				sp->error("TrackingScripting::process - isTrackerType parameter missing.");
			}
			*/
			if (stringData != NULL)
			{
				if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
				{
					if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
					{
						ITrackerObject *t = game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle);
						if (t != NULL)
						{
							if (t->getType()->getTrackerTypeName() == std::string(stringData))
								*lastValue = 1;
							else
								*lastValue = 0;
						} else {
							sp->error("TrackingScripting::process - isTrackerType, tracker does not exist.");
							*lastValue = 0;
						}
					} else {
						sp->error("TrackingScripting::process - isTrackerType, object with given unified handle is not a tracker.");
						*lastValue = 0;
					}
				} else {
					sp->error("TrackingScripting::process - isTrackerType, invalid unified handle.");
					*lastValue = 0;
				}
			} else {
				sp->error("TrackingScripting::process - isTrackerType parameter missing.");
				*lastValue = 0;
			}
			break;

		case GS_CMD_getScriptableTrackerVariable:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
#endif
					{
						ITrackerObject *t = game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle);
						if (t != NULL)
						{
							assert(t->getType() != NULL);
							if (t->getType()->getTypeId() == ScriptableTrackerObjectType::typeId)
							{
								// WARNING: unsafe cast, based on check above.
								ScriptableTrackerObject *st = (ScriptableTrackerObject *)t;
								// TODO: optimize, set value to intData array?
								// NOTE: cannot do that!!! (if the scriptable tracker type is not contant!!!)
								int varNum = st->getVariableNumberByName(std::string(stringData));
								if (varNum != -1)
								{
									*lastValue = st->getVariable(varNum);
								} else {
									sp->error("TrackingScripting::process - getScriptableTrackerVariable, scriptable tracker variable with given name does not exist.");
									*lastValue = 0;
								}
							} else {
								sp->error("TrackingScripting::process - getScriptableTrackerVariable, tracker is not of scriptable tracker type.");
								*lastValue = 0;
							}
						} else {
							sp->error("TrackingScripting::process - getScriptableTrackerVariable, tracker does not exist.");
							*lastValue = 0;
						}
					}
#ifndef UNSAFE_TRACKER_SCRIPTING
				} else {
					sp->error("TrackingScripting::process - getScriptableTrackerVariable, object with given unified handle is not a tracker.");
					*lastValue = 0;
				}
			} else {
				sp->error("TrackingScripting::process - getScriptableTrackerVariable, invalid unified handle.");
				*lastValue = 0;
			}
#endif
			break;

		case GS_CMD_setScriptableTrackerVariable:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
#endif
					{
						ITrackerObject *t = game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle);
						if (t != NULL)
						{
							assert(t->getType() != NULL);
							if (t->getType()->getTypeId() == ScriptableTrackerObjectType::typeId)
							{
								// WARNING: unsafe cast, based on check above.
								ScriptableTrackerObject *st = (ScriptableTrackerObject *)t;
								// TODO: optimize, set value to intData array?
								// NOTE: cannot do that!!! (if the scriptable tracker type is not contant!!!)
								int varNum = st->getVariableNumberByName(std::string(stringData));
								if (varNum != -1)
								{
									st->setVariable(varNum, *lastValue);
								} else {
									sp->error("TrackingScripting::process - setScriptableTrackerVariable, scriptable tracker variable with given name does not exist.");
								}
							} else {
								sp->error("TrackingScripting::process - setScriptableTrackerVariable, tracker is not of scriptable tracker type.");
							}
						} else {
							sp->error("TrackingScripting::process - setScriptableTrackerVariable, tracker does not exist.");
						}
					}
#ifndef UNSAFE_TRACKER_SCRIPTING
				} else {
					sp->error("TrackingScripting::process - setScriptableTrackerVariable, object with given unified handle is not a tracker.");
				}
			} else {
				sp->error("TrackingScripting::process - setScriptableTrackerVariable, invalid unified handle.");
			}
#endif
			break;

		case GS_CMD_isUnifiedHandleObjectTrackedByValue:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
				{
#endif
					{
						// HACK: ...
						TrackableUnifiedHandleObject *tuho = TrackableUnifiedHandleObject::getInstanceFromPool(gsd->unifiedHandle);
						if (tuho != NULL)
						{
							TrackerTypeNumber trackerTypeNumber = *lastValue;
							UnifiedHandle attachedTracker = tuho->getTrackedByForType(trackerTypeNumber);
							if (attachedTracker != UNIFIED_HANDLE_NONE) 
								*lastValue = 1;
							else
								*lastValue = 0;

							tuho->release();
						} else {
							sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByValue, object with given unified handle is not of supported type (TrackableUnifiedHandleObject).");
							*lastValue = 0;
						}
					}
#ifndef UNSAFE_TRACKER_SCRIPTING
				} else {
					sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByValue, object does not exist.");
					*lastValue = 0;
				}
			} else {
				sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByValue, invalid unified handle.");
				*lastValue = 0;
			}
#endif
			break;

		case GS_CMD_isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (intData != 0)
			{
				if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
				{
					if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
					{
#endif
						{
							// HACK: ...
							TrackableUnifiedHandleObject *tuho = TrackableUnifiedHandleObject::getInstanceFromPool(gsd->unifiedHandle);
							if (tuho != NULL)
							{
								TRACKABLE_TYPEID_DATATYPE trackableTypeMask = intData;

								UnifiedHandle attachedTracker = tuho->getTrackedByForTrackableTypes(trackableTypeMask);
								if (attachedTracker != UNIFIED_HANDLE_NONE) 
									*lastValue = 1;
								else
									*lastValue = 0;

								tuho->release();
							} else {
								sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes, object with given unified handle is not of supported type (TrackableUnifiedHandleObject).");
								*lastValue = 0;
							}
						}
#ifndef UNSAFE_TRACKER_SCRIPTING
					} else {
						sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes, object does not exist.");
						*lastValue = 0;
					}
				} else {
					sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes, invalid unified handle.");
					*lastValue = 0;
				}
			} else {
				sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes parameter missing (non-zero bit mask value expected).");
				*lastValue = 0;
			}
#endif
			break;

		case GS_CMD_getTrackedByForUnifiedHandleObjectHavingTrackerInterestedInTrackableTypes:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (intData != 0)
			{
				if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
				{
					if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
					{
#endif
						{
							// HACK: ...
							TrackableUnifiedHandleObject *tuho = TrackableUnifiedHandleObject::getInstanceFromPool(gsd->unifiedHandle);
							if (tuho != NULL)
							{
								TRACKABLE_TYPEID_DATATYPE trackableTypeMask = intData;

								UnifiedHandle attachedTracker = tuho->getTrackedByForTrackableTypes(trackableTypeMask);
								gsd->unifiedHandle = attachedTracker;

								tuho->release();
							} else {
								sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes, object with given unified handle is not of supported type (TrackableUnifiedHandleObject).");
								gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
							}
						}
#ifndef UNSAFE_TRACKER_SCRIPTING
					} else {
						sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes, object does not exist.");
						gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
					}
				} else {
					sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes, invalid unified handle.");
					gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				}
			} else {
				sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByTrackerInterestedInTrackableTypes parameter missing (non-zero bit mask value expected).");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
			}
#endif
			break;

		case GS_CMD_isUnifiedHandleObjectTrackedByName:
			if (stringData != NULL)
			{
				const char *s = stringData;
				if (s[0] == '$' && s[1] == '\0')
				{
					s = gsd->stringValue;
				}

#ifndef UNSAFE_TRACKER_SCRIPTING
				if (s != NULL)
				{
					if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
					{
						if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
						{
#endif
							{
								// HACK: ...
								TrackableUnifiedHandleObject *tuho = TrackableUnifiedHandleObject::getInstanceFromPool(gsd->unifiedHandle);
								if (tuho != NULL)
								{
									TrackerTypeNumber trackerTypeNumber = game->objectTracker->getTrackerTypeNumberByName(s);
									UnifiedHandle attachedTracker = tuho->getTrackedByForType(trackerTypeNumber);
									if (attachedTracker != UNIFIED_HANDLE_NONE) 
										*lastValue = 1;
									else
										*lastValue = 0;

									tuho->release();
								} else {
									sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByName, object with given unified handle is not of supported type (TrackableUnifiedHandleObject).");
									*lastValue = 0;
								}
							}
#ifndef UNSAFE_TRACKER_SCRIPTING
						} else {
							sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByName, object does not exist.");
							*lastValue = 0;
						}
					} else {
						sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByName, invalid unified handle.");
						*lastValue = 0;
					}
				} else {
					sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByName, null string value.");
					*lastValue = 0;
				}
#endif
			} else {
				sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByName parameter missing.");
				*lastValue = 0;
			}
			break;

		case GS_CMD_getTrackedByForUnifiedHandleObjectByValue:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
				{
#endif
					{
						// HACK: ...
						TrackableUnifiedHandleObject *tuho = TrackableUnifiedHandleObject::getInstanceFromPool(gsd->unifiedHandle);
						if (tuho != NULL)
						{
							TrackerTypeNumber trackerTypeNumber = *lastValue;
							UnifiedHandle attachedTracker = tuho->getTrackedByForType(trackerTypeNumber);
							gsd->unifiedHandle = attachedTracker;

							tuho->release();
						} else {
							sp->error("TrackingScripting::process - getTrackedByForUnifiedHandleObjectByValue, object with given unified handle is not of supported type (TrackableUnifiedHandleObject).");
							gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
						}
					}
#ifndef UNSAFE_TRACKER_SCRIPTING
				} else {
					sp->error("TrackingScripting::process - getTrackedByForUnifiedHandleObjectByValue, object does not exist.");
					gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				}
			} else {
				sp->error("TrackingScripting::process - getTrackedByForUnifiedHandleObjectByValue, invalid unified handle.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
			}
#endif
			break;

		case GS_CMD_getTrackedByForUnifiedHandleObjectByName:
			if (stringData != NULL)
			{
				const char *s = stringData;
				if (s[0] == '$' && s[1] == '\0')
				{
					s = gsd->stringValue;
				}

#ifndef UNSAFE_TRACKER_SCRIPTING
				if (s != NULL)
				{
					if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
					{
						if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
						{
#endif
							{
								// HACK: ...
								TrackableUnifiedHandleObject *tuho = TrackableUnifiedHandleObject::getInstanceFromPool(gsd->unifiedHandle);
								if (tuho != NULL)
								{
									TrackerTypeNumber trackerTypeNumber = game->objectTracker->getTrackerTypeNumberByName(s);
									UnifiedHandle attachedTracker = tuho->getTrackedByForType(trackerTypeNumber);
									gsd->unifiedHandle = attachedTracker;

									tuho->release();
								} else {
									sp->error("TrackingScripting::process - getTrackedByForUnifiedHandleObjectByName, object with given unified handle is not of supported type (TrackableUnifiedHandleObject).");
									gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
								}
							}
#ifndef UNSAFE_TRACKER_SCRIPTING
						} else {
							sp->error("TrackingScripting::process - getTrackedByForUnifiedHandleObjectByName, object does not exist.");
							gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
						}
					} else {
						sp->error("TrackingScripting::process - getTrackedByForUnifiedHandleObjectByName, invalid unified handle.");
						gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
					}
				} else {
					sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByName, null string value.");
					gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
				}
#endif
			} else {
				sp->error("TrackingScripting::process - isUnifiedHandleObjectTrackedByName parameter missing.");
				gsd->unifiedHandle = UNIFIED_HANDLE_NONE;
			}
			break;

		case GS_CMD_deleteUnifiedHandleObjectTrackerByName:
			if (stringData != NULL)
			{
				const char *s = stringData;
				if (s[0] == '$' && s[1] == '\0')
				{
					s = gsd->stringValue;
				}

				if (s != NULL)
				{
					if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
					{
						if (game->unifiedHandleManager->doesObjectExist(gsd->unifiedHandle))
						{
							// HACK: ...
							TrackableUnifiedHandleObject *tuho = TrackableUnifiedHandleObject::getInstanceFromPool(gsd->unifiedHandle);
							if (tuho != NULL)
							{
								TrackerTypeNumber trackerTypeNumber = game->objectTracker->getTrackerTypeNumberByName(s);
								UnifiedHandle attachedTracker = tuho->getTrackedByForType(trackerTypeNumber);
								game->objectTracker->deleteTracker(attachedTracker);

								tuho->release();
							} else {
								sp->error("TrackingScripting::process - deleteUnifiedHandleObjectTrackerByName, object with given unified handle is not of supported type (TrackableUnifiedHandleObject).");
								*lastValue = 0;
							}
						} else {
							sp->error("TrackingScripting::process - deleteUnifiedHandleObjectTrackerByName, object does not exist.");
							*lastValue = 0;
						}
					} else {
						sp->error("TrackingScripting::process - deleteUnifiedHandleObjectTrackerByName, invalid unified handle.");
						*lastValue = 0;
					}
				} else {
					sp->error("TrackingScripting::process - deleteUnifiedHandleObjectTrackerByName, null string value.");
					*lastValue = 0;
				}
			} else {
				sp->error("TrackingScripting::process - deleteUnifiedHandleObjectTrackerByName parameter missing.");
				*lastValue = 0;
			}
			break;

		case GS_CMD_addScriptableTrackerTypeVariable:
			if (stringData != NULL)
			{
				if (ScriptableTrackerObjectType::trackerTypeForCurrentlyRunningScript != NULL)
				{
					// TODO: check that variable name is valid (alphanumeric, etc.)
					ScriptableTrackerObjectType::trackerTypeForCurrentlyRunningScript->addScriptableTrackerVariable(stringData);
				} else {
					sp->error("TrackingScripting::process - addScriptableTrackerTypeVariable called outside appropriate scriptable tracker type init script.");
				}
			} else {
				sp->error("TrackingScripting::process - addScriptableTrackerTypeVariable parameter missing.");
			}
			break;

		case GS_CMD_iterateTrackablesForTracker:
#ifndef UNSAFE_TRACKER_SCRIPTING
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{
				if (IS_UNIFIED_HANDLE_TRACKER(gsd->unifiedHandle))
				{
#endif
					{
						ITrackerObject *t = game->objectTracker->getTrackerByUnifiedHandle(gsd->unifiedHandle);
						if (t != NULL)
						{
							UnifiedHandle originalTrackerBeforeCall = ScriptableTrackerObject::trackerForCurrentlyRunningScript;
							ScriptableTrackerObject::trackerForCurrentlyRunningScript = UNIFIED_HANDLE_NONE;

							game->objectTracker->iterateTrackablesForTracker(t);

							ScriptableTrackerObject::trackerForCurrentlyRunningScript = originalTrackerBeforeCall;
						} else {
							sp->error("TrackingScripting::process - iterateTrackablesForTracker, tracker does not exist.");
						}
					}
#ifndef UNSAFE_TRACKER_SCRIPTING
				} else {
					sp->error("TrackingScripting::process - iterateTrackablesForTracker, object with given unified handle is not a tracker.");
				}
			} else {
				sp->error("TrackingScripting::process - iterateTrackablesForTracker, invalid unified handle.");
			}
#endif
			break;

		case GS_CMD_dumpTrackingInfo:
			LOG_INFO("Tracking info follows:");
			LOG_INFO(game->objectTracker->getStatusInfo().c_str());
			break;

		default:
			sp->error("TrackingScripting::process - Unknown command.");
			assert(0);
		}

	}
}



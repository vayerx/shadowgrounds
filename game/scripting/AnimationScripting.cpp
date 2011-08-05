
#include "precompiled.h"

#include "AnimationScripting.h"

#include "scripting_macros_start.h"
#include "animation_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>

#include "../scaledefs.h"
#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../GameMap.h"
#include "../GameScene.h"
#include "../UnitList.h"
#include "../UnitActor.h"
#include "../UnitType.h"
#include "../unittypes.h"
#include "../../ui/AnimationSet.h"
#include "../../ui/AniRecorderWindow.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../../system/Logger.h"

#include "../Ani.h"
#include "../AniManager.h"

#include "../../util/Debug_MemoryManager.h"


using namespace ui;

namespace game
{
	void AnimationScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game, bool *pause)
	{
		Ani *ani = AniManager::getInstance()->getCurrentScriptAni();

		// WARNING: casting intData to float
		// the intData may not always contain a proper float value!!!
		float floatData = intFloat.f;

		switch(command)
		{
		case GS_CMD_ANISTART:
			if (ani != NULL)
			{
				ani->aniStart();
			} else {
				sp->error("AnimationScripting::process - aniStart called for null ani.");
			}
			break;

		case GS_CMD_ANIEND:
			if (ani != NULL)
			{
				ani->aniEnd();
			} else {
				sp->error("AnimationScripting::process - aniEnd called for null ani.");
			}
			break;

		case GS_CMD_ANIWARP:
			if (ani != NULL)
			{
				VC3 tmp(0,0,0);
				if (gs_coordinate_param(game->gameMap, stringData, &tmp))
				{
					float x = tmp.x;
					float y = tmp.z;
					game->gameMap->keepWellInScaledBoundaries(&x, &y);
					ani->aniWarp(x, y);
				} else {
					sp->error("GameScripting::process - Missing or bad aniWarp parameter.");
				}
			} else {
				sp->error("AnimationScripting::process - aniWarp called for null ani.");
			}
			break;

		case GS_CMD_aniWarpToPosition:
			if (ani != NULL)
			{
				float x = gsd->position.x;
				float y = gsd->position.z;
				game->gameMap->keepWellInScaledBoundaries(&x, &y);
				ani->aniWarp(x, y);
			} else {
				sp->error("AnimationScripting::process - aniWarpToPosition called for null ani.");
			}
			break;

		case GS_CMD_ANIHEIGHT:
			if (ani != NULL)
			{
				ani->aniHeight(floatData);
			} else {
				sp->error("AnimationScripting::process - aniHeight called for null ani.");
			}
			break;

		case GS_CMD_ANIROTS:
			if (ani != NULL)
			{
				if (stringData != NULL)
				{
					int slen = strlen(stringData);
					//if (slen < 64)
					//{
						//char buf[64 + 1];
						//strcpy(buf, stringData);
						float coords[3];
						coords[0] = (float)atof(stringData);
						int coordNum = 1;
						for (int i = 0; i < slen; i++)
						{
							if (stringData[i] == ',')
							{
								coords[coordNum] = (float)atof(&stringData[i + 1]);
								coordNum++;
								if (coordNum > 2) break;
							}
						}
						if (coordNum != 3)
						{
							sp->error("AnimationScripting::process - aniRots parameter bad.");
						} else {
							ani->aniRots(coords[0], coords[1], coords[2]);
						}
					//} else {
					//	sp->error("AnimationScripting::process - aniRots parameter bad.");
					//}
				} else {
					sp->error("AnimationScripting::process - aniRots parameter missing.");
				}
			} else {
				sp->error("AnimationScripting::process - aniRots called for null ani.");
			}
			break;

		case GS_CMD_ANIANIM:
			if (ani != NULL)
			{
				if (stringData != NULL)
				{
					int anim = AnimationSet::getAnimNumberByName(stringData);
					if (anim != -1)
					{
						ani->aniAnim(anim);
					} else {
						sp->error("AnimationScripting::process - aniAnim parameter bad, anim with given name does not exist.");
						sp->debug(stringData);
					}
				} else {
					sp->error("AnimationScripting::process - aniAnim parameter missing.");
				}
			} else {
				sp->error("AnimationScripting::process - aniAnim called for null ani.");
			}
			break;

		case GS_CMD_ANIENDANIM:
			if (ani != NULL)
			{
				if (stringData != NULL)
				{
					int anim = AnimationSet::getAnimNumberByName(stringData);
					if (anim != -1)
					{
						ani->aniEndAnim(anim);
					} else {
						sp->error("AnimationScripting::process - aniEndAnim parameter bad, anim with given name does not exist.");
						sp->debug(stringData);
					}
				} else {
					sp->error("AnimationScripting::process - aniEndAnim parameter missing.");
				}
			} else {
				sp->error("AnimationScripting::process - aniEndAnim called for null ani.");
			}
			break;

		case GS_CMD_ANIMOVEX:
			if (ani != NULL)
			{
				ani->aniMoveX(floatData);
			} else {
				sp->error("AnimationScripting::process - aniMoveX called for null ani.");
			}
			break;

		case GS_CMD_ANIMOVEZ:
			if (ani != NULL)
			{
				ani->aniMoveZ(floatData);
			} else {
				sp->error("AnimationScripting::process - aniMoveZ called for null ani.");
			}
			break;

		case GS_CMD_ANIMOVEY:
			if (ani != NULL)
			{
				ani->aniMoveY(floatData);
			} else {
				sp->error("AnimationScripting::process - aniMoveY called for null ani.");
			}
			break;

		case GS_CMD_ANIONGROUND:
			if (ani != NULL)
			{
				if (ani->getUnit() != NULL)
				{
					VC3 pos = ani->getUnit()->getPosition();
					float h = game->gameMap->getScaledHeightAt(pos.x, pos.z);
					ani->aniOnGround(h);
				}
			} else {
				sp->error("AnimationScripting::process - aniOnGround called for null ani.");
			}
			break;

		case GS_CMD_ANIROTY:
			if (ani != NULL)
			{
				ani->aniRotY(floatData);
			} else {
				sp->error("AnimationScripting::process - aniRotY called for null ani.");
			}
			break;

		case GS_CMD_ANIROTX:
			if (ani != NULL)
			{
				ani->aniRotX(floatData);
			} else {
				sp->error("AnimationScripting::process - aniRotX called for null ani.");
			}
			break;

		case GS_CMD_ANIROTZ:
			if (ani != NULL)
			{
				ani->aniRotZ(floatData);
			} else {
				sp->error("AnimationScripting::process - aniRotZ called for null ani.");
			}
			break;

		case GS_CMD_ANIAXIS:
			if (ani != NULL)
			{
				ani->aniAxis(floatData);
			} else {
				sp->error("AnimationScripting::process - aniAxis called for null ani.");
			}
			break;

		case GS_CMD_aniAxisToValue:
			if (ani != NULL)
			{
				ani->aniAxis((float)(*lastValue));
			} else {
				sp->error("AnimationScripting::process - aniAxisToValue called for null ani.");
			}
			break;

		case GS_CMD_ANIAXISTOVALUE:
			if (ani != NULL)
			{
				ani->aniAxis((float)(*lastValue));
			} else {
				sp->error("AnimationScripting::process - aniAxisToValue called for null ani.");
			}
			break;

		case GS_CMD_ANIAIM:
			if (ani != NULL)
			{
				ani->aniAim(floatData);
			} else {
				sp->error("AnimationScripting::process - aniAim called for null ani.");
			}
			break;

		case GS_CMD_ANIENDAIM:
			if (ani != NULL)
			{
				ani->aniEndAim();
			} else {
				sp->error("AnimationScripting::process - aniEndAim called for null ani.");
			}
			break;

		case GS_CMD_ANIMARK:
			if (stringData != NULL)
			{
				*pause = ani->reachedMark(stringData);
			} else {
				sp->error("AnimationScripting::process - aniMark parameter missing.");
			}
			break;

		case GS_CMD_ANITICK:
			*pause = ani->reachedTick();
			break;

		case GS_CMD_ANIWAITUNTILANISENDED:
			*pause = ani->aniWaitUntilAnisEnded();
			if (*pause)
			{
				gsd->waitCounter = ANI_MAX_TICKS;
				gsd->waitDestination = false;
			}
			break;

		case GS_CMD_ISALLANICOMPLETE:
			if (AniManager::getInstance()->isAllAniComplete())
				*lastValue = 1;
			else
				*lastValue = 0;
			break;

		case GS_CMD_HASANIPLAYENDED:
			/*
			if (ani != NULL)
			{
				if (ani->hasPlayEnded())
					*lastValue = 1;
				else
					*lastValue = 0;
			} else {
				sp->error("AnimationScripting::process - hasCurrentAniPlayEnded called for null ani.");
			}
			*/
			if (stringData != NULL)
			{
				char *aniname;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					aniname = gsd->stringValue;
				else
					aniname = stringData;

				Ani *tmp = AniManager::getInstance()->getAniByName(aniname);
				if (tmp != NULL)
				{
					if (tmp->hasPlayEnded())
					{
						*lastValue = 1;
					} else {
						*lastValue = 0;
					}
				} else {
					sp->error("AnimationScripting::process - hasAniPlayEnded, no ani found with given name.");
				}
			} else {
				sp->error("AnimationScripting::process - hasAniPlayEnded parameter missing.");
			}
			break;

		case GS_CMD_STARTANIPLAY:
			if (stringData != NULL)
			{
				char *aniname;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					aniname = gsd->stringValue;
				else
					aniname = stringData;

				if (gsd->unit != NULL)
				{
					UnitActor *ua = getUnitActorForUnit(gsd->unit);
					ua->resetToNormalState(gsd->unit);

// TEMP: HACK TEST
// the hack seems to work... :)					
ua->act(gsd->unit);
ua->resetToNormalState(gsd->unit);

					Ani *tmp = AniManager::getInstance()->createNewAniInstance(gsd->unit);
					//AniManager::getInstance()->setCurrentScriptAni(tmp);
					tmp->setName(aniname);
					tmp->startPlay();
				} else {
					sp->error("AnimationScripting::process - Attempt to call startAniPlay for null unit.");
				}
			} else {
				sp->error("AnimationScripting::process - startAniPlay parameter missing.");
			}
			break;

		case GS_CMD_STOPANIPLAY:
			if (stringData != NULL)
			{
				char *aniname;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					aniname = gsd->stringValue;
				else
					aniname = stringData;

				Ani *tmp = AniManager::getInstance()->getAniByName(aniname);
				if (tmp != NULL)
				{
					tmp->stopPlay();
					AniManager::getInstance()->deleteAni(tmp);
				} else {
					sp->error("AnimationScripting::process - stopAniPlay, no ani found with given name.");
				}
			} else {
				sp->error("AnimationScripting::process - stopAniPlay parameter missing.");
			}
			break;

		case GS_CMD_STARTANIRECORD:
			if (stringData != NULL)
			{
				char *aniname;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					aniname = gsd->stringValue;
				else
					aniname = stringData;

				if (gsd->unit != NULL)
				{
					Ani *tmp = AniManager::getInstance()->createNewAniInstance(gsd->unit);
					//AniManager::getInstance()->setCurrentScriptAni(tmp);
					tmp->setName(aniname);
					tmp->startRecord();
				} else {
					sp->error("AnimationScripting::process - Attempt to call startAniRecord for null unit.");
				}
			} else {
				sp->error("AnimationScripting::process - startAniRecord parameter missing.");
			}
			break;

		case GS_CMD_CANCELANIRECORD:
			if (stringData != NULL)
			{
				char *aniname;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					aniname = gsd->stringValue;
				else
					aniname = stringData;

				Ani *tmp = AniManager::getInstance()->getAniByName(aniname);
				if (tmp != NULL)
				{
					tmp->cancelRecord();
					AniManager::getInstance()->deleteAni(tmp);
				} else {
					sp->error("AnimationScripting::process - cancelAniRecord, no ani found with given name.");
				}
			} else {
				sp->error("AnimationScripting::process - cancelAniRecord parameter missing.");
			}
			break;

		case GS_CMD_STOPANIRECORD:
			if (stringData != NULL)
			{
				char *aniname;
				if (stringData[0] == '$' && stringData[1] == '\0' && gsd->stringValue != NULL)
					aniname = gsd->stringValue;
				else
					aniname = stringData;

				Ani *tmp = AniManager::getInstance()->getAniByName(aniname);
				if (tmp != NULL)
				{
					tmp->stopRecord();
					AniManager::getInstance()->deleteAni(tmp);
				} else {
					sp->error("AnimationScripting::process - stopAniRecord, no ani found with given name.");
				}
			} else {
				sp->error("AnimationScripting::process - stopAniRecord parameter missing.");
			}
			break;

		case GS_CMD_SETANIRECORDPATH:
			if (stringData != NULL)
			{
				Ani::setRecordPath(stringData);
				if (game->gameUI->getAniRecorderWindow() != NULL)
				{
					game->gameUI->getAniRecorderWindow()->reload();
				}
			} else {
				sp->error("AnimationScripting::process - setAniRecordPath parameter missing.");
			}
			break;

		case GS_CMD_GETANIRECORDPATH:
			gsd->setStringValue(Ani::getRecordPath());
			break;

		case GS_CMD_STOPALLANIPLAY:
			AniManager::getInstance()->stopAllAniPlay();
			break;

		case GS_CMD_LEAPALLANIPLAYTOEND:
			AniManager::getInstance()->leapAllAniPlayToEnd();
			break;

		case GS_CMD_ADDANISCRIPTCOMMANDS:
			if (stringData != NULL)
			{
				if (game->gameUI->getAniRecorderWindow() != NULL)
				{
					game->gameUI->getAniRecorderWindow()->addAniScriptCommands(stringData);
				}
			} else {
				sp->error("AnimationScripting::process - addAniScriptCommands parameter missing.");
			}
			break;

		case GS_CMD_ANIMUZZLEFLASH:
			if (ani != NULL)
			{
				ani->aniMuzzleflash();
			} else {
				sp->error("AnimationScripting::process - aniAnim called for null ani.");
			}
			break;

		case GS_CMD_ANIBLEND:
			if (ani != NULL)
			{
				if (stringData != NULL)
				{
					int anim = AnimationSet::getAnimNumberByName(stringData);
					if (anim != -1)
					{
						ani->aniBlend(anim);
					} else {
						sp->error("AnimationScripting::process - aniBlend parameter bad, anim with given name does not exist.");
						sp->debug(stringData);
					}
				} else {
					sp->error("AnimationScripting::process - aniBlend parameter missing.");
				}
			} else {
				sp->error("AnimationScripting::process - aniBlend called for null ani.");
			}
			break;

		case GS_CMD_ANIENDBLEND:
			if (ani != NULL)
			{
				ani->aniEndBlend();
			} else {
				sp->error("AnimationScripting::process - aniEndBlend called for null ani.");
			}
			break;

		case GS_CMD_ANIFIREPOSITION:
			if (ani != NULL)
			{
				VC3 tmp(0,0,0);
				if (gs_coordinate_param(game->gameMap, stringData, &tmp))
				{
					float x = tmp.x;
					float y = tmp.z;
					game->gameMap->keepWellInScaledBoundaries(&x, &y);
					gsd->position = VC3(x, game->gameMap->getScaledHeightAt(x, y), y);
				} else {
					sp->error("GameScripting::process - Missing or bad aniFirePosition parameter.");
				}
			} else {
				sp->error("AnimationScripting::process - aniFirePosition called for null ani.");
			}
			break;

		case GS_CMD_ANIFIREHEIGHT:
			if (ani != NULL)
			{
				gsd->position.y = floatData;
			} else {
				sp->error("AnimationScripting::process - aniFireHeight called for null ani.");
			}
			break;

		case GS_CMD_ANIFIRETARGETPOSITION:
			if (ani != NULL)
			{
				VC3 tmp(0,0,0);
				if (gs_coordinate_param(game->gameMap, stringData, &tmp))
				{
					float x = tmp.x;
					float y = tmp.z;
					game->gameMap->keepWellInScaledBoundaries(&x, &y);
					gsd->secondaryPosition = VC3(x, game->gameMap->getScaledHeightAt(x, y), y);
				} else {
					sp->error("GameScripting::process - Missing or bad aniFireTargetPosition parameter.");
				}
			} else {
				sp->error("AnimationScripting::process - aniFireTargetPosition called for null ani.");
			}
			break;

		case GS_CMD_ANIFIRETARGETHEIGHT:
			if (ani != NULL)
			{
				gsd->secondaryPosition.y = floatData;
			} else {
				sp->error("AnimationScripting::process - aniFireTargetHeight called for null ani.");
			}
			break;

		case GS_CMD_ANIFIREPROJECTILE:
			if (stringData != NULL)
			{
				PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
				if (pt == NULL) 
				{ 
					sp->error("AnimationScripting::process - aniFireProjectile, reference to unloaded part type.");
				} else {
					if (pt->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
					{ 
						// WARNING: unsafe cast!
						Weapon *weap = (Weapon *)pt;

						VC3 projpos = gsd->position;
						VC3 projtarg = gsd->secondaryPosition;

						ani->aniFireProjectile(game, weap, projpos, projtarg);

						// NOTE: should go to projtarg - not raytrace to it...
					} else {
						sp->error("AnimationScripting::process - aniFireProjectile, attempt to use non-weapon part type.");
					}
				}
			} else {
				sp->error("AnimationScripting::process - aniFireProjectile parameter missing, weapon name expected.");
			}
			break;

		case GS_CMD_setGlobalAniOffsetSourceToPosition:
			Ani::setGlobalOffsetSource(gsd->position);
			break;

		case GS_CMD_setGlobalAniOffsetTargetToPosition:
			Ani::setGlobalOffsetTarget(gsd->position);
			break;

		case GS_CMD_setGlobalAniRotationToValue:
			Ani::setGlobalRotation((float)(*lastValue));
			break;

		case GS_CMD_resetGlobalAniOffsetAndRotation:
			Ani::resetGlobalOffsetAndRotation();
			break;

		default:
			sp->error("AnimationScripting::process - Unknown command.");
			assert(0);
		}
	}


}




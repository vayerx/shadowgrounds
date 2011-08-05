
#include "precompiled.h"

#include "HitChainScripting.h"

#include "scripting_macros_start.h"
#include "hitchain_script_commands.h"
#include "scripting_macros_end.h"

#include <math.h>
#include "../Game.h"
#include "../GameMap.h"
#include "../hitchaindefs.h"
#include "../PartType.h"
#include "GameScriptingUtils.h"
#include "GameScriptData.h"
#include "../GameRandom.h"
#include "../Projectile.h"
#include "../ProjectileActor.h"
#include "../ProjectileList.h"
#include "../../util/ScriptProcess.h"
#include "../../util/Script.h"

#include "../../util/Debug_MemoryManager.h"

using namespace ui;

namespace game
{
	VC3 projs_originPosition;
	VC3 projs_originalPosition;
	VC3 projs_chainedPosition;

	VC3 projs_originDirection;
	VC3 projs_originalDirection;
	VC3 projs_chainedDirection;

	VC3 projs_hitPlaneNormal;

	float projs_chainedRange;
	float projs_chainedVelocityFactor;

	float projs_skipRaytraceDistance;

	int projs_hitChain;
	int projs_chainedCustomValue;
	int projs_lifeTime;

	Unit *projs_hitUnit;
	Unit *projs_shooter;

  Bullet *projs_chainedBulletType;
  Bullet *projs_originBulletType;

	bool projs_chainedRotationRandom;
	bool projs_chainedOriginToHitUnit;


	void HitChainScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game)
	{
		int intData = intFloat.i;
		switch(command)
		{
			case GS_CMD_PROJGETORIGINDIRECTIONX:
				*lastValue = (int)(projs_originDirection.x * 100.0f);
				break;

			case GS_CMD_PROJGETORIGINDIRECTIONY:
				*lastValue = (int)(projs_originDirection.y * 100.0f);
				break;

			case GS_CMD_PROJGETORIGINDIRECTIONZ:
				*lastValue = (int)(projs_originDirection.z * 100.0f);
				break;

			case GS_CMD_PROJGETORIGINALCHAINEDDIRECTIONX:
				*lastValue = (int)(projs_originalDirection.x * 100.0f);
				break;

			case GS_CMD_PROJGETORIGINALCHAINEDDIRECTIONY:
				*lastValue = (int)(projs_originalDirection.y * 100.0f);
				break;

			case GS_CMD_PROJGETORIGINALCHAINEDDIRECTIONZ:
				*lastValue = (int)(projs_originalDirection.z * 100.0f);
				break;

			case GS_CMD_PROJGETCHAINEDDIRECTIONX:
				*lastValue = (int)(projs_chainedDirection.x * 100.0f);
				break;

			case GS_CMD_PROJGETCHAINEDDIRECTIONY:
				*lastValue = (int)(projs_chainedDirection.y * 100.0f);
				break;

			case GS_CMD_PROJGETCHAINEDDIRECTIONZ:
				*lastValue = (int)(projs_chainedDirection.z * 100.0f);
				break;

			case GS_CMD_PROJSETCHAINEDDIRECTIONX:
				projs_chainedDirection.x = (float)*lastValue / 100.0f;
				break;

			case GS_CMD_PROJSETCHAINEDDIRECTIONY:
				projs_chainedDirection.y = (float)*lastValue / 100.0f;
				break;

			case GS_CMD_PROJSETCHAINEDDIRECTIONZ:
				projs_chainedDirection.z = (float)*lastValue / 100.0f;
				break;

			case GS_CMD_PROJROTATECHAINEDDIRECTIONONAXISY:
			case GS_CMD_PROJROTATECHAINEDDIRECTIONONAXISYBYVALUE:
				{
					float angle;
					if (command == GS_CMD_PROJROTATECHAINEDDIRECTIONONAXISYBYVALUE)
						angle = (float)*lastValue;
					else
						angle = (float)intData;
					angle *= 3.1415927f / 180.0f;
					float oldx = projs_chainedDirection.x;
					float oldz = projs_chainedDirection.z;
					projs_chainedDirection.x = oldx * cosf(angle) + oldz * sinf(angle);
					projs_chainedDirection.z = oldz * cosf(angle) - oldx * sinf(angle);
				}
				break;

			case GS_CMD_PROJGETHITPLANENORMALX:
				*lastValue = (int)(projs_hitPlaneNormal.x * 100.0f);
				break;

			case GS_CMD_PROJGETHITPLANENORMALY:
				*lastValue = (int)(projs_hitPlaneNormal.y * 100.0f);
				break;

			case GS_CMD_PROJGETHITPLANENORMALZ:
				*lastValue = (int)(projs_hitPlaneNormal.z * 100.0f);
				break;

			case GS_CMD_PROJISCHAINEDINBOUNCELIMITFROMORIGINAL:
				{
					VC3 tmp = -projs_originalDirection;
					float angle = tmp.GetAngleTo(projs_hitPlaneNormal) / 3.1415927f * 180.0f;
					if (angle >= (float)intData && angle <= 90.0f)
						*lastValue = 1;
					else 
						*lastValue = 0;
				}
				break;

			case GS_CMD_PROJISCHAINEDINBOUNCELIMITFROMORIGIN:
				{
					VC3 tmp = -projs_originDirection;
					float angle = tmp.GetAngleTo(projs_hitPlaneNormal) / 3.1415927f * 180.0f;
					if (angle >= (float)intData && angle <= 90.0f)
						*lastValue = 1;
					else 
						*lastValue = 0;
				}
				break;

			case GS_CMD_PROJBOUNCECHAINEDOFFPLANEFROMORIGINAL:
			case GS_CMD_PROJBOUNCECHAINEDOFFPLANEFROMORIGINALKEEPINGDIRECTIONY:
				{
					VC3 tmp = projs_originalDirection;
					float projected = projs_hitPlaneNormal.GetDotWith(tmp);
					projs_chainedDirection = tmp - projs_hitPlaneNormal * (2 * projected);

					if (projs_chainedDirection.GetSquareLength() > 0.00001f)
					{
						projs_chainedDirection.Normalize();
					}
					if (command == GS_CMD_PROJBOUNCECHAINEDOFFPLANEFROMORIGINALKEEPINGDIRECTIONY)
					{
						projs_chainedDirection.y = projs_originalDirection.y;
					}
				}
				break;

			case GS_CMD_PROJBOUNCECHAINEDOFFPLANEFROMORIGIN:
			case GS_CMD_PROJBOUNCECHAINEDOFFPLANEFROMORIGINKEEPINGDIRECTIONY:
				{
					VC3 tmp = projs_originDirection;
					float projected = projs_hitPlaneNormal.GetDotWith(tmp);
					projs_chainedDirection = tmp - projs_hitPlaneNormal * (2 * projected);

					if (projs_chainedDirection.GetSquareLength() > 0.00001f)
					{
						projs_chainedDirection.Normalize();
					}
					if (command == GS_CMD_PROJBOUNCECHAINEDOFFPLANEFROMORIGINKEEPINGDIRECTIONY)
					{
						projs_chainedDirection.y = projs_originalDirection.y;
					}
				}
				break;

			case GS_CMD_PROJRESTOREORIGINALDIRECTIONTOCHAINED:
				projs_chainedDirection = projs_originalDirection;
				break;

			case GS_CMD_PROJCOPYORIGINDIRECTIONTOCHAINED:
				projs_chainedDirection = projs_originDirection;
				break;

			case GS_CMD_PROJSTOPCHAINED:
				projs_chainedDirection = projs_originalDirection;
				projs_chainedRange = 0.0f;
				projs_skipRaytraceDistance = 0.0f;
				projs_lifeTime = -1;
				break;

			case GS_CMD_PROJGETORIGINPOSITION:
				gsd->position = projs_originPosition;
				break;

			case GS_CMD_PROJGETORIGINALCHAINEDPOSITION:
				gsd->position = projs_originalPosition;
				break;

			case GS_CMD_PROJGETCHAINEDPOSITION:
				gsd->position = projs_chainedPosition;
				break;

			case GS_CMD_PROJSETCHAINEDPOSITION:
				projs_chainedPosition = gsd->position;
				break;

			case GS_CMD_PROJSETCHAINEDLIFETIMETOVALUE:
				projs_lifeTime = *lastValue;
				break;

			case GS_CMD_PROJSOLVECHAINEDPASSDISTANCE:
				// TODO: proper implementation
				if (projs_hitChain == HITCHAIN_BUILDING)
					*lastValue = 5;
				else
					*lastValue = 35; // 35 cm (ok for now?)
				break;

			case GS_CMD_PROJMOVECHAINEDPOSITIONFORWARD:
				{
					VC3 dir = projs_chainedDirection;
					if (dir.GetSquareLength() > 0.00001f)
					{
						dir.Normalize();
					}
					projs_chainedPosition += dir * ((float)intData / 100.0f);
				}
				break;

			case GS_CMD_PROJMOVECHAINEDPOSITIONFORWARDBYVALUE:
				{
					VC3 dir = projs_chainedDirection;
					if (dir.GetSquareLength() > 0.00001f)
					{
						dir.Normalize();
					}
					projs_chainedPosition += dir * ((float)*lastValue / 100.0f);
				}
				break;

			case GS_CMD_PROJSETSKIPRAYTRACEDISTANCE:
				projs_skipRaytraceDistance = ((float)intData / 100.0f);
				break;

			case GS_CMD_PROJSETSKIPRAYTRACEDISTANCEBYVALUE:
				projs_skipRaytraceDistance = ((float)*lastValue / 100.0f);
				break;

			case GS_CMD_PROJGETHITUNIT:
				gsd->unit = projs_hitUnit;
				if (gsd->unit != NULL)
					*lastValue = 1;
				else
					*lastValue = 0;
				break;

			case GS_CMD_PROJGETSHOOTERUNIT:
				gsd->unit = projs_shooter;
				if (gsd->unit != NULL)
					*lastValue = 1;
				else
					*lastValue = 0;
				break;

			case GS_CMD_PROJISORIGINBULLET:
				if (stringData != NULL)
				{
					*lastValue = 0;

					if (!PARTTYPE_ID_STRING_VALID(stringData))
					{
						sp->error("UnitScripting::process - projIsOriginBullet, illegal part type id.");
					} else {
						PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
						if (pt == NULL) 
						{ 
							sp->error("UnitScripting::process - projIsOriginBullet, reference to unloaded part type.");
						} else {
							if (pt->isInherited(
								getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
							{ 
								// WARNING: unsafe cast (based on check above)
								Bullet *bullet = (Bullet *)pt;
								if (projs_originBulletType == bullet)
								{
									*lastValue = 1;
								} else {
									*lastValue = 0;
								}
							} else {
								sp->error("UnitScripting::process - projIsOriginBullet, part type is not of bullet type.");
							}
						}
					}
				} else {
					sp->error("HitChainScripting::process - projIsOriginBullet parameter missing, bullet type expected.");
				}
				break;

			case GS_CMD_PROJISHITCHAINNAME:
				if (stringData != NULL)
				{					
					if (projs_hitChain >= 0 && projs_hitChain < HITCHAIN_AMOUNT)
					{
						if (hitChainName[projs_hitChain] != NULL 
							&& strcmp(stringData, hitChainName[projs_hitChain]) == 0)
							*lastValue = 1;
						else
							*lastValue = 0;
					} else {
						sp->error("HitChainScripting::process - projIsHitChainName internal error, bad hitchain number.");
					}
				} else {
					sp->error("HitChainScripting::process - projIsHitChainName parameter missing, hit chain name expected.");
				}
				break;

			case GS_CMD_PROJGETHITCHAINNAME:
				if (projs_hitChain >= 0 && projs_hitChain < HITCHAIN_AMOUNT)
				{
					gsd->setStringValue(hitChainName[projs_hitChain]);
				} else {
					sp->error("HitChainScripting::process - projGetHitChainName internal error, bad hitchain number.");
				}
				break;

			case GS_CMD_PROJISHITCHAINNUMBER:
				if (projs_hitChain == intData)
					*lastValue = 1;
				else
					*lastValue = 0;
				break;

			case GS_CMD_PROJGETHITCHAINNUMBER:
				*lastValue = projs_hitChain;
				break;

			case GS_CMD_PROJSETCHAINEDBULLET:
				if (stringData != NULL)
				{
					*lastValue = 0;

					if (!PARTTYPE_ID_STRING_VALID(stringData))
					{
						sp->error("UnitScripting::process - projSetChainedBullet, illegal part type id.");
					} else {
						PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(stringData));
						if (pt == NULL) 
						{ 
							sp->error("UnitScripting::process - projSetChainedBullet, reference to unloaded part type.");
						} else {
							if (pt->isInherited(
								getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
							{ 
								// WARNING: unsafe cast (based on check above)
								Bullet *bullet = (Bullet *)pt;
								projs_chainedBulletType = bullet;
							} else {
								sp->error("UnitScripting::process - projSetChainedBullet, part type is not of bullet type.");
							}
						}
					}
				} else {
					sp->error("HitChainScripting::process - projSetChainedBullet parameter missing, bullet type expected.");
				}
				break;

			case GS_CMD_PROJSETCHAINEDRANGE:
				{
					float floatData = intFloat.f;
					projs_chainedRange = floatData;
				}
				break;

			case GS_CMD_PROJSETCHAINEDVELOCITYFACTOR:
				{
					float floatData = intFloat.f;
					projs_chainedVelocityFactor = floatData;
				}
				break;

			case GS_CMD_PROJSPAWNCHAINED:
				if (projs_chainedDirection.GetSquareLength() > 0.00001f)
				{
					projs_chainedDirection.Normalize();
				}
				if (projs_chainedBulletType != NULL)
				{
					Projectile *cproj = new Projectile(projs_shooter, projs_chainedBulletType);
					game->projectiles->addProjectile(cproj);

					cproj->setPosition(projs_chainedPosition);

					cproj->setChainCustomValue(projs_chainedCustomValue);

					VC3 target = projs_chainedPosition + (projs_chainedDirection * projs_chainedRange);

					VC3 raypos = projs_chainedPosition + (projs_chainedDirection * projs_skipRaytraceDistance);

					// TODO: pass through wall/hitunit/... (skipraytraceamount?)
					ProjectileActor pa = ProjectileActor(game);
					pa.doProjectileRaytrace(projs_shooter, projs_hitUnit, cproj, projs_chainedBulletType, projs_chainedPosition,
						raypos, target, projs_chainedDirection, projs_chainedRange, NULL, projs_chainedVelocityFactor);

					if (projs_lifeTime > 0)
					{
						cproj->setLifeTime(projs_lifeTime);
					}

					if (projs_chainedRotationRandom)
					{
						float rx = (float)(game->gameRandom->nextInt() % 360);
						float ry = (float)(game->gameRandom->nextInt() % 360);
						float rz = (float)(game->gameRandom->nextInt() % 360);
						cproj->setRotation(rx, ry, rz);
					}

					pa.createVisualForProjectile(cproj, projs_chainedOriginToHitUnit, projs_hitUnit);
				}
				break;

			case GS_CMD_PROJGETCHAINEDCUSTOMVALUE:
				*lastValue = projs_chainedCustomValue;
				break;

			case GS_CMD_PROJSETCHAINEDCUSTOMVALUE:
				projs_chainedCustomValue = *lastValue;
				break;

			case GS_CMD_PROJSETCHAINEDROTATIONRANDOMIZATION:
				if (intData != 0)
					projs_chainedRotationRandom = true;
				else
					projs_chainedRotationRandom = false;
				break;

			case GS_CMD_PROJSETCHAINEDORIGINTOHITUNIT:
				if (intData != 0)
					projs_chainedOriginToHitUnit = true;
				else
					projs_chainedOriginToHitUnit = false;
				break;

			case GS_CMD_PROJFLATTENCHAINEDONPLANE:
				{
					float floatData = intFloat.f;
					float projected = projs_hitPlaneNormal.GetDotWith(projs_chainedDirection);
					projs_chainedDirection -= projs_hitPlaneNormal * (floatData*projected);

					if (projs_chainedDirection.GetSquareLength() > 0.00001f)
					{
						projs_chainedDirection.Normalize();
					}
				}
				break;


			default:
				sp->error("HitChainScripting::process - Unknown command.");				
				assert(0);
		}
	}
}



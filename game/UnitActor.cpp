
#include "precompiled.h"

#include "UnitActor.h"
#include "../system/Logger.h"
#include "Unit.h"
#include "UnitType.h"
#include "Game.h"
#include "GameMap.h"
#include "GameUI.h"
#include "UnitLevelAI.h"
#include "GameRandom.h"
#include "Part.h"
#include "Flashlight.h"
#include "UnitActAnimationRequests.h"
#include "scaledefs.h"
#include "GameScene.h"
#include "UnitList.h"
#include "Weapon.h"
#include "Character.h"
#include "GameCollisionInfo.h"
#include "Projectile.h"
#include "ProjectileActor.h"
#include "ProjectileList.h"
#include "MaterialManager.h"
#include "ReconChecker.h"
#include "CoverMap.h"
#include "scripting/GameScripting.h"
#include "SimpleOptions.h"
#include "options/options_game.h"
#include "options/options_effects.h"
#include "../sound/sounddefs.h"
#include "../sound/SoundLooper.h"

#include "../util/LightAmountManager.h"
#include "../util/ColorMap.h"
#include "../util/HelperPositionCalculator.h"
#include "../ui/Animator.h"
#include "../ui/AnimationSet.h"
#include "../ui/animdefs.h"
#include "../ui/VisualEffectManager.h"
#include "../ui/CombatWindow.h"
#include "../convert/str2int.h"

#include "../util/fb_assert.h"

#include "../util/AngleRotationCalculator.h"
#include "../util/PositionsDirectionCalculator.h"
#include "../util/Debug_MemoryManager.h"

#include "DHLocaleManager.h"
#include "../ui/VisualEffect.h"
#include "IAIDirectControl.h"

#include <sstream>

// how often is line of fire to target checked (value must be 2^n)
#define LINE_OF_FIRE_CHECK_INTERVAL 32

// range within which we ignore frienly units that would block 
// the line of fire
#define UNITACTOR_LOF_IGNORE_FRIENDLY_RANGE 100

// area within which we accept ground target line of fire checks.
#define UNITACTOR_GROUND_TARGET_AREA 5

// range of noise made by noisy units
#define UNITACTOR_NOISY_RANGE 8

// max range of "mindless" rush
#define UNITACTOR_RUSH_DISTANCE 5.0f

// 70 msec muzzleflashes
#define UNITACTOR_MUZZLEFLASH_DURATION (70 / GAME_TICK_MSEC)
#define UNITACTOR_FLAMERFLASH_DURATION (700 / GAME_TICK_MSEC)
#define UNITACTOR_FLAMERFLASH_RESET_TIME (100 / GAME_TICK_MSEC)

// HACK: 2 ticks eject effects?
#define UNITACTOR_EJECT_DURATION 2

#define UNITACTOR_RECHARGE_DURATION (100 / GAME_TICK_MSEC)

// 2 meters execute range for normal units
#define UNITACTOR_EXECUTE_RANGE 1.8f

// 2.5 meters execute range for doors
#define DOOR_EXECUTE_RANGE 2.3f

// 20 meters max autoaim
#define AUTOAIM_MAX_DIST 20

// 20 degree autoaim to each side (= 40 degree foaf ;)
// THIS VALUE APPLIES TO VERTICAL AUTOAIM, HORIZONTAL(/FULL)
// AUTOAIM IS THIS VALUE HALVED
#define AUTOAIM_MAX_ANGLE 20

// 0.4 meter squash radius
#define SQUASHABLE_MAX_RANGE 0.4f

// TODO: this should be in some conf... 
#define DIRECTCTRL_JUMP_TIME 1000

// TODO: should be in some conf
// first 100msec of roll jump can be "cancelled" (if hits a wall)
// after that, the roll must be fully completed 
// (or else the animations would look improperly blended)
#ifdef PROJECT_AOV
#define ROLL_JUMP_ALLOW_STOP_TIME 0
#else
#define ROLL_JUMP_ALLOW_STOP_TIME 200
#endif

#define JUMP_NOT_ALLOWED_TIME 200

// how near friends must be for faster hpgain...
#define HPGAIN_FRIENDS_NEAR_RANGE 5

// how far away hostiles must be for faster hpgain...
// (note, if some nearby friendly sees a hostile at max. 2x this distance, that counts too)
#define HPGAIN_HOSTILES_NEAR_RANGE 10


namespace game
{
	/*
  extern bool foofoo_diag1;
  extern bool foofoo_diag2;
	*/

namespace 
{
	bool IsCloseEnough2D( const VC3& pos1, const VC3 pos2, float range )
	{
		VC3 temp = pos1 - pos2;
		return ( ( temp.x * temp.x + temp.z * temp.z ) < ( range * range ) );
	}
}

	void UnitActor::act(Unit *unit)
	{
		// nop  
		assert(!"UnitActor::act - Base class method called.");
	}


	void UnitActor::frictionVelocity(VC3 *velocity, float friction)
	{
		float vx = velocity->x;
		float vz = velocity->z;

		if (fabs(vx) < 0.002f)
		{
			vx = 0.0f;
		}
		if (fabs(vz) < 0.002f)
		{
			vz = 0.0f;
		}

		if (vx > 0)
		{
			vx -= vx * friction;
			if (vx < 0) vx = 0;
		} else {
			if (vx < 0)
			{
				vx -= vx * friction;
				if (vx > 0) vx = 0;
			}
		}
		if (vz > 0)
		{
			vz -= vz * friction;
			if (vz < 0) vz = 0;
		} else {
			if (vz < 0)
			{
				vz -= vz * friction;
				if (vz > 0) vz = 0;
			}
		}

		velocity->x = vx;
		velocity->z = vz;
	}


	void UnitActor::balanceRotation(VC3 *rotation, float factor)
	{
		float rx = rotation->x;
		float rz = rotation->z;

		if (rx > 180)
		{
			rx += (360-rx) * factor;
			if (rx >= 360) rx = 0;
		} else {
			if (rx > 0)
			{
				rx -= rx * factor;
				if (rx < 0) rx = 0;
			}
		}
		if (rz > 180)
		{
			rz += (360-rz) * factor;
			if (rz >= 360) rz = 0;
		} else {
			if (rz > 0)
			{
				rz -= rz * factor;
				if (rz < 0) rz = 0;
			}
		}

		rotation->x = rx;
		rotation->z = rz;
	}


	// rotateTo is an offset value to balance, between -180 to 180.
	// (not a normal 0-360 value).
	void UnitActor::rotateTo(const VC3 &rotateTo, VC3 *rotation, float factor)
	{
		float rx = rotation->x - rotateTo.x;
		float rz = rotation->z - rotateTo.z;
		if (rx >= 360) rx -= 360.0f;
		if (rz >= 360) rz -= 360.0f;
		if (rx < 0) rx += 360.0f;
		if (rz < 0) rz += 360.0f;

		if (rx > 180) 
		{
			rx += (360-rx) * factor;
			if (rx > 360) rx = 0;
		} else {
			if (rx > 0)
			{
				rx -= rx * factor;
				if (rx < 0) rx = 0;
			}
		}
		if (rz > 180)
		{
			rz += (360-rz) * factor;
			if (rz > 360) rz = 0;
		} else {
			if (rz > 0)
			{
				rz -= rz * factor;
				if (rz < 0) rz = 0;
			}
		}

		rotation->x = rx + rotateTo.x;
		rotation->z = rz + rotateTo.z;
		if (rotation->x >= 360) rotation->x -= 360.0f;
		if (rotation->z >= 360) rotation->z -= 360.0f;
		if (rotation->x < 0) rotation->x += 360.0f;
		if (rotation->z < 0) rotation->z += 360.0f;
	}


	void UnitActor::createMuzzleflash(Unit *unit, int weaponNumber)
	{
		int weap = weaponNumber;

		Weapon *w = unit->getWeaponType(weap);
		if (w == NULL)
		{
			assert(!"UnitActor::createMuzzleflash - Weapon type null.");
			return;
		}

		// muzzleflash
		VC3 position = unit->getPosition();
		VC3 rotation = unit->getRotation();
		
		const char *muzzleflashEffect = w->getMuzzleflashEffect();
		if (muzzleflashEffect != NULL)
		{
			if (unit->isMuzzleflashVisible())
			{
				Logger::getInstance()->debug("UnitActor::createMuzzleflash - Unit's muzzleflash already visible while trying to create another.");
				//assert(!"Unit's muzzleflash already visible while trying to create another.");

				if (strncmp(muzzleflashEffect, "flamer", 6) == 0)
				{
					unit->resetMuzzleflashDuration(UNITACTOR_FLAMERFLASH_DURATION, UNITACTOR_FLAMERFLASH_RESET_TIME);
				} else {
					unit->resetMuzzleflashDuration(UNITACTOR_MUZZLEFLASH_DURATION);
				}

				VC3 lightRot = unit->getRotation();
				lightRot.y += unit->getLastBoneAimDirection();
				game->gameUI->getVisualEffectManager()->updateSpotlightPosition(unit->getMuzzleflashVisualEffect(),
					unit->getPosition(), unit, lightRot);

			} else {
				// HACK: if spotlight on, use xxx_low effect instead.
				// same goes for well illuminated areas.
				// TODO: need gradiental change, not just low/high change!
				int visId = -1;
				float colx = position.x / game->gameMap->getScaledSizeX() + .5f;
				float coly = position.z / game->gameMap->getScaledSizeY() + .5f;
				COL colm = game->gameMap->colorMap->getColor(colx, coly);
				if ((unit->getFlashlight() != NULL
					&& unit->getFlashlight()->isFlashlightOn())
					|| (colm.r > 0.1f && colm.g > 0.1f && colm.b > 0.1f))
				{
					//Logger::getInstance()->error(int2str(colm.r * 100.0f));
					char muzzleflashEffectLow[64 + 1];
					if (strlen(muzzleflashEffect) < 60)
					{
						strcpy(muzzleflashEffectLow, muzzleflashEffect);
						strcat(muzzleflashEffectLow, "_low");
					}

					//visId = VisualEffectManager::getVisualEffectIdByName(muzzleflashEffectLow);
					static int last_muzzleflash_low_ve_id = -1;
					static std::string last_muzzleflash_low_ve_name = std::string();
					if (strcmp(last_muzzleflash_low_ve_name.c_str(), muzzleflashEffectLow) != 0)
					{
						last_muzzleflash_low_ve_id = VisualEffectManager::getVisualEffectIdByName(muzzleflashEffectLow);
						last_muzzleflash_low_ve_name = std::string(muzzleflashEffectLow);
					}
					visId = last_muzzleflash_low_ve_id;

				}
				if (visId == -1)
				{
					//visId = VisualEffectManager::getVisualEffectIdByName(muzzleflashEffect);
					static int last_muzzleflash_ve_id = -1;
					static std::string last_muzzleflash_ve_name = std::string();
					if (strcmp(last_muzzleflash_ve_name.c_str(), muzzleflashEffect) != 0)
					{
						last_muzzleflash_ve_id = VisualEffectManager::getVisualEffectIdByName(muzzleflashEffect);
						last_muzzleflash_ve_name = std::string(muzzleflashEffect);
					}
					visId = last_muzzleflash_ve_id;
				}

				//float angle = unit->getLastBoneAimDirection(); 
				//float angle = unit->getFlashlightDirection() - 45;
				float angle = unit->getFlashlightDirection();
				// ??? ..above.. ??????????

				VC3 rot = unit->getRotation();
				rot.y += angle;
				if (rot.y >= 360.0f) rot.y -= 360.0f;
				if (rot.y < 0.0f) rot.y += 360.0f;

				VC3 weaponPosition(unit->getPosition());
				if (w->doesFireFromWeaponBarrel())
				{
					VisualObject *unitVisual = unit->getVisualObject();
					IStorm3D_Model *unitModel = 0;
					if(unitVisual)
					{
						if(unitVisual)
							unitModel = unitVisual->getStormModel();

						if(unitModel)
						{
							// custom helper
							if(unit->getWeaponType(weap)->getCustomWeaponBarrelHelper())
							{
								bool helperFound = util::getHelperPosition(unitModel, unit->getWeaponType(weap)->getCustomWeaponBarrelHelper(), weaponPosition);
								if (!helperFound)
								{
									Logger::getInstance()->warning("UnitActor::actWeaponry - Custom weapon barrel helper not found.");
								}
							}
							else
							{
								std::string barrelName = "WeaponBarrel";
								if (unit->getRotatedWeaponBarrel(weap) >= 2)
									barrelName += int2str(unit->getRotatedWeaponBarrel(weap));

								std::string modelHelperName = std::string("HELPER_MODEL_") + barrelName;
								bool helperFound = util::getHelperPosition(unitModel, modelHelperName.c_str(), weaponPosition);
								if (!helperFound)
								{
									std::string boneHelperName = std::string("HELPER_BONE_") + barrelName;
									helperFound = util::getHelperPosition(unitModel, boneHelperName.c_str(), weaponPosition);
									if (!helperFound)
									{
										Logger::getInstance()->warning("UnitActor::actWeaponry - Weapon barrel helper not found.");
									}
								}
							}
						}
					}
				}

				VisualEffect *ve = game->gameUI->getVisualEffectManager()->createNewVisualEffect(
					visId, NULL, unit, weaponPosition, weaponPosition,
					rot, unit->getVelocity(), game, unit->getRotatedWeaponBarrel(weap));
				if (strncmp(muzzleflashEffect, "flamer", 6) == 0)
				{
					unit->setMuzzleflashVisualEffect(ve, UNITACTOR_FLAMERFLASH_DURATION);
				} else {
					unit->setMuzzleflashVisualEffect(ve, UNITACTOR_MUZZLEFLASH_DURATION);
				}
			}

			VC3 dir = VC3(0,0,0);
			dir.x = -sinf(UNIT_ANGLE_TO_RAD(rotation.y));
			dir.z = -cosf(UNIT_ANGLE_TO_RAD(rotation.y));

			int len = UNITACTOR_MUZZLEFLASH_DURATION * GAME_TICK_MSEC;
			unit->getDirectionalLight().addFlash(dir, 0, len / 2, len);
		}
	}


	void UnitActor::createEject(Unit *unit, int weaponNumber)
	{
		int weap = weaponNumber;

		Weapon *w = unit->getWeaponType(weap);
		if (w == NULL)
		{
			assert(!"UnitActor::createEject - Weapon type null.");
			return;
		}

		if (w->getEjectEffect() == NULL)
		{
			// no eject effect for this weapon, just return.
			return;
		}

		int rateMult = 1;
		if (SimpleOptions::getInt(DH_OPT_I_WEAPON_EJECT_EFFECT_LEVEL) == 0)
		{
			return;
		}
		else if (SimpleOptions::getInt(DH_OPT_I_WEAPON_EJECT_EFFECT_LEVEL) == 50)
		{
			rateMult = 2;
		}

		// HACK: first shot (in not repetitive shots), always do the eject! (???)
		if (unit->getAimStopCounter() > GAME_TICKS_PER_SECOND / 3)
		{
			// HACK: HACK
			rateMult = 0;
		}

		if (!unit->increaseEjectRateCounter(w->getEjectRate() * rateMult))
		{
			// eject rate counter has not reached limit...
			return;
		}

		// eject
		VC3 position = unit->getPosition();
		VC3 rotation = unit->getRotation();
		
		const char *ejectEffect = w->getEjectEffect();
		if (ejectEffect != NULL)
		{
			if (unit->isEjectVisible())
			{
				Logger::getInstance()->debug("UnitActor::createEject - Unit's eject \"already visible\" (in progress) while trying to create another.");
				//assert(!"Unit's eject already visible while trying to create another.");

				//unit->resetEjectDuration(UNITACTOR_EJECT_DURATION);
				//???
			} else {

				unit->rotateWeaponEjectBarrel(weap);

				int visId = VisualEffectManager::getVisualEffectIdByName(ejectEffect);

				//float angle = unit->getLastBoneAimDirection(); 
				//float angle = unit->getFlashlightDirection() - 45;
				float angle = unit->getFlashlightDirection();
				// ??? ..above.. ??????????

				VC3 rot = unit->getRotation();
				rot.y += angle;
				if (rot.y >= 360.0f) rot.y -= 360.0f;
				if (rot.y < 0.0f) rot.y += 360.0f;

				rot.y += (float)(w->getBarrelEjectAngle(unit->getRotatedWeaponEjectBarrel(weap)));
				if (rot.y >= 360.0f) rot.y -= 360.0f;
				if (rot.y < 0.0f) rot.y += 360.0f;

				VC3 weaponPosition(unit->getPosition());
				if (w->doesFireFromWeaponBarrel())
				{
					VisualObject *unitVisual = unit->getVisualObject();
					IStorm3D_Model *unitModel = 0;
					if(unitVisual)
					{
						if(unitVisual)
							unitModel = unitVisual->getStormModel();

						if(unitModel)
						{
							bool helperFound = false;

#ifdef PROJECT_SURVIVOR
							if(unit->getUnitType()->hasMechControls())
							{
								// mech uberhack
								std::string barrelName = "WeaponBarrel";
								std::string barrelIndex;
								float offset = 1.5f;
								if( unit->getRotatedWeaponEjectBarrel(weap) >= 2 )
								{
									barrelIndex = int2str(unit->getRotatedWeaponEjectBarrel(weap));
									offset = -1.5f;
								}
								std::string boneHelperName = std::string("HELPER_BONE_") + barrelName + barrelIndex;
								helperFound = util::getHelperPositionOffset(unitModel, boneHelperName.c_str(), VC3(offset,0,0), weaponPosition);
							}
#endif
							std::string barrelName = "WeaponEject";
							if(!helperFound)
							{
								if (unit->getRotatedWeaponEjectBarrel(weap) >= 2)
								{
									// try to find weaponeject helper with barrel index
									std::string barrelIndex = int2str(unit->getRotatedWeaponEjectBarrel(weap));
									std::string modelHelperName = std::string("HELPER_MODEL_") + barrelName + barrelIndex;
									helperFound = util::getHelperPosition(unitModel, modelHelperName.c_str(), weaponPosition);
									if (!helperFound)
									{
										std::string boneHelperName = std::string("HELPER_BONE_") + barrelName + barrelIndex;
										helperFound = util::getHelperPosition(unitModel, boneHelperName.c_str(), weaponPosition);
									}
								}
							}

							if(!helperFound)
							{
								std::string modelHelperName = std::string("HELPER_MODEL_") + barrelName;
								helperFound = util::getHelperPosition(unitModel, modelHelperName.c_str(), weaponPosition);
								if (!helperFound)
								{
									std::string boneHelperName = std::string("HELPER_BONE_") + barrelName;
									helperFound = util::getHelperPosition(unitModel, boneHelperName.c_str(), weaponPosition);
									if (!helperFound)
									{
										Logger::getInstance()->warning("UnitActor::createEject - Weapon eject helper not found.");
									}
								}
							}
						}
					}
				}

				VisualEffect *ve = game->gameUI->getVisualEffectManager()->createNewVisualEffect(
					visId, NULL, unit, weaponPosition, weaponPosition,
					rot, unit->getVelocity(), game, unit->getRotatedWeaponBarrel(weap));

				unit->setEjectVisualEffect(ve, UNITACTOR_EJECT_DURATION);
			}
		}
	}


	void UnitActor::resetToNormalState(Unit *unit)
	{
		UnitType *unitType = unit->getUnitType();

		// get rid of possible jumping..
		unit->setJumpCounter(0);
		unit->setJumpTotalTime(0);
		unit->restoreDefaultSpeed();

		// get rid of target
		unit->targeting.clearTarget();

		// end movement (paths) and velocity
		this->stopUnit(unit);
		unit->setVelocity(VC3(0,0,0));

		// clear strafe offsets...
		unit->setStrafeAimOffset(0);

		unit->setHitAnimationCounter(0);
		unit->setHitAnimationBoneAngle(0);

		unit->setBurnedCrispyAmount(0);

		// get rid of aiming direction..
		char *aimBone = unitType->getAimBone();
		if (aimBone != NULL)
		{
			if (unit->getVisualObject() != NULL)
			{
				//unit->getVisualObject()->rotateBone(aimBone, 0);
				unit->getVisualObject()->releaseRotatedBone(aimBone);
			}
		}

		Animator::endBlendAnimation(unit, 2, false);
		Animator::endBlendAnimation(unit, 1, false);
	}


	void UnitActor::stopJumpBecauseCollided(Unit *unit)
	{
		// HACK: if the unit was a jumping, end the jump
		// but only if allowed!
		if (unit->getJumpCounter() > 0)
		{
			if (!unit->getUnitType()->doesRollJump()
				|| unit->getJumpTotalTime() - unit->getJumpCounter() < ROLL_JUMP_ALLOW_STOP_TIME / GAME_TICK_MSEC)
			{
				unit->setJumpCounter(0);
				unit->setJumpTotalTime(0);
				unit->restoreDefaultSpeed();
				// HACK: 0.5 sec delay!!! (would need a conf)
				unit->setWalkDelay(unit->getUnitType()->getJumpLandDelay() / GAME_TICK_MSEC);
			}
		}
	}


	void UnitActor::getAutoAimNear(Unit *unit, const VC3 &target, Unit **autoAimed, bool *aimedVertical, bool *aimedHorizontal)
	{
		// TODO: vertical/horizontal autoaim need proper implementation
		// now they are just hacks which work "a bit like they should"...
		// (horizontal uses a small vertical autoaim)
		// (vertical always means horizontal autoaim too)

		if (!game->isAutoAimVertical() && !game->isAutoAimHorizontal())
		{
			*autoAimed = NULL;
			*aimedVertical = false;
			*aimedHorizontal = false;
			return;
		}

		VC3 unitPos = unit->getPosition();

		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter(ulist);
		Unit *nearest = NULL;
		bool nearestAimVert = false;
		bool nearestAimHoriz = false;
		float nearestDistSq = AUTOAIM_MAX_DIST * AUTOAIM_MAX_DIST;
		while (iter.iterateAvailable())
		{
			Unit *other = (Unit *)iter.iterateNext();
			if (other->isActive() && !other->isDestroyed()
				&& other != unit
				&& game->isHostile(unit->getOwner(), other->getOwner()))
			{
				VC3 distVec = other->getPosition() - unitPos;
				float distSq = distVec.GetSquareLength();
				if (distSq < nearestDistSq)
				{
					// if very near, double autoaim angle...
					float aaAngle = AUTOAIM_MAX_ANGLE;
					//if (distSq < 3 * 3)	
					//	aaAngle *= 2;
					
					float aaAngleHoriz = aaAngle * 0.5f;

					// no horizontal autoaim? (just vertical)
					// then drop the vertical autoaim angle to 1/2 (a total hack)
					if (!game->isAutoAimHorizontal())
					{
						//aaAngle *= 0.5f;
						aaAngleHoriz *= 0.5f;
					}

					// if _very very_ near, get almost 90 (180) autoaim angle
					// everything in front gets aimed...
					if (distSq < 2 * 2)
					{
						aaAngle = 25; // 25 should be enough?
						aaAngleHoriz = aaAngle;
					}

					// TODO: not very effective maybe... =/
					float targAngle = util::PositionDirectionCalculator::calculateDirection(unitPos, target);
					float otherAngle = util::PositionDirectionCalculator::calculateDirection(unitPos, other->getPosition());
					float angleRot = util::AngleRotationCalculator::getRotationForAngles(targAngle, otherAngle, aaAngle);
					float angleRotVert = util::AngleRotationCalculator::getRotationForAngles(targAngle, otherAngle, aaAngleHoriz);

					if (angleRotVert == 0)
					{
						// we have a horizontal (sideways) autoaim hit, which means we are 
						// going to do full autoaim.
						nearestDistSq = distSq;
						nearest = other;
						nearestAimVert = true;
						nearestAimHoriz = true;
					} else {
						if (angleRot == 0) 
						{
							// did not have horiz autoaim hit? then maybe should
							// just do a vertical autoaim...
							nearestDistSq = distSq;
							nearest = other;
							nearestAimVert = true;
							nearestAimHoriz = false;
						}
					}
				}
			}
		}
		if (nearest != NULL)
		{
			*autoAimed = nearest;
			*aimedVertical = nearestAimVert;
			*aimedHorizontal = nearestAimHoriz;
		} else {
			*autoAimed = NULL;
			*aimedVertical = false;
			*aimedHorizontal = false;
		}
	}


	void UnitActor::makeFireSound(Unit *unit, int weapon, const char *fireSound)
	{
		VC3 pos = unit->getPosition();
		int handle = unit->getWeaponLoopSoundHandle(weapon);
		int key;
		bool looped;
		if (handle != -1)
		{
			// attempt to continue old sound
			key = unit->getWeaponLoopSoundKey(weapon);
			handle = game->gameUI->parseSoundFromDefinitionString(fireSound, pos.x, pos.y, pos.z, &looped, &handle, &key, true, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_NORMAL);
		} else {
			// not looped, the maybe should cut the previous sound?
			int nonLoopedHandle = unit->getWeaponSoundHandle(weapon);
			if (nonLoopedHandle != -1)
			{
				game->gameUI->stopSound(nonLoopedHandle);
				unit->setWeaponSoundHandle(weapon, -1);
			}
		}
		if (handle == -1)
		{
			// old looping sound was not continued, need to create a new sound
			handle = game->gameUI->parseSoundFromDefinitionString(fireSound, pos.x, pos.y, pos.z, &looped, &handle, &key, false, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_NORMAL);
			if (looped)
			{
				unit->setWeaponSoundHandle(weapon, -1);
				unit->setWeaponLoopSoundHandle(weapon, handle, key);
			} else {
				unit->setWeaponLoopSoundHandle(weapon, -1, -1);
				unit->setWeaponSoundHandle(weapon, handle);
			}
		}
	}


	void UnitActor::doSpecialMove(Unit *unit, bool forward, bool backward, bool left, bool right, int moveTime)
	{
		unit->setSpeed(Unit::UNIT_SPEED_JUMP);
		unit->setJumpCounter(moveTime);
		unit->setJumpTotalTime(moveTime);

		unit->setJumpNotAllowedTime(JUMP_NOT_ALLOWED_TIME / GAME_TICK_MSEC);

		// NEW: jump is no-longer relative to camera, need to rotate based on 
		// unit rotation...
		{
			//VC3 zeroVec = VC3(0,0,0);
			//VC3 vel = unit->getVelocity();
			//float velDir = util::PositionDirectionCalculator::calculateDirection(zeroVec, vel);

			VC3 rotation = unit->getRotation();

			float angle = rotation.y;
			if (unit->isDirectControl())
//				&& game->gameUI->isThirdPersonView(unit->getOwner()))
			{
				angle = 270-game->gameUI->getGameCamera()->getAngleY();
			}
			if (backward)
			{
				angle += 180;
				if (left)
					angle += 45;
				if (right)
					angle -= 45;
			} else {
				if (forward)
				{
					if (left)
						angle += 270+45;
					if (right)
						angle += 45;
				} else {
					if (left)
						angle += 270;
					if (right && !left)
						angle += 90;
				}
			}
			while (angle < 0) angle += 360;
			while (angle >= 360) angle -= 360;

			float rotDirFactored = util::AngleRotationCalculator::getFactoredRotationForAngles(rotation.y, angle, 0);
			if ((float)fabs(rotDirFactored) < 45-22.5f)
			{
				// fwd
				unit->setUnitRelativeJumpDirections(true, false, false, false);
			} 
			else if ((float)fabs(rotDirFactored) < 90-22.5f)
			{
				// fwd+left/right
				if (rotDirFactored < 0)
					unit->setUnitRelativeJumpDirections(true, false, true, false);
				else
					unit->setUnitRelativeJumpDirections(true, false, false, true);
			} 
			else if ((float)fabs(rotDirFactored) < 135-22.5f)
			{
				// left/right
				if (rotDirFactored < 0)
					unit->setUnitRelativeJumpDirections(false, false, true, false);
				else
					unit->setUnitRelativeJumpDirections(false, false, false, true);
			} 
			else if ((float)fabs(rotDirFactored) < 180-22.5f)
			{
				// back+left/right
				if (rotDirFactored < 0)
					unit->setUnitRelativeJumpDirections(false, true, true, false);
				else
					unit->setUnitRelativeJumpDirections(false, true, false, true);
			} else {
				// back
				unit->setUnitRelativeJumpDirections(false, true, false, false);
			}
		}

		unit->setCameraRelativeJumpDirections(forward, backward, left, right);

		unit->setJumpAnim(ANIM_NONE);

		// HACK: keep destination on target for turnable jumps (rushes)...
		// but for forward jumps only!
		if ((unit->getUnitType()->isJumpTurnable()
			|| (unit->getJumpTotalTime() - unit->getJumpCounter()) < unit->getUnitType()->getTurnTimeAtJumpStart() / GAME_TICK_MSEC)
			&& unit->targeting.hasTarget()
			&& unit->targeting.getTargetUnit() != NULL
			&& forward && !left && !right)
		{
			VC3 tpos = unit->targeting.getTargetUnit()->getPosition();
			unit->setWaypoint(tpos);
			unit->setFinalDestination(tpos);
		}
	}


	void UnitActor::actStationary(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		VC3 position = unit->getPosition();
    float mapY = game->gameMap->getScaledHeightAt(position.x, position.z);
    position.y = mapY;
		position.y += unit->getUnitType()->getHeightOffset();
    VC3 velocity = VC3(0,0,0);
	  unit->setVelocity(velocity);
		unit->setPosition(position);
	}

	

	void UnitActor::actDestroyed(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		unit->setDestroyedTime(unit->getDestroyedTime() + 1);

		VC3 velocity = unit->getVelocity();
		UnitType *unitType = unit->getUnitType();
		if (!unitType->isBlockIfDestroyed())
		{
			// FIXME: this should be called only once, maybe when the
			// projectile sets the unit destroyed. However, the 
			// removeUnitObstacle does check that the obstacle exists,
			// thus, calling the actual removal only once.
			removeUnitObstacle(unit);
		}
		if (unitType->isSticky() && unit->isOnGround() && !unit->isSideways())
    {
      velocity = VC3(0,0,0);
    } else {
			if (unit->isGroundFriction())
				frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
			else
				frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
    }
	  unit->setVelocity(velocity);

		// death bleed still to add?
		if (unit->getDeathBleedDelay() > 0)
		{
			// decrease counter if almost zero velocity
			if (fabs(velocity.x) < 0.001f
				&& fabs(velocity.y) < 0.001f
				&& fabs(velocity.z) < 0.001f)
			{
				unit->setDeathBleedDelay(unit->getDeathBleedDelay() - 1);
			}
			if (unit->getDeathBleedDelay() == 0)
			{
				Bullet *unitDeathBleedBullet = unitType->getDeathBleedBullet();
				if (unitDeathBleedBullet != NULL)
				{
					Projectile *bleedproj = new Projectile(NULL, unitDeathBleedBullet);
					game->projectiles->addProjectile(bleedproj);

					VC3 bleedpos = unit->getPosition();
					VC3 rotation = unit->getRotation();

					VC3 dir = VC3(0,0,0);
					dir.x = -sinf(UNIT_ANGLE_TO_RAD(rotation.y));
					dir.z = -cosf(UNIT_ANGLE_TO_RAD(rotation.y));

					dir *= unitType->getDeathBleedOffset();

					bleedpos.x += dir.x;
					bleedpos.z += dir.z;

					bleedproj->setDirectPath(bleedpos, bleedpos, 
						unitDeathBleedBullet->getVelocity());

					ProjectileActor pa = ProjectileActor(game);
					pa.createVisualForProjectile(bleedproj);
				}
			}
		}

		// removal of destroyed units (by fading them out)
		if (unitType->doesRemoveDestroyed())
		{
			int disappearTicks = (SimpleOptions::getInt(DH_OPT_I_CORPSE_DISAPPEAR_TIME) * 1000) / GAME_TICK_MSEC;
			if (disappearTicks > 0 
				|| unitType->doesRemoveImmediately())
			{
				if (unit->getDisappearCounter() < disappearTicks
					&& !unitType->doesRemoveImmediately())
				{
					int fadeStartTick = disappearTicks - UNIT_DISAPPEAR_FADE_TICKS;
					if (fadeStartTick < 0) fadeStartTick = 0;

					// if far away from camera, 4x faster disappear... (but not faster fadeout)
					VC3 camPos = game->gameUI->getGameCamera()->getPosition();
					camPos.y = unit->getPosition().y;
					VC3 camDiff = camPos - unit->getPosition();
					if (camDiff.GetSquareLength() > 25*25
						&& unit->getDisappearCounter() < fadeStartTick)
					{
						unit->setDisappearCounter(unit->getDisappearCounter() + 4);
					} else {
						unit->setDisappearCounter(unit->getDisappearCounter() + 1);
					}

					if (unit->getDisappearCounter() >= fadeStartTick)
					{
						//if (unit->getVisualObject() != NULL)
						//{
							int fadeRange = disappearTicks - fadeStartTick;
							if (fadeRange <= 0) fadeRange = 1;
							float visFactor = 1.0f - (float)(unit->getDisappearCounter() - fadeStartTick) / fadeRange;
							//unit->getVisualObject()->setVisibilityFactor(visFactor);

							bool cannotDrop = false;

							if (unit->isSideways())
							{
								cannotDrop = true;
							}

							VC3 pos = unit->getPosition();
							if (!game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
							{
								cannotDrop = true;
							} else {
								float mapH = game->gameMap->getScaledHeightAt(pos.x, pos.z);
								if (pos.y - mapH > 0.2f)
								{
									cannotDrop = true;
								}
							}
							if (cannotDrop)
							{
								unit->setFadeVisibilityImmediately(visFactor);
							} else {
								pos.y = game->gameMap->getScaledHeightAt(pos.x, pos.z);
								pos.y -= (1.0f - visFactor) * 1.5f;

								unit->setPosition(pos);
							}
						//}
					}
				} else {
					//if (unit->getVisualObject() != NULL)
					//{
						//if (unit->getVisualObject()->getVisibilityFactor() < 1.0f)
						//{
						//	unit->getVisualObject()->setVisibilityFactor(1.0f);
						//}
					//}
					if (unit->getCurrentVisibilityFadeValue() > 0.0f)
					{
						unit->setFadeVisibilityImmediately(0.0f);
						if (unit->getVisualObject() != NULL)
						{
							unit->getVisualObject()->setVisible(false);
						}
					}

					unit->setPosition(VC3(-99999,-99999,-99999));
					unit->setVelocity(VC3(0,0,0));
					this->stopUnit(unit);
				}
			}
		}

	}



	void UnitActor::decideIdle(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		VC3 position = unit->getPosition();
    // idle...?
    if (unit->getAnimationSet() != NULL 
      && unit->getAnimationSet()->isAnimationInSet(ANIM_IDLE1))
    {
			// unit may be idle if...
			// - not directly controlled by player
			// - is at destination
			// - does not see any hostile unit
			// - does not have a target

      if (((game->gameTimer & 7) == 0 && !unit->isDirectControl())
				|| unit->getIdleRequest() != 0)
      {
        VC3 dest = unit->getFinalDestination();
				// unit idle if it has no destination, not controlled, not prone
        if ((!unit->targeting.hasTarget() && unit->getSeeUnit() == NULL
          && position.x == dest.x && position.z == dest.z
					&& unit->getSpeed() != Unit::UNIT_SPEED_CRAWL)
					|| unit->getIdleRequest() != 0)
        {
					// count the idle time...
          int idle = unit->getIdleTime() + 8 * GAME_TICK_MSEC;
          if (idle > 5000 || unit->getIdleRequest() != 0)
          {
						// has been more than 5 seconds idle
						// make the unit "idle".
						// (play some animation and freeze the unit for a while)
            unit->setIdleTime(0);
						int ianum = 0;
						if (unit->getIdleRequest())
						{
							ianum = unit->getIdleRequest() - 1;
							unit->clearIdleRequest();
						} else {
	            ianum = (game->gameRandom->nextInt() & 15);
						}
            if (ianum < 8)
            {
							int anim = ANIM_IDLE1;
							if (ianum == 1) anim = ANIM_IDLE2;
							else if (ianum == 2) anim = ANIM_IDLE3;
							else if (ianum == 3) anim = ANIM_IDLE4;
							else if (ianum == 4) anim = ANIM_IDLE5;
							else if (ianum == 5) anim = ANIM_IDLE6;
							else if (ianum == 6) anim = ANIM_IDLE7;
							else if (ianum == 7) anim = ANIM_IDLE8;
							if (unit->getAnimationSet() != NULL
								&& unit->getAnimationSet()->isAnimationInSet(anim))
							{
								animRequests->setIdleAnim = ianum + 1; 
								unit->setMoveState(Unit::UNIT_MOVE_STATE_IDLE);
								unit->setMoveStateCounter(400); // 4 secs								
							}
            }
          } else {
            unit->setIdleTime(idle);
          }
        } else {
          unit->setIdleTime(0);
        }
      }
    }
	  unit->setPosition(position);
	}



	void UnitActor::actNotYetAllowedToWalk(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		UnitType *unitType = unit->getUnitType();
		VC3 velocity = unit->getVelocity();
    if (unitType->isSticky() && unit->isOnGround() && !unit->isSideways()
			&& unit->getGamePhysicsObject() == NULL)
    {
      velocity = VC3(0,0,0);
    } else {
			if (unit->isGroundFriction())
	      frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
			else
	      frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
    }
		unit->setVelocity(velocity);
// ??????
//    if (unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
    if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_NORMAL)
		{
			unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
			animRequests->setStandAnim = true;
		}
	}


	void UnitActor::handleRechargeableWeapons(Unit *unit, bool &primaryPressed, bool &secondaryPressed)
	{
		// HACK: rechargeable weapons 

		bool rechargeablePrimaryWeapon = false;
		bool rechargeableSecondaryWeapon = false;
		int rechargeMax = 0;
		int rechargePeak = 0;
		int rechargeMin = 0;
		int rechargeSteps = 0;
		int rechargeAmmoRate = 0;
		int rechargeAmmoPeakHoldRate = 0;
		const char *rechargeSound = NULL;
		const char *rechargeEffect = NULL;
		int rechargeWeap = -1;
		bool doRechargeEffect = false;

		if (unit->getSelectedWeapon() != -1)
		{
			Weapon *wPrim = unit->getWeaponType(unit->getSelectedWeapon());
			Weapon *w = NULL;
			if (wPrim != NULL)
			{
				w = wPrim->getAttachedWeaponType();

				fb_assert(w != wPrim);

				// new: support rechargeable primary weapons
				if (w == NULL)
				{
					if (wPrim->getRechargeBulletSteps() > 0
						&& wPrim->getRechargePeakTime() > 0)
					{
						w = wPrim;
					}
				}
			}

			if (w != NULL
				&& w->getRechargeBulletSteps() > 0
				&& w->getRechargePeakTime() > 0)
			{
				if (w == wPrim)
					rechargeablePrimaryWeapon = true;
				else
					rechargeableSecondaryWeapon = true;
				rechargeMax = w->getRechargeMaxTime();
				rechargeMin = w->getRechargeMinTime();
				rechargePeak = w->getRechargePeakTime();
				rechargeSteps = w->getRechargeBulletSteps();
				rechargeSound = w->getRechargeSound();
				rechargeEffect = w->getRechargeEffect();
				rechargeAmmoRate = w->getRechargeAmmoRate();
				rechargeAmmoPeakHoldRate = w->getRechargeAmmoPeakHoldRate();

				for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
				{
					if (unit->getWeaponType(i) == w)
					{
						rechargeWeap = i;
						break;
					}
				}

				if (rechargeWeap == -1)
				{
					// did not have the attached weapon after all???
					rechargeableSecondaryWeapon = false;
					
					// this should never happen for primary weaps, but just in case...
					rechargeablePrimaryWeapon = false;
				}

			}
		}

		if (rechargeableSecondaryWeapon
			|| rechargeablePrimaryWeapon)
		{
			if (unit->isWeaponRecharging())
			{
				bool ammoOk = true;
				if (unit->getWeaponRechargeAmount() < rechargePeak)
				{
					if (rechargeAmmoRate > 0)
					{
						if ((unit->getWeaponRechargeAmount() % rechargeAmmoRate) == 0)
						{
							ammoOk = unit->useWeaponAmmo(rechargeWeap);
						}
					}
				} else {
					if (rechargeAmmoPeakHoldRate > 0)
					{
						if ((unit->getWeaponRechargeAmount() % rechargeAmmoPeakHoldRate) == 0)
						{
							ammoOk = unit->useWeaponAmmo(rechargeWeap);
						}
					}
				}

				if (rechargeablePrimaryWeapon)
					secondaryPressed = false;
				else
					primaryPressed = false;
				if ((
					(((secondaryPressed && rechargeableSecondaryWeapon) || (primaryPressed && rechargeablePrimaryWeapon))
					&& unit->getWeaponRechargeAmount() < rechargeMax)
						|| unit->getWeaponRechargeAmount() < rechargeMin)
					&& ammoOk)
				{
					// continue recharging
					if (rechargeablePrimaryWeapon)
						primaryPressed = false;
					else
						secondaryPressed = false;

					if ((game->gameTimer % (UNITACTOR_RECHARGE_DURATION/2)) == 0)
					{
						doRechargeEffect = true;
					}

					unit->setWeaponRechargeAmount(unit->getWeaponRechargeAmount() + 1);
					unit->setWeaponRechargeRange(rechargeMin, rechargePeak, rechargeSteps);

					// continue fire sound
					//if (unit->getWeaponRechargeAmount() > rechargePeak)
					//{
						int handle = unit->getWeaponLoopSoundHandle(unit->getSelectedWeapon());
						int key = unit->getWeaponLoopSoundKey(unit->getSelectedWeapon());
						if (handle != -1)
						{
							VC3 pos = unit->getPosition();
							//handle = game->gameUI->parseSoundFromDefinitionString(rechargeSound, pos.x, pos.y, pos.z, &looped, &handle, &key, true, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_NORMAL);
							game->gameUI->getSoundLooper()->continueLoopedSound(handle, key, 100);
							game->gameUI->getSoundLooper()->setSoundPosition(handle, key, pos.x, pos.y, pos.z);
						}
					//}

				} else {
					// fire recharged / discharge

					if (unit->getWeaponRechargeAmount() < rechargeMax
						&& ammoOk)
					{
						// fire!
						if (rechargeablePrimaryWeapon)
							primaryPressed = true;
						else
							secondaryPressed = true;

						// stop recharge sound immediately wo/ end loop sound
						int handle = unit->getWeaponLoopSoundHandle(unit->getSelectedWeapon());
						int key = unit->getWeaponLoopSoundKey(unit->getSelectedWeapon());
						if (handle != -1)
						{
							game->gameUI->getSoundLooper()->stopLoopedSound(handle, key, true);
							unit->setWeaponSoundHandle(unit->getSelectedWeapon(), -1);
						}

					} else {
						// discharge (peak hold overtime or out of ammo)
						if (rechargeablePrimaryWeapon)
							primaryPressed = false;
						else
							secondaryPressed = false;

						// some delay until re-firing possible..
						if (unit->getSelectedWeapon() != -1)
						{
							// NOTE, some magic numbers here! (about 1.0 sec)
							if (unit->getFireReloadDelay(unit->getSelectedWeapon()) < GAME_TICKS_PER_SECOND)
								unit->setFireReloadDelay(unit->getSelectedWeapon(), GAME_TICKS_PER_SECOND);
							if (unit->getFireReloadDelay(rechargeWeap) < GAME_TICKS_PER_SECOND)
								unit->setFireReloadDelay(rechargeWeap, GAME_TICKS_PER_SECOND);
						}

						// stop recharge sound slowly w/ end loop sound
						int handle = unit->getWeaponLoopSoundHandle(unit->getSelectedWeapon());
						int key = unit->getWeaponLoopSoundKey(unit->getSelectedWeapon());
						if (handle != -1)
						{
							game->gameUI->getSoundLooper()->stopLoopedSound(handle, key, false);
							unit->setWeaponSoundHandle(unit->getSelectedWeapon(), -1);
						}
					}
					unit->setWeaponRecharging(false);
				}
			} else {
				if ((secondaryPressed && rechargeableSecondaryWeapon) || (primaryPressed && rechargeablePrimaryWeapon))
				{
					if (unit->getWeaponAmmoAmount(rechargeWeap) > 0
						&& unit->getFireReloadDelay(rechargeWeap) == 0)
					{
						// start recharging
						unit->setWeaponRecharging(true);
						unit->setWeaponRechargeAmount(1);
						if (rechargeSound != NULL)
						{
							makeFireSound(unit, unit->getSelectedWeapon(), rechargeSound);
						}
						primaryPressed = false;
						secondaryPressed = false;

						doRechargeEffect = true;
					} else {
						// new: try to fix problems with recharging primary weapons trying to shoot when no ammo
						primaryPressed = false;
						secondaryPressed = false;
					}
				}
			}
		}

		if (doRechargeEffect)
		{
			if (rechargeEffect != NULL)
			{
				if (unit->isMuzzleflashVisible())
				{
					unit->setMuzzleflashVisualEffect(NULL, 0);
				}

				Weapon *w = unit->getWeaponType(rechargeWeap);
				int weap = rechargeWeap;

				static int last_recharge_ve_id = -1;
				static std::string last_recharge_ve_name = std::string();
				if (strcmp(last_recharge_ve_name.c_str(), rechargeEffect) != 0)
				{
					last_recharge_ve_id = VisualEffectManager::getVisualEffectIdByName(rechargeEffect);
					last_recharge_ve_name = std::string(rechargeEffect);
				}

				int visId = last_recharge_ve_id;

				float angle = unit->getFlashlightDirection();

				VC3 rot = unit->getRotation();
				rot.y += angle;
				if (rot.y >= 360.0f) rot.y -= 360.0f;
				if (rot.y < 0.0f) rot.y += 360.0f;

				VC3 weaponPosition(unit->getPosition());
				if (w->doesFireFromWeaponBarrel())
				{
					VisualObject *unitVisual = unit->getVisualObject();
					IStorm3D_Model *unitModel = 0;
					if(unitVisual)
					{
						if(unitVisual)
							unitModel = unitVisual->getStormModel();

						if(unitModel)
						{
							// custom helper
							if(unit->getWeaponType(weap)->getCustomWeaponBarrelHelper())
							{
								bool helperFound = util::getHelperPosition(unitModel, unit->getWeaponType(weap)->getCustomWeaponBarrelHelper(), weaponPosition);
								if (!helperFound)
								{
									Logger::getInstance()->warning("UnitActor::handleRechargeableWeapons - Custom weapon barrel helper not found.");
								}
							}
							else
							{
								std::string barrelName = "WeaponBarrel";
								if (unit->getRotatedWeaponBarrel(weap) >= 2)
									barrelName += int2str(unit->getRotatedWeaponBarrel(weap));

								std::string modelHelperName = std::string("HELPER_MODEL_") + barrelName;
								bool helperFound = util::getHelperPosition(unitModel, modelHelperName.c_str(), weaponPosition);
								if (!helperFound)
								{
									std::string boneHelperName = std::string("HELPER_BONE_") + barrelName;
									helperFound = util::getHelperPosition(unitModel, boneHelperName.c_str(), weaponPosition);
									if (!helperFound)
									{
										Logger::getInstance()->warning("UnitActor::handleRechargeableWeapons - Weapon barrel helper not found.");
									}
								}
							}
						}
					}
				}

				VisualEffect *ve = game->gameUI->getVisualEffectManager()->createNewVisualEffect(
					visId, NULL, unit, weaponPosition, weaponPosition,
					rot, unit->getVelocity(), game, 1);

				// HACK: magic number here..
				unit->setMuzzleflashVisualEffect(ve, UNITACTOR_RECHARGE_DURATION);
			}
		}
	}


	void UnitActor::decideBasedOnAIDirectControl(Unit *unit,
		bool *doMove, bool *doRotation, bool *doForwardMove, bool *doBackMove, 
		bool *doLeftMove, bool *doRightMove, 
		bool *doLeftRotation, bool *doRightRotation, 
		bool *doFire, float *rotationAngle)
	{


		// ---------------------------------------------------------------------------------------------
		// TODO: refactor the decideBasedOnLocalPlayerDirectControl so that decideBasedOnAIDirectControl 
		// can easily do the same stuff
		// ---------------------------------------------------------------------------------------------


		IAIDirectControl *dc = unit->getAIDirectControl();
		if (dc != NULL)
		{
			AIDirectControlActions actions;
			dc->doDirectControls(actions);

			// either forward or backward or none, but not both.
			if (actions.directControlOn[DIRECT_CTRL_FORWARD])
			{
				*doMove = true;
				*doForwardMove = true;
			}
			else if (actions.directControlOn[DIRECT_CTRL_BACKWARD])
			{
				*doMove = true;
				*doBackMove = true;
			}

			// either left or right or none, but not both.
			if (actions.directControlOn[DIRECT_CTRL_LEFT])
			{
				*doMove = true;
				*doLeftMove = true;
			}
			else if (actions.directControlOn[DIRECT_CTRL_RIGHT])
			{
				*doMove = true;
				*doRightMove = true;
			}

			// jumps?
			if (actions.directControlOn[DIRECT_CTRL_SPECIAL_MOVE])
			{
				// TODO: ... (refactor from local player...)
			}

			// firing 
			if (actions.directControlOn[DIRECT_CTRL_FIRE])
			{
				*doFire = true;
			}

			// (handle secondary fire, etc. somehow sensibly?)
			if (actions.directControlOn[DIRECT_CTRL_FIRE_SECONDARY])
			{
				// select secondary weapon first, and fire
				// TODO: (refactor from local player...)

				//*doFire = true;
			}

			unit->targeting.setAimingPosition(actions.aimPosition);
		}

	}


	void UnitActor::decideBasedOnLocalPlayerDirectControl(Unit *unit,
		bool *doMove, bool *doRotation, bool *doForwardMove, bool *doBackMove, 
		bool *doLeftMove, bool *doRightMove, 
		bool *doLeftRotation, bool *doRightRotation, 
		bool *doFire, float *rotationAngle)
	{


		// ---------------------------------------------------------------------------------------------
		// TODO: refactor the decideBasedOnLocalPlayerDirectControl so that decideBasedOnAIDirectControl 
		// can easily do the same stuff
		// ---------------------------------------------------------------------------------------------


		UnitType *unitType = unit->getUnitType();

		VC3 rotation = unit->getRotation();
		float turnspeed = unitType->getTurning() / GAME_TICKS_PER_SECOND;
		if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
		{
			turnspeed *= 0.5f;
		}
		float rotacc = turnspeed * unitType->getTurningAccuracy();

    if (game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FORWARD, unit))
    {
      *doForwardMove = true;
      *doMove = true;
    }
    if (game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_BACKWARD, unit))
    {
      *doBackMove = true;
      *doMove = true;
    }

		// this if fixes the move+rotate combination when in free camera mode (non-aim-upward)
		// TODO: fix move+rotate combo for aim upward too
		if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
		{
			if (game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_TURN_LEFT, unit))
			{
				*doRotation = true;
				*doLeftRotation = true;
				*rotationAngle = rotation.y - rotacc * 1.1f;
				if (*rotationAngle < 0) *rotationAngle += 360;
			}
			if (game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_TURN_RIGHT, unit))
			{
				*doRotation = true;
				*doRightRotation = true;
				*rotationAngle = rotation.y + rotacc * 1.1f;
				if (*rotationAngle >= 360) *rotationAngle -= 360;
			}
		}

    if (game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_LEFT, unit))
    {
      *doLeftMove = true;
      *doMove = true;
    }
    if (game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_RIGHT, unit))
    {
      *doRightMove = true;
      *doMove = true;
    }

		// stop doing any further jumping/firing if pending weap change
		bool pendingWeapChange = false;
		if (unit->getPendingWeaponChangeRequest() != -1)
			pendingWeapChange = true;

		// HACK: continuous fire hack... (flamethrower extreme heat)
		// (just doing this here, cos' it's easiest that weay.)
		// FIXME: will not work properly in every situation (when jumping and still
		// pressing fire? etc.)
		if (game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FIRE, unit))
		{
			unit->increaseContinuousFireTime();
		} else {
			unit->clearContinuousFireTime();
		}

    if (game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_SPECIAL_MOVE, unit))
    {
			if (unit->getJumpCounter() == 0 && *doMove
				&& unit->getKeepFiringCount() == 0)
			{
				bool slowWeap = false;
				if (unit->getSelectedWeapon() != -1
					&& unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
				{
					if (unit->getWeaponType(unit->getSelectedWeapon())->doesFiringRequireWalk())
					{
						slowWeap = true;
					}
				}
				if (!slowWeap && !pendingWeapChange 
					&& unit->getJumpNotAllowedTime() == 0)
				{
					this->doSpecialMove(unit, *doForwardMove, *doBackMove,
						*doLeftMove, *doRightMove, DIRECTCTRL_JUMP_TIME / GAME_TICK_MSEC);
				}
			}
    }

		bool primaryPressed = game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FIRE, unit);
		bool secondaryPressed = game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FIRE_SECONDARY, unit);

		game->gameUI->setFiresPreviously(unit, primaryPressed, secondaryPressed);

		handleRechargeableWeapons(unit, primaryPressed, secondaryPressed);

		if (unit->getSelectedWeapon() != -1
			&& unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
		{
			if (secondaryPressed)
			{
				// HACK: if shotgun+secondary && ammo in clip < 2, fire primary instead
				if (unit->getWeaponType(unit->getSelectedWeapon())->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_Shotg"))
				{
					if (unit->getWeaponAmmoInClip(unit->getSelectedWeapon()) < 2)
					{
						primaryPressed = true;
						secondaryPressed = false;
					}
				}
				// HACK: if rocket+secondary && ammo in total < 4, fire primary instead
				if (unit->getWeaponType(unit->getSelectedWeapon())->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_Rocket"))
				{
					if (unit->getWeaponAmmoAmount(unit->getSelectedWeapon()) < 4)
					{
						primaryPressed = true;
						secondaryPressed = false;
					}
				}
			}

			// auto reload timer reset...
			if (unit->getWeaponType(unit->getSelectedWeapon())->doesFireByClick())
			{
				if (primaryPressed || secondaryPressed)
				{
					unit->setClipEmptyTime(0);
				}
			}

			// handle clip empty sounds...
			if ((primaryPressed || secondaryPressed) 
				&& ((unit->getWeaponAmmoInClip(unit->getSelectedWeapon()) == 0
				&& unit->getWeaponType(unit->getSelectedWeapon())->getClipSize() > 0)
				|| (unit->getWeaponAmmoAmount(unit->getSelectedWeapon()) == 0) || !unit->isWeaponOperable(unit->getSelectedWeapon())))
			{
				if (!unit->isClipEmptySoundDone())
				{
					const char *soundfile = unit->getWeaponType(unit->getSelectedWeapon())->getClipEmptySound();
					if (soundfile != NULL)
					{
						VC3 pos = unit->getPosition();
						game->gameUI->playSoundEffect(soundfile, pos.x, pos.y, pos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_NORMAL);
					}
					unit->setClipEmptySoundDone(true);

					if(!unit->isWeaponOperable(unit->getSelectedWeapon()))
					{
						Weapon *w = unit->getWeaponType(unit->getSelectedWeapon());
						std::string locale = "gui_weapon_" + std::string(w->getPartTypeIdString()) + "_inoperable_message";
						std::transform(locale.begin(),locale.end(),locale.begin(),(int(*)(int))tolower);

						const char *message = NULL;
						if( ::game::DHLocaleManager::getInstance()->getString( ::game::DHLocaleManager::BANK_GUI, locale.c_str(), &message) )
						{
							game->gameUI->getCombatWindow(0)->showHintMessage(message);
						}
					}
				}
			}

			if (!primaryPressed && !secondaryPressed)
			{
				unit->setClipEmptySoundDone(false);
			}

		}

		if (unit->getJumpCounter() == 0
			&& (!pendingWeapChange || unit->getKeepFiringCount() > 0))
		{
			// --- fire (primary) ---
			if (primaryPressed
				|| unit->getKeepFiringCount() > 0)
			{
				// HACK: attempt to shoot immediately in case of shotgun..?
				if (unit->getSelectedWeapon() != -1
					&& unit->getWeaponType(unit->getSelectedWeapon()) != NULL
					&& unit->getWeaponType(unit->getSelectedWeapon())->isSingleReloading())
				{
					if (//unit->doesKeepReloading()
						unit->isClipReloading()
						&& unit->getFireReloadDelay(unit->getSelectedWeapon()) > 0
						&& unit->getWeaponAmmoInClip(unit->getSelectedWeapon()) > 1)
					{
						unit->setFireReloadDelay(unit->getSelectedWeapon(), 1);
						int inclip = unit->getWeaponAmmoInClip(unit->getSelectedWeapon());
						inclip--;
						if (inclip < 0)
							inclip = 0;
						// THIS IS TOTAL BULLSHIT...
						unit->setWeaponAmmoInClip(unit->getSelectedWeapon(), inclip);
						unit->setKeepReloading(false);
					}
				} else {
					unit->setKeepReloading(false);
				}

				if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
				{
					// FIXME: some weird third person view requirement here too...? 
					// to keep it backward compatible, not removed, but rather just accepting sideways flag too --jpk
					if (game->gameUI->isThirdPersonView(unit->getOwner())
						|| unit->isSideways())
					{
						if (game->gameUI->isControlModeDirect(unit->getOwner()))
						{
							// (change weap to normal only if not burst, that is, keep firing count...)
							if (unit->getKeepFiringCount() == 0)
							{
								bool wasAnyWeaponActivated = false;
								for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
								{
									if (w == unit->getSelectedWeapon())
									{
										unit->setWeaponActive(w, true);
										wasAnyWeaponActivated = true;
									} else {
										unit->setWeaponActive(w, false);
									}
								}
								// HACK: if no weapons active, but some existed, and
								// this is a vehicle, please set _basic_ weapons on...
								if (unit->getUnitType()->isVehicle())
								{
									if (!wasAnyWeaponActivated)
									{
										unit->setWeaponsActiveByFiretype(Unit::FireTypeBasic);
									}
								}
							}
							*doFire = true;
						}
					} else {
						*doFire = true;
					}
				} else {
					*doFire = true;
				}
			}
			// --- fire secondary ---
			if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
			{
				if (secondaryPressed)
				{
					// FIXME: some weird third person view requirement here too...? 
					// to keep it backward compatible, not removed, but rather just accepting sideways flag too --jpk
					if (game->gameUI->isThirdPersonView(unit->getOwner())
						|| unit->isSideways())
					{
						if (game->gameUI->isControlModeDirect(unit->getOwner()))
						{
							bool wasAnyWeaponActivated = false;
							int secondaryW = -1;
							Weapon *secondaryWType = NULL;
							Weapon *wType = unit->getWeaponType(unit->getSelectedWeapon());
							if (wType != NULL)
							{
								secondaryWType = wType->getAttachedWeaponType();
							}
							// first check that there actually is the attachment weapon...
							for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
							{
								if (secondaryWType != NULL && unit->getWeaponType(w)
									&& unit->getWeaponType(w)->getPartTypeId() == secondaryWType->getPartTypeId())
								{
									secondaryW = w;
									wasAnyWeaponActivated = true;
									break;
								}
							}

							// then activate/deactivate weapons...
							if (wasAnyWeaponActivated)
							{
								// HACK: attempt to shoot immediately in case of shotgun..?
								if (secondaryWType->isSingleReloading())
								{
									if (//unit->doesKeepReloading()
										unit->isClipReloading()
										&& unit->getFireReloadDelay(secondaryW) > 0
										&& unit->getWeaponAmmoInClip(secondaryW) > 1)
									{
										unit->setFireReloadDelay(secondaryW, 1);
										int inclip = unit->getWeaponAmmoInClip(secondaryW);
										inclip--;
										if (inclip < 0)
											inclip = 0;
										// THIS IS TOTAL BULLSHIT...
										unit->setWeaponAmmoInClip(secondaryW, inclip);
										unit->setKeepReloading(false);
									}
								} else {
									unit->setKeepReloading(false);
								}

								for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
								{
									if (secondaryWType != NULL && unit->getWeaponType(w)
										&& unit->getWeaponType(w)->getPartTypeId() == secondaryWType->getPartTypeId())
									{
										unit->setWeaponActive(w, true);
									} else {
										unit->setWeaponActive(w, false);
									}
								}

								*doFire = true;

							} else {
								if (wType != NULL)
								{
									// NEW: if no secondary available (not yet upgraded),
									// don't use any weapon...
									for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
									{
										// don't change state of primary weapon
										if (unit->getWeaponType(w) &&
											unit->getWeaponType(w)->getPartTypeId() == wType->getPartTypeId())
										{
											// nop
										}
										else if (unit->getWeaponType(w) != NULL)
										{
											unit->setWeaponActive(w, false);
										}
									}
								}
							}
							// HACK: if no weapons active, but some existed, and
							// this is a vehicle, please set _heavy_ weapons on...
							if (unit->getUnitType()->isVehicle())
							{
								if (!wasAnyWeaponActivated)
								{
									unit->setWeaponsActiveByFiretype(Unit::FireTypeHeavy);
								}
							}
							//*doFire = true;
						}
					} else {
						//*doFire = true;
					}
					//*doFire = true;
				}
			}
			// --- fire grenade ---
			if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
			{
				if (game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FIRE_GRENADE, unit))
				{
					// FIXME: some weird third person view requirement here too...? 
					// to keep it backward compatible, not removed, but rather just accepting sideways flag too --jpk
					if (game->gameUI->isThirdPersonView(unit->getOwner())
						|| unit->isSideways())
					{
						if (game->gameUI->isControlModeDirect(unit->getOwner()))
						{
							bool wasAnyWeaponActivated = false;
							for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
							{
								//if (unit->getWeaponType(w) != NULL
								//	&& unit->getWeaponType(w)->isThrowable())
								if (w == unit->getSelectedSecondaryWeapon())
								{
									unit->setWeaponActive(w, true);
									wasAnyWeaponActivated = true;
								} else {
									unit->setWeaponActive(w, false);
								}
							}
							*doFire = true;
						}
					} else {
						*doFire = true;
					}
					*doFire = true;
				}
			}
			// ---
		}
	}



	void UnitActor::decideTurnAndWalk(Unit *unit, 
		bool *doMove, bool *doRotation, float *rotationAngle)
	{
		VC3 waypoint = unit->getWaypoint();
		VC3 position = unit->getPosition();
		VC3 rotation = unit->getRotation();

		UnitType *unitType = unit->getUnitType();

		float pathAcc = unitType->getPathAccuracy();

		// at the end of the path, be much more accurate
		if (unit->isAtPathEnd())
		{
			pathAcc *= 0.6f;
			// and if we're heading to the right direction (no need to turn),
			// make it even more accurate... (after 1/10th second prediction)
			VC3 velocity = unit->getVelocity();
			VC3 predictPosition = position + (velocity * 10);
			if (fabs(waypoint.x-predictPosition.x) < pathAcc*0.5f
				&& fabs(waypoint.z-predictPosition.z) < pathAcc*0.5f)
			{
				// halve pathAcc _again_ (make more accurate again)
				pathAcc *= 0.5f;
			}
		} else {
			// hack, sprinting units are not so accurate...
			if (unit->getSpeed() == Unit::UNIT_SPEED_SPRINT)
				pathAcc *= 1.4f;
		}

		// hack, if jumping, even more inaccurate :)
		if (unit->getSpeed() == Unit::UNIT_SPEED_JUMP)
			pathAcc *= 2.0f;

		if (fabs(waypoint.x-position.x) > pathAcc
			|| fabs(waypoint.z-position.z) > pathAcc)
		{
			// we are not at waypoint, rotate and run there...
			//VC3 finalDestination = unit->getFinalDestination();
			//if (position.x != finalDestination.x || position.z != finalDestination.z)
			//{
				*doMove = true;
				*doRotation = true;
				*rotationAngle = unit->getAngleTo(waypoint);
			//}
		} else {
			// rotate to given angle
			if (unit->isTurning())
			{
				*rotationAngle = unit->getTurnToAngle();
				if (rotationAngle != 0)
				{
					*doRotation = true;
				//} else {
					//unit->stopTurning();
				}
			}
			// HACK: 
			// special case, rushers
			if (((unit->isRushing() && unit->targeting.hasTarget()) || unit->isFollowPlayer())
				&& (unit->getSpeed() == Unit::UNIT_SPEED_FAST
				|| unit->getSpeed() == Unit::UNIT_SPEED_SPRINT)
				&& unit->getRushDistance() < UNITACTOR_RUSH_DISTANCE
				&& unit->getRushDistance() > 0.0f)
			{
				if (unit->isFollowPlayer())
				{
					// HACK: ...
					// TODO: proper impl...
					Unit *player1 = game->units->getUnitByIdString("player1");
					if (player1 != NULL)
					{
						VC3 plpos = player1->getPosition();
						VC3 pldistvec = plpos - position;
						float pldistSq = pldistvec.GetSquareLength();
						if (pldistSq > 3*3)
						{
							*rotationAngle = unit->getAngleTo(player1->getPosition());
							*doRotation = true;
						}
					}
				} else {
					if (unit->targeting.getTargetUnit() != NULL)
					{
						*rotationAngle = unit->getAngleTo(unit->targeting.getTargetUnit()->getPosition());
					} else {
						*rotationAngle = unit->getAngleTo(unit->targeting.getTargetPosition());
					}
					*doRotation = true;
				}
			}
		}

		//unit->setWaypoint(waypoint);
		//unit->setPosition(position);
	}


	void UnitActor::actJump(Unit *unit, bool *doMove, bool *doRotation, 
		bool *doForwardMove, bool *doBackMove, bool *doLeftMove, bool *doRightMove)
	{
		// special case, jump
		if (unit->getSpeed() == Unit::UNIT_SPEED_JUMP)
		{
			*doMove = true;
			if (!unit->getUnitType()->isJumpTurnable()
				&& (unit->getJumpTotalTime() - unit->getJumpCounter()) > unit->getUnitType()->getTurnTimeAtJumpStart() / GAME_TICK_MSEC)
			{
				*doRotation = false;
			}
			if (unit->isSideways())
			{
				if (!unit->getUnitType()->isSidewaysJumpMovable())
					unit->getCameraRelativeJumpDirections(doForwardMove, doBackMove, doLeftMove, doRightMove);
			} else {
				unit->getCameraRelativeJumpDirections(doForwardMove, doBackMove, doLeftMove, doRightMove);
			}

			// HACK: clear any not-yet-allowed to walk time...
			unit->setWalkDelay(0);
		}   
		
		int jnotall = unit->getJumpNotAllowedTime();
		if (jnotall > 0)
		{
			jnotall--;
			unit->setJumpNotAllowedTime(jnotall);
		}

		int jc = unit->getJumpCounter();
		if (jc > 0)
		{
			//if ((jc & 15) == 0)
			if ((jc & 7) == 0)
			{
				// HACK: keep destination on target
				if ((unit->getUnitType()->isJumpTurnable()
					|| (unit->getJumpTotalTime() - unit->getJumpCounter()) < unit->getUnitType()->getTurnTimeAtJumpStart() / GAME_TICK_MSEC)
					&& unit->targeting.hasTarget()
					&& unit->targeting.getTargetUnit() != NULL)
				{
					VC3 tpos = unit->targeting.getTargetUnit()->getPosition();
					unit->setWaypoint(tpos);
					unit->setFinalDestination(tpos);
				}
			}

			// firing gets inaccurate when jumping...
			// (bad idea, bad for gameplay, disabled)
			/*
			if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
			{
				if (unit->getSelectedWeapon() != -1)
				{
					Weapon *w = (Weapon *)unit->getWeaponType(unit->getSelectedWeapon());
					if (w != NULL)
					{
						float firingSpread = unit->getFiringSpreadFactor();
						firingSpread += 0.03f * GAME_TICK_MSEC;
						if (firingSpread > w->getMaxSpread())
							firingSpread = w->getMaxSpread();
						if (firingSpread < w->getMinSpread())
							firingSpread = w->getMinSpread();
						unit->setFiringSpreadFactor(firingSpread);
					}
				}
			}
			*/

			// make jump sound
			if (unit->getJumpTotalTime() - jc == ROLL_JUMP_ALLOW_STOP_TIME / GAME_TICK_MSEC)
			{
				// the sound cannot be made earlier, as the roll jump 
				// may still be cancelled
				// TODO: non-roll jumps could still make the sound earlier!

				VC3 pos = unit->getPosition();
				char *jsound = NULL;
				if (*doForwardMove)
				{
					jsound = unit->getUnitType()->getJumpSoundForward();
				}
				if (*doBackMove)
				{
					jsound = unit->getUnitType()->getJumpSoundBackward();
				}
				if ((*doLeftMove || *doRightMove) && !*doForwardMove && !*doBackMove)
				{
					jsound = unit->getUnitType()->getJumpSoundSideways();
				}
				if (jsound != NULL)
					game->gameUI->playSoundEffect(jsound, pos.x, pos.y, pos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_LOW);
			}

			jc--;
			unit->setJumpCounter(jc);
			if (jc == 0)
			{
				// TODO: should maybe restore the speed before the jump?
				// (should save it somewhere first though ;)
				unit->restoreDefaultSpeed();
				// TODO: check that this works with 0 delay (walk animations
				// get properly set up without walk delay?)
				unit->setWalkDelay(unit->getUnitType()->getJumpLandDelay() / GAME_TICK_MSEC);
			}
		}
	}


	void UnitActor::actTurn(Unit *unit, UnitActAnimationRequests *animRequests,
		bool doRotation, float rotationAngle, bool doMove, bool *rotated)
	{
		VC3 position = unit->getPosition();
		VC3 velocity = unit->getVelocity();
		VC3 rotation = unit->getRotation();

		UnitType *unitType = unit->getUnitType();

		float turnspeed = unitType->getTurning() / GAME_TICKS_PER_SECOND;
		if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
		{
			if (unit->isDirectControl())
				turnspeed *= 0.5f;
		}

		if (unit->getJumpCounter() > 0)
		{
			if ((unit->getJumpTotalTime() - unit->getJumpCounter()) < unit->getUnitType()->getTurnTimeAtJumpStart() / GAME_TICK_MSEC)
			{
				turnspeed = unitType->getTurningAtJumpStart() / GAME_TICKS_PER_SECOND;
			} else {
				turnspeed *= 0.2f;
			}
		}

		// HACK: sweeping fire makes turning impossible
		if (unit->isAnyWeaponFiring()
			&& unit->targeting.hasSweepTargetPosition())
		{
			//doRotation = false;
			turnspeed *= 0.2f;
		}

		float rotacc = turnspeed * unitType->getTurningAccuracy();

		if (unitType->isVehicle() && !unitType->isFlying())
		{
			if (game->gameMap->isWellInScaledBoundaries(position.x, position.z))
			{
				int ox = game->gameMap->scaledToObstacleX(position.x);
				int oy = game->gameMap->scaledToObstacleY(position.z);
				int coverDist = game->gameMap->getCoverMap()->getDistanceToNearestCover(ox, oy);
				if (coverDist < 10)
				{
					// 33% - 100% turning speed. (turn slower in NO obstacles near)
					if (!unit->isDirectControl())
						turnspeed *= ((float)(15 - coverDist) / 15.0f);
				}
			}
		}

		if (doRotation)
		{
			// rotate to the angle...
			float rotSpeed = 0;

			if (unitType->isVehicle() && !unitType->isFlying())
			{
				// vehicles rotationspeed depends on how much they need to rotate
				// in total. bigger turn, faster rotation speed, smaller turn, 
				// less rotation speed
				float factSpeed = util::AngleRotationCalculator::getFactoredRotationForAngles(rotation.y, rotationAngle, rotacc);
				if (factSpeed != 0)
				{
					if (factSpeed > 0)
					{
						if (factSpeed < 5) factSpeed = 5;
						if (factSpeed > 90) factSpeed = 90;
					} else {
						if (factSpeed > -5) factSpeed = -5;
						if (factSpeed < -90) factSpeed = -90;
					}
					factSpeed /= 90.0f;
				}
				rotSpeed = turnspeed * factSpeed;
			} else {
				rotSpeed = turnspeed * util::AngleRotationCalculator::getRotationForAngles(rotation.y, rotationAngle, rotacc);
			}

			// if airborne, don't rotate (except for flying/hovering units)
			if (!unit->isOnGround() && (!unitType->isFlying()
				|| unitType->getHover() == 0.0f))
			{
				// NOTICE: may possibly mix up the rotation set for unit..
				// (see below, stops turning if rotSpeed == 0)
				rotSpeed = 0;
			}

			//if (unit->getSpeed() == Unit::UNIT_SPEED_SLOW
			//	&& !doMove)
			//	rotation.y += rotSpeed / 2;
			//else

			if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
				rotation.y += rotSpeed / 3;
			else
				rotation.y += rotSpeed;
			
			if (rotation.y < 0) rotation.y += 360;
			if (rotation.y >= 360) rotation.y -= 360;

			if (rotSpeed != 0)
			{
				*rotated = true;
				if (!doMove)
					animRequests->setMoveAnim = true;
				unit->setTurnedTime(unit->getTurnedTime() + 1);
				if (rotSpeed < 0)
				{
					unit->setLastRotationDirection(-1);
				} else {
					unit->setLastRotationDirection(1);
				}
			} else {
				if (!doMove)
				{
					unit->stopTurning();
					animRequests->setStandAnim = true;
				}
				unit->setTurnedTime(0);
				unit->setLastRotationDirection(0);
			}
		} else {
			unit->setTurnedTime(0);
			unit->setLastRotationDirection(0);
		}

// AOV
/*
if (!unit->isDirectControl())
{
	if (rotationAngle < 180)
	{
		unit->setTurnToAngle(90);
		rotation.y = 90;
	} else {
		unit->setTurnToAngle(270);
		rotation.y = 270;
	}
}
*/

		unit->setPosition(position);
		unit->setVelocity(velocity);
		unit->setRotation(rotation.x, rotation.y, rotation.z);
	}



	void UnitActor::actWalk(Unit *unit, UnitActAnimationRequests *animRequests,
		bool doMove, bool doForwardMove, bool doBackMove, 
		bool doLeftMove, bool doRightMove, bool rotated, bool *accelerated)
	{
		VC3 position = unit->getPosition();
		VC3 velocity = unit->getVelocity();
		VC3 rotation = unit->getRotation();

		UnitType *unitType = unit->getUnitType();

		if (doMove)
		{
			// move forward (accelerate)
			if (unitType->isSticky())
			{
				if (unit->isOnGround() || (unit->isSideways() && unitType->isSidewaysJumpAllowedInAir()))
				{
					if (!rotated || unit->isDirectControl()
						|| fabs(velocity.x) > 0.01 || fabs(velocity.z) > 0.01)
					{
						float angle = rotation.y;
						if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
						{
							if (unit->isDirectControl())
//								&& game->gameUI->isThirdPersonView(unit->getOwner()))
							{
								angle = 270-game->gameUI->getGameCamera()->getAngleY();
							}
						}

						// walking direction based on forward / backward / strafing left/right
						if (doBackMove)
						{
							angle += 180;
							if (doLeftMove)
								angle += 45;
							if (doRightMove)
								angle -= 45;
						} else {
							if (doForwardMove)
							{
								if (doLeftMove)
									angle += 270+45;
								if (doRightMove)
									angle += 45;
							} else {
								if (doLeftMove)
									angle += 270;
								if (doRightMove && !doLeftMove)
									angle += 90;
							}
						}
						
						// mech always walks where the legs point
						if(unit->getUnitType()->hasMechControls())
						{
							angle = rotation.y;
							if(unit->moveInReverseDir)
							  angle += 180;
						}

						while (angle < 0) angle += 360;
						while (angle >= 360) angle -= 360;
						float accel = unitType->getAcceleration() / GAME_TICKS_PER_SECOND;
						accel *= unit->getCustomTimeFactor();

						if (!unit->isSideways())
						{
							// TODO: non-sideways unit might really use the jumpAcceleration as well, 
							// but that would require some sensible default value to be backward compatible...
							velocity.x += -accel * sinf(UNIT_ANGLE_TO_RAD(angle));
							velocity.z += -accel * cosf(UNIT_ANGLE_TO_RAD(angle));
						} else {


							if (unit->getSpeed() == Unit::UNIT_SPEED_JUMP)
							{
								accel = unitType->getJumpAcceleration() / GAME_TICKS_PER_SECOND;
								float acceldir = -accel * sinf(UNIT_ANGLE_TO_RAD(angle));
								float velx = unit->getVelocity().x;
								if (acceldir < 0)
								{
									if (velx < -unitType->getJumpAccelerationSpeedLimit())
										accel = 0;
								} else {
									if (velx > unitType->getJumpAccelerationSpeedLimit())
										accel = 0;
								}
							}

							if (unit->getUnitType()->getBackwardMovementFactor() == 0.0f)
							{
								if (doLeftMove)
								{
									if ((unit->getPosition() - getAimTargetPosition(unit)).x < 0.0f)
									{
										accel = 0.0f;
										if (unit->isGroundFriction())
											frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
										else
											frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
										animRequests->setStandAnim = true;
									}
								}
								if (doRightMove)
								{
									if ((unit->getPosition() - getAimTargetPosition(unit)).x > 0.0f)
									{
										accel = 0.0f;
										if (unit->isGroundFriction())
											frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
										else
											frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
										animRequests->setStandAnim = true;
									}
								}
							}

							velocity.x += -accel * sinf(UNIT_ANGLE_TO_RAD(angle));
							if (doForwardMove)
							{
								// TODO: this would need a better specific onsideground flag!
								if (((unit->isGroundFriction() 
									&& unit->getSpeed() != Unit::UNIT_SPEED_JUMP)
									|| unitType->isSidewaysJumpAllowedInAir())
									&& unit->getJumpNotAllowedTime() == 0
)
								{
									// TODO: sidejumpspeed conf
									velocity.z = unitType->getJumpSpeed() / GAME_TICKS_PER_SECOND;
									unit->setSpeed(Unit::UNIT_SPEED_JUMP);
									// HACK: ...
									// WARNING: unsafe cast!
									UnitLevelAI *ai = (UnitLevelAI *)unit->getAI();
									ai->setRunContinueJumpEvent();
									// HACK: a hacky solution, expecting script to handle these things...
									unit->setJumpCounter(100000 / GAME_TICKS_PER_SECOND);
									unit->setJumpTotalTime(100000 / GAME_TICKS_PER_SECOND);
									//unit->setJumpNotAllowedTime(100000 / GAME_TICKS_PER_SECOND);
									unit->setJumpAnim(ANIM_NONE);
								}
							}
						}

#ifdef LEGACY_FILES
						if (true)
#else
						if (accel != 0.0f)
#endif
						{
							animRequests->setMoveAnim = true;
							*accelerated = true;
						}
					} else {
						animRequests->setMoveAnim = true;
					}

					float maxspeed = 0;
					if (unit->getSpeed() == Unit::UNIT_SPEED_FAST)
					{
						maxspeed = unitType->getMaxSpeed() / GAME_TICKS_PER_SECOND;
					}
					else if (unit->getSpeed() == Unit::UNIT_SPEED_SLOW)
					{
						maxspeed = unitType->getSlowSpeed() / GAME_TICKS_PER_SECOND;
					}
					else if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
					{
						maxspeed = unitType->getCrawlSpeed() / GAME_TICKS_PER_SECOND;
					}
					else if (unit->getSpeed() == Unit::UNIT_SPEED_SPRINT)
					{
						if (unit->isAtPathEnd())
						{
							// at the very end of the path, don't sprint.
							maxspeed = unitType->getMaxSpeed() / GAME_TICKS_PER_SECOND;
						} else {
							maxspeed = unitType->getSprintSpeed() / GAME_TICKS_PER_SECOND;
						}
						// HACK: energy usage at sprint disabled
						/*
						if (unit->getEnergy() > 5)
						{
							int energyUseRate = unit->getRunningValue() / 10;
							if (energyUseRate < 1) energyUseRate = 1;
							if ((game->gameTimer % energyUseRate) == 0)
								unit->setEnergy(unit->getEnergy() - 5);
						} else {
							unit->setSpeed(Unit::UNIT_SPEED_FAST);
							game->gameUI->setPointersChangedFlag(unit->getOwner());
						}
						*/
					}
					else if (unit->getSpeed() == Unit::UNIT_SPEED_JUMP)
					{
						if (!unit->isSideways())
						{
							maxspeed = unitType->getJumpSpeed() / GAME_TICKS_PER_SECOND;
						} else {
							maxspeed = unitType->getMaxSpeed() / GAME_TICKS_PER_SECOND;
						}

						char *tstr = NULL;
						tstr = unitType->getJumpTiming();

						bool jumpForward = false;
						bool jumpBackward = false;
						bool jumpLeft = false;
						bool jumpRight = false;
						// get camera relative jump directions (direct ctrl keys pressed on jump)
						if (!unit->isSideways())
						{
							unit->getUnitRelativeJumpDirections(&jumpForward, &jumpBackward, &jumpLeft, &jumpRight);
						}

						if (jumpBackward)
						{
							tstr = unitType->getJumpTimingBackward();
						}
						if ((jumpLeft || jumpRight) && !jumpForward && !jumpBackward)
						{
							tstr = unitType->getJumpTimingSideways();
						}

						// scale time based on jump timing string...
						if (tstr != NULL
							&& unit->getJumpTotalTime() > 0)
						{
							assert(strlen(tstr) == JUMPTIMING_STRING_LENGTH);
							int timeAtString = ((unit->getJumpTotalTime() - unit->getJumpCounter()) * JUMPTIMING_STRING_LENGTH) / unit->getJumpTotalTime();
							assert(timeAtString >= 0 && timeAtString < JUMPTIMING_STRING_LENGTH);
							maxspeed *= (float)(tstr[timeAtString] - '0') / 8.0f;
						}
					}

					if (unit->getSpeed() != Unit::UNIT_SPEED_JUMP
						|| unitType->doesSlowdownJump())
					{
						maxspeed *= (1.0f - unit->getSlowdown());
					}

					if (unit->wasMovingBackward())
					{
						maxspeed *= unitType->getBackwardMovementFactor();
					} else {
						if (unit->wasMovingSideways())
						{
							//maxspeed /= 1.3f;
						}
					}

					// HACK: if turned for a long time, scale 
					// speed/accel down... (max -50%)
					int turnedtime = unit->getTurnedTime();
					if (turnedtime > 50)
					{
						turnedtime -= 50;
						if (turnedtime > 100)
						{
							maxspeed *= 0.5f;
						} else {
							maxspeed *= (1.0f - 0.5f * float(unit->getTurnedTime()) / 100.0f);
						}
					}

					if (unit->isSideways())
					{
						float velTotal = fabs(velocity.x);
						if (velTotal > maxspeed)
						{
							velocity.x *= maxspeed / velTotal;
						}
					} else {

						maxspeed *= unit->getCustomTimeFactor();

						float velTotalSq = velocity.GetSquareLength();
						if (velTotalSq > maxspeed*maxspeed)
						{
							velocity *= maxspeed / (float)sqrtf(velTotalSq);
						}
					}
					// prevent too much sliding when turning...
					//if (rotated)
					//{
	 				//	frictionVelocity(&velocity, unitType->getFriction());
					//}
				} else {
					if (unit->isGroundFriction())
						frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
					else
						frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
				}
			} else { // non-sticky

				if (unit->isOnGround() || unitType->isFlying())
				{
					float accel = unitType->getAcceleration() / GAME_TICKS_PER_SECOND;
					if (unitType->isVehicle() && !unitType->isFlying())
					{
						int ox = game->gameMap->scaledToObstacleX(position.x);
						int oy = game->gameMap->scaledToObstacleY(position.z);
						int coverDist = game->gameMap->getCoverMap()->getDistanceToNearestCover(ox, oy);
						if (coverDist < 10)
						{
							// 10% - 100% acceleration. (drive slower if obstacles near)
							if (coverDist < 1) coverDist = 1;
							accel *= ((float)coverDist / 10.0f);

							// HACK: if turned for a long time, scale 
							// speed/accel down... (max -50%)
							if (unit->getTurnedTime() > 100)
							{
								accel *= 0.5f;
							} else {
								accel *= (1.0f - 0.5f * float(unit->getTurnedTime()) / 100.0f);
							}
						}
					}

					if (!unit->isSideways())
					{
						velocity.x += -accel * sinf(UNIT_ANGLE_TO_RAD(rotation.y));
						velocity.z += -accel * cosf(UNIT_ANGLE_TO_RAD(rotation.y));
					} else {

						if (unit->getUnitType()->getBackwardMovementFactor() == 0.0f)
						{
							if (doLeftMove)
							{								
								if ((unit->getPosition() - getAimTargetPosition(unit)).x < 0.0f)
								{
									accel = 0.0f;
									if (unit->isGroundFriction())
										frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
									else
										frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
									animRequests->setStandAnim = true;
								}
							}
							if (doRightMove)
							{
								if ((unit->getPosition() - getAimTargetPosition(unit)).x > 0.0f)
								{
									accel = 0.0f;
									if (unit->isGroundFriction())
										frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
									else
										frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
									animRequests->setStandAnim = true;
								}
							}
						}

						velocity.x += -accel * sinf(UNIT_ANGLE_TO_RAD(rotation.y));
						if (doForwardMove)
						{
							// TODO: this would need a better specific onsideground flag!
							if (unit->isGroundFriction())
							{
								// TODO: sidejumpspeed conf
								velocity.z = unitType->getJumpSpeed() / GAME_TICKS_PER_SECOND;
							}
						}
					}

#ifdef LEGACY_FILES
					// nop
#else
					if (accel != 0.0f)
					{
						animRequests->setMoveAnim = true;
						*accelerated = true;
					}
#endif
				}

#ifdef LEGACY_FILES
				animRequests->setMoveAnim = true;
				*accelerated = true;
#endif

				// TODO: skip friction when not firmly on ground. (?)
				// not needed here?
				if (unit->isGroundFriction())
					frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
				else
					frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
			}

		} else {

			if (!unit->isAtPathEnd())
			{
				// got to waypoint, continue to next waypoint
				int index = unit->getPathIndex();
				frozenbyte::ai::Path *path = unit->getPath();
				assert(path != NULL);
				//float px = (float)path->getPointX(index) + 0.5f;
				//float py = (float)path->getPointY(index) + 0.5f;
				//px = (px - (float)(game->gameMap->getSizeX()/2)) * game->gameMap->getScaleX();
				//py = (py - (float)(game->gameMap->getSizeY()/2)) * game->gameMap->getScaleY();
				int pxPath = path->getPointX(index);
				int pyPath = path->getPointY(index);
				float px = game->gameMap->pathfindToScaledX(pxPath);
				float py = game->gameMap->pathfindToScaledY(pyPath);
				assert(game->gameMap->isInScaledBoundaries(px, py));
				unit->setWaypoint(VC3(px, game->gameMap->getScaledHeightAt(px, py), py));
				unit->setPathIndex(index + 1);
				unit->setRushDistance(0.0f);
			} else {

				VC3 finalDestination = unit->getFinalDestination();
				if (position.x != finalDestination.x || position.z != finalDestination.z)
				{
					// we got near final destination, stop here.
					unit->setWaypoint(unit->getPosition());
					unit->setFinalDestination(unit->getPosition());
					// this is not 100% sure to happen every time we get to destination.
					game->gameUI->setPointersChangedFlag(unit->getOwner());
				}
				// we are at destination. stand still.
				// (in future, cannot just halt because of explosions, etc.)
				if (unitType->isSticky() && unit->isOnGround() && !unit->isSideways()
					&& unit->getGamePhysicsObject() == NULL)
				{
					velocity.x = 0;
					velocity.y = 0;
					velocity.z = 0;
				} else {
					if (unit->isGroundFriction())
						frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
					else
						frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
				}

				// HACK!
				// special case, rushers just keep on rushing even when at
				// destination...
				bool targetClose = false;
				if (unit->targeting.hasTarget())
				{
					if (unit->targeting.getTargetUnit() != NULL)
					{
						VC3 posdiff = 
							unit->targeting.getTargetUnit()->getPosition()
								-	unit->getPosition();
						if (posdiff.GetSquareLength() < UNITACTOR_RUSH_DISTANCE*UNITACTOR_RUSH_DISTANCE)
						{
							targetClose = true;
						}
					}
				}

				// ???
				//if (unit->isRushing() && targetClose

				bool followPlayerFar = false;
				if (unit->isFollowPlayer())
				{
					Unit *player1 = game->units->getUnitByIdString("player1");
					if (player1 != NULL)
					{
						VC3 plpos = player1->getPosition();
						VC3 pldistvec = plpos - position;
						float pldistSq = pldistvec.GetSquareLength();
						if (pldistSq > 3*3)
						{
							followPlayerFar = true;
						} else {
							unit->setRushDistance(UNITACTOR_RUSH_DISTANCE);
						}
					}
				}

				if (((unit->isRushing() && unit->targeting.hasTarget()) || followPlayerFar)
//					&& game->gameMap->getCoverMap()->getDistanceToNearestCover(ox, oy) > 2
					&& (unit->getSpeed() == Unit::UNIT_SPEED_FAST
					|| unit->getSpeed() == Unit::UNIT_SPEED_SPRINT)
					&& unit->getRushDistance() < UNITACTOR_RUSH_DISTANCE)
				{
					//Logger::getInstance()->error(int2str(unit->getRushDistance()));
					float accel = unitType->getAcceleration() / GAME_TICKS_PER_SECOND;
					float maxspeed = unitType->getMaxSpeed() / GAME_TICKS_PER_SECOND;					
					if (unit->getSpeed() == Unit::UNIT_SPEED_SPRINT)
					{
						accel *= 2;
						maxspeed = unitType->getSprintSpeed() / GAME_TICKS_PER_SECOND;
					}

					if (unit->getSpeed() != Unit::UNIT_SPEED_JUMP
						|| unitType->doesSlowdownJump())
					{
						maxspeed *= (1.0f - unit->getSlowdown());
					}

					float angle = rotation.y;
					velocity.x += -accel * sinf(UNIT_ANGLE_TO_RAD(angle));
					if (!unit->isSideways())
					{
						velocity.z += -accel * cosf(UNIT_ANGLE_TO_RAD(angle));
					} else {
						if (doForwardMove)
						{
							// TODO: this would need a better specific onsideground flag!
							if (unit->isGroundFriction())
							{
								// TODO: sidejumpspeed conf
								velocity.z = unitType->getJumpSpeed() / GAME_TICKS_PER_SECOND;
							}
						}
					}
					animRequests->setMoveAnim = true;
					animRequests->setStandAnim = false;
					*accelerated = true;

					float velTotalSq = velocity.GetSquareLength();
					if (velTotalSq > maxspeed*maxspeed)
					{
						velocity *= maxspeed / (float)sqrtf(velTotalSq);
					}
					unit->setRushDistance(unit->getRushDistance() + maxspeed);
				} else {
					//unit->setRushDistance(0.0f); ///????

					animRequests->setStandAnim = true;
				}
			}
		}
		unit->setPosition(position);
		unit->setVelocity(velocity);
		unit->setRotation(rotation.x, rotation.y, rotation.z);

		//---------------------------------------------------------------------
		// <walkonmaterial-effect>
		const float walkOnMaterialRange = 30.0f;
		if( doMove && unit->getUnitType()->getWalkOnMaterialEnabled() && game->gameUI->getFirstPerson( game->singlePlayerNumber ) && IsCloseEnough2D( game->gameUI->getFirstPerson( game->singlePlayerNumber  )->getPosition(), position, walkOnMaterialRange ) )
		{
			WalkOnMaterialData* material_data = NULL;

			bool enough_time_passed_for_effect = (game->gameTimer - movementParticleEffectCreationTime) * GAME_TICK_MSEC > unit->getUnitType()->getWalkOnMaterialParticleEffectTimeOut();
			bool enough_time_passed_for_sound = (game->gameTimer - movementSoundEffectCreationTime) * GAME_TICK_MSEC > unit->getUnitType()->getWalkOnMaterialSoundEffectTimeOut();

			bool stepped = forceFootStep > 0;
			std::string foot_helper;

			// enough time passed for next step
			if(enough_time_passed_for_sound || enough_time_passed_for_effect)
			{
				// get animation time
				int anim_number = unit->getAnimation();
				int anim_time = Animator::getAnimationTime(unit, anim_number);
				int current_time = Timer::getTime();

				bool found_footstep = false;

				// loop through all footstep triggers
				const std::vector<FootStepTrigger *> &footStepTriggers = unit->getFootStepTriggers();
				for(unsigned int i = 0; i < footStepTriggers.size(); i++)
				{
					FootStepTrigger *trigger = footStepTriggers[i];

					// trigger for correct animation
					if(unit->getAnimationSet()->getAnimationFileNumber(trigger->animation) != anim_number) continue;

					found_footstep = true;

					// not enough time passed since last trigger
					int trigger_duration = (int)((trigger->time_max - trigger->time_min) / unit->getAnimationSpeedFactor());
					if(current_time <= trigger->last_triggered + trigger_duration) continue;

					// in the right animation spot
					if(anim_time > trigger->time_min && anim_time < trigger->time_max)
					{
						foot_helper = trigger->helperbone;
						stepped = true;
						trigger->last_triggered = current_time;
					}
				}

				// no triggers?
				if(!found_footstep)
				{
					// always step for compatibility
					stepped = true;
				}
			}

			if(stepped && (enough_time_passed_for_effect || enough_time_passed_for_sound))
			{

				if( game->gameMap->isInScaledBoundaries( position.x, position.z ) )
				{
					const int x = game->gameMap->scaledToObstacleX( position.x );
					const int y = game->gameMap->scaledToObstacleY( position.z );
					int material = game->gameMap->getAreaMap()->getAreaValue( x, y, AREAMASK_MATERIAL );

					material = getMaterialByPalette( material >> AREASHIFT_MATERIAL );
					
					material_data = unit->getUnitType()->getWalkOnMaterial( material );
				}

				if(forceFootStep > 0) forceFootStep--;
			}

			// <particle effect>
			if( stepped && enough_time_passed_for_effect )
			{
				movementParticleEffectCreationTime = game->gameTimer;

				if( material_data != NULL && material_data->particleEffectName.empty() == false )
				{
					ui::VisualEffectManager *vman = game->gameUI->getVisualEffectManager();
					
					VC3 pos = position;

					// get position from foot helper
					if(!foot_helper.empty())
					{
						VisualObject *unitVisual = unit->getVisualObject();
						IStorm3D_Model *unitModel = 0;
						if(unitVisual)
							unitModel = unitVisual->getStormModel();
						if(unitModel)
						{
							const std::string helperName = "HELPER_BONE_" + foot_helper;

							bool helperFound = util::getHelperPosition( unitModel, helperName.c_str(), pos );
							if (!helperFound)
							{
								Logger::getInstance()->warning("UnitType::actWalk - No such bone helper for footstep");
								Logger::getInstance()->warning(helperName.c_str());
							}
						}
					}

					int effId = vman->getVisualEffectIdByName( material_data->particleEffectName.c_str() );
					if (effId == -1)
					{
						Logger::getInstance()->error("UnitType::actWalk - No walk on material visual effect with given name found.");
						// Logger::getInstance()->debug(objectEvents[i].effect.c_str());
					} else {
						// TODO: proper lifetime
						int lifetime = GAME_TICKS_PER_SECOND / 2;
						VisualEffect *effect = vman->createNewManagedVisualEffect( effId, lifetime, NULL, NULL, pos, pos, rotation, velocity, game );
						if (effect != NULL)
						{
							// effect->setParticleExplosion( pos, true );
						}
					}
				}
				
				/**/
			}
			// </particle effect>


			// <sound effect hack>
			if( stepped && enough_time_passed_for_sound )
			{
				movementSoundEffectCreationTime = game->gameTimer;

				if( material_data != NULL && material_data->soundEffects.empty() == false )
				{
					const bool loop = false;
					const int sound_pos = game->gameRandom->nextInt() % material_data->soundEffects.size();
					const int volume = material_data->soundEffects[ sound_pos ].volume;
					const float range = (float)material_data->soundEffects[ sound_pos ].range;


					game->gameUI->playSoundEffect( material_data->soundEffects[ sound_pos ].file.c_str(), position.x, position.y, position.z, loop, volume, range, DEFAULT_SOUND_PRIORITY_NORMAL );
				}
			}
			// </sound effect hack>
		
		}
		// </walkonmaterial-effect>

		if( doMove ) 
		{
			breathingParticleEffectCreationTime = game->gameTimer;
		}

	}

	void UnitActor::actOutdoorBreathingSteam( Unit* unit )
	{
		
		// höyryävä hengitys hack
		UnitType* ut = unit->getUnitType();
		const float walkOnMaterialRange = 30.0f;
		if( ut->getBreathingParticleEffect().empty() == false &&
			game->gameUI->getFirstPerson( game->singlePlayerNumber ) &&
			IsCloseEnough2D( game->gameUI->getFirstPerson( game->singlePlayerNumber )->getPosition(), unit->getPosition(), walkOnMaterialRange ) && 
			( game->gameTimer - breathingParticleEffectCreationTime ) * GAME_TICK_MSEC > ut->getBreathingParticleEffectTimeOut() )
		{
			const int x = game->gameMap->scaledToObstacleX( unit->getPosition().x );
			const int y = game->gameMap->scaledToObstacleY( unit->getPosition().z );
			if( game->gameMap->getAreaMap()->isAreaAnyValue( x, y, AREAMASK_INBUILDING ) == 0 )
			{
				VC3 velocity = unit->getVelocity();
				VC3 rotation = unit->getRotation();

				breathingParticleEffectCreationTime = game->gameTimer;
				VC3 mouthPosition(unit->getPosition());
				VisualObject *unitVisual = unit->getVisualObject();
				IStorm3D_Model *unitModel = 0;
				if(unitVisual)
					unitModel = unitVisual->getStormModel();

				if(unitModel)
				{
					const std::string helperName = "HELPER_BONE_Mouth";

					bool helperFound = util::getHelperPosition( unitModel, helperName.c_str(), mouthPosition );
					if (!helperFound)
					{
						Logger::getInstance()->warning("UnitActor::actOutdoorBreathingSteam - Could not find a helper to put the particle breath animation onto.");
					}
					else
					{
						ui::VisualEffectManager *vman = game->gameUI->getVisualEffectManager();
						int effId = vman->getVisualEffectIdByName( ut->getBreathingParticleEffect().c_str() );
						if (effId == -1)
						{
							Logger::getInstance()->error("UnitType::actOutdoorBreathingSteam - No particle effect with given name found.");
							// Logger::getInstance()->debug(objectEvents[i].effect.c_str());
						} else {
							// TODO: proper lifetime
							int lifetime = GAME_TICKS_PER_SECOND / 2;
							VisualEffect *effect = vman->createNewManagedVisualEffect( effId, lifetime, NULL, NULL, mouthPosition, mouthPosition, rotation, velocity, game );
							if (effect != NULL)
							{
								// effect->setParticleExplosion( pos, true );
							}
						}

					}
				}
			}
		}
	}


	void UnitActor::actMisc(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		UnitType *unitType = unit->getUnitType();

		// slowdown (tranquilizer) wear-off
		if ((game->gameTimer & 7) == 0)
		{
			unit->wearOffSlowdown();
		}

		// recharge
		if ((game->gameTimer & (unitType->getRechargeRate() - 1)) == 0)
		{
			if (unit->getEnergy() < unit->getMaxEnergy())
			{
				unit->setEnergy(unit->getEnergy() + unit->getRechargingAmount());
			}
		}
		// cool down
		if ((game->gameTimer & (unitType->getCoolRate() - 1)) == 0)
		{
			if (unit->getHeat() > 0)
			{
				unit->setHeat(unit->getHeat() - unit->getCoolingAmount());
			}
		}
		// increase hp
		if ((game->gameTimer & 255) == 0)
		{
			int hp = unit->getHP();
			int oldhp = hp;
			if (hp < unit->getMaxHP())
			{
				if (hp < -30)
				{
					if (hp < -100)
						unit->setHP(-100);
					else
						unit->setHP(hp + 10);
				} else {
					// HACK: if no hostiles near, but friendlys really near,
					// power up unconscious fast, otherwise, really slow
					// and if enemies near, no powerup at all.
					if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
					{
						LinkedList *ulist = game->units->getAllUnits();
						LinkedListIterator iter = LinkedListIterator(ulist);
						bool friendsNear = false;
						bool hostilesNear = false;
						while (iter.iterateAvailable())
						{
							Unit *u = (Unit *)iter.iterateNext();
							if (u->isActive() && !u->isDestroyed()
								&& u->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
							{								
								if (u->getOwner() == unit->getOwner())
								{
									VC3 posdiff = u->getPosition() - unit->getPosition();
									if (posdiff.GetSquareLength() < HPGAIN_FRIENDS_NEAR_RANGE*HPGAIN_FRIENDS_NEAR_RANGE)
									{
										friendsNear = true;
										if (u->getSeeUnit() != NULL
											&& u->getSeeUnitDistance() < (HPGAIN_HOSTILES_NEAR_RANGE * 2))
										{
											hostilesNear = true;
											break;
										}
									}
								} else {
									if (game->isHostile(unit->getOwner(), u->getOwner()))
									{
										VC3 posdiff = u->getPosition() - unit->getPosition();
										if (posdiff.GetSquareLength() < HPGAIN_HOSTILES_NEAR_RANGE*HPGAIN_HOSTILES_NEAR_RANGE)
										{
											hostilesNear = true;
											break;
										}
									}
								}
							}
						}

						if (!unit->isDirectControl())
						{
							if (friendsNear && !hostilesNear)
							{
								// NEW behaviour: even units with no actual HP-gain, receive 
								// some hpgain (at least +1*4) when unconscious...
								unit->setHP(hp + (unit->getUnitType()->getHPGain() + 1) * 5);
							} else {
								if (!hostilesNear)
									unit->setHP(hp + (unit->getUnitType()->getHPGain() + 1) * 3);
								else
									unit->setHP(hp + unit->getUnitType()->getHPGain());
							}
						}

					} else {
#ifdef PROJECT_SURVIVOR
						if( unit->getHP() < unit->getUnitType()->getHPGainMax() )
						{
							if( ( unit->getHP() + unit->getUnitType()->getHPGain() ) > unit->getUnitType()->getHPGainMax() )
								unit->setHP( unit->getUnitType()->getHPGainMax() );
							else
								unit->setHP(hp + unit->getUnitType()->getHPGain());
						}
#else
						unit->setHP(hp + unit->getUnitType()->getHPGain());
#endif
					}
				}
				// TODO: optimize this...
				if ((oldhp / 10) != unit->getHP() / 10)
				{
					game->gameUI->setUnitDamagedFlag(unit->getOwner());
				}
			}
		}

#ifdef PROJECT_SURVIVOR
		// increase hp using per-unit parameters
		//

		// hp gain has not started
		if(unit->getHPGainStartTime() == 0)
		{
			// enough time passed since last damaged
			if(game->gameTimer - unit->getLastTimeDamaged() > unit->getHPGainStartDelay())
			{
				// HP under the limit
				int max_gain = (int)ceil(unit->getHPGainLimit() * unit->getMaxHP());
				if(unit->getHP() > 0 && unit->getHP() < max_gain)
				{
					// start gaining
					unit->setHPGainStartTime(game->gameTimer);
				}
			}
		}
		// hp gain is active
		else if(unit->getHPGainStartTime() > 0)
		{
			int max_gain = (int)ceil(unit->getHPGainLimit() * unit->getMaxHP());

			// was recently damaged
			if(game->gameTimer - unit->getLastTimeDamaged() < unit->getHPGainStartDelay())
			{
				// stop gaining
				unit->setHPGainStartTime(0);
			}
			// hp is over the limit
			else if(unit->getHP() <= 0 || unit->getHP() >= max_gain)
			{
				// stop gaining
				unit->setHPGainStartTime(0);
			}
			else
			{
				int delta = game->gameTimer - unit->getHPGainStartTime();

				// enough time passed
				if(delta >= unit->getHPGainDelay())
				{
					// gain hp
					int hp = unit->getHP() + unit->getHPGainAmount();
					if(hp > max_gain) hp = max_gain;
					unit->setHP( hp );
					game->gameUI->setUnitDamagedFlag(unit->getOwner());
					unit->setHPGainStartTime( game->gameTimer );
				}
			}
		}

#endif

		// update target locks
		unit->updateUnitTargetLock(game->gameTimer, false);
	}



	void UnitActor::actGravity(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		UnitType *unitType = unit->getUnitType();

		VC3 position = unit->getPosition();
		VC3 velocity = unit->getVelocity();

		float mapY = game->gameMap->getScaledHeightAt(position.x, position.z);

		if (position.y > mapY + 0.001f)
		{
			if ((!unitType->isFlying() && unitType->getHover() == 0) 
				|| unit->isDestroyed())
			{

				// max speed down is 100m/s (1m/tick)
				if (velocity.y > -1.0f) 
					velocity.y -= 0.01f * unitType->getGravityRatio();
					//velocity.y -= 0.098f * unitType->getGravityRatio();
			}
		}

		unit->setPosition(position);
		unit->setVelocity(velocity);
	}



	void UnitActor::actHover(Unit *unit, UnitActAnimationRequests *animRequests,
		bool accelerated)
	{
		UnitType *unitType = unit->getUnitType();

		VC3 position = unit->getPosition();
		VC3 velocity = unit->getVelocity();

		float mapY = game->gameMap->getScaledHeightAt(position.x, position.z);

		if (unitType->getHover() > 0 && !unit->isDestroyed())
		{
			float curHoverAlt = mapY + unitType->getHover() 
				+ unitType->getHoverVary() * (float)sin((game->gameTimer % 360) * (3.1415f/180.0f));

			if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
			{
				curHoverAlt = mapY;

				if (position.y < mapY + 10.0f && position.y > mapY + 0.01f)
				{
					// HACK: gimme landing smoke!

					/*
					if ((game->gameTimer & 63) == 0
						|| ((game->gameTimer & 31) == 0 && position.y < mapY + 5.0f))
					{
						VC3 vel = VC3(0,0,0);
						// TODO: rotation based on groundplane.
						VC3 groundRot = VC3(0,0,0);
						VC3 groundPos = VC3(position.x, mapY, position.z);
						int efId = game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("landingsmoke");
						if (efId != -1)
						{
							VisualEffect *v = game->gameUI->getVisualEffectManager()->createNewVisualEffect(
								efId, NULL, NULL, groundPos, groundPos, groundRot, vel, game);
							if (v != NULL)
							{
								// TODO: leaking memory!!!
								//game->gameUI->getVisualEffectManager()->deleteVisualEffect(v);
							} else {
								assert(0);
							}
						}
					}
					*/
				}
			}

			if (position.y < curHoverAlt)
			{
				position.y += unitType->getElevationSpeed() / GAME_TICKS_PER_SECOND;
				if (position.y > curHoverAlt) position.y = curHoverAlt;
			} else {
				if (!unitType->isFlying() || !accelerated)
				{
					if (position.y > curHoverAlt)
					{
						position.y -= unitType->getElevationSpeed() / GAME_TICKS_PER_SECOND;
						if (position.y < curHoverAlt) position.y = curHoverAlt;
					}
				}
			}
			if (velocity.y < 0)
			{
				velocity.y += unitType->getElevationSpeed() / GAME_TICKS_PER_SECOND;
				if (velocity.y > 0) velocity.y = 0;
			} else {
				if (velocity.y > 0)
				{
					velocity.y -= unitType->getElevationSpeed() / GAME_TICKS_PER_SECOND;
					if (velocity.y < 0) velocity.y = 0;
				}
			}
		}
		unit->setPosition(position);
		unit->setVelocity(velocity);
	}



	void UnitActor::actVelocity(Unit *unit, UnitActAnimationRequests *animRequests,
		VC3 oldPosition)
	{
		VC3 position = unit->getPosition();
		VC3 velocity = unit->getVelocity();

		if (velocity.x != 0 || velocity.y != 0 || velocity.z != 0)
		{
			position += velocity;
		
			// keep within the map area
			if (!game->gameMap->isWellInScaledBoundaries(position.x, position.z))
				position = oldPosition;
		}

		unit->setPosition(position);
		unit->setVelocity(velocity);
	}



	void UnitActor::actOnGround(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		UnitType *unitType = unit->getUnitType();

		VC3 position = unit->getPosition();
		VC3 velocity = unit->getVelocity();
		VC3 rotation = unit->getRotation();

		float mapY = game->gameMap->getScaledHeightAt(position.x, position.z);

		if (unitType->isSticky())
		{
			if (fabs(velocity.x) < 0.01f && fabs(velocity.z) < 0.01f)
			{
				unit->setOnGround(true);
			}
			if (!unit->isSideways())
			{
				unit->setGroundFriction(true);
			}
		} else {
			unit->setOnGround(true);
			if (!unit->isSideways())
			{
				unit->setGroundFriction(true);
			}
		}

//		unit->setGroundFriction(true);

		position.y = mapY;
		velocity.y = 0;
		if (unitType->isGroundPlaneDirection()
			|| unit->isDestroyed()
			|| unit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
		{
			float tmpx = sinf(UNIT_ANGLE_TO_RAD(rotation.y));
			float tmpz = cosf(UNIT_ANGLE_TO_RAD(rotation.y));
			float noseDiff = 
				game->gameMap->getScaledHeightAt(position.x + tmpx, position.z + tmpz)
				- game->gameMap->getScaledHeightAt(position.x - tmpx, position.z - tmpz);

			tmpx = sinf(UNIT_ANGLE_TO_RAD(90+rotation.y));
			tmpz = cosf(UNIT_ANGLE_TO_RAD(90+rotation.y));
			float sideDiff = 
				game->gameMap->getScaledHeightAt(position.x + tmpx, position.z + tmpz)
				- game->gameMap->getScaledHeightAt(position.x - tmpx, position.z - tmpz);

			//rotation.x = RAD_TO_UNIT_ANGLE(atanf(-noseDiff/2));
			//if (rotation.x < 0) rotation.x += 360;
			//rotation.z = RAD_TO_UNIT_ANGLE(atanf(-sideDiff/2));
			//if (rotation.z < 0) rotation.z += 360;

			// HACK: try to prevent rotations near non-interpolated
			// slopes... (for sticky units only)
			if (unitType->isSticky())
			{
				if (noseDiff < -2.0f || noseDiff > 2.0f
					|| sideDiff < -2.0f || sideDiff > 2.0f)
				{
					noseDiff = 0;
					sideDiff = 0;
				}
			}

			VC3 prefer(0,0,0);
			if (unitType->isGroundPlaneDirection())
			{
				prefer.x = RAD_TO_UNIT_ANGLE(atanf(-noseDiff/2));
				if (prefer.x < 0) prefer.x += 360;
				prefer.z = RAD_TO_UNIT_ANGLE(atanf(-sideDiff/2));
				if (prefer.z < 0) prefer.z += 360;
			} else {
				if (noseDiff < -1.0f) noseDiff = -1.0f;
				if (noseDiff > 1.0f) noseDiff = 1.0f;
				if (sideDiff < -1.0f) sideDiff = -1.0f;
				if (sideDiff > 1.0f) sideDiff = 1.0f;
				prefer.x = RAD_TO_UNIT_ANGLE(atanf(-noseDiff/2));
				if (prefer.x < 0) prefer.x += 360;
				prefer.z = RAD_TO_UNIT_ANGLE(atanf(-sideDiff/4));
				if (prefer.z < 0) prefer.z += 360;
			}
			
			VC3 oldRot = rotation;
			rotateTo(prefer, &rotation, 0.07f);
			unit->lastXRotation = rotation.x - oldRot.x;
			if (unit->lastXRotation > 180) 
				unit->lastXRotation -= 360;
			if (unit->lastXRotation < -180) 
				unit->lastXRotation += 360;
			unit->lastZRotation = rotation.z - oldRot.z;
			if (unit->lastZRotation > 180) 
				unit->lastZRotation -= 360;
			if (unit->lastZRotation < -180) 
				unit->lastZRotation += 360;
		} else {
			if (fabs(rotation.x) > 0.01f || fabs(rotation.z) > 0.01f)
			{
				balanceRotation(&rotation, 0.05f);
			} else {
				rotation.x = 0;
				rotation.z = 0;
			}
			unit->lastXRotation = 0;
			unit->lastZRotation = 0;
		}

		unit->setPosition(position);
		unit->setVelocity(velocity);
		unit->setRotation(rotation.x, rotation.y, rotation.z);
	}



	void UnitActor::actAirborne(Unit *unit, UnitActAnimationRequests *animRequests,
		bool accelerated)
	{
		UnitType *unitType = unit->getUnitType();

		// THIS WAS NOT HERE EARLIER, DOES IT WORK CORRETLY?
    if (!unitType->isSticky())
    {
			unit->setOnGround(false);
			if (!unit->isSideways())
			{
				unit->setGroundFriction(false);
			}
		}

		VC3 rotation = unit->getRotation();

		if (unitType->isFlying() && unitType->getFlyVelocityRotation() > 0)
		{
			if (accelerated)
			{
				if (rotation.x > -unitType->getFlyVelocityRotation()) rotation.x -= 0.15f;
			} else {
				if (rotation.x < 0) rotation.x += 0.15f;
			}
		}

		if (unitType->isFlying())
		{
			balanceRotation(&rotation, 0.002f);
		} else {
		  rotation.x += unit->lastXRotation * 0.2f;
		  rotation.z += unit->lastZRotation * 0.2f;
		}
		unit->setRotation(rotation.x, rotation.y, rotation.z);
	}


	// HACK: for distributing raytrace load...
	// FIXME: may not work properly!!! should replace with unit
	// id numbering.
	// FIXME: this would be fatal for netgame sync...
	int unitactor_acttargeting_counter = 0;

	void UnitActor::actTargeting(Unit *unit, UnitActAnimationRequests *animRequests,
		bool doFire, bool rotated, 
		bool doMove, bool doForwardMove, bool doBackMove, 
		bool doLeftMove, bool doRightMove)
	{
		UnitType *unitType = unit->getUnitType();

		VC3 position = unit->getPosition();
		VC3 rotation = unit->getRotation();
		VC3 velocity = unit->getVelocity();

		float turnspeed = unitType->getTurning() / GAME_TICKS_PER_SECOND;
		float rotacc = turnspeed * unitType->getTurningAccuracy();

		if (((unit->targeting.hasTarget() && !unit->isDirectControl()) 
			|| doFire)
			//&& unit->hasAnyWeaponReady()
			&& ((unit->getSpeed() != Unit::UNIT_SPEED_SPRINT
			|| (velocity.x == 0 && velocity.z == 0))
			|| unitType->doesAllowFireOnSprint()))
		{
			// check for line of fire every once a while...

			// this would make raytrace spikes to every 16th frame.
			//if ((game->gameTimer & LINE_OF_FIRE_CHECK_INTERVAL) == 0)
			// now, using the random instead of 0, we get equal distribution...
			//if ((game->gameTimer & (LINE_OF_FIRE_CHECK_INTERVAL - 1)) 
			//	== (game->gameRandom->nextInt() & (LINE_OF_FIRE_CHECK_INTERVAL - 1)))
			// even better distribution... (HACK)
			if ((game->gameTimer & (LINE_OF_FIRE_CHECK_INTERVAL - 1)) 
				== (unitactor_acttargeting_counter & (LINE_OF_FIRE_CHECK_INTERVAL - 1)))
			{

				// NEW BEHAVIOUR: refresh target unit (position)...
				/*
				if (unit->targeting.getTargetUnit() != NULL)
				{
					if (unit->targeting.isAutoTarget())
						unit->targeting.setAutoTarget(unit->targeting.getTargetUnit());
					else
						unit->targeting.setTarget(unit->targeting.getTargetUnit());
				}
				*/

				VC3 dir;
				VC3 target;
				VC3 weaponPosition = unit->getPosition();

#ifdef PROJECT_CLAW_PROTO
// HACK: claw proto hack...
int foo_prone = 1;
static int proningvarnamenum = -1;
if (proningvarnamenum == -1) proningvarnamenum = unit->variables.getVariableNumberByName("proning");
if (unit->variables.getVariable(proningvarnamenum) == 1)
{
	foo_prone = 0;
	weaponPosition += VC3(0,0.6f,0);
} else {
	weaponPosition += VC3(0,1.7f,0);
}
unit->targeting.setLineOfFireToTarget(false, foo_prone * unitType->getLineOfFireLoseTime() / GAME_TICK_MSEC, game->gameTimer);
#else
				if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
				{
					weaponPosition += VC3(0,0.4f,0);
				} else {
					weaponPosition += VC3(0,1.7f,0);
				}

				unit->targeting.setLineOfFireToTarget(false, unitType->getLineOfFireLoseTime() / GAME_TICK_MSEC, game->gameTimer);
#endif

				if (unit->targeting.getTargetUnit() != NULL)
				{
					target = unit->targeting.getTargetUnit()->getPosition();
					if (unit->targeting.getTargetUnit()->getSpeed() == Unit::UNIT_SPEED_CRAWL)
					{
						target += VC3(0, unit->targeting.getTargetUnit()->
							getUnitType()->getAimHeightCrawling(), 0);
					} else {
						target += VC3(0, unit->targeting.getTargetUnit()->
							getUnitType()->getAimHeightStanding(), 0);
					}
				} else {
					target = unit->targeting.getTargetPosition();
				}
				dir = target - weaponPosition;
				VC3 posDiff = target - weaponPosition;
				
				float posDiffSqLen = posDiff.GetSquareLength();

				// closer than 10 meters - and we see/saw that enemy? shoot!
				// or closer than 2 meters
				if (posDiffSqLen < 2*2
#ifdef PROJECT_CLAW_PROTO
					|| (posDiffSqLen < 5*5
#else
					|| (posDiffSqLen < 10*10
#endif
					&& unit->getSeeUnit() != NULL
					&& unit->getSeeUnit() == unit->targeting.getTargetUnit()))
				{
					unit->targeting.setLineOfFireToTarget(true, unitType->getLineOfFireLoseTime() / GAME_TICK_MSEC, game->gameTimer);
				}

				// no LOF yet? well, continue for the actual raytracing check...
				if (!unit->targeting.hasLineOfFireToTarget()
					&& posDiffSqLen < (unit->getMaxWeaponRange() * unit->getMaxWeaponRange()))
				{
					dir.Normalize();
					float targdist = sqrtf(posDiffSqLen);
					GameCollisionInfo cinfo;

					// ignore friendly units that are withing certain range...
					// note: actually owned units, not friendly
					LinkedList ownUnitsNear;
					LinkedList *ownulist = game->units->getOwnedUnits(unit->getOwner());
					LinkedListIterator ownUnitsIter = LinkedListIterator(ownulist);
					while (ownUnitsIter.iterateAvailable())
					{
						Unit *ownu = (Unit *)ownUnitsIter.iterateNext();
						VC3 posdiff = ownu->getPosition() - weaponPosition;
						if (posdiff.GetSquareLength() < UNITACTOR_LOF_IGNORE_FRIENDLY_RANGE * UNITACTOR_LOF_IGNORE_FRIENDLY_RANGE)
						{
							if (unit->targeting.getTargetUnit() != ownu
								&& ownu->isActive())
							{
								ownUnitsNear.append(ownu);
								ownu->getVisualObject()->setCollidable(false);
							}
						} 					 
					}

					// don't hit self...
					unit->getVisualObject()->setCollidable(false);

#ifdef PROJECT_CLAW_PROTO
					game->getGameScene()->rayTrace(weaponPosition, dir, targdist, cinfo, false, false, true, true);
#else
					game->getGameScene()->rayTrace(weaponPosition, dir, targdist, cinfo, false, false);
#endif

					// restore self collidable
					unit->getVisualObject()->setCollidable(true);

					// restore collision check for nearby own units
					while (!ownUnitsNear.isEmpty())
					{
						Unit *ownu = (Unit *)ownUnitsNear.popLast();
						ownu->getVisualObject()->setCollidable(true);
					}

					if (cinfo.hit)
					{
						if (cinfo.hitUnit)
						{
							if ((game->isHostile(unit->getOwner(), cinfo.unit->getOwner()))
								|| cinfo.unit == unit->targeting.getTargetUnit())
							{
								unit->targeting.setLineOfFireToTarget(true, unitType->getLineOfFireLoseTime() / GAME_TICK_MSEC, game->gameTimer);
							}
						} else {
							if (unit->targeting.getTargetUnit() == NULL)
							{
								VC3 targOffset = unit->targeting.getTargetPosition() - cinfo.position;
								// 5 meters radius for ground targets...
								if (targOffset.GetLength() < UNITACTOR_GROUND_TARGET_AREA)
								{
									unit->targeting.setLineOfFireToTarget(true, unitType->getLineOfFireLoseTime() / GAME_TICK_MSEC, game->gameTimer);
								}
							}
						}
					} else {
						unit->targeting.setLineOfFireToTarget(true, unitType->getLineOfFireLoseTime() / GAME_TICK_MSEC, game->gameTimer);
					}
				}
			}

			// is this unit possibly using a parabolic weapon?
			bool parabolicWeapInUse = false;
			for (int weap = 0; weap < UNIT_MAX_WEAPONS; weap++)
			{
				Weapon *w = unit->getWeaponType(weap);
				if (w == NULL)
					continue;

				if (unit->isWeaponActive(weap))
				{
					Bullet *b = w->getBulletType();
					if (b != NULL)
					{
						if (b->getFlyPath() == Bullet::FLYPATH_PARABOLIC)
						{
							parabolicWeapInUse = true;
						}
					}
				}
			}


			// shoot if line of fire exists
			// or parabolic weapon in use
			// or directly controlling unit
			if ((unit->targeting.hasLineOfFireToTarget() || unitType->hasAlwaysLOF())
				|| parabolicWeapInUse
				|| doFire)
			{
				VC3 target;
				if (doFire)
				{
					// direct control firing

					float wBetaAngle = rotation.x + unit->getLookBetaAngle();
					if (wBetaAngle >= 360) wBetaAngle -= 360;
					target = unit->getPosition() + VC3(0,1.7f,0);
					
					// TODO!!!
					// FIX ME: this calculation is not correct!
					// will not give correct vector when looking up or down.
					target.x += -1.0f * sinf(UNIT_ANGLE_TO_RAD(rotation.y));
					target.y += -1.0f * sinf(UNIT_ANGLE_TO_RAD(wBetaAngle));
					target.z += -1.0f * cosf(UNIT_ANGLE_TO_RAD(rotation.y));

					unit->targeting.setTarget(target);
					bool shotok = shoot(unit, target);
					if (shotok)
					{
						// HACK: if it was a jump attack,
						// don't make the shoot anim.
						if (unit->getJumpCounter() == 0
							|| unitType->doesFireStopJump()
							|| unitType->doesFireWhileJumping())
						{
							animRequests->setShootAnim = true;
						}
						unit->setKeepReloading(false);
					}
					unit->targeting.clearTarget();
				} else {

					// AI control firing

					if (unit->targeting.getTargetUnit() != NULL)
					{
						target = unit->targeting.getTargetUnit()->getPosition() + VC3(0,1,0);
					} else {
						target = unit->targeting.getTargetPosition();
					}

					/*		
					VC2 destFloat = VC2(
						(float)(target.x-position.x), (float)(target.z-position.z));
					float destAngleFloat = destFloat.CalculateAngle();
					float destAngle = -RAD_TO_UNIT_ANGLE(destAngleFloat) + (360+270);
					while (destAngle >= 360) destAngle -= 360;
					*/
					float destAngle = util::PositionDirectionCalculator::calculateDirection(position, target);

					// HACK: sweeping fire makes turning impossible
					bool cannotRotate = false;
					if (unit->isAnyWeaponFiring()
						&& unit->targeting.hasSweepTargetPosition())
					{
						cannotRotate = true;
					}

					if (!rotated && velocity.x == 0 && velocity.y == 0 && velocity.z == 0)
//						&& !cannotRotate)
					{
						float rotSpeed = 0;

						rotSpeed = turnspeed * util::AngleRotationCalculator::getRotationForAngles(rotation.y, destAngle, rotacc);

						if (cannotRotate)
							rotSpeed *= 0.1f;

						rotation.y += rotSpeed;
						if (rotation.y < 0) rotation.y += 360;
						if (rotation.y >= 360) rotation.y -= 360;
					}

					int fofAngle;
					if (doMove)
						fofAngle = unitType->getAimingFOF() / 2;
					else
						fofAngle = unitType->getPreferFOF() / 2;

					float fofRotate = 0;
					fofRotate = util::AngleRotationCalculator::getRotationForAngles(rotation.y, destAngle, (float)fofAngle);

					if (fofRotate == 0)
					{
						bool shotok = shoot(unit, target);
						if (shotok)
						{
							// HACK: if it was a jump attack,
							// don't make the shoot anim.
							if (unit->getJumpCounter() == 0
								|| unitType->doesFireStopJump()
								|| unitType->doesFireWhileJumping())
							{
								animRequests->setShootAnim = true;
							}
							unit->setKeepReloading(false);

							// stop sprint?
							if (unitType->doesFireStopSprint()
								&& unit->getSpeed() == Unit::UNIT_SPEED_SPRINT)
							{
								unit->setSpeed(Unit::UNIT_SPEED_FAST);
							}
							// stop jump?
							if (unitType->doesFireStopJump()
								&& unit->getJumpCounter() > 0)
							{
								//unit->setJumpCounter(1); // stops on next tick.
								unit->setJumpCounter(0);
								unit->setJumpTotalTime(0);
								unit->setSpeed(Unit::UNIT_SPEED_FAST);
								unit->setWalkDelay(unit->getUnitType()->getJumpLandDelay() / GAME_TICK_MSEC);
							}
						}
					}
					else if(unit->isFiringInProgress())
					{
						// target outside field of fire - cancel shooting (firewait is re-activated)
						// (fixes laser turret)
						for (int weap = 0; weap < UNIT_MAX_WEAPONS; weap++)
						{
							Weapon *w = unit->getWeaponType(weap);
							if (w == NULL)
								continue;

							if(w->getFireWaitReset())
							{
								unit->setFireWaitDelay(weap, 0);
								unit->setFiringInProgress(false);
							}
						}
					}
				}
			}
		} else {
			if (!unit->targeting.hasTarget())			
			{
#ifdef PROJECT_CLAW_PROTO
// HACK: claw proto
int foo_prone = 1;
static int proningvarnamenum = -1;
if (proningvarnamenum == -1) proningvarnamenum = unit->variables.getVariableNumberByName("proning");
if (unit->variables.getVariable(proningvarnamenum) == 1)
{
	foo_prone = 0;
}
unit->targeting.setLineOfFireToTarget(false, foo_prone * unitType->getLineOfFireLoseTime() / GAME_TICK_MSEC, game->gameTimer);
#else
				unit->targeting.setLineOfFireToTarget(false, unitType->getLineOfFireLoseTime() / GAME_TICK_MSEC, game->gameTimer);
#endif
			}
		}

		if ((unit->targeting.hasTarget()
			|| unit->isDirectControl()))
// HACK: 
// now units can fire again while sprinting
//			&& (unit->getSpeed() != Unit::UNIT_SPEED_SPRINT
//			|| (velocity.x == 0 && velocity.z == 0)))
		{
#ifdef PROJECT_SHADOWGROUNDS
			if (!unit->isDirectControl() || doFire)
#else
			if (!unit->isDirectControl() || doFire || unit->isWeaponRecharging())
#endif
			{
				// aim animations...
				for (int weap = 0; weap < UNIT_MAX_WEAPONS; weap++)
				{
					Weapon *w = unit->getWeaponType(weap);
					if (w == NULL)
						continue;

					if (!unit->isWeaponActive(weap))
						continue;

#ifdef PROJECT_CLAW_PROTO
// HACK: ...
if (!unit->targeting.hasLineOfFireToTarget())
{
	if (unit->getFireReloadDelay(weap) <= 1600-200
		&& unit->getFireWaitDelay(weap) == 0)
	{
		animRequests->endAimAnim = true;
	}
	continue;
}
if (unit->getVelocity().GetSquareLength() > 0.01f * 0.01f
	|| unit->isPhysicsObjectLock())
{
	if (unit->getFireReloadDelay(weap) <= 1600-200
		&& unit->getFireWaitDelay(weap) == 0)
	{
		animRequests->endAimAnim = true;
	}
	continue;
}
#endif

					// no aim animation for launchspeed
					if(w->usesLaunchSpeedAnimation())
						continue;

					if (unit->getWeaponPosition(weap) == SLOT_POSITION_RIGHT_ARM)
					{
						if (animRequests->setAimAnim == AIM_ANIMATION_LEFT)
							animRequests->setAimAnim = AIM_ANIMATION_BOTH;
						else
							animRequests->setAimAnim = AIM_ANIMATION_RIGHT;
					}
					else if (unit->getWeaponPosition(weap) == SLOT_POSITION_LEFT_ARM)
					{
						if (animRequests->setAimAnim == AIM_ANIMATION_RIGHT)
							animRequests->setAimAnim = AIM_ANIMATION_BOTH;
						else
							animRequests->setAimAnim = AIM_ANIMATION_LEFT;
					}
					else
					{
						animRequests->setAimAnim = AIM_ANIMATION_OTHER;
					}
				}
			}

			// need target angle and stuff for torso twisting...
			VC3 target;

			target = getAimTargetPosition(unit);

			float destAngle = util::PositionDirectionCalculator::calculateDirection(position, target);

// HACK: fix the aim upward camera "bouncing" back-and-forth...
// WARNING: quite untested, may result into some incorrect behaviour
if (unit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER
	&& SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
{
	destAngle = game->gameUI->getGameCamera()->getAngleY();
	destAngle = 270 - destAngle;
	if (destAngle < 0) destAngle += 360;
}

			// TODO, nearest camera 45 degree angles.
			// then strafe/run/... decision.
// FIXME: hmm? something fishy here? why is the strafe dependant of third person view flag?
// isn't having direct control good enough? maybe not?
// for now, just changed this to include sideways units always, so it will be backward compatible --jpk
//			if (unit->isDirectControl() && game->gameUI->isThirdPersonView(unit->getOwner()))
			if (unit->isDirectControl() && (game->gameUI->isThirdPersonView(unit->getOwner())
				|| unit->isSideways()))
			{
				actDirectControlStrafing(unit, rotation, destAngle,
					doLeftMove, doRightMove, doForwardMove, doBackMove);
			}

			// sentry gun hack
			if(unit->getUnitType()->hasSentryGunControls())
			{
				rotation.y = destAngle;
			}

			// bone aiming (turrets, and other units too)
			actBoneAiming(unit, rotation, destAngle);

			// firing in progress logic...
			// (restore fast movement after a weapon shoot slowing down)
			// (end shoot anim?)
			if (unit->hasAnyWeaponReady() || unit->isClipReloading())
			{
				if (unit->isDirectControl())
				{
					int aimstop = unit->getAimStopCounter();

					int aimstop_time = GAME_TICKS_PER_SECOND / 10; // 0.1 secs
					Weapon *w = unit->getSelectedWeapon() != -1 ? unit->getWeaponType(unit->getSelectedWeapon()) : NULL;
					if(w)
					{
						aimstop_time = w->getAimEndDelay() * GAME_TICKS_PER_SECOND / 1000;
					}

					if (
#ifdef PROJECT_SURVIVOR
						aimstop < aimstop_time
#else
						aimstop < GAME_TICKS_PER_SECOND * 4  // 4 secs
#endif

						&& !unit->isClipReloading())
					{
						aimstop++;
						unit->setAimStopCounter(aimstop);

						animRequests->endShootAnim = true;
					} else {
						unit->setAimStopCounter(0);
						animRequests->endAimAnim = true;
						animRequests->endShootAnim = true;
					}

					// TODO: handle this properly for minigun...
					// this "kinda" does it...
					// FIXME: causes some minor bugs with pistol though...?
					if (unit->hasAnyWeaponReady()
						|| unit->getSelectedWeapon() == -1
						|| unit->getWeaponType(unit->getSelectedWeapon()) == NULL
						|| unit->getWeaponType(unit->getSelectedWeapon())->getClipSize() != 0)
					{
						unit->setFiringInProgress(false);
						unit->targeting.clearSweepTargetPosition();
					}

					// HACK: restore fast (run) speed after firing a walk 
					// requiring weapon (should not have set walk speed in
					// the first place (cos that loses the previous speed)
					// (thus FIXME)
					//if (unit->getSelectedWeapon() != -1
					//	&& unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
					//{
					//	if (unit->getWeaponType(unit->getSelectedWeapon())->doesFiringRequireWalk())
					//		unit->setSpeed(Unit::UNIT_SPEED_FAST);
					//}
					// (see below too for a copy)
					unit->resetSpeedAfterFiring();

				} else {
					// ??? will this f*ck up some soldier animations, or something...
					animRequests->endShootAnim = true;
				}
			}
		} else {
			unit->setFiringInProgress(false);
			unit->targeting.clearSweepTargetPosition();
			//if (unit->getSelectedWeapon() != -1
			//	&& unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
			//{
			//	if (unit->getWeaponType(unit->getSelectedWeapon())->doesFiringRequireWalk())
			//		unit->setSpeed(Unit::UNIT_SPEED_FAST);
			//}
			unit->resetSpeedAfterFiring();
			animRequests->endAimAnim = true;

			if (unit->getAnimationSet() != NULL)
			{
				if (unit->getAnimationSet()->isAnimationInSet(ANIM_TORSOTWIST_LEFT)
					&& unit->getAnimationSet()->isAnimationInSet(ANIM_TORSOTWIST_RIGHT))
				{
					animRequests->endTwistAnim = true;
				}
			}

			if (unit->getVisualObject() != NULL)
			{
				char *aimBone = unitType->getAimBone();
				if (aimBone != NULL)
				{
					if (unit->getHitAnimationCounter() == 0)
					{
						unit->getVisualObject()->releaseRotatedBone(aimBone);
					}
				}
			}
		}

		if (unit->getHitAnimationCounter() > 0)
		{
			unit->setHitAnimationCounter(unit->getHitAnimationCounter() - 1);
			if (unit->getHitAnimationCounter() == 0)
			{
				unit->setHitAnimationBoneAngle(0);
			}
		}

		{
			bool launchspeed_fire_released = false;

			// get weapon that uses launch speed
			int wnum = -1;
			Weapon *w = NULL;
			if(unit->getSelectedWeapon() != -1)
			{
				wnum = unit->getSelectedWeapon();
				w = unit->getWeaponType(wnum);
				if(w && w->usesLaunchSpeed())
				{
					// primary weapon uses it
					launchspeed_fire_released = !game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FIRE, unit);
				}
				else if(w)
				{
					if(unit->getAttachedWeapon(wnum) != -1)
					{
						wnum = unit->getAttachedWeapon(wnum);
						w = unit->getWeaponType(wnum);
						if(w && w->usesLaunchSpeed())
						{
							// attached weapon uses it
							launchspeed_fire_released = !game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FIRE_SECONDARY, unit);
						}
						else
						{
							w = NULL;
						}
					}
					else
					{
						w = NULL;
					}
				}
			}

			// try secondary weapon
			if(w == NULL)
			{
				if(unit->getSelectedSecondaryWeapon() != -1 && unit->getWeaponType(unit->getSelectedSecondaryWeapon()))
				{
					wnum = unit->getSelectedSecondaryWeapon();
					w = unit->getWeaponType(wnum);
					if(w && w->usesLaunchSpeed())
					{
						// secondary weapon uses it
						launchspeed_fire_released = !game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FIRE_GRENADE, unit);
					}
				}
			}
			// primary is same as secondary
			else if(unit->getSelectedSecondaryWeapon() == unit->getSelectedWeapon())
			{
				// both secondary and primary must be released
				launchspeed_fire_released = !game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FIRE, unit) && !game->gameUI->isLocalPlayerDirectControlOn(DIRECT_CTRL_FIRE_GRENADE, unit);
			}

			if(launchspeed_fire_released && w && w->usesLaunchSpeed())
			{
				// not already waiting to fire
				if(unit->getFireWaitDelay(wnum) <= 0)
				{
					// reloading or no ammo
					if((unit->getWeaponAmmoAmount(wnum) == 0 && w->getAmmoUsage() != 0) || unit->getFireReloadDelay(wnum) > 0)
					{
						// reset launch speed
						unit->setLaunchSpeed(0);
					}
					// has been holding button
					else if(unit->getLaunchSpeed() > 0.0f)
					{
						// button released -> fire
						unit->setLaunchNow(true);

						// de-activate other weapons
						bool was_active[UNIT_MAX_WEAPONS];
						for (int wc = 0; wc < UNIT_MAX_WEAPONS; wc++)
						{
							was_active[wc] = unit->isWeaponActive(wc);
							unit->setWeaponActive(wc, false);
						}
						unit->setWeaponActive(wnum, true);

						// shoot
						shoot(unit, unit->targeting.getTargetPosition());

						// restore activity
						for (int wc = 0; wc < UNIT_MAX_WEAPONS; wc++)
						{
							unit->setWeaponActive(wc, was_active[wc]);
						}
					}
				}
			}
		}

		unit->setPosition(position);
		unit->setVelocity(velocity);
		unit->setRotation(rotation.x, rotation.y, rotation.z);
	}


	VC3 UnitActor::getAimTargetPosition(Unit *unit)
	{
		VC3 target = unit->getPosition();
		if (unit->isDirectControl())
		{
			target = unit->targeting.getAimingPosition();
		} else {
			// TODO: && unit->isSweepFiring()
//			if (unit->isFiringInProgress() 
//				&& unit->targeting.hasSweepTargetPosition())
//			{
//				unit->targeting.getSweepTargetPosition();
//			} else {
				if (unit->targeting.hasTarget())
				{
					if (unit->targeting.getTargetUnit() != NULL)
					{
						target = unit->targeting.getTargetUnit()->getPosition();
					} else {
						target = unit->targeting.getTargetPosition();
					}
				} else {
					if (unit->targeting.hasLastTargetPosition())
					{
						target = unit->targeting.getLastTargetPosition();
					}
				}
//			}
		}

		return target;
	}


	void UnitActor::actBoneAiming(Unit *unit, const VC3 &rotation, float destAngle)
	{
		UnitType *unitType = unit->getUnitType();

		char *aimBone = unitType->getAimBone();
		if (aimBone != NULL)
		{
			float fofRotate = 0;
			float maxAimAngle = (float)(unitType->getAimBoneFOF() / 2);
			if (unitType->getAimBoneFOF() < 360)
			{
				fofRotate = util::AngleRotationCalculator::getRotationForAngles(rotation.y, destAngle, maxAimAngle);
			}
			float aimAngle = destAngle - rotation.y;
			if (aimAngle < 0) aimAngle += 360.0f;
			//if (fabs(unit->getLastBoneAimDirection() - aimAngle) > 0.1f)
			{
				aimAngle -= unitType->getAimBoneDirection();
				if (fofRotate < 0)
				{
					aimAngle = -maxAimAngle;
				} else if (fofRotate > 0) {
					aimAngle = maxAimAngle;
				}

				// some units (mech) rotate the turret with a constant speed, rather than instantly
				//
				float aimspeed = unitType->getAimRotSpeed();
				float aimacc = aimspeed * unitType->getAimRotAccuracy();
				if(aimspeed >= 0 && aimacc >= 0)
				{
					float lastAngle = unit->getLastBoneAimDirection();
					float turndir = util::AngleRotationCalculator::getRotationForAngles(lastAngle, aimAngle, aimacc);
					int handle = unit->getTurningSound();

					// sound was stopped (probably because of priorities)
					if(handle != -1 && game->gameUI->getSoundMixer() && !game->gameUI->getSoundMixer()->isSoundPlaying(handle))
					{
						game->gameUI->getSoundMixer()->stopSound(handle);
						handle = -1;
					}

					// turning and not playing sound yet
					if(turndir != 0 && handle == -1)
					{
						char *sound_name = unit->getUnitType()->getTurningSound();
						if(sound_name && game->gameUI->getSoundMixer())
						{
							VC3 pos = unit->getPosition();
							handle = game->gameUI->playSoundEffect(sound_name, pos.x, pos.y, pos.z, true, DEFAULT_SOUND_EFFECT_VOLUME, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_LOW);
							unit->setTurningSound(handle, Timer::getTime());
							unit->setTurningSoundVolume(100);
							// set initial frequency
							unit->setTurningSoundFrequency(game->gameUI->getSoundMixer()->getSoundFrequency(handle));
						}
					}
					// already playing sound
					else if(handle != -1)
					{
						int timeDelta = Timer::getTime() - unit->getTurningSoundStartTime();
						int timeReq = (int)(1000.0f * unitType->getTurningSoundMinPlayTime());

						// not turning anymore and played sound long enough
						if(turndir == 0 && timeDelta > timeReq && game->gameUI->getSoundMixer())
						{
							// fade out sound
							int vol = unit->getTurningSoundVolume() - 2;
							if(vol < 0) vol = 0;
							unit->setTurningSoundVolume(vol);
							game->gameUI->getSoundMixer()->setSoundVolume(handle, vol);
							game->gameUI->getSoundMixer()->setSoundFrequency(handle, unit->getTurningSoundFrequency() * vol / 100);
						}
						else if(game->gameUI->getSoundMixer())
						{
							// fade in sound
							int vol = unit->getTurningSoundVolume() + 10;
							if(vol > 100) vol = 100;
							game->gameUI->getSoundMixer()->setSoundVolume(handle, vol);
							unit->setTurningSoundVolume(vol);

							int freq = game->gameUI->getSoundMixer()->getSoundFrequency(handle) + 1600;
							if(freq > unit->getTurningSoundFrequency()) freq = unit->getTurningSoundFrequency();
							game->gameUI->getSoundMixer()->setSoundFrequency(handle, freq);

							if(turndir != 0)
							{
								unit->setTurningSound(handle, Timer::getTime());
							}
						}
					}
					aimAngle = lastAngle + aimspeed * turndir;
					if(aimAngle > 360.0f) aimAngle -= 360.0f;
					if(aimAngle < 0.0f) aimAngle += 360.0f;
				}

				// update turning sound position
				int handle = unit->getTurningSound();
				if(handle != -1 && game->gameUI->getSoundMixer())
				{
					VC3 pos = unit->getPosition();
					VC3 vel(0,0,0);
					int vol = unit->getTurningSoundVolume();
					game->gameUI->getSoundMixer()->setSoundPosition(handle, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, vol, 0);
				}

				float flashlightAngle = aimAngle;
				// THIS SHIT REMOVED?
				/*
				// HACK: hack hack... vehicles don't lose aim rotation even when reloading.
				if (!unitType->isVehicle())
				{
					if (unit->hasAnyWeaponReady() && unit->isDirectControl())
					{
						aimAngle += unitType->getAimBoneDirection();
					} else {
						// HACK: special case: W_Pistol
						// HACK: special case: all other weapons too! =P
						if (unit->getSelectedWeapon() != -1
							&& unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
							//&& unit->getWeaponType(unit->getSelectedWeapon())->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_Pistol"))
						{
							aimAngle += unitType->getAimBoneDirection();
						}
					}
				}
				*/
				unit->setLastBoneAimDirection(aimAngle);
				if (unitType->isVehicle())
				{
					// vehicle don't have lights in turret, instead they
					// have just headlights?
					unit->setFlashlightDirection(180);
					// TODO: this should still be the flashlight angle,
					// because it is actually the turret light,
					// the vehicle headlights should be a seperate light..
				} else {
					unit->setFlashlightDirection(flashlightAngle);
				}

				// no bone aiming when jumping.
				/*
				if (unit->getSpeed() == Unit::UNIT_SPEED_JUMP)
				{
					aimAngle = 0;
				}
				*/

				if (!unit->isDirectControl() && !unit->targeting.hasTarget())
				{
					aimAngle = 0;
				}

				if (unit->getVisualObject() != NULL)
				{
					float visAimAngle = aimAngle + unit->getStrafeAimOffset();
					if (visAimAngle >= 360.0f) visAimAngle -= 360.0f;
					if (visAimAngle < 0.0f) visAimAngle += 360.0f;

					float betaAngle = 0;
					if (unit->getHitAnimationCounter() > 0)
					{
						int totalHitTime = unit->getUnitType()->getHitAnimationTime() / GAME_TICK_MSEC;
						if (totalHitTime < 1) totalHitTime = 1;

						float hitBlendFact = 1.0f - (unit->getHitAnimationCounter() / (float)totalHitTime);
						// hit goes to max blend faster, hit blendout goes out slower
						if (hitBlendFact < 0.25f)
						{
							hitBlendFact *= 2.0f;
							hitBlendFact = sinf(3.1415f * hitBlendFact);
						} else {
							//hitBlendFact = 0.5f + (hitBlendFact - 0.25f) / 1.5f;
							hitBlendFact = 0.5f + (hitBlendFact - 0.25f) * 1.333f;
							hitBlendFact = 0.5f + 0.5f * sinf(3.1415f * hitBlendFact);
						}
						betaAngle = unit->getUnitType()->getHitAnimationAngle() * hitBlendFact;
					}

					// interpolate...
					float newAngle = betaAngle;
					float lastAngle = unit->getHitAnimationBoneAngle();
					betaAngle = (lastAngle * 3 + betaAngle * 1) / 4;
					if (betaAngle < newAngle) 
					{
						betaAngle += 1.0f;
						if (betaAngle > newAngle) betaAngle = newAngle;
					}
					if (betaAngle > newAngle) 
					{
						betaAngle -= 1.0f;
						if (betaAngle < newAngle) betaAngle = newAngle;
					}

					float hitFactor = 0.5f + unit->getHitAnimationFactor() * 0.5f;
					betaAngle *= hitFactor;

					VC3 rot = unit->getRotation();
					VC3 hitVec = unit->getHitAnimationVector();
					hitVec.y = 0;
					if (hitVec.GetSquareLength() >= 0.001f)
					{
						hitVec.Normalize();
					}
					float tmpz = hitVec.z;
					hitVec.z = hitVec.z * cosf(UNIT_ANGLE_TO_RAD(rot.y)) - hitVec.x * sinf(UNIT_ANGLE_TO_RAD(rot.y));
					hitVec.x = hitVec.x * cosf(UNIT_ANGLE_TO_RAD(rot.y)) + tmpz * sinf(UNIT_ANGLE_TO_RAD(rot.y));

					float twistFact = unit->getUnitType()->getHitAnimationTwistFactor();

					float sideNudge = twistFact * betaAngle * hitVec.x;
					float frontNudge = betaAngle * hitVec.z;

					frontNudge += unit->getLastBoneAimBetaAngle();

					unit->setHitAnimationBoneAngle(betaAngle);
					unit->getVisualObject()->rotateBone(aimBone, visAimAngle + sideNudge, frontNudge);
				}
			}
		}
	}


	void UnitActor::actDirectControlStrafing(Unit *unit, VC3 &rotation, float destAngle, 
		bool doLeftMove, bool doRightMove, bool doForwardMove, bool doBackMove)
	{

		float camAngle = 270-game->gameUI->getGameCamera()->getAngleY();
		float destInCameraSystem = destAngle - camAngle;
		while (destInCameraSystem < 0) destInCameraSystem += 360;
		while (destInCameraSystem >= 360) destInCameraSystem -= 360;

		//int destAngleNearestCamera45 = (int)(game->gameUI->getGameCamera()->getAngleY() / 45.0f) * 45;
		// TODO: a better solution
		int nearest45InCameraSystem = 0;
		bool nearest90IsNext = false;
		for (int ang = 1; ang <= 8; ang++)
		{			
			float borderAngle = (float)(ang * 45);
			if (unit->isSideways())
			{
				//if (ang == 1) continue;
				//if (ang == 3) continue;
				//if (ang == 4) continue;
				//if (ang == 5) continue;
				//if (ang == 7) continue;
				//if (ang == 8) continue;

				if (ang == 2 || ang == 6)
				{
					if (ang == 2) borderAngle = 180;
					if (ang == 6) borderAngle = 360;
				} else {
					continue;
				}
			} else {
				if (((ang & 1) == 0) && unit->isLastMoveStrafed())
					borderAngle += 10;
				if (((ang & 1) == 1) && unit->isLastMoveStrafed())
					borderAngle -= 10;
				if (((ang & 1) == 0) && !unit->isLastMoveStrafed())
					borderAngle -= 10;
				if (((ang & 1) == 1) && !unit->isLastMoveStrafed())
					borderAngle += 10;
			}

			if (destInCameraSystem < borderAngle)
			{
				nearest45InCameraSystem = (ang-1) * 45;
				if ((ang & 1) == 0) nearest90IsNext = true;
				break;
			}
		}

// AOV
//nearest90IsNext = false;

		if (nearest90IsNext)
		{
			unit->setLastMoveStrafed(true);
		} else {
			unit->setLastMoveStrafed(false);
		}
		
		// TODO:
		// diagonal movement? (n*90+45 or n*90 angles)
		bool diagonalMovement = false;
		if (game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD))
		{
			// ?
		} else {
			if ((doLeftMove || doRightMove) && (doForwardMove || doBackMove))
				diagonalMovement = true;
		}

		if (unit->isSideways())
		{
			diagonalMovement = false;
		}

		float preferredRotation = rotation.y;
		if ((nearest90IsNext && !diagonalMovement)
			|| (!nearest90IsNext && diagonalMovement))
		{
			preferredRotation = (float)(nearest45InCameraSystem + 45);
		} else {
			preferredRotation = (float)nearest45InCameraSystem;
		}
		preferredRotation += camAngle;


		// mech controls hack
		//
		if(unit->getUnitType()->hasMechControls())
		{
			preferredRotation = camAngle;

			// determine direction for legs
			//
			if (doBackMove)
			{
				if(doLeftMove)
				{
					preferredRotation -= 135.0f;
				}
				else if(doRightMove)
				{
					preferredRotation += 135.0f;
				}
				else
				{
					preferredRotation += 180.0f;
				}
			}
			else if(doForwardMove)
			{
				if(doLeftMove)
				{
					preferredRotation -= 45.0f;
				}
				else if(doRightMove)
				{
					preferredRotation += 45.0f;
				}
			}
			else
			{
				if(doLeftMove)
				{
					preferredRotation -= 90.0f;
				}
				else if(doRightMove)
				{
					preferredRotation += 90.0f;
				}
				else
				{
					// not moving in any dir; don't rotate legs at all
					preferredRotation = rotation.y;
				}
			}

			// calculate difference in current and preferred angles
			// (dot product to be on the safe side, something simpler would probably work)
			float angle_diff = sinf(UNIT_ANGLE_TO_RAD(preferredRotation)) * sinf(UNIT_ANGLE_TO_RAD(rotation.y))
												+ cosf(UNIT_ANGLE_TO_RAD(preferredRotation)) * cosf(UNIT_ANGLE_TO_RAD(rotation.y));
			// greater than 110 degrees
			if(angle_diff < -0.35f)
			{
				// move in reverse direction
				preferredRotation -= 180.0f;
				unit->moveInReverseDir = true;
			}
			else
			{
				unit->moveInReverseDir = false;
			}

			unit->setStrafeAimOffset(0);

		} // end of mech controls

		while (preferredRotation >= 360) preferredRotation -= 360;
		while (preferredRotation < 0) preferredRotation += 360;

		float preferredRotate = 0;
		preferredRotate = util::AngleRotationCalculator::getRotationForAngles(rotation.y, preferredRotation, (unit->getUnitType()->getTurning() / GAME_TICKS_PER_SECOND) / 1.8f);
		if (preferredRotate != 0)				
		{
			// TODO: should use the unit conf's turningspeed?
			// (note, must then also scale the rotation accuracy above,
			// so that the unit won't start rotating "back and forward"
			// every second tick)
			rotation.y += preferredRotate * (unit->getUnitType()->getTurning() / GAME_TICKS_PER_SECOND);
			if (rotation.y < 0) rotation.y += 360;
			if (rotation.y >= 360) rotation.y -= 360;
		}

		if (game::SimpleOptions::getBool(DH_OPT_B_GAME_MODE_AIM_UPWARD)
			&& !unit->getUnitType()->hasMechControls())
		{
			// total crap...

			bool foofoo_diag1 = false;
			bool foofoo_diag2 = false;

			if ((doLeftMove && doForwardMove) || (doRightMove && doBackMove))
				foofoo_diag1 = true;
			else
				foofoo_diag1 = false;

			if ((doRightMove && doForwardMove) || (doLeftMove && doBackMove))
				foofoo_diag2 = true;
			else
				foofoo_diag2 = false;

			// sentry gun hack
			//
			if(unit->getUnitType()->isStationary())
			{
				foofoo_diag1 = false;
				foofoo_diag2 = false;
			}

			VC3 rot = rotation;
			float fooroty = rot.y;
			unit->setStrafeAimOffset(0);
			if (foofoo_diag1)
			{
				fooroty = rot.y-45;
				if (fooroty < 0) fooroty += 360.0f;
				assert(fooroty >= 0);
				//if (unit->getVisualObject() != NULL)
				//	unit->getVisualObject()->rotateBone(unit->getUnitType()->getAimBone(), +45);
				unit->setStrafeAimOffset(45);
			}
			if (foofoo_diag2)
			{
				fooroty = rot.y+45;
				if (fooroty >= 360) fooroty -= 360.0f;
				assert(fooroty <= 360);
				//if (unit->getVisualObject() != NULL)
				//	unit->getVisualObject()->rotateBone(unit->getUnitType()->getAimBone(), -45);
				unit->setStrafeAimOffset(-45);
			}
			unit->setStrafeRotationOffset(fooroty - rot.y);
			if (unit->getFlashlight() != NULL)
			{
				unit->getFlashlight()->setRotation(rot.y * 3.1415926f / 180.0f);
			}
			//unit->setRotation(rot.x, fooroty, rot.z);
//			rotation = VC3(rot.x, fooroty, rot.z);
		}

	}



	void UnitActor::actWeaponry(Unit *unit, UnitActAnimationRequests *animRequests,
		bool doFire, bool doMove)
	{
		UnitType *unitType = unit->getUnitType();

		VC3 position = unit->getPosition();
		VC3 rotation = unit->getRotation();

		// laser pointer thingy
		handlePointerWeapon(unit);
		// target locker thingy
		handleTargetLocker(unit);

		// weapon spread in topdown style (non-rts) game
		if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
		{
			float firingSpread = unit->getFiringSpreadFactor();
			if (firingSpread > 1.0f)
			{
				bool spreadDown = true;
				float minSpread = 1.0f;
				for (int wc = 0; wc < UNIT_MAX_WEAPONS; wc++)
				{
					if (unit->getFireReloadDelay(wc) > 0)
						spreadDown = false;
					if (unit->getWeaponType(wc) != NULL
						&& unit->isWeaponActive(wc))
					{
						if (unit->getWeaponType(wc)->getMinSpread() > minSpread)
							minSpread = unit->getWeaponType(wc)->getMinSpread();
					}
				}
				if (spreadDown)
				{
					firingSpread -= 0.002f * GAME_TICK_MSEC;
					if (firingSpread < minSpread)
						firingSpread = minSpread;
					unit->setFiringSpreadFactor(firingSpread);
				}
			}
		}

		// reload/shoot wait
		for (int weap = 0; weap < UNIT_MAX_WEAPONS; weap++)
		{
			if (unit->getWeaponType(weap) == NULL)
				continue;

			Bullet *bulletType = NULL;

			bulletType = unit->getWeaponType(weap)->getBulletType();
			if (unit->getWeaponType(weap)->getContinuousFireTime() > 0
				&& unit->getWeaponType(weap)->getContinuousFireBulletType() != NULL)
			{
				if (unit->getContinuousFireTime() > unit->getWeaponType(weap)->getContinuousFireTime())
				{
					bulletType = unit->getWeaponType(weap)->getContinuousFireBulletType();
				}
			}

			bool firstShot = false; // the very first shot
			bool doRealShot = false; // real, raytraced shot
			bool doCopyShot = false; // repeated copy shot

			int shotNumber = 0;

			int repeatNow = 1; // how many times to loop this weap shoot...

			int parallelShots = 1;

			// HACK: TEST: xenon...
			if (bulletType != NULL
				&& bulletType->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("B_Xenon1"))
			{
				parallelShots = 4;
			}

			// are we waiting for the first shot (fire wait delay)?
			if (unit->getFireWaitDelay(weap) > 0)
			{
				unit->setFireWaitDelay(weap, unit->getFireWaitDelay(weap) - 1);
				// HACK: if it was a jump attack,
				// don't make the shoot anim. only make shoot anim if
				// not jump attack...
				if (unit->getJumpCounter() == 0
					|| unitType->doesFireStopJump()
					|| unitType->doesFireWhileJumping())
				{
					animRequests->setShootAnim = true;
				}

				// done waiting, shoot first shot now...
				if (unit->getFireWaitDelay(weap) == 0)
				{
					doRealShot = true;
					firstShot = true;
					Weapon *w = unit->getWeaponType(weap);
					if (w != NULL)
					{
						// shotty style firing repeat?
						if (w->getRepeatDelay() == 0 && w->getRepeatAmount() > 1)
						{
							doCopyShot = true;
						}

						if (w->getWalkStopAtPhase() == WEAPON_WALK_STOP_AT_PHASE_FIRE)
						{
							if (unit->getWalkDelay() < w->getWalkStopTime())
								unit->setWalkDelay(w->getWalkStopTime());
						}
					}
				}
			} else {				
				int firet = unit->getWeaponFireTime(weap);
				if (firet > 0)
				{
					firet--;
					unit->setWeaponFireTime(weap, firet);
				}

				// 0 is no-longer accepted for fire time ('cos shotty with 
				// fire time of 2 would shoot 3 shots, for times 2,1,0)
				// and all others would shoot at zero time too. +1 shot.
				if (firet > 0)
				{
					// are we possibly firing right now (repeated sequences)?
					Weapon *w = unit->getWeaponType(weap);
					if (w != NULL)
					{
						// repeating weapons...
						int repeatDelay = w->getRepeatDelay();

						// shotty type (try to shoot lots of bullets immediately)
						if (repeatDelay == 0 && w->getRepeatAmount() > 1)
						{
							doRealShot = true;
							doCopyShot = true;
							repeatNow = w->getRepeatAmount();
						}

            // other repeating weapons...
						if (repeatDelay != 0 && (firet % repeatDelay) == 0)
						{
							int totalRepeats = w->getRepeatAmount();
							int repeatNum = totalRepeats - (firet / repeatDelay);
							shotNumber = repeatNum;
							int raytraces = w->getRaytraceAmount();
							if (raytraces < 1) raytraces = 1;
							if ((repeatNum % ((totalRepeats + (raytraces - 1)) / raytraces)) == 0)
							{
								doRealShot = true;
							} else {
								doCopyShot = true;
							}
						}
					}
				}

				// reloading? (after fire delay)
				int rdelay = unit->getFireReloadDelay(weap);
				if (rdelay > 0)
				{
					rdelay--;
					unit->setFireReloadDelay(weap, rdelay);
					// HACK: if it was a jump attack,
					// don't make the shoot anim.
					if (unit->getJumpCounter() == 0
						|| unitType->doesFireStopJump()
						|| unitType->doesFireWhileJumping())
					{
						animRequests->setShootAnim = true;
					}
					if (unit->getFireReloadDelay(weap) == 0)
					{
						// TODO: stop only left or right shoot animation, not both!
						//animRequests->endShootAnim = true;

						if(unit->getWeaponType(weap))
						{
							const char *soundfile = unit->getWeaponType(weap)->getReloadFinishedSound();
							if(soundfile)
							{
								VC3 pos = unit->getPosition();
								game->gameUI->playSoundEffect(soundfile, pos.x, pos.y, pos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_NORMAL);
							}
						}
					}

					// delete copy projectile if one exists
					if (rdelay == 0)
					{
						Projectile *proj = unit->getWeaponCopyProjectile(weap);
						if (proj != NULL) 
						{
							delete proj;
							unit->setWeaponCopyProjectile(weap, NULL);
						}
					}
				}
			}

			if (doRealShot || doCopyShot)
			{
				// reset launch request
				unit->setLaunchNow(false);

				// make sound...
				Weapon *w = unit->getWeaponType(weap);
				const char *fireSound;
				if (firstShot)
				{
					fireSound = w->getFireSound();
				} else {
					fireSound = w->getRepeatSound();
				}
				if (fireSound != NULL)
				{
					makeFireSound(unit, weap, fireSound);
				}

				unit->rotateWeaponBarrel(weap);

				if (w->getRepeatAmount() == 1 || w->getRepeatDelay() != 0 || firstShot)
				{
					this->createMuzzleflash(unit, weap);
					this->createEject(unit, weap);

					// fixed, this was never reset earlier when shooting, so the aim stop counter
					// was actually more or less just random.
					unit->setAimStopCounter(0);
				}

				// handle recharging weapon bullet types...
				if (unit->getWeaponType(weap)->getRechargeBulletSteps() > 0
					&& unit->getWeaponType(weap)->getRechargePeakTime() > 0)
				{
					int stepNumber = 0;

					if (unit->getWeaponRechargeSteps() > 0
						&& unit->getWeaponRechargePeak() > unit->getWeaponRechargeMin())
					{
						stepNumber = 1 + (unit->getWeaponRechargeAmount() - unit->getWeaponRechargeMin()) * unit->getWeaponRechargeSteps() / (unit->getWeaponRechargePeak() - unit->getWeaponRechargeMin());
						if (stepNumber > unit->getWeaponRechargeSteps())
							stepNumber = unit->getWeaponRechargeSteps();
					} else {
						Logger::getInstance()->error("UnitActor::actWeaponry - Erronous recharge min/peak/step values given for recharging weapon.");
						assert(!"UnitActor::actWeaponry - Erronous recharge min/peak/step values given for recharging weapon.");
					}

					const char *bullStr = bulletType->getPartTypeIdString();
					char *tmp = new char[strlen(bullStr) + 4];
					strcpy(tmp, bullStr);
					strcat(tmp, int2str(stepNumber));
					if (PARTTYPE_ID_STRING_VALID(tmp))
					{
						PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(tmp));
						if (pt != NULL)
						{
							if (pt->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
							{
								// WARNING: unsafe cast (check above)
								bulletType = (Bullet *)pt;
							} else {
								Logger::getInstance()->error("UnitActor::actWeaponry - Non-bullet type part-type id given for recharging weapon.");
								assert(!"UnitActor::actWeaponry - Non-bullet type part-type id given for recharging weapon.");
							}
						} else {
							Logger::getInstance()->error("UnitActor::actWeaponry - Given part-type id has no numbered part-types for recharging weapon.");
							assert(!"UnitActor::actWeaponry - Given part-type id has no numbered part-types for recharging weapon.");
						}
					}
					delete [] tmp;
				}
			}

			// make a real raytraced shot
			if (doRealShot)
			{
				if (unit->getWeaponType(weap) == NULL
					|| bulletType == NULL) 
				{
					Logger::getInstance()->error("ArmorUnitActor::act - Weapon or bullet type null.");
					//assert(0);
					return;
				}

				Projectile *proj = new Projectile(unit, bulletType);

				proj->criticalHitDamageMax = unit->getWeaponType(weap)->getCriticalHitDamageMax();
				proj->criticalHitDamageMultiplier = unit->getWeaponType(weap)->getCriticalHitDamageMultiplier();
				proj->criticalHitProbabilityMultiplier = unit->getWeaponType(weap)->getCriticalHitProbabilityMultiplier();
			
				VC3 dir;
				VC3 target;
				Unit *targUnit = NULL; // can't be const
				// we have a weapon position (visualization, that's where the 
				// bullet comes from) and the weapon ray position (gameplay,
				// that's where we check the hits from).
				VC3 weaponPosition = unit->getPosition();
				VC3 weaponRayPosition = unit->getPosition();
				if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
				{
					weaponPosition += VC3(0,0.3f,0);
					weaponRayPosition += VC3(0,0.4f,0);
				} else {
					weaponPosition += VC3(0,1.6f,0);
					weaponRayPosition += VC3(0,1.7f,0);
				}

				// old armor arms
				/*
				if (unit->getWeaponPosition(weap) == SLOT_POSITION_RIGHT_ARM)
				{
					float wangle = rotation.y + 90;
					if (wangle >= 360) wangle -= 360;
					//weaponPosition.x += -0.4f * sinf(UNIT_ANGLE_TO_RAD(wangle));
					//weaponPosition.z += -0.4f * cosf(UNIT_ANGLE_TO_RAD(wangle));
				}
				if (unit->getWeaponPosition(weap) == SLOT_POSITION_LEFT_ARM)
				{
					float wangle = rotation.y + 270;
					if (wangle >= 360) wangle -= 360;
					//weaponPosition.x += -0.4f * sinf(UNIT_ANGLE_TO_RAD(wangle));
					//weaponPosition.z += -0.4f * cosf(UNIT_ANGLE_TO_RAD(wangle));
				}
				*/

				// psd: disabled positions .. changed to helper

				if (unit->getWeaponType(weap)->doesFireFromWeaponBarrel())
				{
					VisualObject *unitVisual = unit->getVisualObject();
					IStorm3D_Model *unitModel = 0;
					if(unitVisual)
					{
						if(unitVisual)
							unitModel = unitVisual->getStormModel();

						if(unitModel)
						{
							// custom helper
							if(unit->getWeaponType(weap)->getCustomWeaponBarrelHelper())
							{
								bool helperFound = util::getHelperPosition(unitModel, unit->getWeaponType(weap)->getCustomWeaponBarrelHelper(), weaponPosition);
								if (!helperFound)
								{
									Logger::getInstance()->warning("UnitActor::actWeaponry - Custom weapon barrel helper not found.");
								}
							}
							else
							{
								std::string barrelName = "WeaponBarrel";
								if (unit->getRotatedWeaponBarrel(weap) >= 2)
									barrelName += int2str(unit->getRotatedWeaponBarrel(weap));

								std::string modelHelperName = std::string("HELPER_MODEL_") + barrelName;
								bool helperFound = util::getHelperPosition(unitModel, modelHelperName.c_str(), weaponPosition);
								if (!helperFound)
								{
									std::string boneHelperName = std::string("HELPER_BONE_") + barrelName;
									helperFound = util::getHelperPosition(unitModel, boneHelperName.c_str(), weaponPosition);
									if (!helperFound)
									{
										Logger::getInstance()->warning("UnitActor::actWeaponry - Weapon barrel helper not found.");
									}
								}
							}
						}
					}

					// NEW BEHAVIOUR: enemies shoot the ray from barrel...
#if defined(PROJECT_SHADOWGROUNDS) || defined(PROJECT_SURVIVOR) 
					if (!unit->isDirectControl())
					{
						weaponRayPosition = weaponPosition;
					}

					// HACK: SAME FOR PLAYER IF PINPOINT LASER
					if (unit->getWeaponType(weap)->hasNoAutoAim())
					{
						weaponRayPosition = weaponPosition;
					}
#endif
					if (unit->getWeaponType(weap)->doesRaytraceFromWeaponBarrel())
					{
						weaponRayPosition = weaponPosition;
					}

				}

				bool surprised = false;
				bool doDirectControlFire = unit->isDirectControl();

				// direct or parabolic? totally different... affects the way
				// the projectile behaves quite a bit.
				bool parabolic = false;
				if (bulletType != NULL)
				{
					if (bulletType->getFlyPath() == Bullet::FLYPATH_PARABOLIC)
					{
						parabolic = true;
					}
				}
				bool heavyWeapon = false;
				Weapon *w = unit->getWeaponType(weap);
				if (w->isHeavyWeapon())
				{
					heavyWeapon = true;
				}

				if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
				{
					// FIXME: some weird third person view requirement here too...? 
					// to keep it backward compatible, not removed, but rather just accepting sideways flag too --jpk
					if (unit->isDirectControl()
						&& (game->gameUI->isThirdPersonView(unit->getOwner()) || unit->isSideways()))
					{
						doDirectControlFire = false;
						SceneSelection *sceneSel = game->gameUI->getSceneSelection(game->gameUI->getClientNumberForUnit(unit));
						if (sceneSel->hit)
						{
							VC3 tpos;
							if (sceneSel->unit != NULL)
							{						
								unit->targeting.setTarget(sceneSel->unit);
								tpos = sceneSel->unit->getPosition();
							} else {
								//tpos = VC3(sceneSel->scaledMapX, 
								//	1.2f + game->gameMap->getScaledHeightAt(sceneSel->scaledMapX, sceneSel->scaledMapY), 
								//	sceneSel->scaledMapY);

								// NOTE: is this REALLY ok???
								// using aiming position instead of sceneselection.
								tpos = unit->targeting.getAimingPosition();
								tpos.y += SimpleOptions::getFloat(DH_OPT_F_GAME_AIM_HEIGHT_OFFSET);

								Unit *autoAimed = NULL;

								// autoaim?
								bool aimedVertical = false;
								bool aimedHorizontal = false;

								if (!w->hasNoAutoAim())
								{
									getAutoAimNear(unit, tpos, &autoAimed, &aimedVertical, &aimedHorizontal);
								}

								if (autoAimed != NULL && aimedHorizontal && aimedVertical)
								{
									// if fully autoaimed, set unit as the target
									unit->targeting.setTarget(autoAimed);
									tpos = autoAimed->getPosition();
								} else {
									// if partially (horiz/vert) autoaimed, just 
									// adjust the aim height or 2d position
									if (aimedVertical)
									{
										float ah;
										if (autoAimed->getSpeed() == Unit::UNIT_SPEED_CRAWL
											|| autoAimed->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
										{
											ah = autoAimed->getUnitType()->getAimHeightCrawling();
										} else {
											ah = autoAimed->getUnitType()->getAimHeightStanding();
										}
										tpos.y = autoAimed->getPosition().y + ah;
									}
									if (aimedHorizontal)
									{
										tpos.x = autoAimed->getPosition().x;
										tpos.z = autoAimed->getPosition().z;
									}

									unit->targeting.setTarget(tpos);
								}
							}
							if (parabolic)
							{
								VC3 dirtmp = tpos - weaponRayPosition;
								dirtmp.x = dirtmp.x * 1.5f;
								dirtmp.z = dirtmp.z * 1.5f;
								tpos = weaponRayPosition + dirtmp;
								game->gameMap->keepWellInScaledBoundaries(&tpos.x, &tpos.z);
								tpos.y = game->gameMap->getScaledHeightAt(tpos.x, tpos.z);
								unit->targeting.setTarget(tpos);
							}
						}
					}
				}

				if (w->isWeaponRayHeightFromBarrel())
				{
					weaponRayPosition.y = weaponPosition.y;
				}

				if (doDirectControlFire)
				{
					float wBetaAngle = rotation.x + unit->getLookBetaAngle();
					if (wBetaAngle >= 360) wBetaAngle -= 360;

					target = unit->getPosition() + VC3(0,1.7f,0);

					float angle = rotation.y;

					// TODO!!!
					// FIX ME: this calculation is not correct!
					// will not give correct vector when looking up or down.
					target.x += (float)(0.1f-unit->getWeaponType(weap)->getRange()) * sinf(UNIT_ANGLE_TO_RAD(angle));
					target.y += (float)(0.1f-unit->getWeaponType(weap)->getRange()) * sinf(UNIT_ANGLE_TO_RAD(wBetaAngle));
					target.z += (float)(0.1f-unit->getWeaponType(weap)->getRange()) * cosf(UNIT_ANGLE_TO_RAD(angle));

					dir = target - weaponRayPosition;
				} else {
					// TODO: && unit->isSweepFiring()
					if (unit->isFiringInProgress()
						&& unit->targeting.hasSweepTargetPosition()
						&& shotNumber > 0)
					{
						targUnit = unit->targeting.getTargetUnit();
						if (targUnit != NULL)
						{
							// weighted avg based on sweep position and real target position
							float nonSweepWeight = 0.8f * (shotNumber / (float)w->getRepeatAmount());
							if (unit->targeting.getFireSweepsSinceTarget() > 1)
							{
								nonSweepWeight += 0.6f;
								if (nonSweepWeight > 1.0f)
								{
									nonSweepWeight = 1.0f;
								}
							}
							target = unit->targeting.getSweepTargetPosition() * (1.0f - nonSweepWeight);
							target += ((targUnit->getPosition() + VC3(0,targUnit->getUnitType()->getAimHeightStanding(),0)) * nonSweepWeight);
						} else {
							target = unit->targeting.getSweepTargetPosition();
						}
					} else {
						targUnit = unit->targeting.getTargetUnit();
						if (targUnit != NULL)
						{
							// shoot a bit lower if target unconscious
							// or crawling
							// adding a little random to it too...
							int ra = (game->gameRandom->nextInt() & 7);
							if (((targUnit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS
								|| targUnit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
								&& ra != 0)
								|| ra == 1)
								target = targUnit->getPosition() + VC3(0,targUnit->getUnitType()->getAimHeightCrawling(),0);
							else
								target = targUnit->getPosition() + VC3(0,targUnit->getUnitType()->getAimHeightStanding(),0);

							if (targUnit->getUnitType()->doesUseAimpointHelper())
							{
								// HACK: ...
								if (targUnit->getVisualObject() != NULL
									&& targUnit->getVisualObject()->getStormModel() != NULL)
								{
									VC3 tmp = target;
									bool helperFound = util::getHelperPosition(targUnit->getVisualObject()->getStormModel(), "HELPER_BONE_Aimpoint", tmp);
									if (helperFound)
									{
										target = tmp;
									} else {
										LOG_ERROR_W_DEBUG("UnitActor::actWeaponry - No aim point helper found in target unit model.", targUnit->getUnitType()->getName());
									}
								}
							}

							// a crappy check - did we surprise the target?
							if (game->isHostile(targUnit->getOwner(), unit->getOwner())
								&& targUnit->getSeeUnit() == NULL
								&& !targUnit->targeting.hasTarget()
								&& targUnit->getSpottedScriptDelay() == 0)
								surprised = true;

						} else {
							target = unit->targeting.getTargetPosition();
						}
					}
					dir = target - weaponRayPosition;
				}

				dir.Normalize();

#ifndef PROJECT_SHADOWGROUNDS
				// beta angle rotation for aimbone (mech)
				//
				float aimspeed = unitType->getAimBetaRotSpeed();
				float aimacc = aimspeed * unitType->getAimBetaRotAccuracy();
				if(aimspeed > 0 && aimacc > 0)
				{
					VC3 targetAtSameLevel(target.x, weaponRayPosition.y, target.z);

					VC3 targVec = target - weaponRayPosition;
					VC3 flatTargVec = targetAtSameLevel - weaponRayPosition;

					float betaAngle = RAD_TO_UNIT_ANGLE(flatTargVec.GetAngleTo(targVec));
					if(targVec.y < weaponRayPosition.y) betaAngle = 360.0f - betaAngle;

					float lastAngle = unit->getLastBoneAimBetaAngle();

					float turndir = util::AngleRotationCalculator::getRotationForAngles(lastAngle, betaAngle, aimacc);
					betaAngle = lastAngle + aimspeed * turndir;
					if(betaAngle > 360.0f) betaAngle -= 360.0f;
					if(betaAngle < 0.0f) betaAngle += 360.0f;

					float max = unitType->getAimBetaRotLimit();
					float min = 360.0f - unitType->getAimBetaRotLimit();
					if(betaAngle < min)
					{
						betaAngle = min;
					}
					else if(betaAngle < max)
					{
						betaAngle = max;
					}

					unit->setLastBoneAimBetaAngle(betaAngle);
				}

			// sg does this later on (which is stupid and erronous) but not going to change that to avoid breaking sg
				if (w->getShootDirectionLimit() > 0.0f && w->getShootDirectionLimit() < (180.0f - 0.001f))
				{
					// horiz only limit proto
				  /*
					VC3 flatTargVec = flattenedTarget - weaponRayPosition;
					VC3 horizTargVec = flattenedTarget - weaponRayPosition;
					horizTargVec.y = 0;

					float horizDiffAngle = flatTargVec.GetAngleTo(horizTargVec);
					float limitRad = w->getShootDirectionLimit() * 3.1415f / 180.0f;

					if (horizDiffAngle > limitRad)
					{
						float limitVecY = horizTargVec.GetLength() * atanf(limitRad);
					}
					*/

					// NOTE: some modified COPY&PASTE from DecalPositionCalculator::rotateToward
					VC3 rot = unit->getRotation();

					VC3 a = target - weaponRayPosition;
					float aLen = a.GetLength();
					float xdir = -sinf(UNIT_ANGLE_TO_RAD(rot.y + unit->getLastBoneAimDirection()));
					float zdir = -cosf(UNIT_ANGLE_TO_RAD(rot.y + unit->getLastBoneAimDirection()));
					VC3 b = VC3(xdir, 0, zdir);

					VC3 axis = a.GetNormalized().GetCrossWith(b.GetNormalized());
					axis.Normalize();

					float angle = a.GetAngleTo(b);

					float limitRad = w->getShootDirectionLimit() * 3.1415f / 180.0f;
					if (fabs(angle) > limitRad)
					{
						if (angle < 0)
						{
							angle = -limitRad;
						} else {
							angle = limitRad;
						}

						QUAT q;
						q.MakeFromAxisRotation(axis, angle);

						q.RotateVector(b);

						target = weaponRayPosition + b * aLen;
						dir = b.GetNormalized();
					} else {
						target = weaponRayPosition + a;
						dir = a.GetNormalized();
					}
				}
				else if(w->getShootDirectionLimit() == 0.0f)
				{
					// just shoot where ever pointing
					VC3 rot = unit->getRotation();

					QUAT q;
					q.MakeFromAngles(UNIT_ANGLE_TO_RAD(unit->getLastBoneAimBetaAngle()), UNIT_ANGLE_TO_RAD(rot.y + unit->getLastBoneAimDirection()), 0);

					VC3 a = target - weaponRayPosition;
					float aLen = a.GetLength();

					dir = VC3(0,0,-1);
					q.RotateVector(dir);
					dir.Normalize();

					target = weaponRayPosition + dir * aLen;
				}

#endif

				float accavg = (float)unitType->getAimingAccuracy();
				float charAimRatio = 0;
				if (unit->getCharacter() != NULL)
				{
					charAimRatio = unitType->getCharacterAimRatio();
					accavg += (float)unit->getCharacter()->getSkillAmount(CHAR_SKILL_AIMING)
						* charAimRatio;
				}

				// when moving, accuracy is worse than when standing still.
				// except for crawling units
				if (doMove && unit->getSpeed() != Unit::UNIT_SPEED_CRAWL)
					accavg += (float)unit->getWeaponType(weap)->getLowAccuracy()
						* unitType->getWeaponAimRatio();
				else
					accavg += (float)unit->getWeaponType(weap)->getAccuracy()
						* unitType->getWeaponAimRatio();

				accavg /= (1.0f + charAimRatio + unitType->getWeaponAimRatio());

				int acc = 100 - (int)accavg;
				if (acc < 0) acc = 0;

				if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
				{
					// umm... some really shitty hacks here, but keeping them to be backward compatible... --jpk

					// HACK!
					if (heavyWeapon)
						acc /= 3;

					if (unit->isDirectControl()
						&& game->gameUI->isThirdPersonView(unit->getOwner())
						&& !parabolic && !heavyWeapon)
					{
						acc = (int)((float)acc * unit->getFiringSpreadFactor());
					}
				}

				// surprise accuracy bonus... same for direct control
				// but not for parabolic path
				if ((surprised || unit->isDirectControl()) && !parabolic)
					acc /= 2;

				// if weapon needs recon, and that is not available, make
				// it twice as inaccurate as it normally would.
				if (w->doesNeedRecon())
				{
					if (unit->getReconValue() > 0
						|| unit->targeting.hasLineOfFireToTarget() 
						|| ReconChecker::isReconAvailableAtPosition(game, unit->getOwner(), target))
					{
						unit->setReconAvailableFlag(true);
					} else {
						unit->setReconAvailableFlag(false);
						acc *= 2;
					}
				}

				if (bulletType != NULL)
				{
					// note: negated here, as acc is already reversed (0 accurate, 100 inaccurate)
					// so a value of 100 accuracy change will make more accurate (acc to 0)
					// and a value of -100 accuracy change will make less accurate (acc to 100+)
					acc -= bulletType->getAccuracyChange();
					if (acc < 0) acc = 0;
				}

				if (parabolic)
				{
					int accMod = acc * 2 + 1;
					target.x += (float)((game->gameRandom->nextInt() % accMod) - acc) / 4.0f;
					target.z += (float)((game->gameRandom->nextInt() % accMod) - acc) / 4.0f;
					 
					if (game->gameMap->isWellInScaledBoundaries(target.x, target.z))
					{
						//parabolicHitHeight = game->gameMap->getScaledHeightAt(target.x, target.z);
						float tmph = game->gameMap->getScaledHeightAt(target.x, target.z);
						target.y = tmph;
					}
				} else {
					int accMod = acc * 2 + 1;

					dir.x += ((float)((game->gameRandom->nextInt() % accMod) - acc)) / 1000.0f;
					dir.y += ((float)((game->gameRandom->nextInt() % accMod) - acc)) / 1000.0f;
					dir.z += ((float)((game->gameRandom->nextInt() % accMod) - acc)) / 1000.0f;
				}
				dir.Normalize();

				float projectileRange = w->getProjectileRange();

				VC3 flattenedTarget = target;
				if (w->getFlattenShootDirection() > 0.0f && w->getFlattenShootDirection() <= 1.0f)
				{
					if (targUnit == NULL)
					{
						float fact = w->getFlattenShootDirection();
						flattenedTarget.y = target.y * (1.0f - fact) + weaponRayPosition.y * fact;
						// TODO: SHOULD ACTUALLY CALCULATE THIS CRAP EARLIER (AND PROPERLY)
						dir.y *= (1.0f - fact);
						dir.Normalize();
					}
				}
				if (w->doesSweep())
				{

					if (shotNumber == 0)
					{
						unit->targeting.setSweepTargetPosition(target);
						if ((game->gameRandom->nextInt() & 1) == 0)
						{
							unit->setFireSweepDirection(1);
						} else {
							unit->setFireSweepDirection(-1);
						}
					}

					float angle = (float)w->getSweepAngle() * ((shotNumber / (float)w->getRepeatAmount()) - 0.5f);

					if (unit->getFireSweepDirection() == -1)
					{
						angle = -angle;
					}

					// HACK: sweep only once
					if (unit->targeting.getFireSweepsSinceTarget() > 1)
					{
						angle /= 10.0f;
					}

					// change target position first...

					VC3 targdistVec = flattenedTarget - weaponRayPosition;
					float targdist = targdistVec.GetLength();
					if (targdist > 0.001f)
					{
						//float angle = PositionDirectionCalculator::calculateDirection(weaponRayPosition, flattenedTarget);
						// TODO: sweep "field of fire"
						VC3 newtarg = targdistVec.GetNormalized();

						float origx = newtarg.x;
						newtarg.x = origx * cosf(angle * 3.1415927f / 180) + newtarg.z * sinf(angle * 3.1415927f / 180);
						newtarg.z = newtarg.z * cosf(angle * 3.1415927f / 180) - origx * sinf(angle * 3.1415927f / 180);

						newtarg *= targdist;

						//flattenedTarget = weaponRayPosition + newtarg;
					}

					// and change the direction too...
					{
						//float angle = PositionDirectionCalculator::calculateDirection(weaponRayPosition, flattenedTarget);
						// TODO: sweep "field of fire"
						VC3 newdir = dir;

						float origx = newdir.x;
						newdir.x = origx * cosf(angle * 3.1415927f / 180) + newdir.z * sinf(angle * 3.1415927f / 180);
						newdir.z = newdir.z * cosf(angle * 3.1415927f / 180) - origx * sinf(angle * 3.1415927f / 180);

						dir = newdir;

						assert(fabs(dir.GetLength() - 1.0f) < 0.01f);
					}

					// HACK: "lag" behind player 
					if (targUnit != NULL)
					{
						VC3 tvel = targUnit->getVelocity();
						if (tvel.GetLength() > 0.001f)
						{
							tvel.Normalize();
							tvel *= 0.06f;
							dir -= tvel;
							dir.Normalize();
						}
					}

					flattenedTarget = weaponRayPosition + (dir * targdist);
					targUnit = NULL;
				}


#ifdef PROJECT_SHADOWGROUNDS
				if (w->getShootDirectionLimit() > 0.0f && w->getShootDirectionLimit() < (180.0f - 0.001f))
				{
					// horiz only limit proto
				  /*
					VC3 flatTargVec = flattenedTarget - weaponRayPosition;
					VC3 horizTargVec = flattenedTarget - weaponRayPosition;
					horizTargVec.y = 0;

					float horizDiffAngle = flatTargVec.GetAngleTo(horizTargVec);
					float limitRad = w->getShootDirectionLimit() * 3.1415f / 180.0f;

					if (horizDiffAngle > limitRad)
					{
						float limitVecY = horizTargVec.GetLength() * atanf(limitRad);
					}
					*/

					// NOTE: some modified COPY&PASTE from DecalPositionCalculator::rotateToward

					VC3 rot = unit->getRotation();

					VC3 a = flattenedTarget - weaponRayPosition;
					float aLen = a.GetLength();
					float xdir = -sinf(UNIT_ANGLE_TO_RAD(rot.y + unit->getLastBoneAimDirection()));
					float zdir = -cosf(UNIT_ANGLE_TO_RAD(rot.y + unit->getLastBoneAimDirection()));
					VC3 b = VC3(xdir, 0, zdir);

					VC3 axis = a.GetNormalized().GetCrossWith(b.GetNormalized());
					axis.Normalize();

					float angle = a.GetAngleTo(b);

					float limitRad = w->getShootDirectionLimit() * 3.1415f / 180.0f;
					if (fabs(angle) > limitRad)
					{
						if (angle < 0)
						{
							angle = -limitRad;
						} else {
							angle = limitRad;
						}

						QUAT q;
						q.MakeFromAxisRotation(axis, angle);

						q.RotateVector(b);

						flattenedTarget = weaponRayPosition + b * aLen;
						dir = b.GetNormalized();
					} else {
						flattenedTarget = weaponRayPosition + a;
						dir = a.GetNormalized();
					}
				}
#endif

				if (parallelShots > 1)
				{
					// HACK: ...
					float posOff = 0.17f * (shotNumber % parallelShots) - (0.12f * parallelShots / 2) ;
					if (unit->getLastRotationDirection() > 0)
					{
						posOff = -posOff;
					}
					VC3 posOffVec = VC3(0,0,0);

					float uangle = unit->getRotation().y + 90;

					posOffVec.x = posOff * sinf(UNIT_ANGLE_TO_RAD(uangle));
					posOffVec.z = posOff * cosf(UNIT_ANGLE_TO_RAD(uangle));

					weaponPosition += posOffVec;
					weaponRayPosition += posOffVec;
					flattenedTarget += posOffVec;
				}

				float velocityFactor = 1.0f;
				if(w->usesLaunchSpeed())
				{
					velocityFactor = 1.0f + unit->getLaunchSpeed();
				}

				ProjectileActor pa = ProjectileActor(game);
				pa.doProjectileRaytrace(unit, unit, proj, bulletType, weaponPosition,
					weaponRayPosition, flattenedTarget, dir, projectileRange, targUnit, velocityFactor);

				unit->setAniRecordFirePosition(weaponPosition, proj->getDestination());

				if (bulletType->doesConnectToParent())
				{
					proj->setParentUnit(unit);
				}

				if (bulletType->doesParentToNextBullet())
				{
					LinkedList *projlist = game->projectiles->getAllProjectiles();
					LinkedListIterator iter(projlist);
					while (iter.iterateAvailable())
					{
						Projectile *otherproj = (Projectile *)iter.iterateNext();
						// TODO: make this projectile parent to previously fired projectile
						// (if firing was not interrupted - there may be some variable for this? for minigun startup i think)
						if (otherproj->getParentUnit() == unit
							&& otherproj->getBulletType() == bulletType
							&& otherproj != proj)
						{
							otherproj->setParentProjectile(proj);
							break;
						}
					}
				}


				// make this as a copy projectile or just use it directly if
				// there is no need to repeat.
				//if (firstShot)
				if (doRealShot)
				{
					Projectile *copyProj;
					if (unit->getWeaponType(weap)->getRepeatAmount() > 1)
					{
						Projectile *oldProj = unit->getWeaponCopyProjectile(weap);
						if (oldProj != NULL) 
						{
							//assert(0);
							delete oldProj;
							unit->setWeaponCopyProjectile(weap, NULL);
						}
						//assert(unit->getWeaponCopyProjectile(weap) == NULL);
						unit->setWeaponCopyProjectile(weap, proj);
						copyProj = proj->getCopy();
					} else {
						copyProj = proj;
					}

					game->projectiles->addProjectile(copyProj);
					ProjectileActor pa = ProjectileActor(game);
					pa.createVisualForProjectile(copyProj);

					// HACK: 
					if (!firstShot)
					{
						if (unit->getWeaponType(weap)->getRepeatDelay() == 0)
						{
							copyProj->setHitSound(false);
						}					
					}
				}
			}

			// do a copy shot
			//if (!firstShot && (doRealShot || doCopyShot))
			for (int rep = 0; rep < repeatNow; rep++)
			{
				if (doCopyShot)
				{
					Projectile *proj = unit->getWeaponCopyProjectile(weap);
					if (proj == NULL)
					{
						Logger::getInstance()->error("UnitActor::actWeaponry - Null projectile to copy (possibly weapon repeat amount and delay too big).");
						assert(!"Null projectile to copy (see log).");
						return;
					}

					Projectile *copyProj = proj->getCopy();

					// a copy projectile will not inflict damage.
					copyProj->setInflictDamage(false);

					game->projectiles->addProjectile(copyProj);

					// make it spread...
					float spreadAmount = (float)unit->getWeaponType(weap)->getRepeatSpread();
					VC3 spreadVec = VC3(0,0,0);
					// (a spread of 100 means a maximum offset of one meter)
					// (this is for 100 meter distance, spread scaled to match that
					// on any other distance)
					spreadVec.x += spreadAmount * (float)((game->gameRandom->nextInt() % 201) - 100) / (100.0f * 100.0f);
					spreadVec.y += spreadAmount * (float)((game->gameRandom->nextInt() % 201) - 100) / (100.0f * 100.0f);
					spreadVec.z += spreadAmount * (float)((game->gameRandom->nextInt() % 201) - 100) / (100.0f * 100.0f);
					copyProj->makeSpread(spreadVec);

					ProjectileActor pa = ProjectileActor(game);
					pa.createVisualForProjectile(copyProj);

					// HACK: 
					if (unit->getWeaponType(weap)->getRepeatDelay() == 0)
					{
						copyProj->setHitSound(false);
					}					
				}
			}
		}

		unit->setPosition(position);
		unit->setRotation(rotation.x, rotation.y, rotation.z);
	}


	void UnitActor::actCollisions(Unit *unit, UnitActAnimationRequests *animRequests,
		VC3 oldPosition)
	{
		UnitType *unitType = unit->getUnitType();

		//float scaleX = game->gameMap->getScaleX() / GAMEMAP_PATHFIND_ACCURACY;
		//float scaleY = game->gameMap->getScaleY() / GAMEMAP_PATHFIND_ACCURACY;
		float scaleX = game->gameMap->getScaleX() / GAMEMAP_HEIGHTMAP_MULTIPLIER;
		float scaleY = game->gameMap->getScaleY() / GAMEMAP_HEIGHTMAP_MULTIPLIER;

		int collRadius = unitType->getCollisionCheckRadius();

		//if (!unit->isGhostOfFuture())
		{
			if (collRadius > 0 && 
				(!unit->isDestroyed() || unitType->isBlockIfDestroyed())
				&& unit->doesCollisionCheck())
			{
				// first, let's hax a collision with all _friendly_ units...
				// CHANGED: collision with nearby friendly units...

				VC3 ownPos = unit->getPosition();

				//LinkedList *fulist = game->units->getOwnedUnits(unit->getOwner());
				//LinkedListIterator iter(fulist);
				
				// HACK: 10m radius				
				IUnitListIterator *iter = game->units->getNearbyOwnedUnits(unit->getOwner(), ownPos, 10.0f);

				assert(scaleX == scaleY);
				float checkRad = (float)(unitType->getCollisionCheckRadius() - 1) * scaleX;
				while (iter->iterateAvailable())
				{
					//Unit *u = (Unit *)iter.iterateNext();
					Unit *u = iter->iterateNext();
					UnitType *ut2 = u->getUnitType();

					//if (!u->isGhostOfFuture())
					{
						if (u != unit && u->doesCollisionBlockOthers()
							&& u->isActive() && (!u->isDestroyed() || ut2->isBlockIfDestroyed())
							&& !ut2->isLineBlock() && ut2->getBlockRadius() > 1
							&& ut2->getSize() >= unitType->getSize())
						{
							VC3 diff = u->getPosition() - ownPos;
							diff.y = 0;
							if (fabs(diff.x) > 0.001f || fabs(diff.z) > 0.001f)
							{
								float blockRad = (ut2->getBlockRadius() - 1) * scaleX;
								float totalRad = blockRad + checkRad;
								float diffLenSq = diff.GetSquareLength();
								if (diffLenSq < totalRad * totalRad)
								{
									float intrusionDepth = totalRad - sqrtf(diffLenSq);
									diff.Normalize();
									ownPos -= diff * intrusionDepth;
								}
							}
						}
					}
				}

				delete iter; 

				unit->setPosition(ownPos);
			}

			// if this is player, get pushed by "pushplayer" units...
			if (unit->isDirectControl() && !unit->getUnitType()->hasMechControls())
			{
				// NOTE: COPY & PASTED (AND MODIFIED) FROM ABOVE
				VC3 ownPos = unit->getPosition();

				LinkedList *fulist = game->units->getAllUnits();
				LinkedListIterator iter(fulist);
				assert(scaleX == scaleY);
				float checkRad = (float)(unitType->getCollisionCheckRadius() - 1) * scaleX;
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					UnitType *ut2 = u->getUnitType();

					//if (!u->isGhostOfFuture())
					{
						if (ut2->doesPushPlayer())
						{
							if (u != unit && u->doesCollisionBlockOthers()
								&& u->isActive() && (!u->isDestroyed() || ut2->isBlockIfDestroyed())
								&& !ut2->isLineBlock() && ut2->getBlockRadius() > 1
								&& ut2->getSize() >= unitType->getSize())
							{
								VC3 diff = u->getPosition() - ownPos;
								diff.y = 0;
								if (fabs(diff.x) > 0.001f || fabs(diff.z) > 0.001f)
								{
									float blockRad = (ut2->getBlockRadius() - 1) * scaleX;
									float totalRad = blockRad + checkRad;
									float diffLenSq = diff.GetSquareLength();
									if (diffLenSq < totalRad * totalRad)
									{
										float intrusionDepth = totalRad - sqrtf(diffLenSq);
										diff.Normalize();
										ownPos -= diff * intrusionDepth;
									}
								}
							}
						}
					}
				}
				unit->setPosition(ownPos);
			}
		}

		// then the actual collisions

		if (collRadius > 0)
		{
			VC3 realMidPosition = unit->getPosition();
			VC3 oldOldPosition = oldPosition;

			//bool midColl = false;
			//VC3 midMove = newMidPos - realMidPosition;

			if (collRadius > 1 && !unit->isDestroyed()
				&& unit->doesCollisionCheck() && !unit->getUnitType()->hasMechControls())
			{
				collRadius--;
				// NOTE: assuming pathfind accuracy == obstacle map multiplier
				VC3 realPosition = unit->getPosition();
				VC3 realOldPosition = oldPosition;
				VC3 moveSum = VC3(0,0,0);
				removeUnitObstacle(unit);
				//bool isGhost = unit->isGhostOfFuture();
				for (int ty = -collRadius; ty <= collRadius; ty++)
				{
					for (int tx = -collRadius; tx <= collRadius; tx++)
					{
						// skip mid pos (and outer corners)
						// HACK: do not check outer corners. is that good?
						if ((tx != 0 || ty != 0)
							&& !((tx == collRadius || tx == -collRadius)
							&& (ty == collRadius || ty == -collRadius)))
						{
							VC3 pos = realPosition;
							pos.x += tx * scaleX;
							pos.z += ty * scaleY;
							//if (game->getGameScene()->isBlockedAtScaled(pos.x, pos.z, game->gameMap->getScaledHeightAt(pos.x, pos.y))
							//	&& (!isGhost || !game->gameMap->isMovingObstacle(game->gameMap->scaledToPathfindX(pos.x), game->gameMap->scaledToPathfindX(pos.z))))
							if (game->getGameScene()->isBlockedAtScaled(pos.x, pos.z, game->gameMap->getScaledHeightAt(pos.x, pos.y)))
							{
								if (tx < 0)
									moveSum.x += scaleX * 0.20f;
								if (tx > 0)
									moveSum.x -= scaleX * 0.20f;
								if (ty < 0)
									moveSum.z += scaleY * 0.20f;
								if (ty > 0)
									moveSum.z -= scaleY * 0.20f;
							}
							
							/*
							VC3 pos = realPosition;
							pos.x += tx * scaleX;
							pos.z += ty * scaleY;
							VC3 oldPos = realOldPosition;
							oldPos.x += tx * scaleX;
							oldPos.z += ty * scaleY;
							unit->setPosition(pos);
							//VC3 realVelocity = unit->getVelocity();
							actCollisionsImpl(unit, animRequests, oldPos);
							//unit->setVelocity(realVelocity);
							VC3 newPos = unit->getPosition();
							if (tx != 0 || ty != 0)
							{
								newPos.y = pos.y;
							}
							if (tx > 0 && newPos.x > pos.x)
							{
								newPos.x = pos.x;
							}
							else if (tx < 0 && newPos.x < pos.x)
							{
								newPos.x = pos.x;
							}
							if (ty > 0 && newPos.z > pos.z)
							{
								newPos.z = pos.z;
							}
							else if (ty < 0 && newPos.z < pos.z)
							{
								newPos.z = pos.z;
							}

							VC3 newMoveSum = (newPos - pos);
							if (newMoveSum.GetSquareLength() > moveSum.GetSquareLength())
							{
								moveSum = newMoveSum;
							}
							//moveSum += (newPos - pos);
							*/
						}
					}
				}
				addUnitObstacle(unit);
				realPosition += moveSum;
				unit->setPosition(realPosition);

				oldPosition = unit->getPosition();
			}

			// TODO: should actually collide with other ghosts...
			//if (!unit->isGhostOfFuture())
			{
				actCollisionsImpl(unit, animRequests, oldOldPosition);
			}

			unit->addPositionToBacktrack(unit->getPosition(), oldOldPosition);

			// NOTE: should be based on backtracking values (interval*amount)
			float movTreshold = 0.5f;
			if (!unit->hasMovedSinceOldestBacktrack(movTreshold)
				&& unit->hasAttemptedToMoveAllBacktrackTime())
			{
				VC3 position = unit->getPosition();
				unit->setPath(NULL);
				unit->setFinalDestination(position);
				unit->setWaypoint(position);
				game->gameUI->setPointersChangedFlag(unit->getOwner());				

				stopJumpBecauseCollided(unit);
			}

		} // if("collision radius > 0 and stuff check")

	}

// how much colliding with wall affects velocity. 
// (new velocity factor, 1.0f = no effect, 0 = immediate stop)
#ifdef GAME_SIDEWAYS
#define UA_WALL_VEL_FACTOR 1.00f
#else
#define UA_WALL_VEL_FACTOR 0.75f
#endif

	void UnitActor::actCollisionsImpl(Unit *unit, UnitActAnimationRequests *animRequests,
		VC3 oldPosition)
	{
		UnitType *unitType = unit->getUnitType();

		VC3 position = unit->getPosition();

		int obstX = game->gameMap->scaledToObstacleX(position.x);
		int obstY = game->gameMap->scaledToObstacleY(position.z);

		int obstSizeX = game->gameMap->getObstacleSizeX();
		int obstSizeY = game->gameMap->getObstacleSizeY();

		// HORRIBLE OPTIMIZATION HACKS AHEAD...

		moveUnitObstacle(unit, obstX, obstY);

		if (obstX <= 0 || obstY <= 0 || obstX >= obstSizeX - 1 || obstY >= obstSizeY - 1)
			return;

		float unit_obst_height;
		int unit_obst_amount;
		int unit_obst_amount_near;
		float unit_obst_height_near;
		if (unit->obstacleExists)
		{
			unit_obst_amount = 1;
		  unit_obst_height = UNIT_OBSTACLE_HEIGHT;
		} else {
			unit_obst_amount = 0;
			unit_obst_height = 0;
		}
		
		// adjacent blocks have unit's obstacle?
		if (unitType->getBlockRadius() > 1)
		{
			unit_obst_amount_near = unit_obst_amount;
			unit_obst_height_near = unit_obst_height;
		} else {
			unit_obst_amount_near = 0;
			unit_obst_height_near = 0;
		}

		float extraheight = 0.02f;  
		// 2 cm extra to check height (because of possible rounding inaccur.)

		int oldObstX = game->gameMap->scaledToObstacleX(oldPosition.x);
		int oldObstY = game->gameMap->scaledToObstacleY(oldPosition.z);

		// WARNING: assumes GAMEMAP_PATHFIND_ACCURACY == OBSTACLEMAP_SIZE_MULT
		float tooSlope = false;
		int maxSlope = (int)(1.0f / game->gameMap->getScaleHeight()); // max _heightmap_ diff.
		if (unit->isDirectControl()
			&& abs(game->gameMap->getHeightmapHeightAt(obstX / GAMEMAP_PATHFIND_ACCURACY, obstY / GAMEMAP_PATHFIND_ACCURACY)
			- game->gameMap->getHeightmapHeightAt(oldObstX / GAMEMAP_PATHFIND_ACCURACY, oldObstY / GAMEMAP_PATHFIND_ACCURACY)) > maxSlope)
		{
			tooSlope = true;
		}

		if (!unitType->isFlying()
			&& unit->doesCollisionCheck()
			&& (game->getGameScene()->getConditionalBlockingCountForUnit(obstX, obstY, position.y 
			+ unit_obst_height + extraheight) > unit_obst_amount
			|| tooSlope))
		{
			// are we on top of some obstacle?

			// NEW BEHAVIOUR: FOR DESTROYED UNITS ONLY!!!
			// Only destroyed units can be on top of obstacles!

			if (unit->isDestroyed()
				&& oldPosition.y > position.y 
				&& game->getGameScene()->getConditionalBlockingCountForUnit(obstX, obstY, oldPosition.y 
				+ unit_obst_height + extraheight) <= unit_obst_amount
				&& !tooSlope)
			{
				// yeah. just hold height.
				position.y = oldPosition.y;

				// and slide/walk to some direction if not already moving...
				VC3 velocity = unit->getVelocity();
				if (fabs(velocity.x) < 0.1f && fabs(velocity.z) < 0.1f
					&& !unit->isDestroyed()
					&& unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
				{
					if (velocity.y > 0) 
					{
						velocity.y = 0;
					}

					VC3 rotation = unit->getRotation();

					removeUnitObstacle(unit);

					// FIXME: is this supposed to be / GAME_TICKS_PER_SECOND or what...???
					//float vel = unit->getUnitType()->getMaxSpeed() / 100.0f;
					float vel = unit->getUnitType()->getMaxSpeed() / GAME_TICKS_PER_SECOND;
					vel = -vel;
					velocity.x = vel * sinf(UNIT_ANGLE_TO_RAD(rotation.y));
					velocity.z = vel * cosf(UNIT_ANGLE_TO_RAD(rotation.y));

					position.x += velocity.x;
					position.z += velocity.z;

					unit->setVelocity(velocity);

					obstX = game->gameMap->scaledToObstacleX(position.x);
					obstY = game->gameMap->scaledToObstacleY(position.z);
					// recheck for collision with a higher obstacle.
					if (game->getGameScene()->getBlockingCount(obstX, obstY, position.y 
						+ 0 + extraheight) > 0)
					{
						position.x = oldPosition.x;
						position.z = oldPosition.z;
						obstX = game->gameMap->scaledToObstacleX(position.x);
						obstY = game->gameMap->scaledToObstacleY(position.z);

						// darn, collided with a higher obstacle, try to turn...
						rotation.y += unit->getUnitType()->getTurning() / 100.0f;
						if (rotation.y >= 360.0f) rotation.y -= 360.0f;
						unit->setRotation(rotation.x, rotation.y, rotation.z);
					}

					addUnitObstacle(unit);

					animRequests->setStandAnim = false;
					animRequests->setMoveAnim = true;

					moveUnitObstacle(unit, obstX, obstY);
				}
			} else {
				// nope. just "bounce" the unit back to previous position.
				// or maybe we want to "slide" along it...
				// see which part of the obstacle the movement line intersects.
				bool intersectWestWall = false;
				bool intersectEastWall = false;
				bool intersectSouthWall = false;
				bool intersectNorthWall = false;

				//int oldObstX = game->gameMap->scaledToObstacleX(oldPosition.x);
				//int oldObstY = game->gameMap->scaledToObstacleY(oldPosition.z);
				if (oldObstX == obstX - 1 && oldObstY == obstY) 
					intersectWestWall = true;
				if (oldObstX == obstX + 1 && oldObstY == obstY) 
					intersectEastWall = true;
				// NOTE: south and north may be wrong (may need to swap).
				if (oldObstY == obstY - 1 && oldObstX == obstX) 
					intersectNorthWall = true;
				if (oldObstY == obstY + 1 && oldObstX == obstX) 
					intersectSouthWall = true;

				// from south west?
				if (oldObstY == obstY + 1 && oldObstX == obstX - 1) 
				{
					// blocked at south?
					if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX, obstY, position.y 
						+ unit_obst_height_near + extraheight) > unit_obst_amount_near)
					{
						intersectSouthWall = true;
					} else {
						intersectWestWall = true;
					}
				}
				// from south east?
				if (oldObstY == obstY + 1 && oldObstX == obstX + 1) 
				{
					// blocked at south?
					if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX, obstY, position.y 
						+ unit_obst_height_near + extraheight) > unit_obst_amount_near)
					{
						intersectSouthWall = true;
					} else {
						intersectEastWall = true;
					}
				}
				// from north west?
				if (oldObstY == obstY - 1 && oldObstX == obstX - 1) 
				{
					if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX, obstY, position.y 
						+ unit_obst_height_near + extraheight) > unit_obst_amount_near)
					{
						intersectNorthWall = true;
					} else {
						intersectWestWall = true;
					}
				}
				// from north east?
				if (oldObstY == obstY - 1 && oldObstX == obstX + 1) 
				{
					if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX, obstY, position.y 
						+ unit_obst_height_near + extraheight) > unit_obst_amount_near)
					{
						intersectNorthWall = true;
					} else {
						intersectEastWall = true;
					}
				}


				// if diagonal, don't use regular 90 degree checks
				bool diagBlock = false;
				// already moved diagonally, don't do it again?
				bool diagMoved = false;

				if (unit->isDirectControl() && !tooSlope)
				{
					// HACK: HAX HAX!!!
					// 45 degree blocking...
					bool curIsBlocked = false;
					if (game->getGameScene()->getConditionalBlockingCountForUnit(obstX, obstY, position.y 
						+ unit_obst_height_near + extraheight) > unit_obst_amount_near
						&& game->gameMap->isRoundedObstacle(obstX, obstY))
						curIsBlocked = true;

					if (curIsBlocked)
					{
						if (obstX > 1 && obstY > 1 
							&& obstX < game->gameMap->getObstacleSizeX() - 1
							&& obstY < game->gameMap->getObstacleSizeY() - 1)
						{
							bool bn = false; // blocked north
							bool bs = false; // south
							bool be = false; // east
							bool bw = false; // west
							/*
							if (game->getGameScene()->getConditionalBlockingCountForUnit(obstX, obstY - 1, position.y 
								+ unit_obst_height_near + extraheight) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(obstX, obstY - 1))
								bn = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(obstX, obstY + 1, position.y 
								+ unit_obst_height_near + extraheight) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(obstX, obstY + 1))
								bs = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(obstX - 1, obstY, position.y 
								+ unit_obst_height_near + extraheight) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(obstX - 1, obstY))
								bw = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(obstX + 1, obstY, position.y 
								+ unit_obst_height_near + extraheight) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(obstX + 1, obstY))
								be = true;
							*/
							if (game->getGameScene()->getConditionalBlockingCountForUnit(obstX, obstY - 1, 0) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(obstX, obstY - 1))
								bn = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(obstX, obstY + 1, 0) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(obstX, obstY + 1))
								bs = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(obstX - 1, obstY, 0) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(obstX - 1, obstY))
								bw = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(obstX + 1, obstY, 0) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(obstX + 1, obstY))
								be = true;

							bool diagNWtoSE1 = false;
							bool diagNWtoSE2 = false;
							bool diagNEtoSW1 = false;
							bool diagNEtoSW2 = false;
							if ((bn && bw && bs && be)
								|| (int)bn + (int)bs + (int)be + (int)bw >= 3)
							{
								// blocked all-around
								// (or at least at 3 directions)
								//if ((bn && bw && bs && be))
									//Logger::getInstance()->error("ALL");
								//else
									//Logger::getInstance()->error("3DIR");
							} else {
								if (bn && bw)
								{
									diagNEtoSW2 = true;
									diagBlock = true;
									//Logger::getInstance()->error("NE-SW 2 (NW)");
								}
								if (bs && be)
								{
									diagNEtoSW1 = true;
									diagBlock = true;
									//Logger::getInstance()->error("NE-SW 1 (SE)");
								}
								if (bn && be)
								{
									diagNWtoSE2 = true;
									diagBlock = true;
									//Logger::getInstance()->error("NW-SE 2 (NE)");
								}
								if (bs && bw)
								{
									diagNWtoSE1 = true;
									diagBlock = true;
									//Logger::getInstance()->error("NW-SE 1 (SW)");
								}
								if ((int)bn + (int)bs + (int)be + (int)bw == 0)
								{
									//Logger::getInstance()->error("0");
									//diagBlock = true;
								}
								if ((int)bn + (int)bs + (int)be + (int)bw == 1)
								{
									//Logger::getInstance()->error("1DIR");
									diagBlock = true;
								}
								if (bw && be)
								{
									//Logger::getInstance()->error("-WE-");
								}
								if (bn && bs)
								{
									//Logger::getInstance()->error("-NS-");
								}
							}

							float blockStartX = ((float)obstX * game->gameMap->getScaledSizeX() / (float)game->gameMap->getObstacleSizeX() - game->gameMap->getScaledSizeX() * 0.5f);
							float blockStartY = ((float)obstY * game->gameMap->getScaledSizeY() / (float)game->gameMap->getObstacleSizeY() - game->gameMap->getScaledSizeY() * 0.5f);
							float blockSizeX = ((float)(obstX + 1) * game->gameMap->getScaledSizeX() / (float)game->gameMap->getObstacleSizeX() - game->gameMap->getScaledSizeX() * 0.5f) - blockStartX;
							float blockSizeY = ((float)(obstY + 1) * game->gameMap->getScaledSizeY() / (float)game->gameMap->getObstacleSizeY() - game->gameMap->getScaledSizeY() * 0.5f) - blockStartY;
							float blockPosX = position.x - blockStartX;
							float blockPosY = position.z - blockStartY;
							float oldBlockPosX = oldPosition.x - blockStartX;
							float oldBlockPosY = oldPosition.z - blockStartY;
							assert(blockSizeX == blockSizeY);

							if (diagNEtoSW1)
							{
								bool intersectNEtoSW1 = false;
								if (blockPosX + blockPosY >= blockSizeX
									&& oldBlockPosX + oldBlockPosY <= blockSizeX)
									intersectNEtoSW1 = true;

								if (intersectNEtoSW1)
								{
									VC2 crossing = VC2(0.7071068f,0.7071068f);
									float dprod = crossing.GetDotWith(VC2(blockPosX, blockPosY));
									VC2 dprodVec = VC2(dprod * 0.7071068f, dprod * 0.7071068f);
									VC2 offset = dprodVec - VC2(0.5f * blockSizeX, 0.5f * blockSizeY);

									//char buf[256];
									//sprintf(buf, "%f ... (%f - %f)", offset.x / blockSizeX, blockPosX / blockSizeX, (blockPosX + offset.x) / blockSizeX);
									//Logger::getInstance()->error(buf);

									position.x -= offset.x * 1.4142136f;
									position.z -= offset.y * 1.4142136f;
									// offset.x == offset.y, always.

									VC3 velocity = unit->getVelocity();
									velocity.x *= UA_WALL_VEL_FACTOR;
									velocity.z *= UA_WALL_VEL_FACTOR;
									velocity.y *= UA_WALL_VEL_FACTOR;
									unit->setVelocity(velocity);
									diagMoved = true;
								} 
							}
							if (diagNEtoSW2)
							{
								bool intersectNEtoSW2 = false;
								if (blockPosX + blockPosY <= blockSizeY
									&& oldBlockPosX + oldBlockPosY >= blockSizeY)
									intersectNEtoSW2 = true;

								if (intersectNEtoSW2)
								{
									VC2 crossing = VC2(0.7071068f,0.7071068f);
									float dprod = crossing.GetDotWith(VC2(blockSizeX - blockPosX, blockSizeY - blockPosY));
									VC2 dprodVec = VC2(dprod * 0.7071068f, dprod * 0.7071068f);
									VC2 offset = dprodVec - VC2(0.5f * blockSizeX, 0.5f * blockSizeY);

									//char buf[256];
									//sprintf(buf, "%f ... (%f - %f)", offset.x / blockSizeX, blockPosX / blockSizeX, (blockPosX + offset.x) / blockSizeX);
									//Logger::getInstance()->error(buf);

									position.x += offset.x * 1.4142136f;
									position.z += offset.y * 1.4142136f;

									VC3 velocity = unit->getVelocity();
									velocity.x *= UA_WALL_VEL_FACTOR;
									velocity.z *= UA_WALL_VEL_FACTOR;
									velocity.y *= UA_WALL_VEL_FACTOR;
									unit->setVelocity(velocity);
									diagMoved = true;
								}
							}
							if (diagNWtoSE1)
							{
								bool intersectNWtoSE1 = false;
								if (blockPosX <= blockPosY
									&& oldBlockPosX >= oldBlockPosY)
									intersectNWtoSE1 = true;

								if (intersectNWtoSE1)
								{
									VC2 crossing = VC2(0.7071068f,0.7071068f);
									float dprod = crossing.GetDotWith(VC2(blockSizeX - blockPosX, blockPosY));
									VC2 dprodVec = VC2(dprod * 0.7071068f, dprod * 0.7071068f);
									VC2 offset = dprodVec - VC2(0.5f * blockSizeX, 0.5f * blockSizeY);

									//char buf[256];
									//sprintf(buf, "%f ... (%f - %f)", offset.x / blockSizeX, blockPosX / blockSizeX, (blockPosX + offset.x) / blockSizeX);
									//Logger::getInstance()->error(buf);

									position.x += offset.x * 1.4142136f;
									position.z -= offset.y * 1.4142136f;

									VC3 velocity = unit->getVelocity();
									velocity.x *= UA_WALL_VEL_FACTOR;
									velocity.z *= UA_WALL_VEL_FACTOR;
									velocity.y *= UA_WALL_VEL_FACTOR;
									unit->setVelocity(velocity);
									diagMoved = true;
								} 
							}
							if (diagNWtoSE2)
							{
								bool intersectNWtoSE2 = false;
								if (blockPosX > blockPosY 
									&& oldBlockPosX <= oldBlockPosY)
									intersectNWtoSE2 = true;

								if (intersectNWtoSE2)
								{
									VC2 crossing = VC2(0.7071068f,0.7071068f);
									float dprod = crossing.GetDotWith(VC2(blockPosX, blockSizeY - blockPosY));
									VC2 dprodVec = VC2(dprod * 0.7071068f, dprod * 0.7071068f);
									VC2 offset = dprodVec - VC2(0.5f * blockSizeX, 0.5f * blockSizeY);

									//char buf[256];
									//sprintf(buf, "%f ... (%f - %f)", offset.x / blockSizeX, blockPosX / blockSizeX, (blockPosX + offset.x) / blockSizeX);
									//Logger::getInstance()->error(buf);

									position.x -= offset.x * 1.4142136f;
									position.z += offset.y * 1.4142136f;

									VC3 velocity = unit->getVelocity();
									velocity.x *= UA_WALL_VEL_FACTOR;
									velocity.z *= UA_WALL_VEL_FACTOR;
									velocity.y *= UA_WALL_VEL_FACTOR;
									unit->setVelocity(velocity);
									diagMoved = true;
								}
							}
						}
					}

					// HACK: HAX HAX!!! - NUMBER 2
					// 45 degree blocking... FOR OLD BLOCK POSITION

					bool oldIsBlocked = false;
					if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX, oldObstY, position.y 
						+ unit_obst_height_near + extraheight) > unit_obst_amount_near
						&& game->gameMap->isRoundedObstacle(oldObstX, oldObstY))
						oldIsBlocked = true;

					if (oldIsBlocked && (oldObstX != obstX || oldObstY != obstY)
						&& !(oldObstX != obstX && oldObstY != obstY))
					{
						if (oldObstX > 1 && oldObstY > 1 
							&& oldObstX < game->gameMap->getObstacleSizeX() - 1
							&& oldObstY < game->gameMap->getObstacleSizeY() - 1)
						{
							bool bn = false; // blocked north
							bool bs = false; // south
							bool be = false; // east
							bool bw = false; // west
							/*
							if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX, oldObstY - 1, position.y 
								+ unit_obst_height_near + extraheight) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(oldObstX, oldObstY - 1))
								bn = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX, oldObstY + 1, position.y 
								+ unit_obst_height_near + extraheight) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(oldObstX, oldObstY + 1))
								bs = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX - 1, oldObstY, position.y 
								+ unit_obst_height_near + extraheight) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(oldObstX - 1, oldObstY))
								bw = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX + 1, oldObstY, position.y 
								+ unit_obst_height_near + extraheight) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(oldObstX + 1, oldObstY))
								be = true;
							*/
							if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX, oldObstY - 1, 0) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(oldObstX, oldObstY - 1))
								bn = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX, oldObstY + 1, 0) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(oldObstX, oldObstY + 1))
								bs = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX - 1, oldObstY, 0) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(oldObstX - 1, oldObstY))
								bw = true;
							if (game->getGameScene()->getConditionalBlockingCountForUnit(oldObstX + 1, oldObstY, 0) > unit_obst_amount_near
								&& game->gameMap->isRoundedObstacle(oldObstX + 1, oldObstY))
								be = true;

							bool diagNWtoSE1 = false;
							bool diagNWtoSE2 = false;
							bool diagNEtoSW1 = false;
							bool diagNEtoSW2 = false;
							if ((bn && bw && bs && be)
								|| (int)bn + (int)bs + (int)be + (int)bw >= 3)
							{
								// blocked all-around
								// (or at least at 3 directions)
								//if ((bn && bw && bs && be))
									//Logger::getInstance()->error("ALL");
								//else
									//Logger::getInstance()->error("3DIR");
							} else {
								if (bn && bw)
								{
									diagNEtoSW2 = true;
									//Logger::getInstance()->error("NE-SW 2 (NW)");
								}
								if (bs && be)
								{
									diagNEtoSW1 = true;
									//Logger::getInstance()->error("NE-SW 1 (SE)");
								}
								if (bn && be)
								{
									diagNWtoSE2 = true;
									//Logger::getInstance()->error("NW-SE 2 (NE)");
								}
								if (bs && bw)
								{
									diagNWtoSE1 = true;
									//Logger::getInstance()->error("NW-SE 1 (SW)");
								}
								if ((int)bn + (int)bs + (int)be + (int)bw == 0)
								{
									//Logger::getInstance()->error("0");
								}
								if ((int)bn + (int)bs + (int)be + (int)bw == 1)
								{
									//Logger::getInstance()->error("1DIR");
								}
								if (bw && be)
								{
									//Logger::getInstance()->error("-WE-");
								}
								if (bn && bs)
								{
									//Logger::getInstance()->error("-NS-");
								}
							}

							float blockStartX = ((float)oldObstX * game->gameMap->getScaledSizeX() / (float)game->gameMap->getObstacleSizeX() - game->gameMap->getScaledSizeX() * 0.5f);
							float blockStartY = ((float)oldObstY * game->gameMap->getScaledSizeY() / (float)game->gameMap->getObstacleSizeY() - game->gameMap->getScaledSizeY() * 0.5f);
							float blockSizeX = ((float)(oldObstX + 1) * game->gameMap->getScaledSizeX() / (float)game->gameMap->getObstacleSizeX() - game->gameMap->getScaledSizeX() * 0.5f) - blockStartX;
							float blockSizeY = ((float)(oldObstY + 1) * game->gameMap->getScaledSizeY() / (float)game->gameMap->getObstacleSizeY() - game->gameMap->getScaledSizeY() * 0.5f) - blockStartY;
							float blockPosX = position.x - blockStartX;
							float blockPosY = position.z - blockStartY;
							float oldBlockPosX = oldPosition.x - blockStartX;
							float oldBlockPosY = oldPosition.z - blockStartY;
							assert(blockSizeX == blockSizeY);

							if (diagNEtoSW1)
							{
								bool intersectNEtoSW1 = false;
								if (blockPosX + blockPosY >= blockSizeX
									&& oldBlockPosX + oldBlockPosY <= blockSizeX)
									intersectNEtoSW1 = true;

								if (intersectNEtoSW1 && !diagMoved)
								{
									VC2 crossing = VC2(0.7071068f,0.7071068f);
									float dprod = crossing.GetDotWith(VC2(blockPosX, blockPosY));
									VC2 dprodVec = VC2(dprod * 0.7071068f, dprod * 0.7071068f);
									VC2 offset = dprodVec - VC2(0.5f * blockSizeX, 0.5f * blockSizeY);

									//char buf[256];
									//sprintf(buf, "%f ... (%f - %f)", offset.x / blockSizeX, blockPosX / blockSizeX, (blockPosX + offset.x) / blockSizeX);
									//Logger::getInstance()->error(buf);

									position.x -= offset.x * 1.4142136f;
									position.z -= offset.y * 1.4142136f;
									// offset.x == offset.y, always.

									VC3 velocity = unit->getVelocity();
									velocity.x *= UA_WALL_VEL_FACTOR;
									velocity.z *= UA_WALL_VEL_FACTOR;
									velocity.y *= UA_WALL_VEL_FACTOR;
									unit->setVelocity(velocity);
									diagBlock = true;
									diagMoved = true;
								} 
							}
							if (diagNEtoSW2)
							{
								bool intersectNEtoSW2 = false;
								if (blockPosX + blockPosY <= blockSizeY && oldBlockPosX + oldBlockPosY >= blockSizeY)
									intersectNEtoSW2 = true;

									//char buf[256];
									//sprintf(buf, "%f,%f ... (old %f,%f) (relsum %f reloldsum %f)", blockPosX, blockPosY, oldBlockPosX, oldBlockPosY, (blockPosX + blockPosY) / blockSizeX, (oldBlockPosX + oldBlockPosY) / blockSizeX);
									//Logger::getInstance()->error(buf);

								if (intersectNEtoSW2)
								{
									VC2 crossing = VC2(0.7071068f,0.7071068f);
									float dprod = crossing.GetDotWith(VC2(blockSizeX - blockPosX, blockSizeY - blockPosY));
									VC2 dprodVec = VC2(dprod * 0.7071068f, dprod * 0.7071068f);
									VC2 offset = dprodVec - VC2(0.5f * blockSizeX, 0.5f * blockSizeY);

									//char buf[256];
									//sprintf(buf, "%f ... (%f - %f)", offset.x / blockSizeX, blockPosX / blockSizeX, (blockPosX + offset.x) / blockSizeX);
									//Logger::getInstance()->error(buf);

									position.x += offset.x * 1.4142136f;
									position.z += offset.y * 1.4142136f;

									VC3 velocity = unit->getVelocity();
									velocity.x *= UA_WALL_VEL_FACTOR;
									velocity.z *= UA_WALL_VEL_FACTOR;
									velocity.y *= UA_WALL_VEL_FACTOR;
									unit->setVelocity(velocity);
									diagBlock = true;
									diagMoved = true;
								}
							}
							if (diagNWtoSE1)
							{
								bool intersectNWtoSE1 = false;
								if (blockPosX <= blockPosY
									&& oldBlockPosX >= oldBlockPosY)
									intersectNWtoSE1 = true;

								if (intersectNWtoSE1)
								{
									VC2 crossing = VC2(0.7071068f,0.7071068f);
									float dprod = crossing.GetDotWith(VC2(blockSizeX - blockPosX, blockPosY));
									VC2 dprodVec = VC2(dprod * 0.7071068f, dprod * 0.7071068f);
									VC2 offset = dprodVec - VC2(0.5f * blockSizeX, 0.5f * blockSizeY);

									//char buf[256];
									//sprintf(buf, "%f ... (%f - %f)", offset.x / blockSizeX, blockPosX / blockSizeX, (blockPosX + offset.x) / blockSizeX);
									//Logger::getInstance()->error(buf);

									position.x += offset.x * 1.4142136f;
									position.z -= offset.y * 1.4142136f;

									VC3 velocity = unit->getVelocity();
									velocity.x *= UA_WALL_VEL_FACTOR;
									velocity.z *= UA_WALL_VEL_FACTOR;
									velocity.y *= UA_WALL_VEL_FACTOR;
									unit->setVelocity(velocity);
									diagBlock = true;
									diagMoved = true;
								} 
							}
							if (diagNWtoSE2)
							{
								bool intersectNWtoSE2 = false;
								if (blockPosX > blockPosY 
									&& oldBlockPosX <= oldBlockPosY)
									intersectNWtoSE2 = true;

								if (intersectNWtoSE2)
								{
									VC2 crossing = VC2(0.7071068f,0.7071068f);
									float dprod = crossing.GetDotWith(VC2(blockPosX, blockSizeY - blockPosY));
									VC2 dprodVec = VC2(dprod * 0.7071068f, dprod * 0.7071068f);
									VC2 offset = dprodVec - VC2(0.5f * blockSizeX, 0.5f * blockSizeY);

									//char buf[256];
									//sprintf(buf, "%f ... (%f - %f)", offset.x / blockSizeX, blockPosX / blockSizeX, (blockPosX + offset.x) / blockSizeX);
									//Logger::getInstance()->error(buf);

									position.x -= offset.x * 1.4142136f;
									position.z += offset.y * 1.4142136f;

									VC3 velocity = unit->getVelocity();
									velocity.x *= UA_WALL_VEL_FACTOR;
									velocity.z *= UA_WALL_VEL_FACTOR;
									velocity.y *= UA_WALL_VEL_FACTOR;
									unit->setVelocity(velocity);
									diagBlock = true;
									diagMoved = true;
								}
							}
						}
					}

				} // if unit->isDirectControl()


				// TODO: corners, when both west/east and south/north, ...???

				// TODO: move a bit away from the wall.
				// (not just oldposition? but how could the oldposition
				// be inside the wall... maybe squeezes between 2 blocks?)

				if (!diagBlock)
				{
					if (intersectWestWall || intersectEastWall)
					{
						// we intersected with the west or east wall...
						position.x = oldPosition.x;	

						// NEW BEHAVIOUR: y restore to original!!!
						// HACK: to prevent units that are sliding near a ramp
						// wall from keeping their incorrect height
						// see if the height difference is too big and
						// only then keep the height... (umm. explained clearly ;)
						if (fabs(position.y - oldPosition.y) > 1.0f)
							position.y = oldPosition.y;

						VC3 velocity = unit->getVelocity();
						velocity.x = 0;
						velocity.z *= UA_WALL_VEL_FACTOR;
						velocity.y *= UA_WALL_VEL_FACTOR;
						unit->setVelocity(velocity);
						//if (xdiff > 0)
						//	position.x = gameMap->obstacleToScaledX(obstX + 1) + 0.01f;
						//else
						//	position.x = gameMap->obstacleToScaledX(obstX) - 0.01f;
					} 
					if (intersectNorthWall || intersectSouthWall)
					{
						// we did not intersect with the west or east wall,
						// thus is must be the north or south wall. (right?)
						position.z = oldPosition.z;

						// NEW BEHAVIOUR: y restore to original!!!
						// HACK: see the above west/east explanation for this...
						if (fabs(position.y - oldPosition.y) > 1.0f)
							position.y = oldPosition.y;

						VC3 velocity = unit->getVelocity();
						velocity.z = 0;
						velocity.x *= UA_WALL_VEL_FACTOR;
						velocity.y *= UA_WALL_VEL_FACTOR;
						unit->setVelocity(velocity);
						//if (zdiff > 0)
						//	position.z = gameMap->obstacleToScaledY(obstY + 1) + 0.01f;
						//else
						//	position.z = gameMap->obstacleToScaledY(obstY) - 0.01f;
					}
				}

				obstX = game->gameMap->scaledToObstacleX(position.x);
				obstY = game->gameMap->scaledToObstacleY(position.z);

				int obstam;
				float obsth;
				if (obstX != oldObstX || obstY != oldObstY)
				{
					obstam = unit_obst_amount_near;
					obsth = unit_obst_height_near;
				} else {
					obstam = unit_obst_amount;
					obsth = unit_obst_height;
				}

				bool tooSlopeAgain = false;
				if (unit->isDirectControl()
				  && abs(game->gameMap->getHeightmapHeightAt(obstX / GAMEMAP_PATHFIND_ACCURACY, obstY / GAMEMAP_PATHFIND_ACCURACY)
					- game->gameMap->getHeightmapHeightAt(oldObstX / GAMEMAP_PATHFIND_ACCURACY, oldObstY / GAMEMAP_PATHFIND_ACCURACY)) > maxSlope)
				{
					tooSlopeAgain = true;
				}

				// recheck that we did not slide into another obstacle (inner corner)
				// (note, moving obstacles are checked for even if slided
				// by a diagonal block...)
				if (((!diagBlock
					|| !game->gameMap->isRoundedObstacle(obstX, obstY))
					&& game->getGameScene()->getConditionalBlockingCountForUnit(obstX, obstY, position.y 
					+ obsth + extraheight) > obstam)
					|| tooSlopeAgain)
				{
					// slided inside another obstacle (inner corner), bounce back

					// except if stuck to another player - try to get free...
					// NEW: don't go thru other units...
					/*
					if (unit->getBlockedTime() < 9000 / GAME_TICK_MSEC
						|| !game->gameMap->isMovingObstacle(obstX, obstY))
					*/
					{
						position.x = oldPosition.x;
						position.z = oldPosition.z;
						// NEW BEHAVIOUR: y restore to original!!!
						position.y = oldPosition.y;
						VC3 velocity = unit->getVelocity();
						velocity.z = 0;
						velocity.x = 0;
						velocity.y *= UA_WALL_VEL_FACTOR;
						unit->setVelocity(velocity);
						obstX = game->gameMap->scaledToObstacleX(position.x);
						obstY = game->gameMap->scaledToObstacleY(position.z);
					}
				}

				moveUnitObstacle(unit, obstX, obstY);
			}

			//unit->setPosition(position);

			if (unit->getBlockedTime() < 7500 / GAME_TICK_MSEC)
			//if (unit->getBlockedTime() < 10000 / GAME_TICK_MSEC)
			{
				//Logger::getInstance()->error(int2str(unit->getBlockedTime()));
				unit->increaseBlockedTime();
				unit->clearNonBlockedTime();
				if (unit->getBlockedTime() == 2500 / GAME_TICK_MSEC || unit->getBlockedTime() == 5000 / GAME_TICK_MSEC)
//					|| unit->getBlockedTime() == 7500 / GAME_TICK_MSEC)
				{
					if (unit->doesCollisionCheck())
					{
						if (unit->isRushing() || unit->isFollowPlayer())
							unit->setRushDistance(UNITACTOR_RUSH_DISTANCE);
						if (!unit->isDirectControl())
						{
							setPathTo(unit, const_cast<VC3 &> (unit->getFinalDestination()));
							game->gameUI->setPointersChangedFlag(unit->getOwner());
						}
					}
				}
				if (!unit->isDestroyed()
					&& unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS
					&& ((position.x == oldPosition.x && position.z == oldPosition.z)
					|| (unit->getBlockedTime() > 3000 / GAME_TICK_MSEC
					&& fabs(position.x - oldPosition.x) < (1.2f / GAME_TICKS_PER_SECOND)
					&& fabs(position.z - oldPosition.z) < (1.2f / GAME_TICKS_PER_SECOND))))
				{
					animRequests->setStandAnim = true;
					animRequests->setMoveAnim = false;
				}

				stopJumpBecauseCollided(unit);

			} else {

				//Logger::getInstance()->error("CLEARING BLOCKED TIME");

				unit->clearBlockedTime();
				unit->setPath(NULL);
				unit->setFinalDestination(position);
				unit->setWaypoint(position);
				game->gameUI->setPointersChangedFlag(unit->getOwner());
			} // end if(unit->getBlockedTime() < 7500 / GAME_TICK_MSEC)
		} else {

			if (unit->getNonBlockedTime() < 1000 / GAME_TICKS_PER_SECOND)
			{
				unit->increaseNonBlockedTime();
			} else {
				unit->clearBlockedTime();
			}
			//unit->setPosition(position);

		} // end if("the first isBlocked check")

		unit->setPosition(position);
	}



	void UnitActor::applyAnimations(Unit *unit, const UnitActAnimationRequests &animRequests)
	{
		if (unit->getWalkDelay() > 0)
		{
			// did this unit have a non-blend weapon shoot animation? 
			// if so don't apply any other animations until walk delay zero...
			if (unit->getAnimationSet() != NULL)
			{
				if (unit->getSelectedWeapon() != -1
					&& unit->getAnimationSet()->isAnimationInSet(ANIM_SHOOT_TYPE0)
					&& unit->getWeaponType(unit->getSelectedWeapon()) != NULL
					&& unit->isWeaponVisible(unit->getSelectedWeapon()))
				{
					// non-standing or standing (so that which one of these ifs is to be checked)
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_SHOOT_TYPE0_STANDING + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType())
						&& unit->isShootAnimStanding())
					{
						return;
					} else {
						if (unit->getAnimationSet()->getAnimationBlendNumber(ANIM_SHOOT_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType()) == 0)
						{
							return;
						}
					}					
				} else {
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_SHOOT))
					{
						if (unit->getAnimationSet()->getAnimationBlendNumber(ANIM_SHOOT) == 0)
						{
							return;
						}
					}
				}
			}
		}

		if (unit->getAnimationSet() != NULL)
		{
			if (unit->getForcedAnimation() != ANIM_NONE
				&& !unit->isDestroyed()
				&& unit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
			{
				if (unit->getAnimationSet()->isAnimationInSet(unit->getForcedAnimation()))
				{
					unit->getAnimationSet()->animate(unit, unit->getForcedAnimation());
					if (unit->getAnimationSet()->isAnimationStaticFactored(unit->getForcedAnimation()))
					{
						float fact = unit->getAnimationSet()->getAnimationStaticFactor(unit->getForcedAnimation());
						fact *= unit->getCustomTimeFactor();
						Animator::setAnimationSpeedFactor(unit, fact);
					}
				}
				// when forced animation, don't allow blended animations?

#ifdef PROJECT_CLAW_PROTO
// HACK: claw proto
int foo_prone = 1;
static int proningvarnamenum = -1;
if (proningvarnamenum == -1) proningvarnamenum = unit->variables.getVariableNumberByName("proning");
if (unit->variables.getVariable(proningvarnamenum) == 0
	|| unit->getForcedAnimation() != ANIM_SPECIAL2)
{
	Animator::endBlendAnimation(unit, 2, true);
	Animator::endBlendAnimation(unit, 1, true);
} else {
	return;
}
#endif
				Animator::endBlendAnimation(unit, 2, true);
				Animator::endBlendAnimation(unit, 1, true);
				return;
			}
			if (animRequests.setStandAnim)
			{
				int oldAnim = unit->getAnimation();

				if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
				{
					//assert(unit->getAnimationSet()->isAnimationInSet(ANIM_PRONE));
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_PRONE))
						unit->getAnimationSet()->animate(unit, ANIM_PRONE);
				} else {

					int anim = ANIM_STAND;
					// fix animation for specific weapon (stand)
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_STAND_TYPE0)
						&& unit->getSelectedWeapon() != -1
						&& unit->isWeaponVisible(unit->getSelectedWeapon()))
					{
						if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
						{
							// NOTE: these anims must be incremental in numbers
							if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
								&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
							{
								anim = ANIM_STAND_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
							} else {
								assert(!"UnitActor::applyAnimations - Weapon animation type out or range.");
							}
						} else {
							assert(!"UnitActor::applyAnimations - Selected weapon does not exist.");
						}							
					}

					// HACK: ...
					if (animRequests.turnLeft || animRequests.turnRight)
					{
						if (animRequests.turnLeft)
						{
							if (unit->getAnimationSet()->isAnimationInSet(ANIM_TURN_LEFT))
							{
								anim = ANIM_TURN_LEFT;
								if (unit->getAnimationSet()->isAnimationInSet(ANIM_TURN_LEFT_TYPE0)
									&& unit->getSelectedWeapon() != -1
									&& unit->isWeaponVisible(unit->getSelectedWeapon()))
								{
									if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
									{
										// NOTE: these anims must be incremental in numbers
										if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
											&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
										{
											anim = ANIM_TURN_LEFT_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
										} else {
											assert(!"UnitActor::applyAnimations - Weapon animation type out or range.");
										}
									} else {
										assert(!"UnitActor::applyAnimations - Selected weapon does not exist.");
									}							
								}
							}
						}
						if (animRequests.turnRight)
						{
							if (unit->getAnimationSet()->isAnimationInSet(ANIM_TURN_RIGHT))
							{
								anim = ANIM_TURN_RIGHT;
								if (unit->getAnimationSet()->isAnimationInSet(ANIM_TURN_RIGHT_TYPE0)
									&& unit->getSelectedWeapon() != -1
									&& unit->isWeaponVisible(unit->getSelectedWeapon()))
								{
									if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
									{
										// NOTE: these anims must be incremental in numbers
										if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
											&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
										{
											anim = ANIM_TURN_RIGHT_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
										} else {
											assert(!"UnitActor::applyAnimations - Weapon animation type out or range.");
										}
									} else {
										assert(!"UnitActor::applyAnimations - Selected weapon does not exist.");
									}							
								}
							}
						}
					}

					// was not standing before this
					if(oldAnim != unit->getAnimationSet()->getAnimationFileNumber(anim)
						&& !animRequests.setMoveAnim)
					{
						// do a footstep
						//forceFootStep = 1;
					}
					unit->getAnimationSet()->animate(unit, anim);
				}
			}
			if (animRequests.setMoveAnim)
			{
				int anim = ANIM_RUN;
				VC3 vel = unit->getVelocity();
				if (unit->getAnimationSet()->isAnimationInSet(ANIM_DRIVE))
				{
					if (vel.x != 0 || vel.z != 0)
					{
						anim = ANIM_DRIVE;
					} else {
						anim = ANIM_STAND;
					}
				} else {
					Unit::UNIT_SPEED sp = unit->getSpeed();
					if (sp == Unit::UNIT_SPEED_CRAWL)
						anim = ANIM_CRAWL;
					else if (sp == Unit::UNIT_SPEED_SPRINT)
						anim = ANIM_SPRINT;
					else if (sp == Unit::UNIT_SPEED_JUMP)
						anim = ANIM_JUMP;
					else if (sp == Unit::UNIT_SPEED_SLOW 
						|| (sp == Unit::UNIT_SPEED_FAST && vel.x == 0 && vel.z == 0))
						anim = ANIM_WALK;
					else 
						anim = ANIM_RUN;
				}

				// actually strafing?
				if (anim == ANIM_RUN || anim == ANIM_WALK || 
					anim == ANIM_SPRINT || anim == ANIM_JUMP)
				{
					if (unit->isSideways())
					{
						vel.z = 0;
					}

					VC3 zeroVec = VC3(0,0,0);
					float velDir = util::PositionDirectionCalculator::calculateDirection(zeroVec, vel);
					VC3 rotation = unit->getRotation();
					
					float strafeAnimBias = -5.0f;

					// HACK: jumps prefer sideways animations, whereas
					// normal movement prefers forward/backward animations
					// ... deprecated - NO MORE!
					//if (anim == ANIM_JUMP)
					//	strafeAnimBias = 5.0f;

					float rotDirFactored = util::AngleRotationCalculator::getFactoredRotationForAngles(rotation.y, velDir, 45.0f-strafeAnimBias);

					if (unit->isSideways())
					{
						if (unit->getRotation().y < 180)
						{
							if (vel.x > 0.01f)
								rotDirFactored = 180;
							else
								rotDirFactored = 0;
						} else {
							if (vel.x < -0.01f)
								rotDirFactored = 180;
							else
								rotDirFactored = 0;
						}
					}

					bool mech_reverse_hack = unit->getUnitType()->hasMechControls() && unit->moveInReverseDir;

					if (rotDirFactored != 0 || mech_reverse_hack)
					{
						if (fabs(rotDirFactored) > (90+(45+strafeAnimBias)) || mech_reverse_hack)
						{
							unit->setMovingBackward();
							if (anim == ANIM_JUMP)
							{
// --- VEL BASED ---
								if (unit->getJumpAnim() == ANIM_NONE)
								{
									if (unit->getAnimationSet()->isAnimationInSet(ANIM_JUMP_BACKWARD))
									{
										anim = ANIM_JUMP_BACKWARD;
										//if (unit->getVelocity().GetLength() > 0.05f)
										if (unit->getJumpTotalTime() - unit->getJumpCounter() >= 10
											&& unit->getJumpTotalTime() - unit->getJumpCounter() <= 100)
											unit->setJumpAnim(ANIM_JUMP_BACKWARD);
									}
								}
// --- END VEL BASED ---
							} else {
								if (unit->getAnimationSet()->isAnimationInSet(ANIM_RUN_BACKWARD))
									anim = ANIM_RUN_BACKWARD;
							}
						} else {
							if (rotDirFactored < 0)
							{
								unit->setMovingSideways();
								if (anim == ANIM_JUMP)
								{
// --- VEL BASED ---
									if (unit->getJumpAnim() == ANIM_NONE)
									{
										if (unit->getAnimationSet()->isAnimationInSet(ANIM_JUMP_LEFT))
										{
											anim = ANIM_JUMP_LEFT;
											//if (unit->getVelocity().GetLength() > 0.05f)
											if (unit->getJumpTotalTime() - unit->getJumpCounter() >= 10
												&& unit->getJumpTotalTime() - unit->getJumpCounter() <= 100)
												unit->setJumpAnim(ANIM_JUMP_LEFT);
										}
									}
// --- END VEL BASED ---
								} else {
									if (unit->getAnimationSet()->isAnimationInSet(ANIM_STRAFE_LEFT))
										anim = ANIM_STRAFE_LEFT;
								}
							} else {
								unit->setMovingSideways();
								if (anim == ANIM_JUMP)
								{
// --- VEL BASED ---
									if (unit->getJumpAnim() == ANIM_NONE)
									{
										if (unit->getAnimationSet()->isAnimationInSet(ANIM_JUMP_RIGHT))
										{
											anim = ANIM_JUMP_RIGHT;
											//if (unit->getVelocity().GetLength() > 0.05f)
											if (unit->getJumpTotalTime() - unit->getJumpCounter() >= 10
												&& unit->getJumpTotalTime() - unit->getJumpCounter() <= 100)
												unit->setJumpAnim(ANIM_JUMP_RIGHT);
										}
									}
// --- END VEL BASED ---
								} else {
									if (unit->getAnimationSet()->isAnimationInSet(ANIM_STRAFE_RIGHT))
										anim = ANIM_STRAFE_RIGHT;
								}
							}
						}
// --- VEL BASED ---
						/*
						if (fabs(rotDirFactored) > (90+45))
						{
							unit->setMovingBackward();
							if (unit->getAnimationSet()->isAnimationInSet(ANIM_RUN_BACKWARD))
							{
								anim = ANIM_RUN_BACKWARD;
							}
						} else {
							if (rotDirFactored < 0)
							{
								unit->setMovingSideways();
								if (unit->getAnimationSet()->isAnimationInSet(ANIM_STRAFE_LEFT))
								{
									anim = ANIM_STRAFE_LEFT;
								}
							} else {
								unit->setMovingSideways();
								if (unit->getAnimationSet()->isAnimationInSet(ANIM_STRAFE_RIGHT))
								{
									anim = ANIM_STRAFE_RIGHT;
								}
							}
						}
						*/
// --- END VEL BASED ---
					} else {
						unit->setMovingForward();

						if (anim == ANIM_JUMP)
						{
// --- VEL BASED ---
							if (unit->getJumpAnim() == ANIM_NONE)
							{
								if (unit->getAnimationSet()->isAnimationInSet(ANIM_JUMP))
								{
									anim = ANIM_JUMP;
									//if (unit->getVelocity().GetLength() > 0.05f)
									if (unit->getJumpTotalTime() - unit->getJumpCounter() >= 10
										&& unit->getJumpTotalTime() - unit->getJumpCounter() <= 100)
										unit->setJumpAnim(ANIM_JUMP);
								}
							}
// --- END VEL BASED ---
						}
					}
				}

// --- VEL BASED ---
				if (anim == ANIM_JUMP)
				{
					if (unit->getJumpAnim() == ANIM_JUMP_BACKWARD)
					{
						anim = ANIM_JUMP_BACKWARD;
					}
					if (unit->getJumpAnim() == ANIM_JUMP_LEFT)
					{
						anim = ANIM_JUMP_LEFT;
					}
					if (unit->getJumpAnim() == ANIM_JUMP_RIGHT)
					{
						anim = ANIM_JUMP_RIGHT;
					}
				}
// --- END VEL BASED ---


				// NEW: fix animation for jumps
				// (better than velocity, as this does not change when colliding with a wall or such...)
				/*
				if (anim == ANIM_JUMP)
				{
					bool jumpForward = false;
					bool jumpBackward = false;
					bool jumpLeft = false;
					bool jumpRight = false;
					unit->getUnitRelativeJumpDirections(&jumpForward, &jumpBackward, &jumpLeft, &jumpRight);

					if (jumpBackward && !jumpLeft && !jumpRight)
					{
						anim = ANIM_JUMP_BACKWARD;
					}
					if (jumpLeft && !jumpForward && !jumpBackward)
					{
						anim = ANIM_JUMP_LEFT;
					}
					if (jumpRight && !jumpForward && !jumpBackward)
					{
						anim = ANIM_JUMP_RIGHT;
					}
					if (jumpRight && jumpForward)
					{
						if (unit->getLastBoneAimDirection() < 180)
						{
							// nop
							//anim = ANIM_JUMP_FORWARD;
						} else {
							anim = ANIM_JUMP_RIGHT;
						}
					}
					if (jumpLeft && jumpForward)
					{
						if (unit->getLastBoneAimDirection() > 180)
						{
							// nop
							//anim = ANIM_JUMP_FORWARD;
						} else {
							anim = ANIM_JUMP_LEFT;
						}
					}
					if (jumpRight && jumpBackward)
					{
						if (unit->getLastBoneAimDirection() > 180)
						{
							anim = ANIM_JUMP_BACKWARD;
						} else {
							anim = ANIM_JUMP_RIGHT;
						}
					}
					if (jumpLeft && jumpBackward)
					{
						if (unit->getLastBoneAimDirection() < 180)
						{
							anim = ANIM_JUMP_BACKWARD;
						} else {
							anim = ANIM_JUMP_LEFT;
						}
					}
				}
				*/

				// fix animation for specific weapon (run)
				if (anim == ANIM_RUN)
				{
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_RUN_TYPE0)
						&& unit->getSelectedWeapon() != -1
						&& unit->isWeaponVisible(unit->getSelectedWeapon()))
					{
						if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
						{
							if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
								&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
							{
								anim = ANIM_RUN_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
							} else {
								assert(!"UnitActor::applyAnimations - Weapon animation type out or range.");
							}
						} else {
							assert(!"UnitActor::applyAnimations - Selected weapon does not exist.");
						}
					}
				}
				// fix animation for specific weapon (walk)
				if (anim == ANIM_WALK)
				{
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_WALK_TYPE0)
						&& unit->getSelectedWeapon() != -1
						&& unit->isWeaponVisible(unit->getSelectedWeapon()))
					{
						if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
						{
							if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
								&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
							{
								anim = ANIM_WALK_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
							} else {
								assert(!"UnitActor::applyAnimations - Weapon animation type out or range.");
							}
						} else {
							assert(!"UnitActor::applyAnimations - Selected weapon does not exist.");
						}							
					}
				}
				// fix animation for specific weapon (sprint)
				if (anim == ANIM_SPRINT)
				{
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_SPRINT_TYPE0)
						&& unit->getSelectedWeapon() != -1
						&& unit->isWeaponVisible(unit->getSelectedWeapon()))
					{
						if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
						{
							if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
								&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
							{
								anim = ANIM_SPRINT_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
							} else {
								assert(!"UnitActor::applyAnimations - Weapon animation type out or range.");
							}
						} else {
							assert(!"UnitActor::applyAnimations - Selected weapon does not exist.");
						}							
					}
				}
				// fix animation for specific weapon (run backward)
				if (anim == ANIM_RUN_BACKWARD)
				{
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_RUN_BACKWARD_TYPE0)
						&& unit->getSelectedWeapon() != -1
						&& unit->isWeaponVisible(unit->getSelectedWeapon()))
					{
						if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
						{
							if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
								&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
							{
								anim = ANIM_RUN_BACKWARD_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
							} else {
								assert(!"UnitActor::applyAnimations - Weapon animation type out or range.");
							}
						} else {
							assert(!"UnitActor::applyAnimations - Selected weapon does not exist.");
						}							
					}
				}
				// fix animation for specific weapon (strafe left)
				if (anim == ANIM_STRAFE_LEFT)
				{
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_STRAFE_LEFT_TYPE0)
						&& unit->getSelectedWeapon() != -1
						&& unit->isWeaponVisible(unit->getSelectedWeapon()))
					{
						if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
						{
							if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
								&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
							{
								anim = ANIM_STRAFE_LEFT_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
							} else {
								assert(!"UnitActor::applyAnimations - Weapon animation type out or range.");
							}
						} else {
							assert(!"UnitActor::applyAnimations - Selected weapon does not exist.");
						}							
					}
				}
				// fix animation for specific weapon (strafe right)
				if (anim == ANIM_STRAFE_RIGHT)
				{
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_STRAFE_RIGHT_TYPE0)
						&& unit->getSelectedWeapon() != -1
						&& unit->isWeaponVisible(unit->getSelectedWeapon()))
					{
						if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
						{
							if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
								&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
							{
								anim = ANIM_STRAFE_RIGHT_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
							} else {
								assert(!"UnitActor::applyAnimations - Weapon animation type out or range.");
							}
						} else {
							assert(!"UnitActor::applyAnimations - Selected weapon does not exist.");
						}							
					}
				}

				unit->getAnimationSet()->animate(unit, anim);

				float slowdownFact = 1.0f;

				// if unit has a slowdown factor, use that...
				if (unit->getSlowdown() != 0.0f)
				{
					if (unit->getSpeed() != Unit::UNIT_SPEED_JUMP
						|| unit->getUnitType()->doesSlowdownJump())
					{
						slowdownFact = (1.0f - unit->getSlowdown());
					}
				}

				// some animations are scaled based on unit's speed
				// others are not...
				// some have static factor.
				if (unit->getAnimationSet()->isAnimationStaticFactored(anim))
				{
					float fact = unit->getAnimationSet()->getAnimationStaticFactor(anim);
					if (unit->getAnimationSet()->isAnimationSpeedFactored(anim))
					{
						// FIXME: the several below * 100 may be incorrect - if it is referring to velocity
						// that is meters per game_tick? not meters per 1/100 seconds???

						// old, use intended velocity, which may not actually be true...
						//fact *= vel.GetLength() * 100.0f;
						// new, calculate actual velocity (actual movement since last tick) instead of intended velocity...
						VC3 avel = unit->getPosition() - unit->getAnimationLastPosition();
						if (avel.GetSquareLength() > vel.GetSquareLength())
							avel = vel;
						fact *= avel.GetLength() * 100.0f;
						unit->setAnimationLastPosition(unit->getPosition());						
					}
					fact *= slowdownFact;
					fact *= unit->getCustomTimeFactor();

					Animator::setAnimationSpeedFactor(unit, fact);
				} else {

					float fact = unit->getCustomTimeFactor();

					if (unit->getAnimationSet()->isAnimationSpeedFactored(anim))
					{
						// old, use intended velocity, which may not actually be true...
						//Animator::setAnimationSpeedFactor(unit, vel.GetLength() * 100.0f);
						// new, calculate actual velocity (actual movement since last tick) instead of intended velocity...
						VC3 avel = unit->getPosition() - unit->getAnimationLastPosition();
						if (avel.GetSquareLength() > vel.GetSquareLength())
							avel = vel;
						Animator::setAnimationSpeedFactor(unit, avel.GetLength() * 100.0f * fact);
						unit->setAnimationLastPosition(unit->getPosition());						
					} else {
						if (unit->getSlowdown() != 0.0f)
						{
							Animator::setAnimationSpeedFactor(unit, slowdownFact * fact);
						}
						// FIXME: else, setAnimationSpeedFactor(unit, 1.0f);
						// (currently missing - bug, not fixed to keep backward compatible with SG, and for better performance)
					}
				}



			//} else {
			//	if (!animRequests.setStandAnim)
			//	{
			//		assert(!unit->getAnimationSet()->isAnimationInSet(ANIM_DRIVE));
			//	}
			}

			if (animRequests.setIdleAnim != NO_IDLE_ANIMATION_REQUEST)
			{
				unit->getAnimationSet()->animate(unit, ANIM_IDLE1 + animRequests.setIdleAnim - 1);
			}

			// special anims
			if (animRequests.setSpecialAnim != NO_SPECIAL_ANIMATION_REQUEST)
			{
				if (animRequests.setSpecialAnim == SPECIAL_ANIMATION_ELECTRIFIED)
				{
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_ELECTRIFIED))
						unit->getAnimationSet()->animate(unit, ANIM_ELECTRIFIED);
					else
						if (unit->getAnimationSet()->isAnimationInSet(ANIM_STAND))
							unit->getAnimationSet()->animate(unit, ANIM_STAND);
				}
				if (animRequests.setSpecialAnim == SPECIAL_ANIMATION_STUNNED)
				{
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_STUNNED))
						unit->getAnimationSet()->animate(unit, ANIM_STUNNED);
					else
						if (unit->getAnimationSet()->isAnimationInSet(ANIM_STAND))
							unit->getAnimationSet()->animate(unit, ANIM_STAND);
				}
				if (animRequests.setSpecialAnim == SPECIAL_ANIMATION_PRONE_DOWN)
				{
					if (unit->getAnimationSet()->isAnimationInSet(ANIM_GO_PRONE))
						unit->getAnimationSet()->animate(unit, ANIM_GO_PRONE);
				}
				if (animRequests.setSpecialAnim == SPECIAL_ANIMATION_PRONE_UP)
				{
					unit->getAnimationSet()->animate(unit, ANIM_RISE_PRONE);
				}
				// TODO: rise down/up
			}

			// aim anims (if no special anim)
			if (animRequests.setAimAnim != NO_AIM_ANIMATION_REQUEST
				&& !animRequests.endAimAnim
				&& animRequests.setSpecialAnim == NO_SPECIAL_ANIMATION_REQUEST)
			{
				if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
				{
					unit->getAnimationSet()->animate(unit, ANIM_AIM_PRONE);
				} else {
					if (animRequests.setAimAnim == AIM_ANIMATION_RIGHT)
					{
						unit->getAnimationSet()->animate(unit, ANIM_AIM_RIGHT_ARM);
					} else if (animRequests.setAimAnim == AIM_ANIMATION_LEFT) {
						unit->getAnimationSet()->animate(unit, ANIM_AIM_LEFT_ARM);
					} else if (animRequests.setAimAnim == AIM_ANIMATION_BOTH) {
						unit->getAnimationSet()->animate(unit, ANIM_AIM_BOTH_ARMS);
					} else {
						if (unit->getAnimationSet()->isAnimationInSet(ANIM_AIM_TYPE0)
							&& unit->getSelectedWeapon() != -1
							&& unit->isWeaponVisible(unit->getSelectedWeapon()))
						{
							if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
							{
								if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
									&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
								{
									unit->getAnimationSet()->animate(unit, ANIM_AIM_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType());
								} else {
									assert(0);
								}
							} else {
								assert(0);	
							}							
						} else {
							unit->getAnimationSet()->animate(unit, ANIM_AIM);
						}
					}
				}
			}

			bool weapon_launching = unit->getLaunchSpeed() > 0.0f
				&& (
				      (unit->getSelectedWeapon() != -1
				       && unit->getWeaponType(unit->getSelectedWeapon())
						   && unit->getWeaponType(unit->getSelectedWeapon())->usesLaunchSpeedAnimation())
				   ||
					    (unit->getSelectedSecondaryWeapon() != -1
				       && unit->getWeaponType(unit->getSelectedSecondaryWeapon())
						   && unit->getWeaponType(unit->getSelectedSecondaryWeapon())->usesLaunchSpeedAnimation())
				   );

			if (unit->getSpeed() == Unit::UNIT_SPEED_JUMP)
			{
				// roll jump clears blended anims immediately
				if (unit->getUnitType()->doesRollJump())
				{
					Animator::endBlendAnimation(unit, 2, false);
					Animator::endBlendAnimation(unit, 1, false);
				}
			} else if(!weapon_launching) {
				// special anims will clear aim/twist anims too

				if (animRequests.endTwistAnim
					|| animRequests.setSpecialAnim != NO_SPECIAL_ANIMATION_REQUEST)
				{
					Animator::endBlendAnimation(unit, 0, true);
				}
				if (animRequests.endAimAnim
					|| animRequests.setSpecialAnim != NO_SPECIAL_ANIMATION_REQUEST)
				{
					if (!unit->isClipReloading())
					{
						Animator::endBlendAnimation(unit, 2, true);
					}
					Animator::endBlendAnimation(unit, 1, true);
					unit->setAniRecordBlendEndFlag(true);
				} else {
					if (animRequests.endShootAnim)
					{
						if (!unit->isClipReloading())
						{
							Animator::endBlendAnimation(unit, 2, true);
						}
						unit->setAniRecordBlendEndFlag(true);
					}
				}
			}
		}
	}



	void UnitActor::reload(Unit *unit)
	{
		// NEW: change primary weapon active?

		// HACK: railgun requires different kind of functionality!
		bool isRailgun = false;
		if (unit->getSelectedWeapon() != -1
			&& unit->getWeaponType(unit->getSelectedWeapon()) != NULL
			&& unit->getWeaponType(unit->getSelectedWeapon())->allowsAttachedWeaponReload())
		{
			isRailgun = true;
		}

		if (!isRailgun)
		{
			for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
			{
				if (w == unit->getSelectedWeapon())
				{
					unit->setWeaponActive(w, true);
				} else {
					unit->setWeaponActive(w, false);
				}
			}
		}

		{
			for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
			{
				Weapon *w = (Weapon *)unit->getWeaponType(i);

				if (w != NULL)
				{
					if (unit->isWeaponActive(i))
					{
						int reloadClipNum = i;
						if (w->isSharedClipAttachment())
						{
							int sharedW = unit->getWeaponForSharedClip(i);
							if (sharedW != -1)
							{
								reloadClipNum = sharedW;
							}
						}
						// reload attached weapon too...???
						// (to get proper fire delay...)

						bool firstReload = false;
						// HACK: works for shotgun only (single reloading gun)
						// FIXME: fix for other guns
						if (!unit->doesKeepReloading())
						{
							firstReload = true;
						}

						int clipreloadtime = w->getClipReloadTime();

						if(w->usesCustomTimeFactor())
						{
							clipreloadtime = (int)(clipreloadtime / unit->getCustomTimeFactor());
						}

						if (i != reloadClipNum)
						{
							//unit->reloadWeaponAmmoClip(i);
							if (firstReload)
							{
								unit->setFireReloadDelay(i, w->getClipFirstReloadTime() + clipreloadtime);
							} else {
								unit->setFireReloadDelay(i, clipreloadtime);
							}
						}
						bool reloaded = unit->reloadWeaponAmmoClip(reloadClipNum);
						if (reloaded)
						{
							if (firstReload)
							{
								unit->setFireReloadDelay(reloadClipNum, w->getClipFirstReloadTime() + clipreloadtime);
							}

							int reload_anim = ANIM_RELOAD_CLIP_TYPE0 + w->getWeaponAnimationType();
							if (unit->getAnimationSet() != NULL
								&& unit->getAnimationSet()->isAnimationInSet(reload_anim))
							{
								unit->getAnimationSet()->animate(unit, reload_anim);
								float fact = 1.0f;
								if(w->usesCustomTimeFactor())
								{
									fact = unit->getCustomTimeFactor();
								}
								Animator::setAnimationSpeedFactor(unit, unit->getAnimationSet()->getAnimationFileNumber(reload_anim), fact);
								//unit->setReloadingAnimFlag(true);
							}

							if (w->isSingleReloading())
							{
								unit->setKeepReloading(true);
							}

							// just to be sure... (don't keep reloading & keep firing)
							unit->setKeepFiringCount(0);

							const char *reloadSound = w->getClipReloadSound();
							if (firstReload)
							{
								if (w->getClipFirstReloadSound() != NULL)
									reloadSound = w->getClipFirstReloadSound();
							}
							if (reloadSound != NULL)
							{
								bool looped = false;
								int handle = 0;
								int key = 0;
								VC3 pos = unit->getPosition();
								int h = game->gameUI->parseSoundFromDefinitionString(reloadSound, pos.x, pos.y, pos.z, &looped, &handle, &key, false, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_NORMAL);
								if(h != -1 && w->usesCustomTimeFactor() && game->gameUI->getSoundMixer())
								{
									int freq = game->gameUI->getSoundMixer()->getSoundFrequency(h);
									game->gameUI->getSoundMixer()->setSoundFrequency(h, (int)(freq * unit->getCustomTimeFactor()));
								}
							}
						}
					}
				}
			}	
		}
	}

#ifdef _MSC_VER
#pragma optimize("", off)
#endif

	VC3 unit_sorting_distance_to = VC3(0,0,0);

	struct UnitDistanceSorter: 
	public std::binary_function<Unit *, Unit *, bool>
	{
		bool operator() (const Unit *a, const Unit *b) const
		{
			fb_assert(a != NULL);
			fb_assert(b != NULL);

/*
			Unit *ac = (Unit *)a;
			Unit *bc = (Unit *)b;

			fb_assert(ac->getOwner() == NO_UNIT_OWNER
				|| (ac->getOwner() >= 0 && ac->getOwner() <= 15));
			fb_assert(bc->getOwner() == NO_UNIT_OWNER
				|| (bc->getOwner() >= 0 && bc->getOwner() <= 15));
			fb_assert((ac->getSpeed() >= 1 && ac->getSpeed() <= 5));
			fb_assert((bc->getSpeed() >= 1 && bc->getSpeed() <= 5));

			frozenbyte::debug::Debug_MemoryManager::validatePointer((void *)a);
			frozenbyte::debug::Debug_MemoryManager::validatePointer((void *)b);
*/

			VC3 posa = a->getPosition();
			VC3 posb = b->getPosition();
			VC3 diffa = posa - unit_sorting_distance_to;
			VC3 diffb = posb - unit_sorting_distance_to;
			int aLenSq = (int)(diffa.GetLength());
			int bLenSq = (int)(diffb.GetLength());

/*
			fb_assert((diffa.GetLength() >= 0 && diffa.GetLength() < 999999.0f)
				|| (diffa.GetLength() <= 0 && diffa.GetLength() > -999999.0f));
			fb_assert((diffb.GetLength() >= 0 && diffb.GetLength() < 999999.0f)
				|| (diffb.GetLength() <= 0 && diffb.GetLength() > -999999.0f));
*/

			return aLenSq < bLenSq;
		}
	};


	bool UnitActor::doExecute(Unit *unit)
	{
		bool didExecute = false;

		std::vector<Unit *> vec;

		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter(ulist);
		while(iter.iterateAvailable())
		{
			Unit *other = (Unit *)iter.iterateNext();

			vec.push_back(other);
		}

		unit_sorting_distance_to = unit->getPosition();
		std::sort(vec.begin(), vec.end(), UnitDistanceSorter());
		unit_sorting_distance_to = VC3(0,0,0);

		for (std::vector<Unit *>::iterator i = vec.begin(); i != vec.end(); ++i)
		{
			Unit *other = *i;

			if (canDoUnitExecute(unit, other))
			{
				bool success = game->gameScripting->runExecuteScript(other, unit);
				if (success)
					didExecute = true;
			}

			if (didExecute)
				break;
		}

		return didExecute;
	}

#ifdef _MSC_VER
#pragma optimize("", on)
#endif

	bool UnitActor::shoot(Unit *unit, const VC3 &target)
	{
		bool shot = false;

		VC3 distVec = target - unit->getPosition();
		float distanceSq = distVec.GetSquareLength();

		if(unit->targeting.getTargetUnit() && unit->targeting.getTargetUnit()->getUnitType())
		{
			distanceSq -= unit->targeting.getTargetUnit()->getUnitType()->getWeaponRangeOffset();
			if(distanceSq < 0) distanceSq = 0;
		}

		for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			Weapon *w = (Weapon *)unit->getWeaponType(i);

			if (w != NULL)
			{
				///////////////////////////////////////////////////
				// remote explosive trigger
				if (unit->isWeaponActive(i) && w->isRemoteTrigger())
				{
					bool found = false;
					LinkedList *projlist = game->projectiles->getAllProjectiles();
					LinkedListIterator iter(projlist);
					while (iter.iterateAvailable())
					{
						Projectile *otherproj = (Projectile *)iter.iterateNext();
						if (otherproj->getBulletType()->isRemoteExplosive() && otherproj->getShooter() == unit)
						{
							if(otherproj->getLifeTime() > 1)
							{
								otherproj->setLifeTime(1);
								otherproj->setHitTarget(NULL, NULL);
								otherproj->setChain(HITCHAIN_NOTHING);
								found = true;
							}
						}
					}

					if(found)
					{
						const char *soundfile = w->getRemoteTriggerSound();
						if(soundfile != NULL)
						{
							VC3 pos = unit->getPosition();
							game->gameUI->playSoundEffect(soundfile, pos.x, pos.y, pos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_NORMAL);
						}
						continue;
					}
				}
				// end of remote explosive trigger
				//////////////////////////////////////////

				//////////////////////////////////////////
				// launch speed
				if (unit->isWeaponActive(i) && w->usesLaunchSpeed() && (unit->getWeaponAmmoAmount(i) > 0 || w->getAmmoUsage() == 0) && unit->getFireWaitDelay(i) == 0 && unit->getFireReloadDelay(i) == 0)
				{
					float speed = unit->getLaunchSpeed() + w->getLaunchSpeedAdd();
					float max = w->getLaunchSpeedMax();
					if(speed > max)
					{
						speed = max;
					}

					// just started launching
					if(unit->getLaunchSpeed() == 0.0f && w->usesLaunchSpeedAnimation())
					{
						Animator::endBlendAnimation(unit, 2, true);
						unit->getAnimationSet()->animate(unit, ANIM_SPECIAL30);
						unit->setAniRecordBlendFlag(ANIM_SPECIAL30);
					}

					unit->setLaunchSpeed(speed);

					if(unit->getLaunchNow())
					{
						// launch
						if(w->usesLaunchSpeedAnimation())
						{
							Animator::endBlendAnimation(unit, 2, true);
							unit->getAnimationSet()->animate(unit, ANIM_SPECIAL31);
							unit->setAniRecordBlendFlag(ANIM_SPECIAL31);
						}
					}
					else
					{
					  continue;
					}
				}
				// end of launch speed
				//////////////////////////////////////////

				float wRange = w->getRange();
				if (unit->isWeaponActive(i)
					&& (wRange * wRange >= distanceSq
					|| wRange == 0)
					&& unit->getFireReloadDelay(i) == 0 
					&& unit->getFireWaitDelay(i) == 0
					&& unit->getEnergy() >= w->getPowerUsage()
					&& unit->getHeat() < (unit->getMaxHeat() * 9) / 10)
				{
					if (unit->useWeaponAmmo(i))
					{
						//unit->setWeaponType(i, w);	old stuff?

						int firewait = 1;
						if (!unit->isFiringInProgress() || w->isSingleShot()) 
						{
							firewait = (w->getFireWaitTime() * ((100 - w->getFireWaitVary()) + game->gameRandom->nextInt() % (w->getFireWaitVary() + 1)) / 100);

							// a small delay to weaps that raytrace from barrel, to allow proper aiming pose...
							// TODO: should be applied to firefromweaponbarrel too, but maintaining backward compat. for sg
							if (w->doesRaytraceFromWeaponBarrel())
							{
								// NOTE: some hard coded magical value here.. (45 msec)
								if (firewait < (45 / GAME_TICK_MSEC) && !unit->isFiringInProgress())
								{
									firewait = (45 / GAME_TICK_MSEC);
								}
							}

							unit->setFiringInProgress(true);
						}

						int firereload = (w->getFireReloadTime() * ((100 - w->getFireReloadVary()) + game->gameRandom->nextInt() % (w->getFireReloadVary() + 1)) / 100);
						if(w->usesCustomTimeFactor())
						{
							firereload = (int)(firereload / unit->getCustomTimeFactor());
						}

						// HACK: to get player shoot quicker... not so much waiting.
						if (unit->isDirectControl())
						{
							if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
							{
								if (firereload < w->getRepeatAmount() * w->getRepeatDelay() + 1)
								{
									firereload = w->getRepeatAmount() * w->getRepeatDelay() + 1;
								}
							}
						}

						if (firewait < 1) firewait = 1;
						if (firereload < 1) firereload = 1;
						unit->setFireWaitDelay(i, firewait);
						unit->setFireReloadDelay(i, firereload);

						// if this is an attachment weapon, make the parent
						// wait at least the time that it takes for this 
						// weapon to fire and reload...
						int attachedTo = -1;
						if (!w->hasIndependentFireDelay())
						{
							for (int attach = 0; attach < UNIT_MAX_WEAPONS; attach++)
							{
								if (unit->getWeaponType(attach) != NULL && unit->getWeaponType(attach)->getAttachedWeaponType() != NULL
									&& unit->getWeaponType(attach)->getAttachedWeaponType()->getPartTypeId() == w->getPartTypeId())
								{
									attachedTo = attach;
									if (unit->getFireReloadDelay(attach) < firewait+firereload)
									{
										unit->setFireReloadDelay(attach, firewait+firereload);
									}
								}
							}
						}
						// the same applies to firing the parent weapon...
						// attachment cannot be fired until after a while...
						if (w->getAttachedWeaponType() != NULL)
						{
							if (!w->usesFireDelayHack() && !w->getAttachedWeaponType()->hasIndependentFireDelay())
							{
								for (int attach = 0; attach < UNIT_MAX_WEAPONS; attach++)
								{
									if (unit->getWeaponType(attach) != NULL
										&& w->getAttachedWeaponType()->getPartTypeId() == unit->getWeaponType(attach)->getPartTypeId())
									{
										if (unit->getFireReloadDelay(attach) < firewait+firereload)
										{
											unit->setFireReloadDelay(attach, firewait+firereload);
										}
									}
								}
							}
						}

						// hack to delay secondary weapon
						if(w->usesFireDelayHack())
						{
							int w2 = unit->getSelectedSecondaryWeapon();
							if(w2 != -1)
							{
								unit->setFireReloadDelay(w2, firewait+firereload);
							}
						}

						// this is secondary weapon
						if(unit->getSelectedSecondaryWeapon() != -1
							&& unit->getWeaponType(unit->getSelectedSecondaryWeapon()) == w)
						{
							int w2 = unit->getSelectedWeapon();
							// hack to delay secondary weapon
							if(w2 != -1 && unit->getWeaponType(w2)
								&& unit->getWeaponType(w2)->usesFireDelayHack())
							{
								unit->setFireReloadDelay(w2, firewait+firereload);
							}
						}

						// if the weapon has the delayAllWeapons flag,
						// make all other weapons reload for the same
						// time, as this weapon firewait+reload delay is...
						// (if they are not already waiting for a longer time
						if (w->doesDelayAllWeapons())
						{
							int otherDelay = firewait + firereload;
							for (int j = 0; j < UNIT_MAX_WEAPONS; j++)
							{
								Weapon *otherw = (Weapon *)unit->getWeaponType(j);

								if (otherw != NULL)
								{
									if (unit->getFireReloadDelay(j) < otherDelay)
										unit->setFireReloadDelay(j, otherDelay);
								}
							}
						}

						if (w->getBurstFireAmount() > 0)
						{
							if (unit->getKeepFiringCount() == 0)
							{
								if (attachedTo != -1)
								{
									if (unit->getWeaponAmmoInClip(attachedTo) >= w->getBurstFireAmount())
										unit->setKeepFiringCount(w->getBurstFireAmount());
									else
										unit->setKeepFiringCount(unit->getWeaponAmmoInClip(attachedTo));
								} else {
									if (unit->getWeaponAmmoInClip(i) >= w->getBurstFireAmount())
										unit->setKeepFiringCount(w->getBurstFireAmount());
									else
										unit->setKeepFiringCount(unit->getWeaponAmmoInClip(i));
								}
							} else {
								assert(unit->getKeepFiringCount() > 0);
								unit->setKeepFiringCount(unit->getKeepFiringCount() - 1);
							}
						} else {
							unit->setKeepFiringCount(0);
						}

						UnitType *unitType = unit->getUnitType();

						if (SimpleOptions::getBool(DH_OPT_B_GAME_MODE_TOPDOWN_SHOOTER))
						{
							float firingSpread = unit->getFiringSpreadFactor();
							firingSpread += ((float)w->getKickback() / 100.0f);
							if (firingSpread > w->getMaxSpread())
								firingSpread = w->getMaxSpread();
							if (firingSpread < w->getMinSpread())
								firingSpread = w->getMinSpread();
							unit->setFiringSpreadFactor(firingSpread);
						}
						
						if (w->getRepeatDelay() == 0)
						{
							// shotty (and standard one shot weapons)?
							int am = w->getRaytraceAmount();
							if (am == 0) am = 1;
							unit->setWeaponFireTime(i, am);
						} else {
							unit->setWeaponFireTime(i, 
								w->getRepeatAmount() * w->getRepeatDelay());
						}

						// walk stop unless already stopped for a longer time...

						// HACK: if already standing while firing, ignore all
						// later state walk stop time crap, and just stop right now...
						/*
						bool stopWalkHack = false;
						VC3 hackvel = unit->getVelocity();
						if (fabs(hackvel.x) < 0.01f
							&& fabs(hackvel.z) < 0.01f)
						{
							stopWalkHack = true;
						}
						*/

						if (w->getWalkStopAtPhase() == WEAPON_WALK_STOP_AT_PHASE_PREFIRE)
//							|| stopWalkHack)
						{
							if (unit->getWalkDelay() < w->getWalkStopTime())
								unit->setWalkDelay(w->getWalkStopTime());
						}

						if (w->doesFiringRequireWalk())
							unit->setSpeedWhileFiring(Unit::UNIT_SPEED_SLOW);

						// HACK: no shoot anim if jumping
						if (!w->usesLaunchSpeedAnimation() && // hack: no animations for launchspeed
							unit->getAnimationSet() != NULL
							&& (unit->getJumpCounter() == 0
							|| unitType->doesFireStopJump()))
						{
							if (unit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
							{
								Animator::endBlendAnimation(unit, 2, true);
								unit->getAnimationSet()->animate(unit, ANIM_SHOOT_PRONE);
								unit->setAniRecordBlendFlag(ANIM_SHOOT_PRONE);
								unit->setShootAnimStanding(false);
							} else {
								if (unit->getWeaponPosition(i) == SLOT_POSITION_RIGHT_ARM)
								{
									Animator::endBlendAnimation(unit, 2, true);
									unit->getAnimationSet()->animate(unit, ANIM_SHOOT_RIGHT_ARM);
									unit->setAniRecordBlendFlag(ANIM_SHOOT_RIGHT_ARM);
									unit->setShootAnimStanding(false);
								} else {
									if (unit->getWeaponPosition(i) == SLOT_POSITION_LEFT_ARM)
									{
										Animator::endBlendAnimation(unit, 2, true);
										unit->getAnimationSet()->animate(unit, ANIM_SHOOT_LEFT_ARM);
										unit->setAniRecordBlendFlag(ANIM_SHOOT_LEFT_ARM);
										unit->setShootAnimStanding(false);
									} else {

										if (unit->getAnimationSet()->isAnimationInSet(ANIM_SHOOT_TYPE0)
											&& unit->getSelectedWeapon() != -1
											&& unit->isWeaponVisible(unit->getSelectedWeapon()))
										{
											if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
											{
												// HACK: if non-blended animation, do some tricks...
												if (unit->getAnimationSet()->getAnimationBlendNumber(ANIM_SHOOT_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType()) == 0)
												{
													//if (!unit->getAnimationSet()->isAnimationLooping(ANIM_SHOOT))
													//{
														Animator::endBlendAnimation(unit, 2, true);
													//}

													// first set stand animation to lose the shoot anim.
													// NOTE: assuming that "stand" is defined in the animation set, if not, this will not work!
													assert(unit->getAnimationSet()->isAnimationInSet(ANIM_STAND));
													unit->getAnimationSet()->animate(unit, ANIM_STAND);
												}
												if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
													&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
												{
													VC3 animvel = unit->getVelocity();
													if (((fabs(animvel.x) < 0.01f
														&& fabs(animvel.z) < 0.01f)
														|| unit->getWalkDelay() > 0)
														&& unit->getAnimationSet()->isAnimationInSet(ANIM_SHOOT_TYPE0_STANDING + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType()))
													{
														// NOTE: assuming that all _standing anims are non-blended, thus using the above hack here too...
														// NOTE: assuming that "stand" is defined in the animation set, if not, this will not work!
														assert(unit->getAnimationSet()->isAnimationInSet(ANIM_STAND));
														unit->getAnimationSet()->animate(unit, ANIM_STAND);

														int a = ANIM_SHOOT_TYPE0_STANDING + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();

														if (!unit->getAnimationSet()->isAnimationLooping(a))
														{
															Animator::endBlendAnimation(unit, 2, true);
														}

														unit->getAnimationSet()->animate(unit, a);
														unit->setAniRecordBlendFlag(a);
														unit->setShootAnimStanding(true);
													} else {
														int a = ANIM_SHOOT_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType();
														if (!unit->getAnimationSet()->isAnimationLooping(a))
														{
															Animator::endBlendAnimation(unit, 2, true);
														}

// HACK: ÜBERHACK!!!
// make sure aim animation is set before shoot anim, so the aim won't override the shoot 
if (unit->getAnimationSet()->isAnimationInSet(ANIM_AIM_TYPE0)
	&& unit->getSelectedWeapon() != -1
	&& unit->isWeaponVisible(unit->getSelectedWeapon()))
{
	if (unit->getWeaponType(unit->getSelectedWeapon()) != NULL)
	{
		if (unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() >= 0
			&& unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType() < ANIM_TYPES_AMOUNT) 
		{
			unit->getAnimationSet()->animate(unit, ANIM_AIM_TYPE0 + unit->getWeaponType(unit->getSelectedWeapon())->getWeaponAnimationType());
		} else {
			assert(0);
		}
	} else {
		assert(0);	
	}							
} else {
	unit->getAnimationSet()->animate(unit, ANIM_AIM);
}


														unit->getAnimationSet()->animate(unit, a);
														unit->setAniRecordBlendFlag(a);
														unit->setShootAnimStanding(false);
													}
												} else {
													assert(0);
												}
											} else {
												assert(0);	
											}							
										} else {
											//if (!unit->getAnimationSet()->isAnimationLooping(ANIM_SHOOT))
											//{
												Animator::endBlendAnimation(unit, 2, true);
											//}

											// HACK: if non-blended animation, do some tricks...
											if (unit->getAnimationSet()->getAnimationBlendNumber(ANIM_SHOOT) == 0)
											{
												// first set stand animation to lose the shoot anim.
												unit->getAnimationSet()->animate(unit, ANIM_STAND);
											}
											unit->getAnimationSet()->animate(unit, ANIM_SHOOT);
											unit->setAniRecordBlendFlag(ANIM_SHOOT);
											unit->setShootAnimStanding(false);
										}
									}
								}
							}
						}
						if (unit->getHeat() + w->getHeatGeneration() <= unit->getMaxHeat())
						{
							unit->setHeat(unit->getHeat() + w->getHeatGeneration());
						} else {
							unit->setHeat(unit->getMaxHeat());
						}

						int powerUsage = w->getPowerUsage();
						// hack to get player shoot more with less energy! (-25% usage)
						/*
						if (unit->isDirectControl())
						{
							if (powerUsage > 5)
							{
								powerUsage = (75 * powerUsage) / 100;
							}
						}
						*/

						unit->setEnergy(unit->getEnergy() - powerUsage);
						shot = true;

						// make prepare sound...
						if (firewait > 1 || w->isSingleShot())
						{
							const char *fireSound = w->getPrepareFireSound();
							if (fireSound != NULL)
							{
								makeFireSound(unit, i, fireSound);
							}
						}

						if (w->isDropWeapon() || w->isManualWeapon())
						{
							unit->targeting.clearTarget();
						}

					} else {
						// was outta ammo
					}

				}
			}

		}
		return shot;
	}

	

	void UnitActor::actPossiblyUnconscious(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		UnitType *unitType = unit->getUnitType();
		VC3 velocity = unit->getVelocity();
		if (unitType->isSticky() && unit->isOnGround() && !unit->isSideways())
    {
      velocity = VC3(0,0,0);
    } else {
			if (unit->isGroundFriction())
				frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC);
			else
				frictionVelocity(&velocity, unitType->getFriction() * GAME_TICK_MSEC * 0.1f);
    }
		unit->setVelocity(velocity);

		// unconscious?
		if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
		{
			VC3 position = unit->getPosition();

			// FIXME: called all the time, once should be enough
			// though the remove checks that it has not been removed before

			// (hack fixed.)
			if (unit->obstacleExists)
			{
				removeUnitObstacle(unit);
			}

			if (!unit->atFinalDestination())
			{
				unit->clearBlockedTime();
				unit->setPath(NULL);
				unit->setFinalDestination(position);
				unit->setWaypoint(position);
				game->gameUI->setPointersChangedFlag(unit->getOwner());
			}
			if (unit->targeting.hasTarget())
			{
				unit->targeting.clearTarget();	
			}

			//if (unit->getHP() >= unit->getMaxHP() / 2)
///			if (unit->getHP() >= 50
// ummm....
			if (unit->getHP() >= 10
				|| unit->getHP() >= unit->getMaxHP())
			{
				// make sure no unit is standing on top of us...
				int obstX = game->gameMap->scaledToObstacleX(position.x);
				int obstY = game->gameMap->scaledToObstacleY(position.z);

				if (obstX < 1) obstX = 1;
				if (obstY < 1) obstY = 1;
				if (obstX >= game->gameMap->getObstacleSizeX() - 1) obstX = game->gameMap->getObstacleSizeX() - 2;
				if (obstY >= game->gameMap->getObstacleSizeY() - 1) obstY = game->gameMap->getObstacleSizeY() - 2;


				if (game->getGameScene()->getBlockingCount(obstX, obstY, position.y 
					+ 0.01f) == 0
					|| !game->gameMap->isMovingObstacle(obstX, obstY)
					|| !unit->doesCollisionCheck())
				{
					unit->setMoveState(Unit::UNIT_MOVE_STATE_UNCONSCIOUS_RISING_UP);
					addUnitObstacle(unit);
					if (unit->getAnimationSet() != NULL)
					{
						if (unit->hasFallenOnBack())
						{
							if (unit->getAnimationSet()->isAnimationInSet(ANIM_GET_UP_BACKDOWN))
							{
								unit->getAnimationSet()->animate(unit, ANIM_GET_UP_BACKDOWN);
								unit->setMoveStateCounter(300); // 3 sec
							} else {
								unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
								animRequests->setStandAnim = true;
							}
						} else {
							if (unit->getAnimationSet()->isAnimationInSet(ANIM_GET_UP_FACEDOWN))
							{
								unit->getAnimationSet()->animate(unit, ANIM_GET_UP_FACEDOWN);
								unit->setMoveStateCounter(300); // 3 sec
							} else {
								unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
								animRequests->setStandAnim = true;
							}
						}
					}
				}
			}
		} 
	}



	void UnitActor::actPossiblyRising(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		// TODO: flipping side?

		if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS_RISING_UP)
		{


			int counter = unit->getMoveStateCounter() - 1;
			unit->setMoveStateCounter(counter);
			if (counter == 0)
			{
				unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
				animRequests->setStandAnim = true;
#ifdef PROJECT_CLAW_PROTO
				// HACK: We may want to reset this after throwing from claw,
				// in case the unit happens to be still alive even when thrown.
				unit->setPhysicsObjectLock( false );
#endif
			}

			if (counter > 133 && counter < 200)
			{
		    VC3 position = unit->getPosition();
				VC3 rotation = unit->getRotation();

				float vel = 0.01f;
				if (unit->hasFallenOnBack())
					vel = -vel;

				// TEMP: only for back fall.
				if (unit->hasFallenOnBack())
				{
					position.x += vel * sinf(UNIT_ANGLE_TO_RAD(rotation.y));
					position.z += vel * cosf(UNIT_ANGLE_TO_RAD(rotation.y));
				}

				unit->setPosition(position);
				stopUnit(unit);
			}
		}
	}


	void UnitActor::actPossiblyProning(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_GO_PRONE
		  || unit->getMoveState() == Unit::UNIT_MOVE_STATE_RISE_PRONE)
		{
			int counter = unit->getMoveStateCounter() - 1;
			if (unit->getAnimationSet() != NULL
				&& !unit->getAnimationSet()->isAnimationInSet(ANIM_PRONE))
			{	
				counter = 0;
			}
			unit->setMoveStateCounter(counter);
			if (counter == 0)
			{
				unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
				animRequests->setStandAnim = true;
				animRequests->endAimAnim = true;
				animRequests->setAimAnim = NO_AIM_ANIMATION_REQUEST;
			} else {
				if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_GO_PRONE)
					animRequests->setSpecialAnim = SPECIAL_ANIMATION_PRONE_DOWN;
				else
					animRequests->setSpecialAnim = SPECIAL_ANIMATION_PRONE_UP;
				animRequests->endAimAnim = true;
				animRequests->setAimAnim = NO_AIM_ANIMATION_REQUEST;
			}
		}
	}


	void UnitActor::actPossiblyStagger(Unit *unit, UnitActAnimationRequests *animRequests)
	{
    VC3 position = unit->getPosition();
    VC3 rotation = unit->getRotation();

		// staggering backward/forward?
		if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_STAGGER_BACKWARD
		  || unit->getMoveState() == Unit::UNIT_MOVE_STATE_STAGGER_FORWARD)
		{
			int counter = unit->getMoveStateCounter() - 1;
			unit->setMoveStateCounter(counter);
			if (counter == 0)
			{
				unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
				animRequests->setStandAnim = true;
			}
			if (counter > 30)
			{
				float vel = 0.01f;
				if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_STAGGER_FORWARD)
					vel = -vel;
				position.x += vel * sinf(UNIT_ANGLE_TO_RAD(rotation.y));
				position.z += vel * cosf(UNIT_ANGLE_TO_RAD(rotation.y));
			}
		}

		unit->setPosition(position);
		unit->setRotation(rotation.x, rotation.y, rotation.z);
	}



	void UnitActor::actPossiblyElectrified(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_ELECTRIFIED)
		{
			animRequests->setStandAnim = false;
			animRequests->setMoveAnim = false;
			animRequests->setSpecialAnim = SPECIAL_ANIMATION_ELECTRIFIED;
			int counter = unit->getMoveStateCounter() - 1;
			unit->setMoveStateCounter(counter);
			if (counter == 0)
			{
				unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
				animRequests->setStandAnim = true;
			}
		}
	}



	void UnitActor::actPossiblyStunned(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_STUNNED)
		{
			animRequests->setSpecialAnim = SPECIAL_ANIMATION_STUNNED;
			int counter = unit->getMoveStateCounter() - 1;
			unit->setMoveStateCounter(counter);
			assert(counter >= 0);
			if (counter == 0)
			{
				unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
				animRequests->setStandAnim = true;
			}

			// mech hack
			int handle = unit->getTurningSound();
			if(handle != -1 && game->gameUI->getSoundMixer())
			{
				game->gameUI->getSoundMixer()->stopSound(handle);
				unit->setTurningSound(-1, 0);
			}
			unit->setPointerVisualEffect(NULL);
			unit->setPointerHitVisualEffect(NULL);
		}
	}



	void UnitActor::actPossiblyImpact(Unit *unit, UnitActAnimationRequests *animRequests)
	{
    VC3 position = unit->getPosition();
    VC3 rotation = unit->getRotation();

		// blown off by impact?
		if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_IMPACT_BACKWARD
			|| unit->getMoveState() == Unit::UNIT_MOVE_STATE_IMPACT_FORWARD)
		{
			int counter = unit->getMoveStateCounter() - 1;
			unit->setMoveStateCounter(counter);
			if (counter == 0)
			{
				unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
			}
			float vel;
			if (unit->getMoveState() != Unit::UNIT_MOVE_STATE_IMPACT_FORWARD)
				vel = 0.01f;
			else
				vel = -0.01f;
			position.x += vel * sinf(UNIT_ANGLE_TO_RAD(rotation.y));
			position.z += vel * cosf(UNIT_ANGLE_TO_RAD(rotation.y));
		}

		unit->setPosition(position);
		unit->setRotation(rotation.x, rotation.y, rotation.z);
	}



	void UnitActor::actPossiblyIdle(Unit *unit, UnitActAnimationRequests *animRequests)
	{
		// idling?
		if (unit->getMoveState() == Unit::UNIT_MOVE_STATE_IDLE)
		{
			int counter = unit->getMoveStateCounter() - 1;
			// if we have an enemy contact or some destination, 
			// quit idling immediately
			VC3 position = unit->getPosition();
			VC3 dest = unit->getFinalDestination();
      if (unit->targeting.hasTarget() || unit->getSeeUnit() != NULL
        || position.x != dest.x || position.z != dest.z)
			{
				counter = 0;
			}
			unit->setMoveStateCounter(counter);
			if (counter == 0)
			{
				unit->setMoveState(Unit::UNIT_MOVE_STATE_NORMAL);
			}
		}
	}

	// WARNING: actNoisy feature disabled! (performance optimization)
	// NOTE: actNoisy feature disabled! (performance optimization)
	// (THIS IS NOT CALLED ANYWHERE)
	void UnitActor::actNoisy(Unit *unit)
	{
		int player = unit->getOwner();

		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->isActive() && !u->isDestroyed()
				&& game->isHostile(u->getOwner(), player)
				&& u != unit)
			{
				VC3 posDiff = u->getPosition() - unit->getPosition();
				
				if (posDiff.GetSquareLength() < UNITACTOR_NOISY_RANGE * UNITACTOR_NOISY_RANGE)
				{
					u->setHearNoiseByUnit(unit);
				}
			}
		}
	}


	void UnitActor::actPointed(Unit *unit)
	{
		// reacts to light pointing?
		if (unit->getUnitType()->isPointedWithAny(UNITTYPE_POINTED_WITH_LIGHT))
		{
			util::LightAmountManager *lightman = util::LightAmountManager::getInstance();
			
			VC3 pos = unit->getPosition();

			// HACK: units that have aimheight less than 0.5m, check
			// the light amount at 0.5m, others at 1.5m

			IVisualObjectData *visData = NULL;
			float lightAmount;
			float height = 1.5f;
			if (unit->getUnitType()->getAimHeightStanding() < 0.5f)
				height = 0.5f;
			lightAmount = lightman->getDynamicLightAmount(pos, visData, height);

			if (lightAmount > 0.25f && visData != NULL)
			{
				// WARNING: unsafe cast (based on unitDataId though)
				assert(visData->getVisualObjectDataId() == (void *)&unitDataId);
				unit->setPointedByUnit((Unit *)visData);
			}
		}

	}


	void UnitActor::actSquashable(Unit *unit)
	{
		for (int cl = 0; cl < MAX_PLAYERS_PER_CLIENT; cl++)
		{
			Unit *u = game->gameUI->getFirstPerson(cl);
			if (u != NULL && u != unit
				&& u->isActive() && !u->isDestroyed())
			{
				VC3 vel = u->getVelocity();
				if (fabs(vel.x) > 0.001f || fabs(vel.z) > 0.001f)
				{
					VC3 diffVec = u->getPosition() - unit->getPosition();
					float diffLenSq = diffVec.GetSquareLength();
					if (diffLenSq < SQUASHABLE_MAX_RANGE * SQUASHABLE_MAX_RANGE )
					{
						PartType *squash = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("B_Squash"));
						if (squash == NULL)
						{
							Logger::getInstance()->warning("UnitActor::actSquashable - B_Squash bullet type not found.");
							return;
						}
						if (!squash->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
						{
							Logger::getInstance()->warning("UnitActor::actSquashable - B_Squash part is not of bullet type.");
							return;
						}

						// WARNING: unsafe cast! (based on check above)
						Bullet *squashBull = (Bullet *)squash;

						Projectile *squashproj = new Projectile(u, squashBull);
						game->projectiles->addProjectile(squashproj);

						VC3 squashpos = unit->getPosition();

						squashproj->setDirectPath(squashpos, squashpos, 
							squashBull->getVelocity());

						squashproj->setHitTarget(unit, NULL);

						ProjectileActor pa = ProjectileActor(game);
						pa.createVisualForProjectile(squashproj);
					}
				}
			}
		}
	}

	// WARNING: actStealth feature disabled! (performance optimization)
	// NOTE: actStealth feature disabled! (performance optimization)
	// (THIS IS NOT CALLED ANYWHERE)

	void UnitActor::actStealth(Unit *unit)
	{
		if (unit->getStealthValue() > 0)
		{
			// first check if we are stealthing and sneaking speed...
			bool doStealth = false;
			if ((unit->getSpeed() == Unit::UNIT_SPEED_SLOW
				|| unit->getSpeed() == Unit::UNIT_SPEED_FAST)
				&& unit->isStealthing())
			{
				if (unit->getEnergy() > 3)
				{
					int energyUseRate = unit->getStealthValue() / 10;
					if (energyUseRate < 1) energyUseRate = 1;
					if ((game->gameTimer % energyUseRate) == 0)
						unit->setEnergy(unit->getEnergy() - 3);
					doStealth = true;
				} else {
					doStealth = false;
				}
			}

			// check that no weapons has been fired, if so, drop out of stealth
			if (doStealth)
			{
				for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
				{
					Weapon *w = (Weapon *)unit->getWeaponType(i);

					if (w != NULL && unit->getFireReloadDelay(i) > 0)
					{
						doStealth = false;
					}
				}
			}

			// were we still in stealth mode? do stuff based on that...
			if (doStealth)
			{
				// blink when about to go out of stealth mode...
				if (unit->getEnergy() > 200
					|| (game->gameTimer % 11) < unit->getEnergy() / 20)
					unit->setStealthVisualInUse(true);
				else
					unit->setStealthVisualInUse(false);
			} else {
				unit->setStealthing(false);
				unit->setStealthVisualInUse(false);
			}
		}
	}


	void UnitActor::actEffects(Unit *unit)
	{
		int duration = unit->getUnitEffectLayerDuration();
		if (duration > 0)
		{
			duration--;
			unit->setUnitEffectLayerDuration(duration);

			if (duration == 0)
			{
				// HACK: change cloak hit effect to cloak effect... 
				// only if totally invisible (still in cloak mode)
				// FIXME: does not work properly? sometimes cloak still goes on after a hit?
//					&& !unit->isDestroyed()
//					&& unit->getCurrentVisibilityFadeValue() == 0.0f)
				// NOTE: currently fixed in deleteUnitEffectLayer script command?

				if (unit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_CLOAKHIT)
				{
					unit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_CLOAK, 20000 / GAME_TICK_MSEC);
				}

				game->deleteVisualOfParts(unit, unit->getRootPart(), true);
				game->createVisualForParts(unit, unit->getRootPart(), true);
			}
		}
	}


	bool UnitActor::warpToPosition(Unit *unit, const VC3 &position)
	{
		this->removeUnitObstacle(unit);

		VC3 groundPos = position;

		float sx = groundPos.x;
		float sy = groundPos.z;
		int failcount = 0;
		if (unit->doesCollisionCheck())
		{
			for (failcount = 0; failcount < 1000; failcount++)
			{
				game->gameMap->keepWellInScaledBoundaries(&sx, &sy);
				if (!game->gameScene->isBlockedAtScaled(sx,sy,0))
					//&& !game->gameScene->isBlockedAtScaled(sx-1,sy,0)
					//&& !game->gameScene->isBlockedAtScaled(sx+1,sy,0)
					//&& !game->gameScene->isBlockedAtScaled(sx,sy-1,0)
					//&& !game->gameScene->isBlockedAtScaled(sx,sy+1,0))
				{
					break;
				}
				sx = groundPos.x + ((game->gameRandom->nextInt() % (51 * (1 + failcount / 100))) - (25 * (1 + failcount / 100))) / 50.0f;
				sy = groundPos.z + ((game->gameRandom->nextInt() % (51 * (1 + failcount / 100))) - (25 * (1 + failcount / 100))) / 50.0f;
				//sx += (game->gameRandom->nextInt() % 5) - 2;
				//sy += (game->gameRandom->nextInt() % 5) - 2;
			}
		}

		bool retValue = true;

		if (failcount == 1000)
		{
			retValue = false;
		} else {
			VC3 pos = VC3(sx, game->gameMap->getScaledHeightAt(sx, sy), sy);
			unit->setPosition(pos);
		}

		this->addUnitObstacle(unit);

		return retValue;
	}


	bool UnitActor::canWarpToPosition(Unit *unit, const VC3 &position)
	{
		this->removeUnitObstacle(unit);

		VC3 groundPos = position;

		float sx = groundPos.x;
		float sy = groundPos.z;
		bool canWarp = false;

		game->gameMap->keepWellInScaledBoundaries(&sx, &sy);
		if (!game->gameScene->isBlockedAtScaled(sx,sy,0))
		{
			canWarp = true;
		}

		this->addUnitObstacle(unit);

		return canWarp;
	}

	bool UnitActor::canDoUnitExecute(Unit *unit, Unit *other)
	{
		VC3 position = unit->getPosition();
		VC3 rotation = unit->getRotation();

		// HACK: if this is a door that is executing, use spawn position
		// instead of the real current position
		// (this works better for some spelcial door open/close script maybe)
		// also, later on, use the counterpart spawn position too.
		if (unit->getUnitType()->hasDoorExecute())
		{
			position = unit->getSpawnCoordinates();
		}


		VC3 dir = VC3(0,0,0);
		dir.x = -sinf(UNIT_ANGLE_TO_RAD(rotation.y));
		dir.z = -cosf(UNIT_ANGLE_TO_RAD(rotation.y));

		position.x += dir.x;
		position.z += dir.z;

		game::UnitLevelAI *ai = (game::UnitLevelAI *)other->getAI();
		if (ai != NULL && ai->isThisAndAllEnabled())
		{
			VC3 otherpos = other->getPosition();
			// HACK: if this is a door that is executing, use spawn position
			// instead of the real current position
			// (see above hack too)
			if (unit->getUnitType()->hasDoorExecute())
			{
				otherpos = other->getSpawnCoordinates();
			}

			VC3 distVec = position - otherpos;
			float execRangeSq;

			// HACK: doors get executed from a longer distance...
			// HACK: execute doors from a bit more forward...
			// by actually moving the door position backward =P
			if (other->getUnitType()->hasDoorExecute())
			{
				//otherpos.x -= (dir.x * 0.5f);
				//otherpos.z -= (dir.z * 0.5f);
				execRangeSq = DOOR_EXECUTE_RANGE * DOOR_EXECUTE_RANGE;
			} else {
				execRangeSq = UNITACTOR_EXECUTE_RANGE * UNITACTOR_EXECUTE_RANGE;
			}

			if (other != unit && other->isActive()
				&& distVec.GetSquareLength() < execRangeSq)
			{
				return true;
			}
		}

		return false;		
	}

	void UnitActor::actDelayedProjectiles(Unit *unit)
	{
		if (unit->getDelayedHitProjectileBullet() != NULL
			&& unit->getDelayedHitProjectileAmount() > 0
			&& !unit->isDestroyed())
		{
			// (note, should rather have a counter for this)
			int tickInterval = unit->getDelayedHitProjectileInterval() / GAME_TICK_MSEC;
			if ((game->gameTimer % tickInterval) == 0)
			{
				// HACK: no delayed damage to npcs
				if ((unit->getOwner() != 2 && unit->getOwner() != 3)
					|| unit->getUnitType()->isType2() 
					|| unit->getUnitType()->isMetallic())
				{
					Bullet *b = unit->getDelayedHitProjectileBullet();

					Projectile *hitproj = new Projectile(NULL, b);
					game->projectiles->addProjectile(hitproj);

					VC3 bullpos = unit->getPosition();

#ifdef PROJECT_CLAW_PROTO
					VC3 dir = VC3(0,0,0);
					VC3 rotation = unit->getRotation();
					dir.x = -sinf(UNIT_ANGLE_TO_RAD(rotation.y));
					dir.z = -cosf(UNIT_ANGLE_TO_RAD(rotation.y));
					hitproj->setDirectPath(bullpos, bullpos + dir, b->getVelocity());
#else
					hitproj->setDirectPath(bullpos, bullpos, b->getVelocity());
#endif

					hitproj->setHitTarget(unit, NULL);

					ProjectileActor pa = ProjectileActor(game);
					pa.createVisualForProjectile(hitproj);
				}

				unit->setDelayedHitProjectileAmount(unit->getDelayedHitProjectileAmount() - 1);
				if (unit->getDelayedHitProjectileAmount() == 0)
				{
					unit->setDelayedHitProjectileBullet(NULL);
				}
			}
		}
	}

	// laser pointer thing
	void UnitActor::handlePointerWeapon(Unit *unit)
	{
		// no selected weapon
		int wnum = unit->getSelectedWeapon();
		if(wnum == -1)
		{
			return;
		}
		
		Weapon *w = unit->getWeaponType(wnum);

		// weapon doesn't have pointer
		if(w->getPointerVisualEffect() == NULL)
		{
			// attached weapon doesn't have pointer either
			if(w->getAttachedWeaponType() == NULL || w->getAttachedWeaponType()->getPointerVisualEffect() == NULL)
			{
				// remove visual effect
				unit->setPointerVisualEffect(NULL);
				unit->setPointerHitVisualEffect(NULL);
				return;
			}
			else
			{
				w = w->getAttachedWeaponType();
			}
		}

		if(w->getPointerHelper() == NULL)
		{
			Logger::getInstance()->debug("UnitActor::handlePointerWeapon - no pointer helper set");
			return;
		}

		// invisible unit
		if(unit->getVisualObject() == NULL || !unit->getVisualObject()->isVisible())
		{
			// remove visual effect
			unit->setPointerVisualEffect(NULL);
			unit->setPointerHitVisualEffect(NULL);
			return;
		}

		VC3 weaponPosition = unit->getPosition() + VC3(0,1.0f,0);
		VC3 target = weaponPosition;
		VC3 dir(0,1,0);

		std::string helperName;

		// get helper
		{
			IStorm3D_Model *unitModel = unit->getVisualObject()->getStormModel();
			if(unitModel == NULL)
			{
				return;
			}

			helperName = std::string("HELPER_MODEL_") + w->getPointerHelper();
			bool helperFound = util::getHelperPosition(unitModel, helperName.c_str(), weaponPosition);
			if (!helperFound)
			{
				helperName = std::string("HELPER_BONE_") + w->getPointerHelper();
				helperFound = util::getHelperPosition(unitModel, helperName.c_str(), weaponPosition);
				if(!helperFound)
				{
					Logger::getInstance()->debug("UnitActor::handlePointerWeapon - no helper found");
					Logger::getInstance()->debug(w->getPointerHelper());
					return;
				}
			}
		}

		QUAT q;
		q.MakeFromAngles(UNIT_ANGLE_TO_RAD(unit->getLastBoneAimBetaAngle()), UNIT_ANGLE_TO_RAD(unit->getRotation().y + unit->getLastBoneAimDirection()), 0);
		dir = VC3(0,0,-1);
		q.RotateVector(dir);
		dir.Normalize();
		target = weaponPosition + dir * 20.0f;

		// raytrace
		//
		unit->getVisualObject()->setCollidable(false);
		GameCollisionInfo cinfo;
		game->getGameScene()->rayTrace(weaponPosition, dir, 100.0f, cinfo, true, false);
		if(cinfo.hit)
		{
			target = cinfo.position;
		}
		else
		{
			target = weaponPosition + dir * 20.0f;
		}
		unit->getVisualObject()->setCollidable(true);

		// make pointer visual
		VisualEffect *ve = unit->getPointerVisualEffect();
		if(ve == NULL || !ve->isMuzzleAttachment(unit, "pointer", helperName))
		{
			int visId = VisualEffectManager::getVisualEffectIdByName(w->getPointerVisualEffect());
			if(visId == -1)
			{
				Logger::getInstance()->debug("UnitActor::handlePointerWeapon - no such visual effect");
				Logger::getInstance()->debug(w->getPointerVisualEffect());
				return;
			}
			ve = game->gameUI->getVisualEffectManager()->createNewVisualEffect(visId, NULL, unit, weaponPosition, target, unit->getRotation(), unit->getVelocity(), game, 1);
			ve->makeMuzzleAttachment(unit, "pointer", helperName);
			unit->setPointerVisualEffect(ve);
		}

		// make pointer hit visual
		VisualEffect *ve_hit = unit->getPointerHitVisualEffect();
		if(w->getPointerHitVisualEffect() != NULL && w->getPointerHitVisualEffect()[0] != '\0' && (ve_hit == NULL || !ve_hit->isMuzzleAttachment(unit, "pointer_hit", helperName)))
		{
			int visId = VisualEffectManager::getVisualEffectIdByName(w->getPointerHitVisualEffect());
			if(visId == -1)
			{
				Logger::getInstance()->debug("UnitActor::handlePointerWeapon - no such visual effect");
				Logger::getInstance()->debug(w->getPointerHitVisualEffect());
				return;
			}
			ve_hit = game->gameUI->getVisualEffectManager()->createNewVisualEffect(visId, NULL, unit, target, target, unit->getRotation(), unit->getVelocity(), game, 1);
			ve_hit->makeMuzzleAttachment(unit, "pointer_hit", helperName);
			unit->setPointerHitVisualEffect(ve_hit);
		}


		// decompose direction vector to euler angles
		/*VC2 xz(dir.x, dir.z);
		xz.Normalize();
		float rot_heading = acosf(xz.y);
		if(xz.x < 0) rot_heading = -rot_heading;
		float rot_pitch = acosf(dir.y * 1.0f) - PI*0.5f;
		if(rot_heading < 0) rot_heading += 2*PI;
		if(rot_pitch < 0) rot_pitch += 2*PI;
		ve->moveBetween(weaponPosition, target, VC3(RAD_TO_UNIT_ANGLE(rot_pitch), RAD_TO_UNIT_ANGLE(rot_heading), 0), 1.0f, 1.0f);*/

		float length = (weaponPosition - target).GetLength();
		if(length > 20.0f) length = 20.0f;

		ve->updateMuzzleAttachment("pointer", helperName, VC3(length * 0.5f,0,0), VC3(1, 1, length));

		if(ve_hit)
		{
			ve_hit->updateMuzzleAttachment("pointer_hit", helperName, VC3(length,0,0), VC3(1, 1, 1));
		}
	}

	void UnitActor::handleTargetLocker(Unit *unit)
	{
		// no selected weapon
		int wnum = unit->getSelectedWeapon();
		if(wnum == -1)
		{
			unit->setLastTargetLockUnit(NULL);
			return;
		}
		
		Weapon *w = unit->getWeaponType(wnum);

		// weapon doesn't have target locker
		if(w->hasTargetLock() == false)
		{
			// attached weapon doesn't have target locker either
			if(w->getAttachedWeaponType() == NULL || w->getAttachedWeaponType()->hasTargetLock() == false)
			{
				unit->setLastTargetLockUnit(NULL);
				return;
			}
			else
			{
				w = w->getAttachedWeaponType();
			}
		}

		Unit *hitUnit = NULL;
		Unit *lastUnit = unit->getLastTargetLockUnit();

		// disable collision for all except enemies
		LinkedList *ulist = game->units->getAllUnits();
		LinkedListIterator iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->getVisualObject() != NULL && (u->getOwner() != 1 || !u->getUnitType()->doesAllowTargetLock()))
			{
				u->getVisualObject()->setCollidable(false);
			}
		}

		// raytrace cursor
		int player = 0;
		for(int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
		{
			if(game->gameUI->getFirstPerson(c) == unit)
			{
				player = c;
				break;
			}
		}
		SceneSelection sel = game->gameUI->cursorRayTracePlayer(player, false, true);

		// prefer collision to last unit
		if(lastUnit != NULL)
		{
			if(sel.hit)
			{
				if (lastUnit->getUnitType() != NULL) {
				VC2 pos(sel.scaledMapX, sel.scaledMapY);
				VC2 pos_last(lastUnit->getPosition().x, lastUnit->getPosition().y);
				float scaleX = game->gameMap->getScaleX() / GAMEMAP_HEIGHTMAP_MULTIPLIER;
				float checkRad = (float)(lastUnit->getUnitType()->getCollisionCheckRadius() - 1) * scaleX;
				if((pos - pos_last).GetLength() < checkRad)
				{
					hitUnit = lastUnit;
				}
			}
		}
		}

		// hit directly with cursor raytrace
		if(hitUnit == NULL && sel.hit && sel.unit)
		{
			hitUnit = sel.unit;
		}

		// didn't hit with cursor - raytrace from unit origin
		if(hitUnit == NULL && sel.hit && !sel.unit)
		{
			VC3 source = unit->getPosition() + VC3(0,1.7f,0);
			VC3 target(sel.scaledMapX, source.y - 0.85f, sel.scaledMapY);
			VC3 dir = target - source;
			float length = dir.GetLength();
			GameCollisionInfo cinfo;
			game->getGameScene()->rayTrace(source, dir / length, 2 * length, cinfo, false, true);
			if(cinfo.hit && cinfo.hitUnit)
			{
				hitUnit = cinfo.unit;
			}
		}

		// enable collision again
		iter = LinkedListIterator(ulist);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (u->getVisualObject() != NULL && (u->getOwner() != 1 || !u->getUnitType()->doesAllowTargetLock()))
			{
				u->getVisualObject()->setCollidable(true);
			}
		}

		if(hitUnit && hitUnit->isActive() && !hitUnit->isDestroyed() && hitUnit->getOwner() == 1)
		{
			unit->setLastTargetLockUnit(hitUnit);

			int max = w->getTargetLockTime();
			hitUnit->setTargetLockTimes(w->getTargetLockReleaseTime(), w->getTargetLockCancelTime());
			hitUnit->setTargetLockCounterMax(max);
			hitUnit->updateUnitTargetLock(game->gameTimer, true);

			if(hitUnit->getTargetLockCounter() == hitUnit->getTargetLockCounterMax() && !hitUnit->wasTargetLockSoundPlayed())
			{
				VC3 pos = unit->getPosition();
				const char *soundfile = w->getTargetLockSound();
				game->gameUI->playSoundEffect(soundfile, pos.x, pos.y, pos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, unit->getUnitType()->getSoundRange(), DEFAULT_SOUND_PRIORITY_NORMAL);
				hitUnit->setTargetLockSoundPlayed(true);
			}
		}
	}
}


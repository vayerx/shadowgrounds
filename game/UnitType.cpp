
#include "precompiled.h"

#include <assert.h>

#include "UnitType.h"

#include "../convert/str2int.h"
#include "../sound/sounddefs.h"
#include "Unit.h"
#include "../system/Logger.h"
#include "physics/physics_collisiongroups.h"
#include "../game/DHLocaleManager.h"

#include "../util/Debug_MemoryManager.h"

#include "materials.h"
#include "tracking/trackable_types.h"
#include "../util/StringUtil.h"

#include "../ui/animdefs.h"

#include <boost/lexical_cast.hpp>

// this should result into about 8 seconds (slightly less).
// (note: this value is not seconds though!!!) 
#define UNITTYPE_DEFAULT_TARGET_LOSE_TIME 8

#define UNIT_HASHCODE_CALC(name, result) \
	{ \
		int hashCode = 0; \
		int hashi = 0; \
		int hashmult = 0; \
		while(name[hashi] != '\0') \
		{ \
			hashCode += (name[hashi] << hashmult); \
			hashCode += (name[hashi] * (13 + hashi)); \
			hashmult+=4; \
			if (hashmult > 23) hashmult -= 23; \
			hashi++; \
		} \
		*result = hashCode; \
	}

namespace game
{
///////////////////////////////////////////////////////////////////////////////
namespace {

int getMaterialFromString( const std::string& string )
{
	for( int i = 0; i < MATERIAL_AMOUNT; i++ )
	{
		if( string == std::string( materialName[ i ] ) )
		{
			return i;
		}
	}

	return -1;
}

}
///////////////////////////////////////////////////////////////////////////////

	UnitType::UnitType() :
		walkOnMaterialArray( MATERIAL_AMOUNT ),
		walkOnMaterialEnabled( false )
	{
		unitTypeId = 0;
		name = NULL;
		//actor = NULL;
		visibilityRatio = 1.0f;
		visionRange = 0;
		minVisionRange = 0;
		maxHeat = 0;
		energy = 0;
		rechargeRate = 1;
		rechargeAmount = 1;
		coolRate = 1;
		coolAmount = 1;
		weaponAimRatio = 1.0f;
		characterAimRatio = 1.0f;
		aimingAccuracy = 100;
		climbCost = 0.0f;
		maxClimb = 1.0f;
		turningAccuracy = 1.0f;
		gravityRatio = 1.0f;
		groundPlaneDirection = false;
		sticky = true;
		flying = false;
		hover = 0.0f;
		hoverVary = 0.0f;
		elevationSpeed = 0.0f;
		stationary = false;
		turning = 360.0f;
		friction = 0.0f;
		acceleration = 1.0f;
		pathAccuracy = 1.0f;
		maxSpeed = 1.0f;
		slowSpeed = 1.0f;
		crawlSpeed = 1.0f;
		sprintSpeed = 1.0f;
		jumpSpeed = 1.0f;
		defaultSpeed = UnitType::DEFAULT_SPEED_FAST;
		backwardMovementFactor = 1.0f;
		bonesFilename = NULL;
		infoModelFilename = NULL;
		infoImageFilename = NULL;
		description = NULL;
		//animation = UNITTYPE_ANIMATION_NONE;
		animation = NULL;
		explosionBullet = NULL;
		deathBleedBullet = NULL;
		touchBullet = NULL;
		physicsContactDamageBullet = NULL;
		physicsContactImpactDamageBullet = NULL;
		haloType = NULL;
		size = 0.0f;
		visionFOV = 360;
		aimingFOF = 90;
		preferFOF = 40;
		flyVelocityRotation = 0;
		explosionSound = NULL;
		radarRatio = 1.0f;
		radarRange = 0;
		hp = 100;
		minHP = 0;
		blockIfDestroyed = false;
		blockRadius = 1;
		coverAvoid = 0;
		coverBlock = 0;
		aimBone = NULL;
		aimBoneFOF = 360;
		vehicle = false;
		hpGain = 1;
		hpGainMax = 1000;
		sightBonusAmount = 0.0f;
		stealthRatio = 1.0f;
		removeDestroyed = false;
		allowRespawn = false;
		terrainifyDestroyed = false;
		aimBoneDirection = 0;
		rusher = false;
		aimHeightStanding = 1.7f;
		aimHeightCrawling = 0.5f;
		ignoreSpawnBlocking = false;
		lineBlock = false;
		metallic = false;
		aiDisableRange = 0;
		removeImmediately = false;
		lightAvoid = 0;
		pointedWith = 0;
		doorExecute = false;
		scale = 1.0f;
		randomScale = 0.0f;
		squashable = false;
		deathBleedOffset = 0.0f;
		collisionCheckRadius = 1;
		immortal = false;
		baseRotation = 0;
		flashlight = false;
		halo = false;
		flashlightOffset = 0.0f;
		lineBlockWidth = 1;
		onlyOneWeaponSelected = true;
		alwaysOnPlayerSide = -1;
		rotationInterpolation = 0;
		positionInterpolation = 0;
		jumpTurnable = false;
		fireStopJump = false;
		jumpLandDelay = 0;
		fireStopSprint = false;
		allowFireOnSprint = false;
		rollJump = false;
		jumpTiming = NULL;
		jumpTimingBackward = NULL;
		jumpTimingSideways = NULL;
		jumpSoundForward = NULL;
		jumpSoundBackward = NULL;
		jumpSoundSideways = NULL;
		fireWhileJumping = false;
		heightOffset = 0.0f;
		eyeHeight=1.7f;
		type2 = false;
		maxSlowdown = 0.0f;
		slowdownAddFactor = 0.0f;
		slowdownWearOff = 0.0f;
		slowdownJump = false;
		bothLineSides = false;
		soundRange = DEFAULT_SOUND_RANGE;
		pushPlayer = false;
		act = true;
		for (int gor = 0; gor < GORETYPE_AMOUNT; gor++)
		{
			goreAmount[gor] = 0;
			for (int i = 0; i < UNITTYPE_MAX_GORE_AMOUNT; i++)
			{
				gore[gor][i] = NULL;
				goreSound[gor][i] = NULL;
				gorePart[gor][i] = NULL;
			}
		}
		noBoneCollision = false;
		turnTimeAtJumpStart = 0;
		turningAtJumpStart = 160.0f;

		shadowSizeX = 0.f;
		shadowSizeZ = 0.f;
		shadowStrength = 0.f;

		targetLoseTime = UNITTYPE_DEFAULT_TARGET_LOSE_TIME;
		disableEffectLayer = false;

		reactToStunningWeapons = true;
		hitAnimationTime = 0;
		hitAnimationAngle = 0;
		hitAnimationTwistFactor = 0.0f;

		alwaysLOF = false;

		noCollision = false;

		spottable = true;

#ifdef GAME_SIDEWAYS
		sideways = true;
#else
		sideways = false;
#endif

		sideGravityX = 0.0f;
		sideGravityZ = 0.0f;
		sideVelocityMax = 0.0f;

		rectangularBlock = false;
		blockStartZ = 0;
		blockStopZ = 0;
		blockJumpStartZ = 0;
		blockJumpStopZ = 0;

		rectangularCollisionCheck = false;
		collisionCheckStartZ = 0;
		collisionCheckStopZ = 0;
		collisionCheckJumpStartZ = 0;
		collisionCheckJumpStopZ = 0;

		ledgeGrab = false;
		ledgeGrabX = 0;
		ledgeGrabZ = 0;

		physicsObject = false;
		physicsObjectType = UNITTYPE_PHYSICS_OBJECT_TYPE_CAPSULE;
		physicsObjectDisabledAngularVelocity = true;
		physicsObjectDisabledYMovement = true;

    physicsObjectSizeX = 0.5f;
    physicsObjectSizeY = 0.5f;
    physicsObjectSizeZ = 0.5f;

		physicsObjectHeightOffset = 0.0f;
    physicsObjectImpulseFactor = 0.45f;
    physicsObjectImpulseFeedbackFactor = 0.3f;
    physicsObjectImpulseThreshold = 0.05f;
    physicsObjectWarpThreshold = 1.0f;
    physicsObjectMass = 1.0f;
#ifdef GAME_SIDEWAYS
    physicsObjectImplementationType = UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_SIDEWAYS;
#else
    physicsObjectImplementationType = UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_NORMAL;
#endif
		physicsObjectIfDestroyed = false;
		physicsObjectDoor = false;
		physicsObjectCollisionGroup = PHYSICS_COLLISIONGROUP_UNITS;
		fluidContainmentPhysicsObject = false;
#ifdef PROJECT_SHADOWGROUNDS
		healthTextMultiplier = 0.5f;
#else
		healthTextMultiplier = 1.0f;
#endif

		walkOnMaterialParticleEffectTimeOut = 340;
		walkOnMaterialSoundEffectTimeOut = 340;
		for( unsigned int i = 0; i < walkOnMaterialArray.size(); ++i )
		{
			walkOnMaterialArray[ i ] = NULL;
			walkOnMaterialDefault = NULL;
		}

		breathingParticleEffect = "";
		breathingParticleEffectTimeOut = 1000000;

		lineOfFireLoseTime = 0;
		poisonResistance = 0.0f;
		criticalHitPercent = -1.0f;
		trackableTypeMask = 0;

		directControl = false;
		directControlType = NULL;
		sidewaysJumpMovable = false;
		sidewaysJumpAllowedInAir = false;
		jumpAcceleration = 0.0f;
		jumpAccelerationSpeedLimit = 0.0f;

		useAimpointHelper = false;

#ifdef PROJECT_SURVIVOR
		permanentPlayerUnit = true;
#else
		permanentPlayerUnit = true;
#endif

		weaponRangeOffset = 0.0f;
		mechControls = false;
		sentryGunControls = false;
		aimRotSpeed = -1;
		aimRotAccuracy = -1;
		aimBetaRotSpeed = -1;
		aimBetaRotAccuracy = -1;
		aimBetaRotLimit = 60.0f;

		turningSound = NULL;
		turningSoundMinPlayTime = 0.0f;
		healthSliderScale = 1.0f;
		targetRectHeightScale = 1.0f;
		targetRectScale = 1.0f;

		backDamageFactor = 2.0f;
		runHitScriptWithoutShooter = false;
		allowTargetLock = false;
	}

	UnitActor *UnitType::getActor()
	{
		return NULL;
	}

	char *UnitType::getName()
	{
		return name;
	}

	Bullet *UnitType::getExplosionBullet()
	{
		return explosionBullet;
	}

	Bullet *UnitType::getDeathBleedBullet()
	{
		return deathBleedBullet;
	}

	Bullet *UnitType::getTouchBullet()
	{
		return touchBullet;
	}

	Bullet *UnitType::getPhysicsContactDamageBullet()
	{
		return physicsContactDamageBullet;
	}

	Bullet *UnitType::getPhysicsContactImpactDamageBullet()
	{
		return physicsContactImpactDamageBullet;
	}

	int UnitType::getUnitTypeId()
	{
		return unitTypeId;
	}

	void UnitType::setUnitTypeId(int unitTypeId)
	{
		if (this->unitTypeId != 0)
		{
			unitTypeIds.remove(this);
		}
		this->unitTypeId = unitTypeId;
		LinkedListIterator iter(&unitTypeIds);
		while (iter.iterateAvailable())
		{
			UnitType *tmp = (UnitType *)iter.iterateNext();
			if (tmp->getUnitTypeId() == unitTypeId)
			{
				Logger::getInstance()->warning("UnitType::setUnitTypeId - Duplicate unit type id number.");
				if (name != NULL)
				{
					Logger::getInstance()->debug(name);
				}
				if (tmp->getName() != NULL)
				{
					Logger::getInstance()->debug(tmp->getName());
				}
			}
		}
		if (unitTypeId != 0)
		{
			unitTypeIds.append(this);
		}
	}

	const char *getName()
	{
		return "";
	}

	bool UnitType::setData(char *key, char *value)
	{
		return setRootData(key, value);
	}

	bool UnitType::setSub(char *key)
	{
		return setRootSub(key);
	}

	bool UnitType::setRootSub(char *key)
	{
		return false;
	}

	bool UnitType::setRootData(char *key, char *value)
	{
		if (strcmp(key, "unitid") == 0)
		{
#ifndef PROJECT_SHADOWGROUNDS
			Logger::getInstance()->warning("UnitType::setRootData - Unit has a manually assigned id (deprecated feature).");
#endif
			setUnitTypeId(str2int(value));
			return true;
		}
		if (strcmp(key, "name") == 0)
		{
			if (name != NULL) delete [] name;
			name = new char[strlen(value) + 1];
			strcpy(name, value);
//#ifndef PROJECT_SHADOWGROUNDS
			if (this->unitTypeId == 0)
			{
				int tmp;
				UNIT_HASHCODE_CALC(name, &tmp);
				setUnitTypeId(tmp);
			}
//#endif
			return true;
		}
		if (strcmp(key, "jumptiming") == 0)
		{
			if (jumpTiming != NULL) delete [] jumpTiming;
			jumpTiming = new char[JUMPTIMING_STRING_LENGTH + 1];
			if (strlen(value) == JUMPTIMING_STRING_LENGTH)
			{
				strcpy(jumpTiming, value);
			} else {
				for (int i = 0; i < JUMPTIMING_STRING_LENGTH; i++)
				{
					jumpTiming[i] = '8';
				}
				jumpTiming[JUMPTIMING_STRING_LENGTH] = '\0';
			}
			return true;
		}
		if (strcmp(key, "jumptimingbackward") == 0)
		{
			if (jumpTimingBackward != NULL) delete [] jumpTimingBackward;
			jumpTimingBackward = new char[JUMPTIMING_STRING_LENGTH + 1];
			if (strlen(value) == JUMPTIMING_STRING_LENGTH)
			{
				strcpy(jumpTimingBackward, value);
			} else {
				for (int i = 0; i < JUMPTIMING_STRING_LENGTH; i++)
				{
					jumpTimingBackward[i] = '8';
				}
				jumpTimingBackward[JUMPTIMING_STRING_LENGTH] = '\0';
			}
			return true;
		}
		if (strcmp(key, "jumptimingsideways") == 0)
		{
			const char *str = convertLocaleSpeechString(value);
			if (jumpTimingSideways != NULL) delete [] jumpTimingSideways;
			jumpTimingSideways = new char[JUMPTIMING_STRING_LENGTH + 1];
			if (strlen(str) == JUMPTIMING_STRING_LENGTH)
			{
				strcpy(jumpTimingSideways, str);
			} else {
				for (int i = 0; i < JUMPTIMING_STRING_LENGTH; i++)
				{
					jumpTimingSideways[i] = '8';
				}
				jumpTimingSideways[JUMPTIMING_STRING_LENGTH] = '\0';
			}
			return true;
		}
		if (strcmp(key, "jumpsoundforward") == 0)
		{
			const char *str = convertLocaleSpeechString(value);
			if (jumpSoundForward != NULL) delete [] jumpSoundForward;
			jumpSoundForward	= new char[strlen(str) + 1];
			strcpy(jumpSoundForward , str);
			return true;
		}
		if (strcmp(key, "jumpsoundbackward") == 0)
		{
			const char *str = convertLocaleSpeechString(value);
			if (jumpSoundBackward != NULL) delete [] jumpSoundBackward;
			jumpSoundBackward  = new char[strlen(str) + 1];
			strcpy(jumpSoundBackward , str);
			return true;
		}
		if (strcmp(key, "jumpsoundsideways") == 0)
		{
			const char *str = convertLocaleSpeechString(value);
			if (jumpSoundSideways != NULL) delete [] jumpSoundSideways;
			jumpSoundSideways  = new char[strlen(str) + 1];
			strcpy(jumpSoundSideways , str);
			return true;
		}
		if (strcmp(key, "explosion") == 0)
		{
			if (!PARTTYPE_ID_STRING_VALID(value)) return false;

			int val = PARTTYPE_ID_STRING_TO_INT(value);
			PartType *bullet = getPartTypeById(val);
			if (bullet == NULL) return false;
			if (!bullet->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
				return false;
			// WARNING: Unsafe cast! (although checked above)
			explosionBullet = (Bullet *)bullet;
			return true;
		}
		if (strcmp(key, "halotype") == 0)
		{
			if (haloType != NULL) delete [] haloType;
			haloType = new char[strlen(value) + 1];
			strcpy(haloType, value);
			return true;
		}
		if (strcmp(key, "deathbleed") == 0)
		{
			if (!PARTTYPE_ID_STRING_VALID(value)) return false;

			int val = PARTTYPE_ID_STRING_TO_INT(value);
			PartType *bullet = getPartTypeById(val);
			if (bullet == NULL) return false;
			if (!bullet->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
				return false;
			// WARNING: Unsafe cast! (although checked above)
			deathBleedBullet = (Bullet *)bullet;
			return true;
		}
		if (strcmp(key, "touchbullet") == 0)
		{
			if (!PARTTYPE_ID_STRING_VALID(value)) return false;

			int val = PARTTYPE_ID_STRING_TO_INT(value);
			PartType *bullet = getPartTypeById(val);
			if (bullet == NULL) return false;
			if (!bullet->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
				return false;
			// WARNING: Unsafe cast! (although checked above)
			touchBullet = (Bullet *)bullet;
			return true;
		}
		if (strcmp(key, "physicscontactdamagebullet") == 0)
		{
			if (!PARTTYPE_ID_STRING_VALID(value)) return false;

			int val = PARTTYPE_ID_STRING_TO_INT(value);
			PartType *bullet = getPartTypeById(val);
			if (bullet == NULL) return false;
			if (!bullet->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
				return false;
			// WARNING: Unsafe cast! (although checked above)
			physicsContactDamageBullet = (Bullet *)bullet;
			return true;
		}
		if (strcmp(key, "physicscontactimpactdamagebullet") == 0)
		{
			if (!PARTTYPE_ID_STRING_VALID(value)) return false;

			int val = PARTTYPE_ID_STRING_TO_INT(value);
			PartType *bullet = getPartTypeById(val);
			if (bullet == NULL) return false;
			if (!bullet->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
				return false;
			// WARNING: Unsafe cast! (although checked above)
			physicsContactImpactDamageBullet = (Bullet *)bullet;
			return true;
		}

		/*
		if (strcmp(key, "animation") == 0)
		{
			if (strcmp(value, "none") == 0)
			{
				animation = UNITTYPE_ANIMATION_NONE;
				return true;
			}
			if (strcmp(value, "armor") == 0)
			{
				animation = UNITTYPE_ANIMATION_ARMOR;
				return true;
			}
			if (strcmp(value, "apc") == 0)
			{
				animation = UNITTYPE_ANIMATION_APC;
				return true;
			}
			if (strcmp(value, "lav") == 0)
			{
				animation = UNITTYPE_ANIMATION_LAV;
				return true;
			}
			if (strcmp(value, "soldier") == 0)
			{
				animation = UNITTYPE_ANIMATION_SOLDIER;
				return true;
			}
		}
		*/
		if (strcmp(key, "animation") == 0)
		{
			if (animation != NULL) delete [] animation;
			animation = new char[strlen(value) + 1];
			strcpy(animation, value);
			return true;
		}
		for (int gor = 0; gor < GORETYPE_AMOUNT; gor++)
		{
			for (int gorenum = 0; gorenum < UNITTYPE_MAX_GORE_AMOUNT; gorenum++)
			{
				char bufeff[128];
				char bufsnd[128];
				char bufpart[128];
				assert(strlen(goreTypeName[gor]) < 100);
				strcpy(bufeff, "gore_");
				strcat(bufeff, goreTypeName[gor]);
				strcat(bufeff, "_");
				strcat(bufeff, int2str(gorenum + 1));

				strcpy(bufsnd, "gore_");
				strcat(bufsnd, goreTypeName[gor]);
				strcat(bufsnd, "_sound_");
				strcat(bufsnd, int2str(gorenum + 1));

				strcpy(bufpart, "gore_");
				strcat(bufpart, goreTypeName[gor]);
				strcat(bufpart, "_part_");
				strcat(bufpart, int2str(gorenum + 1));

				if (strcmp(key, bufeff) == 0)
				{
					if (gore[gor][gorenum] != NULL) delete [] gore[gor][gorenum];
					gore[gor][gorenum] = new char[strlen(value) + 1];
					strcpy(gore[gor][gorenum], value);
					if (gorenum >= goreAmount[gor])
					{
						goreAmount[gor] = gorenum + 1;
					}
					return true;
				}
				if (strcmp(key, bufsnd) == 0)
				{
					if (goreSound[gor][gorenum] != NULL) delete [] goreSound[gor][gorenum];
					goreSound[gor][gorenum] = new char[strlen(value) + 1];
					strcpy(goreSound[gor][gorenum], value);
					if (gorenum >= goreAmount[gor])
					{
						goreAmount[gor] = gorenum + 1;
					}
					return true;
				}
				if (strcmp(key, bufpart) == 0)
				{
					if (gorePart[gor][gorenum] != NULL) delete [] gorePart[gor][gorenum];
					gorePart[gor][gorenum] = new char[strlen(value) + 1];
					strcpy(gorePart[gor][gorenum], value);
					if (gorenum >= goreAmount[gor])
					{
						goreAmount[gor] = gorenum + 1;
					}
					return true;
				}
			}
		}
		PDATA_READ_INT("maxheat", maxHeat);
		PDATA_READ_INT("energy", energy);
		PDATA_READ_INT("visionfov", visionFOV);
		PDATA_READ_INT("visionrange", visionRange);
		PDATA_READ_INT("minvisionrange", minVisionRange);
		PDATA_READ_FLOAT("visibilityratio", visibilityRatio);
		PDATA_READ_INT("rechargerate", rechargeRate);
		PDATA_READ_INT("rechargeamount", rechargeAmount);
		PDATA_READ_INT("coolrate", coolRate);
		PDATA_READ_INT("coolamount", coolAmount);
		PDATA_READ_FLOAT("weaponaim", weaponAimRatio);
		PDATA_READ_FLOAT("characteraim", characterAimRatio);
		PDATA_READ_INT("aiming", aimingAccuracy);
		PDATA_READ_INT("aimingfof", aimingFOF);
		PDATA_READ_INT("preferfof", preferFOF);
		PDATA_READ_FLOAT("maxclimb", maxClimb);
		PDATA_READ_FLOAT("climbcost", climbCost);
		PDATA_READ_FLOAT("turningaccuracy", turningAccuracy);
		PDATA_READ_FLOAT("gravity", gravityRatio);
		PDATA_READ_BOOLINT("groundplanedirection", groundPlaneDirection);
		PDATA_READ_BOOLINT("sticky", sticky);
		PDATA_READ_BOOLINT("flying", flying);
		PDATA_READ_FLOAT("hover", hover);
		PDATA_READ_FLOAT("hovervary", hoverVary);
		PDATA_READ_FLOAT("elevationspeed", elevationSpeed);
		PDATA_READ_BOOLINT("stationary", stationary);
		PDATA_READ_FLOAT("turning", turning);
		PDATA_READ_FLOAT("friction", friction);
		PDATA_READ_FLOAT("acceleration", acceleration);
		PDATA_READ_FLOAT("maxspeed", maxSpeed);
		PDATA_READ_FLOAT("slowspeed", slowSpeed);
		PDATA_READ_FLOAT("crawlspeed", crawlSpeed);
		PDATA_READ_FLOAT("sprintspeed", sprintSpeed);
		PDATA_READ_FLOAT("jumpspeed", jumpSpeed);
		PDATA_READ_FLOAT("backwardmovementfactor", backwardMovementFactor);
		PDATA_READ_FLOAT("pathaccuracy", pathAccuracy);
		PDATA_READ_STRING("bones", bonesFilename);
		PDATA_READ_STRING("infomodel", infoModelFilename);
		PDATA_READ_STRING("infoimage", infoImageFilename);
		PDATA_READ_STRING("desc", description);
		PDATA_READ_STRINGPLUS("desc+", description);
		PDATA_READ_FLOAT("size", size);
		PDATA_READ_INT("flyvelocityrotation", flyVelocityRotation);
		PDATA_READ_STRING("destroysound", explosionSound);
		PDATA_READ_INT("radarrange", radarRange);
		PDATA_READ_FLOAT("radarratio", radarRatio);
		PDATA_READ_INT("hp", hp);
		PDATA_READ_INT("minhp", minHP);
		PDATA_READ_BOOLINT("blockifdestroyed", blockIfDestroyed);
		PDATA_READ_INT("blockradius", blockRadius);
		PDATA_READ_INT("coveravoid", coverAvoid);
		PDATA_READ_INT("coverblock", coverBlock);
		PDATA_READ_STRING("aimbone", aimBone);
		PDATA_READ_INT("aimbonefof", aimBoneFOF);
		PDATA_READ_BOOLINT("vehicle", vehicle);
		PDATA_READ_INT("hpgain", hpGain);
		PDATA_READ_INT("hpgainmax", hpGainMax);
		PDATA_READ_FLOAT("sightbonus", sightBonusAmount);
		PDATA_READ_FLOAT("stealthratio", stealthRatio);
		PDATA_READ_BOOLINT("removedestroyed", removeDestroyed);
		PDATA_READ_BOOLINT("allowrespawn", allowRespawn);
		PDATA_READ_BOOLINT("terrainifydestroyed", terrainifyDestroyed);
		PDATA_READ_INT("aimbonedirection", aimBoneDirection);
		PDATA_READ_BOOLINT("rusher", rusher);
		PDATA_READ_FLOAT("aimheightstanding", aimHeightStanding);
		PDATA_READ_FLOAT("aimheightcrawling", aimHeightCrawling);
		PDATA_READ_BOOLINT("ignorespawnblocking", ignoreSpawnBlocking);
		PDATA_READ_BOOLINT("lineblock", lineBlock);
		PDATA_READ_BOOLINT("metallic", metallic);
		PDATA_READ_INT("lineblockwidth", lineBlockWidth);
		PDATA_READ_INT("aidisablerange", aiDisableRange);
		PDATA_READ_BOOLINT("removeimmediately", removeImmediately);
		PDATA_READ_INT("lightavoid", lightAvoid);
		PDATA_READ_FLOAT("scale", scale);
		PDATA_READ_FLOAT("randomscale", randomScale);
		PDATA_READ_BOOLINT("squashable", squashable);
		PDATA_READ_FLOAT("deathbleedoffset", deathBleedOffset);
		if (strcmp(key, "pointedwith") == 0)
		{
			if (strcmp(value, "light") == 0)
			{
				pointedWith = UNITTYPE_POINTED_WITH_LIGHT;
			} else {
				return false;
			}
			return true;
		}
		PDATA_READ_BOOLINT("doorexecute", doorExecute);
		PDATA_READ_INT("collisioncheckradius", collisionCheckRadius);
		PDATA_READ_BOOLINT("immortal", immortal);
		PDATA_READ_INT("baserotation", baseRotation);
		PDATA_READ_BOOLINT("flashlight", flashlight);
		PDATA_READ_BOOLINT("halo", halo);
		PDATA_READ_FLOAT("flashlightoffset", flashlightOffset);
		PDATA_READ_BOOLINT("onlyoneweaponselected", onlyOneWeaponSelected);
		PDATA_READ_INT("alwaysonplayerside", alwaysOnPlayerSide);
		PDATA_READ_INT("rotationinterpolation", rotationInterpolation);
		PDATA_READ_INT("positioninterpolation", positionInterpolation);

		PDATA_READ_BOOLINT("jumpturnable", jumpTurnable);
		PDATA_READ_BOOLINT("firestopjump", fireStopJump);
		PDATA_READ_INT("jumplanddelay", jumpLandDelay);
		PDATA_READ_BOOLINT("firestopsprint", fireStopSprint);
		PDATA_READ_BOOLINT("allowfireonsprint", allowFireOnSprint);
		PDATA_READ_BOOLINT("rolljump", rollJump);
		PDATA_READ_BOOLINT("firewhilejumping", fireWhileJumping);
		PDATA_READ_FLOAT("heightoffset", heightOffset);
		PDATA_READ_FLOAT("eyeheight", eyeHeight);
		PDATA_READ_BOOLINT("type2", type2);
		PDATA_READ_FLOAT("slowdownaddfactor", slowdownAddFactor);
		PDATA_READ_FLOAT("maxslowdown", maxSlowdown);
		PDATA_READ_FLOAT("slowdownwearoff", slowdownWearOff);
		PDATA_READ_BOOLINT("slowdownjump", slowdownJump);
		PDATA_READ_FLOAT("soundrange", soundRange);
		PDATA_READ_BOOLINT("bothlinesides", bothLineSides);
		PDATA_READ_BOOLINT("pushplayer", pushPlayer);
		PDATA_READ_BOOLINT("act", act);
		PDATA_READ_BOOLINT("nobonecollision", noBoneCollision);
		PDATA_READ_INT("turntimeatjumpstart", turnTimeAtJumpStart);
		PDATA_READ_FLOAT("turningatjumpstart", turningAtJumpStart);

		PDATA_READ_FLOAT("shadowsizex", shadowSizeX);
		PDATA_READ_FLOAT("shadowsizez", shadowSizeZ);
		PDATA_READ_FLOAT("shadowstrength", shadowStrength);

		PDATA_READ_INT("targetlosetime", targetLoseTime);
		PDATA_READ_BOOLINT("disableeffectlayer", disableEffectLayer);
		PDATA_READ_BOOLINT("reacttostunningweapons", reactToStunningWeapons);
		PDATA_READ_INT("hitanimationtime", hitAnimationTime);
		PDATA_READ_INT("hitanimationangle", hitAnimationAngle);
		PDATA_READ_FLOAT("hitanimationtwistfactor", hitAnimationTwistFactor);

		PDATA_READ_BOOLINT("alwayslof", alwaysLOF);
		PDATA_READ_BOOLINT("nocollision", noCollision);
		PDATA_READ_BOOLINT("spottable", spottable);
		PDATA_READ_BOOLINT("sideways", sideways);
		PDATA_READ_FLOAT("sidegravityx", sideGravityX);
		PDATA_READ_FLOAT("sidegravityz", sideGravityZ);
		PDATA_READ_FLOAT("sidevelocitymax", sideVelocityMax);

		PDATA_READ_BOOLINT("rectangularblock", rectangularBlock);
		PDATA_READ_INT("blockstartz", blockStartZ);
		PDATA_READ_INT("blockstopz", blockStopZ);
		PDATA_READ_INT("blockjumpstartz", blockStartZ);
		PDATA_READ_INT("blockjumpstopz", blockStopZ);

		PDATA_READ_BOOLINT("rectangularcollisioncheck", rectangularCollisionCheck);
		PDATA_READ_INT("collisioncheckstartz", collisionCheckStartZ);
		PDATA_READ_INT("collisioncheckstopz", collisionCheckStopZ);
		PDATA_READ_INT("collisioncheckjumpstartz", collisionCheckStartZ);
		PDATA_READ_INT("collisioncheckjumpstopz", collisionCheckStopZ);

		PDATA_READ_BOOLINT("ledgegrab", ledgeGrab);
		PDATA_READ_INT("ledgegrabx", ledgeGrabX);
		PDATA_READ_INT("ledgegrabz", ledgeGrabZ);

		PDATA_READ_BOOLINT("physicsobject", physicsObject);
		if (strcmp(key, "physicsobjecttype") == 0)
		{
			physicsObjectType = UNITTYPE_PHYSICS_OBJECT_TYPE_INVALID;
			if (strcmp(value, "capsule") == 0)
			{
				physicsObjectType = UNITTYPE_PHYSICS_OBJECT_TYPE_CAPSULE;
			}
			if (strcmp(value, "box") == 0)
			{
				physicsObjectType = UNITTYPE_PHYSICS_OBJECT_TYPE_BOX;
			}
			// TODO: rest of geoms...
			if (physicsObjectType == UNITTYPE_PHYSICS_OBJECT_TYPE_INVALID)
			{
				Logger::getInstance()->error("UnitType::setRootData - physics object type name unknown.");
				return false;
			}
			return true;
		}
		PDATA_READ_BOOLINT("physicsobjectdisabledangularvelocity", physicsObjectDisabledAngularVelocity);
		PDATA_READ_BOOLINT("physicsobjectdisabledymovement", physicsObjectDisabledYMovement);
		PDATA_READ_FLOAT("physicsobjectsizex", physicsObjectSizeX);
		PDATA_READ_FLOAT("physicsobjectsizey", physicsObjectSizeY);
		PDATA_READ_FLOAT("physicsobjectsizez", physicsObjectSizeZ);

		PDATA_READ_FLOAT("physicsobjectheightoffset", physicsObjectHeightOffset);
		PDATA_READ_FLOAT("physicsobjectimpulsefactor", physicsObjectImpulseFactor);
		PDATA_READ_FLOAT("physicsobjectimpulsefeedbackfactor", physicsObjectImpulseFeedbackFactor);
		PDATA_READ_FLOAT("physicsobjectimpulsethreshold", physicsObjectImpulseThreshold);
		PDATA_READ_FLOAT("physicsobjectwarpthreshold", physicsObjectWarpThreshold);
		PDATA_READ_FLOAT("physicsobjectmass", physicsObjectMass);
		if (strcmp(key, "physicsobjectimplementationtype") == 0)
		{
			physicsObjectImplementationType = UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_INVALID;
			if (strcmp(value, "normal") == 0)
			{
				physicsObjectImplementationType = UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_NORMAL;
			}
			if (strcmp(value, "sideways") == 0)
			{
				physicsObjectImplementationType = UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_SIDEWAYS;
			}
			if (physicsObjectImplementationType == UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_INVALID)
			{
				Logger::getInstance()->error("UnitType::setRootData - physics object implementation type name unknown.");
				return false;
			}
			return true;
		}
		PDATA_READ_BOOLINT("physicsobjectifdestroyed", physicsObjectIfDestroyed);
		PDATA_READ_BOOLINT("physicsobjectdoor", physicsObjectDoor);
		PDATA_READ_INT("physicsobjectcollisiongroup", physicsObjectCollisionGroup);
		PDATA_READ_BOOLINT("fluidcontainmentphysicsobject", fluidContainmentPhysicsObject);

		if (strcmp(key, "defaultspeed") == 0)
		{
			if (strcmp(value, "crawl") == 0)
			{
				defaultSpeed = UnitType::DEFAULT_SPEED_CRAWL;
			}
			else if (strcmp(value, "slow") == 0)
			{
				defaultSpeed = UnitType::DEFAULT_SPEED_SLOW;
			}
			else if (strcmp(value, "fast") == 0)
			{
				defaultSpeed = UnitType::DEFAULT_SPEED_FAST;
			}
			else if (strcmp(value, "sprint") == 0)
			{
				defaultSpeed = UnitType::DEFAULT_SPEED_SPRINT;
			}
			else
			{
				Logger::getInstance()->error("UnitType::setRootData - default speed type name unknown (expected \"crawl\", \"slow\", \"fast\" or \"sprint\").");
				return false;
			}
			return true;
		}

		PDATA_READ_FLOAT( "healthtextmultiplier", healthTextMultiplier );

		if( strcmp( key, "breathingparticleeffect" ) == 0 )
		{
			breathingParticleEffect = value;
			return true;
		}
		PDATA_READ_INT( "breathingparticleeffecttimeout", breathingParticleEffectTimeOut );


		PDATA_READ_INT( "walkonmaterial_particleeffecttimeout", walkOnMaterialParticleEffectTimeOut );
		PDATA_READ_INT( "walkonmaterial_soundeffecttimeout", walkOnMaterialSoundEffectTimeOut );

		if( strncmp( key, "walkonmaterial", 14 ) == 0 && value != NULL )
		{
			bool result = false;
			std::string skey( key );
			std::vector< std::string > splitTemp = util::StringSplit( "_", skey );
			std::string material_name;
			std::string effect_file( value );

			if( splitTemp.size() > 1 )
			{
				material_name = splitTemp[ 1 ];
				// hack: fix problems when material name itself has underscore
				if( splitTemp.size() > 3)
				{
					// [3] is not index number
					char firstChar = splitTemp[3][0];
					if( firstChar < '0' || firstChar > '9' )
					{
						
						material_name += "_" + splitTemp[2];
						for(unsigned int i = 2; i < splitTemp.size() - 1; i++)
						{
							splitTemp[i] = splitTemp[i+1];
						}
						splitTemp.resize(splitTemp.size() - 1);
					}
				}
			}

			int material_num = getMaterialFromString( material_name );

			bool default_material = material_name == "default";
			bool valid_material = material_num >= 0 && (unsigned)material_num < walkOnMaterialArray.size();

			if( splitTemp.size() > 2 && material_name.empty() == false && 
				effect_file.empty() == false &&	(valid_material || default_material))
			{
				WalkOnMaterialData **target;
				if(valid_material)
					target = &(walkOnMaterialArray[material_num]);
				else
					target = &walkOnMaterialDefault;

				if( (*target) == NULL )
				{
					(*target) = new WalkOnMaterialData;
					// (*target)->howOftenDoWeCreate = 20;
					walkOnMaterialEnabled = true;
				}

				if( splitTemp[ 2 ] == "particleeffect" )
				{
					(*target)->particleEffectName = effect_file;
					result = true;
				}
				else if( splitTemp[ 2 ] == "soundeffect" )
				{
					int sound_pos = 0;
					if( splitTemp.size() > 3 )
					{
						sound_pos = boost::lexical_cast< int >( splitTemp[ 3 ] );
					}

					if( sound_pos < 0 ) sound_pos = 0;
					if( sound_pos > 50 ) sound_pos = 50;

					if( (*target)->soundEffects.size() <= (unsigned)sound_pos )
						(*target)->soundEffects.resize( sound_pos + 1 );

					(*target)->soundEffects[ sound_pos ].file = effect_file;
					result = true;
				}
				else if( splitTemp[ 2 ] == "soundvolume" )
				{
					int sound_pos = 0;
					if( splitTemp.size() > 3 )
					{
						sound_pos = boost::lexical_cast< int >( splitTemp[ 3 ] );
					}

					if( sound_pos < 0 ) sound_pos = 0;
					if( sound_pos > 50 ) sound_pos = 50;

					if( (*target)->soundEffects.size() <= (unsigned)sound_pos )
						(*target)->soundEffects.resize( sound_pos + 1 );

					(*target)->soundEffects[ sound_pos ].volume = atoi(effect_file.c_str());
					result = true;
				}
				else if( splitTemp[ 2 ] == "soundrange" )
				{
					int sound_pos = 0;
					if( splitTemp.size() > 3 )
					{
						sound_pos = boost::lexical_cast< int >( splitTemp[ 3 ] );
					}

					if( sound_pos < 0 ) sound_pos = 0;
					if( sound_pos > 50 ) sound_pos = 50;

					if( (*target)->soundEffects.size() <= (unsigned)sound_pos )
						(*target)->soundEffects.resize( sound_pos + 1 );

					(*target)->soundEffects[ sound_pos ].range = atoi(effect_file.c_str());
					result = true;
				}
				else
				{
					Logger::getInstance()->debug(("Unknown walk-on-material setting \"" + splitTemp[ 2 ] + "\"").c_str());
				}
			}
			else
			{
				Logger::getInstance()->debug(("Unknown walk-on-material \"" + material_name + "\"").c_str());
			}

			return result;
		}

		if( strncmp( key, "footstep", 8 ) == 0 && value != NULL )
		{
			std::string skey( key );
			std::vector< std::string > splitTemp = util::StringSplit( "_", skey );
			if(splitTemp.size() == 4)
			{
				int user_index = str2int(splitTemp[2].c_str());

				int animation = 0;
				if(splitTemp[1] == "walk")
					animation = ANIM_WALK;

				else if(splitTemp[1].substr(0, 8) == "walktype")
					animation = ANIM_WALK_TYPE0 + (splitTemp[1][8] - '0');

				else if(splitTemp[1] == "run")
					animation = ANIM_RUN;

				else if(splitTemp[1].substr(0, 7) == "runtype")
					animation = ANIM_RUN_TYPE0 + (splitTemp[1][7] - '0');

				else if(splitTemp[1] == "runbackward")
					animation = ANIM_RUN_BACKWARD;

				else if(splitTemp[1].substr(0, 15) == "runbackwardtype")
					animation = ANIM_RUN_BACKWARD_TYPE0 + (splitTemp[1][15] - '0');

				// find existing
				unsigned int i = 0;
				int j = 0;
				for(i = 0; i < footStepTriggers.size(); i++)
				{
					if(footStepTriggers[i].animation == animation)
					{
						if(j == user_index) break; // ok found it
						j++;
					}
				}
				if(i >= footStepTriggers.size())
				{
					// create new one
					footStepTriggers.resize(i + 1);
					footStepTriggers[i].last_triggered = 0;
				}

				footStepTriggers[i].animation = animation;

				if(splitTemp[3] == "timemin")
					footStepTriggers[i].time_min = str2int(value);
				else if(splitTemp[3] == "timemax")
					footStepTriggers[i].time_max = str2int(value);
				else if(splitTemp[3] == "helperbone")
					footStepTriggers[i].helperbone = value;
			}
			return true;
		}
    
		PDATA_READ_INT( "lineoffirelosetime", lineOfFireLoseTime );
		

		// PDATA_READ_FLOAT("poisonresistance", poisonResistance);
		if (strcmp(key, "poisonresistance") == 0) 
		{ 
			poisonResistance = (float)atof(value); 
			return true; 
		} 

		PDATA_READ_FLOAT( "criticalhitpercent", criticalHitPercent );

		if(strcmp(key, "trackabletypemask") == 0)
		{
			bool wasok = true;
			std::string value_str = value;
			std::vector<std::string> splitted = util::StringSplit(",", value_str);
			for (int i = 0; i < (int)splitted.size(); i++)
			{
				std::string trimmed = util::StringRemoveWhitespace(splitted[i]);
				TRACKABLE_TYPEID_DATATYPE tid = getTrackableTypeIdForName(trimmed.c_str());
				if (tid != 0)
				{
					this->trackableTypeMask |= tid;
				} else {
					Logger::getInstance()->error("UnitType::setRootData - trackable type mask name unknown.");
					wasok = false;
				}
			}
			return wasok;
		}

		PDATA_READ_BOOLINT("directcontrol", directControl);
		PDATA_READ_STRING("directcontroltype", directControlType);

		PDATA_READ_BOOLINT("sidewaysjumpmovable", sidewaysJumpMovable);
		PDATA_READ_BOOLINT("sidewaysjumpallowedinair", sidewaysJumpAllowedInAir);
		PDATA_READ_FLOAT( "jumpacceleration", jumpAcceleration );
		PDATA_READ_FLOAT( "jumpaccelerationspeedlimit", jumpAccelerationSpeedLimit );

		PDATA_READ_BOOLINT("useaimpointhelper", useAimpointHelper);
		PDATA_READ_BOOLINT("permanentplayerunit", permanentPlayerUnit);
		PDATA_READ_FLOAT( "weaponrangeoffset", weaponRangeOffset);
		PDATA_READ_BOOLINT( "mechcontrols", mechControls);
		PDATA_READ_FLOAT( "aimrotspeed", aimRotSpeed);
		PDATA_READ_FLOAT( "aimrotaccuracy", aimRotAccuracy);
		PDATA_READ_FLOAT( "aimbetarotspeed", aimBetaRotSpeed);
		PDATA_READ_FLOAT( "aimbetarotaccuracy", aimBetaRotAccuracy);
		PDATA_READ_FLOAT( "aimbetarotlimit", aimBetaRotLimit);
		if (strcmp(key, "turningsound") == 0)
		{
			if (turningSound != NULL) delete [] turningSound;
			turningSound	= new char[strlen(value) + 1];
			strcpy(turningSound , value);
			return true;
		}
		PDATA_READ_FLOAT( "turningsoundminplaytime", turningSoundMinPlayTime);
		PDATA_READ_FLOAT( "healthsliderscale", healthSliderScale);
		PDATA_READ_FLOAT( "targetrectscale", targetRectScale);
		PDATA_READ_FLOAT( "targetrectheightscale", targetRectHeightScale);
		PDATA_READ_BOOLINT( "sentryguncontrols", sentryGunControls);
		PDATA_READ_FLOAT( "backdamagefactor", backDamageFactor);
		PDATA_READ_BOOLINT( "runhitscriptwithoutshooter", runHitScriptWithoutShooter);
		PDATA_READ_BOOLINT( "allowtargetlock", allowTargetLock);

		return false; 	 
	}

	bool UnitType::isPointedWithAny(int pointedWithMask)
	{
		if ((pointedWith & pointedWithMask) != 0)
			return true;
		else
			return false;
	}

	// returns a new instance of this unit type
	Unit *UnitType::getNewUnitInstance(int player)
	{
		player = player;
		// this should never be used? (this is an "abstract" type)
		//return new Unit();
		assert(0);
		return NULL;
	}

	WalkOnMaterialData* UnitType::getWalkOnMaterial( int material ) const
	{
		WalkOnMaterialData* mat = NULL;
		if( material >= 0 && material < (signed)walkOnMaterialArray.size() )
		{
			mat = walkOnMaterialArray[ material ];
		}
		if(mat == NULL)
			return walkOnMaterialDefault;
		else
			return mat;
	}
	
	bool UnitType::getWalkOnMaterialEnabled() const
	{
		return walkOnMaterialEnabled;
	}


	// TODO: more optimal data structure for searches
	LinkedList unitTypeIds = LinkedList();


	UnitType *getUnitTypeById(int id)
	{
		LinkedListIterator iter = LinkedListIterator(&unitTypeIds);
		while (iter.iterateAvailable())
		{
			UnitType *ut = (UnitType *)iter.iterateNext();
			if (ut->getUnitTypeId() == id) return ut;
		}
		return NULL;
	}

	UnitType *getUnitTypeByName(const char *name)
	{
		LinkedListIterator iter = LinkedListIterator(&unitTypeIds);
		while (iter.iterateAvailable())
		{
			UnitType *ut = (UnitType *)iter.iterateNext();
			if (strcmp(ut->getName(), name) == 0) return ut;
		}
		return NULL;
	}
	

	WalkOnMaterialData::SoundEffect::SoundEffect()
	{
		volume = 100;
		range = (int)(DEFAULT_SOUND_RANGE);
	}
}



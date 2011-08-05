
#ifndef UNITTYPE_H
#define UNITTYPE_H

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "../container/LinkedList.h"
#include "UnitActor.h"
#include "Bullet.h"
#include "goretypedefs.h"
#include "tracking/trackable_typeid.h"

#include "../util/part_data_reader_macros.h"

#include "../util/Debug_MemoryManager.h"

#include <string>
#include <vector>

// (bit mask)
#define UNITTYPE_POINTED_WITH_LIGHT 1
//#define UNITTYPE_POINTED_WITH_AIM 2
//#define UNITTYPE_POINTED_WITH_RESERVED3 4
//#define UNITTYPE_POINTED_WITH_RESERVED4 8

#define UNITTYPE_POINTED_WITH_ALL 255

#define JUMPTIMING_STRING_LENGTH 16

#define UNITTYPE_MAX_GORE_AMOUNT 8

#define UNITTYPE_PHYSICS_OBJECT_TYPE_INVALID 0
#define UNITTYPE_PHYSICS_OBJECT_TYPE_CAPSULE 1
#define UNITTYPE_PHYSICS_OBJECT_TYPE_BOX 2
//#define UNITTYPE_PHYSICS_OBJECT_TYPE_SPHERE 3
//#define UNITTYPE_PHYSICS_OBJECT_TYPE_CYLINDER 4
//#define UNITTYPE_PHYSICS_OBJECT_TYPE_CONVEX 5

#define UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_INVALID 0
#define UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_NORMAL 1
#define UNITTYPE_PHYSICS_OBJECT_IMPLEMENTATION_TYPE_SIDEWAYS 2


namespace game
{
	class Unit;

	// don't use directly, externed only for cleanup
	extern LinkedList unitTypeIds;

	/////////////////////////////////////////////////////////////////////////////

	struct WalkOnMaterialData
	{
		std::string particleEffectName;

		struct SoundEffect
		{
			SoundEffect();
			std::string file;
			int volume;
			int range;
		};
		std::vector< SoundEffect > soundEffects;
	};

	struct FootStepTrigger
	{
		int animation;
		int time_min;
		int time_max;
		int last_triggered;
		std::string helperbone;
	};

	/////////////////////////////////////////////////////////////////////////////

	class UnitType
	{
	public:
		typedef enum {
			DEFAULT_SPEED_CRAWL,
			DEFAULT_SPEED_SLOW,
			DEFAULT_SPEED_FAST,
			DEFAULT_SPEED_SPRINT
		} DEFAULT_SPEED;

		UnitType();

		virtual char *getName();

		void setUnitTypeId(int unitTypeId);

		int getUnitTypeId();

		// returns proper actor instance for this unit type
		// (pointer to shared object, don't delete the returned object)
		virtual UnitActor *getActor();

		// parser calls this function to tell that we're in a sub conf
		// called with NULL key when exiting a sub conf
		// should return true if valid sub key
		virtual bool setSub(char *key);

		// parser calls this function to configure the part type based on file
		// should return true if key and value pair was identified and valid
		virtual bool setData(char *key, char *value);

		// returns a new instance of this unit type
		virtual Unit *getNewUnitInstance(int player);

		Bullet *getExplosionBullet();

		Bullet *getDeathBleedBullet();

		Bullet *getTouchBullet();

		Bullet *getPhysicsContactDamageBullet();
		Bullet *getPhysicsContactImpactDamageBullet();

		bool isPointedWithAny(int pointedWithMask);

		inline const std::vector<FootStepTrigger> &getFootStepTriggers() const { return footStepTriggers; }

	protected:
		virtual bool setRootSub(char *key);
		virtual bool setRootData(char *key, char *value);

		int unitTypeId;
		char *name;
		//UnitActor *actor;

		Bullet *explosionBullet;
		Bullet *deathBleedBullet;
		Bullet *touchBullet;
		Bullet *physicsContactDamageBullet;
		Bullet *physicsContactImpactDamageBullet;

		PDATA_DEF(float, visibilityRatio, getVisibilityRatio);
		PDATA_DEF(int, visionRange, getVisionRange);
		PDATA_DEF(int, minVisionRange, getMinVisionRange);
		PDATA_DEF(int, visionFOV, getVisionFOV);
		PDATA_DEF(int, maxHeat, getMaxHeat);
		PDATA_DEF(int, energy, getEnergy);
		PDATA_DEF(int, rechargeRate, getRechargeRate);
		PDATA_DEF(int, rechargeAmount, getRechargeAmount);
		PDATA_DEF(int, coolRate, getCoolRate);
		PDATA_DEF(int, coolAmount, getCoolAmount);
		PDATA_DEF(float, weaponAimRatio, getWeaponAimRatio);
		PDATA_DEF(float, characterAimRatio, getCharacterAimRatio);
		PDATA_DEF(int, aimingAccuracy, getAimingAccuracy);
		PDATA_DEF(int, aimingFOF, getAimingFOF);
		PDATA_DEF(int, preferFOF, getPreferFOF);
		PDATA_DEF(float, climbCost, getClimbCost);
		PDATA_DEF(float, maxClimb, getMaxClimb);
		PDATA_DEF(float, turningAccuracy, getTurningAccuracy);
		PDATA_DEF(float, gravityRatio, getGravityRatio);
		PDATA_DEF(bool, groundPlaneDirection, isGroundPlaneDirection);
		PDATA_DEF(bool, sticky, isSticky);
		PDATA_DEF(bool, flying, isFlying);
		PDATA_DEF(float, hover, getHover);
		PDATA_DEF(float, hoverVary, getHoverVary);
		PDATA_DEF(float, elevationSpeed, getElevationSpeed);
		PDATA_DEF(bool, stationary, isStationary);
		PDATA_DEF(float, turning, getTurning);
		PDATA_DEF(float, friction, getFriction);
		PDATA_DEF(float, acceleration, getAcceleration);
		PDATA_DEF(float, pathAccuracy, getPathAccuracy);
		PDATA_DEF(float, maxSpeed, getMaxSpeed);
		PDATA_DEF(float, slowSpeed, getSlowSpeed);
		PDATA_DEF(float, crawlSpeed, getCrawlSpeed);
		PDATA_DEF(float, sprintSpeed, getSprintSpeed);
		PDATA_DEF(float, jumpSpeed, getJumpSpeed);
		PDATA_DEF(UnitType::DEFAULT_SPEED, defaultSpeed, getDefaultSpeed);
		PDATA_DEF(float, backwardMovementFactor, getBackwardMovementFactor);
		PDATA_DEF(char *, bonesFilename, getBonesFilename);
		PDATA_DEF(char *, infoModelFilename, getInfoModelFilename);
		PDATA_DEF(char *, infoImageFilename, getInfoImageFilename);
		PDATA_DEF(char *, description, getDescription);
		//PDATA_DEF(int, animation, getAnimation);
		PDATA_DEF(char *, animation, getAnimation);
		PDATA_DEF(float, size, getSize);
		PDATA_DEF(int, flyVelocityRotation, getFlyVelocityRotation);
		PDATA_DEF(char *, explosionSound, getExplosionSound);
		PDATA_DEF(char *, haloType, getHaloType);
		PDATA_DEF(float, radarRatio, getRadarRatio);
		PDATA_DEF(int, radarRange, getRadarRange);
		PDATA_DEF(int, hp, getHP);
		PDATA_DEF(int, minHP, getMinHP);
		PDATA_DEF(int, blockRadius, getBlockRadius);
		PDATA_DEF(bool, blockIfDestroyed, isBlockIfDestroyed);
		PDATA_DEF(int, coverAvoid, getCoverAvoid);
		PDATA_DEF(int, coverBlock, getCoverBlock);
		PDATA_DEF(char *, aimBone, getAimBone);
		PDATA_DEF(int, aimBoneFOF, getAimBoneFOF);
		PDATA_DEF(bool, vehicle, isVehicle);
		PDATA_DEF(int, hpGain, getHPGain);
		PDATA_DEF(int, hpGainMax, getHPGainMax);
		PDATA_DEF(float, sightBonusAmount, getSightBonusAmount);
		PDATA_DEF(float, stealthRatio, getStealthRatio);
		PDATA_DEF(bool, removeDestroyed, doesRemoveDestroyed);
		PDATA_DEF(bool, allowRespawn, doesAllowRespawn);
		PDATA_DEF(bool, terrainifyDestroyed, doesTerrainifyDestroyed);
		PDATA_DEF(int, aimBoneDirection, getAimBoneDirection);
		PDATA_DEF(bool, rusher, isRusher);
		PDATA_DEF(float, aimHeightStanding, getAimHeightStanding);
		PDATA_DEF(float, aimHeightCrawling, getAimHeightCrawling);
		PDATA_DEF(bool, ignoreSpawnBlocking, doesIgnoreSpawnBlocking);
		PDATA_DEF(bool, lineBlock, isLineBlock);
		PDATA_DEF(bool, metallic, isMetallic);
		PDATA_DEF(int, lineBlockWidth, getLineBlockWidth);
		PDATA_DEF(int, aiDisableRange, getAIDisableRange);
		PDATA_DEF(bool, removeImmediately, doesRemoveImmediately);
		PDATA_DEF(int, lightAvoid, getLightAvoid);
		PDATA_DEF(int, pointedWith, getPointedWithMask);
		PDATA_DEF(bool, doorExecute, hasDoorExecute);
		PDATA_DEF(float, scale, getScale);
		PDATA_DEF(float, randomScale, getRandomScale);
		PDATA_DEF(bool, squashable, isSquashable);
		PDATA_DEF(float, deathBleedOffset, getDeathBleedOffset);
		PDATA_DEF(int, collisionCheckRadius, getCollisionCheckRadius);
		PDATA_DEF(bool, immortal, isImmortalByDefault);
		PDATA_DEF(int, baseRotation, getBaseRotation);
		PDATA_DEF(bool, flashlight, hasFlashlight);
		PDATA_DEF(bool, halo, hasHalo);
		PDATA_DEF(float, flashlightOffset, getFlashlightOffset);
		PDATA_DEF(bool, onlyOneWeaponSelected, hasOnlyOneWeaponSelected);
		PDATA_DEF(int, alwaysOnPlayerSide, getAlwaysOnPlayerSide);
		PDATA_DEF(int, rotationInterpolation, getRotationInterpolation);
		PDATA_DEF(int, positionInterpolation, getPositionInterpolation);
		PDATA_DEF(bool, jumpTurnable, isJumpTurnable);
		PDATA_DEF(bool, fireStopJump, doesFireStopJump);
		PDATA_DEF(int, jumpLandDelay, getJumpLandDelay);
		PDATA_DEF(bool, fireStopSprint, doesFireStopSprint);
		PDATA_DEF(bool, allowFireOnSprint, doesAllowFireOnSprint);
		PDATA_DEF(bool, rollJump, doesRollJump);
		PDATA_DEF(char *, jumpTiming, getJumpTiming);
		PDATA_DEF(char *, jumpTimingBackward, getJumpTimingBackward);
		PDATA_DEF(char *, jumpTimingSideways, getJumpTimingSideways);
		PDATA_DEF(char *, jumpSoundForward, getJumpSoundForward);
		PDATA_DEF(char *, jumpSoundBackward, getJumpSoundBackward);
		PDATA_DEF(char *, jumpSoundSideways, getJumpSoundSideways);
		PDATA_DEF(bool, fireWhileJumping, doesFireWhileJumping);
		PDATA_DEF(float, heightOffset, getHeightOffset);
		PDATA_DEF(float, eyeHeight, getEyeHeight);
		PDATA_DEF(bool, type2, isType2);
		PDATA_DEF(float, maxSlowdown, getMaxSlowdown);
		PDATA_DEF(float, slowdownAddFactor, getSlowdownAddFactor);
		PDATA_DEF(float, slowdownWearOff, getSlowdownWearOff);
		PDATA_DEF(bool, slowdownJump, doesSlowdownJump);
		PDATA_DEF(float, soundRange, getSoundRange);
		PDATA_DEF(bool, bothLineSides, hasBothLineSides);
		PDATA_DEF(bool, pushPlayer, doesPushPlayer);
		PDATA_DEF(bool, act, doesAct);
		//PDATA_DEF(char *, goreExplode, getGoreExplode);
		//PDATA_DEF(char *, goreSlice, getGoreSlice);
		PDATA_DEF(bool, noBoneCollision, hasNoBoneCollision);
		PDATA_DEF(int, turnTimeAtJumpStart, getTurnTimeAtJumpStart);
		PDATA_DEF(float, turningAtJumpStart, getTurningAtJumpStart);

		PDATA_DEF(float, shadowSizeX, getShadowSizeX);
		PDATA_DEF(float, shadowSizeZ, getShadowSizeZ);
		PDATA_DEF(float, shadowStrength, getShadowStrength);

		PDATA_DEF(int, targetLoseTime, getTargetLoseTime);
		PDATA_DEF(bool, disableEffectLayer, doesDisableEffectLayer);
		PDATA_DEF(bool, reactToStunningWeapons, doesReactToStunningWeapons);
		PDATA_DEF(int, hitAnimationTime, getHitAnimationTime);
		PDATA_DEF(int, hitAnimationAngle, getHitAnimationAngle);
		PDATA_DEF(float, hitAnimationTwistFactor, getHitAnimationTwistFactor);

		PDATA_DEF(bool, alwaysLOF, hasAlwaysLOF);
		PDATA_DEF(bool, noCollision, hasNoCollision);
		PDATA_DEF(bool, spottable, isSpottable);
		PDATA_DEF(bool, sideways, isSideways);
		PDATA_DEF(float, sideGravityX, getSideGravityX);
		PDATA_DEF(float, sideGravityZ, getSideGravityZ);
		PDATA_DEF(float, sideVelocityMax, getSideVelocityMax);

		PDATA_DEF(bool, rectangularBlock, isRectangularBlock);
		PDATA_DEF(int, blockStartZ, getBlockStartZ);
		PDATA_DEF(int, blockStopZ, getBlockStopZ);
		PDATA_DEF(int, blockJumpStartZ, getBlockJumpStartZ);
		PDATA_DEF(int, blockJumpStopZ, getBlockJumpStopZ);

		PDATA_DEF(bool, rectangularCollisionCheck, isRectangularCollisionCheck);
		PDATA_DEF(int, collisionCheckStartZ, getCollisionCheckStartZ);
		PDATA_DEF(int, collisionCheckStopZ, getCollisionCheckStopZ);
		PDATA_DEF(int, collisionCheckJumpStartZ, getCollisionCheckJumpStartZ);
		PDATA_DEF(int, collisionCheckJumpStopZ, getCollisionCheckJumpStopZ);

		PDATA_DEF(bool, ledgeGrab, doesLedgeGrab);
		PDATA_DEF(int, ledgeGrabX, getLedgeGrabX);
		PDATA_DEF(int, ledgeGrabZ, getLedgeGrabZ);

		PDATA_DEF(bool, physicsObject, hasPhysicsObject);
		PDATA_DEF(int, physicsObjectType, getPhysicsObjectType);
		PDATA_DEF(bool, physicsObjectDisabledAngularVelocity, hasPhysicsObjectDisabledAngularVelocity);
		PDATA_DEF(bool, physicsObjectDisabledYMovement, hasPhysicsObjectDisabledYMovement);
		PDATA_DEF(float, physicsObjectSizeX, getPhysicsObjectSizeX);
		PDATA_DEF(float, physicsObjectSizeY, getPhysicsObjectSizeY);
		PDATA_DEF(float, physicsObjectSizeZ, getPhysicsObjectSizeZ);
		PDATA_DEF(float, physicsObjectHeightOffset, getPhysicsObjectHeightOffset);
		PDATA_DEF(float, physicsObjectImpulseFactor, getPhysicsObjectImpulseFactor);
		PDATA_DEF(float, physicsObjectImpulseFeedbackFactor, getPhysicsObjectImpulseFeedbackFactor);
		PDATA_DEF(float, physicsObjectImpulseThreshold, getPhysicsObjectImpulseThreshold);
		PDATA_DEF(float, physicsObjectWarpThreshold, getPhysicsObjectWarpThreshold);
		PDATA_DEF(float, physicsObjectMass, getPhysicsObjectMass);
		PDATA_DEF(int, physicsObjectImplementationType, getPhysicsObjectImplementationType);
		PDATA_DEF(bool, physicsObjectIfDestroyed, hasPhysicsObjectIfDestroyed);
		PDATA_DEF(bool, physicsObjectDoor, isPhysicsObjectDoor);
		PDATA_DEF(int, physicsObjectCollisionGroup, getPhysicsObjectCollisionGroup);
		PDATA_DEF(bool, fluidContainmentPhysicsObject, hasFluidContainmentPhysicsObject);

		// added by Pete to allow gui to display health as
		PDATA_DEF( float, healthTextMultiplier, getHealthTextMultiplier );
			
		// added by Pete to allow the walk on material effects
		WalkOnMaterialData* getWalkOnMaterial( int material ) const;
		bool				getWalkOnMaterialEnabled() const;
		PDATA_DEF( int, walkOnMaterialParticleEffectTimeOut, getWalkOnMaterialParticleEffectTimeOut );
		PDATA_DEF( int, walkOnMaterialSoundEffectTimeOut, getWalkOnMaterialSoundEffectTimeOut );

		// outdoor breathing animation
		PDATA_DEF( std::string, breathingParticleEffect, getBreathingParticleEffect );
		PDATA_DEF( int, breathingParticleEffectTimeOut, getBreathingParticleEffectTimeOut );

		PDATA_DEF(int, lineOfFireLoseTime, getLineOfFireLoseTime);

		PDATA_DEF(float, poisonResistance, getPoisonResistance);

		PDATA_DEF(float, criticalHitPercent, getCriticalHitPercent );

		PDATA_DEF(TRACKABLE_TYPEID_DATATYPE, trackableTypeMask, getTrackableTypeMask);

		PDATA_DEF(bool, directControl, isDirectControl);
		PDATA_DEF(bool, sidewaysJumpMovable, isSidewaysJumpMovable);
		PDATA_DEF(bool, sidewaysJumpAllowedInAir, isSidewaysJumpAllowedInAir);
		PDATA_DEF(float, jumpAcceleration, getJumpAcceleration);
		PDATA_DEF(float, jumpAccelerationSpeedLimit, getJumpAccelerationSpeedLimit);

		PDATA_DEF(bool, useAimpointHelper, doesUseAimpointHelper);

		// permanent = unit is retired instead of deleted
		PDATA_DEF(bool, permanentPlayerUnit, isPermanentPlayerUnit);

		PDATA_DEF(float, weaponRangeOffset, getWeaponRangeOffset);
		PDATA_DEF(bool, mechControls, hasMechControls);
		PDATA_DEF(float, aimRotSpeed, getAimRotSpeed);
		PDATA_DEF(float, aimRotAccuracy, getAimRotAccuracy);
		PDATA_DEF(float, aimBetaRotSpeed, getAimBetaRotSpeed);
		PDATA_DEF(float, aimBetaRotAccuracy, getAimBetaRotAccuracy);
		PDATA_DEF(float, aimBetaRotLimit, getAimBetaRotLimit);
		PDATA_DEF(char *, turningSound, getTurningSound);
		PDATA_DEF(float, turningSoundMinPlayTime, getTurningSoundMinPlayTime);

		PDATA_DEF(float, healthSliderScale, getHealthSliderScale);
		PDATA_DEF(float, targetRectScale, getTargetRectScale);
		PDATA_DEF(float, targetRectHeightScale, getTargetRectHeightScale);

		PDATA_DEF(bool, sentryGunControls, hasSentryGunControls);
		PDATA_DEF(char *, directControlType, getDirectControlType);

		PDATA_DEF(float, backDamageFactor, getBackDamageFactor);

		PDATA_DEF(bool, runHitScriptWithoutShooter, doesRunHitScriptWithoutShooter);

		PDATA_DEF(bool, allowTargetLock, doesAllowTargetLock);

		std::vector<FootStepTrigger> footStepTriggers;

	private:
		int goreAmount[GORETYPE_AMOUNT];
		char *gore[GORETYPE_AMOUNT][UNITTYPE_MAX_GORE_AMOUNT];
		char *goreSound[GORETYPE_AMOUNT][UNITTYPE_MAX_GORE_AMOUNT];
		char *gorePart[GORETYPE_AMOUNT][UNITTYPE_MAX_GORE_AMOUNT];

		std::vector< WalkOnMaterialData* >	walkOnMaterialArray;
		WalkOnMaterialData* walkOnMaterialDefault;
		bool								walkOnMaterialEnabled;

	public:
		// TODO: assert that goretype/number within MAX limits...
		const char *getGore(int goreType, int number) const { return gore[goreType][number]; }
		const char *getGoreSound(int goreType, int number) const { return goreSound[goreType][number]; }
		const char *getGorePart(int goreType, int number) const { return gorePart[goreType][number]; }
		int getGoreAmount(int goreType) const { return goreAmount[goreType]; }

	public:
		virtual ~UnitType() 
		{
			if (unitTypeId != 0)
			{
				unitTypeIds.remove(this);
				unitTypeId = 0;
			}
			if (name != NULL) { delete [] name; name = NULL; }
			if (animation != NULL) { delete [] animation; animation = NULL; }
			//if (actor != NULL) { delete actor; actor = NULL; }
			PDATA_DELETE_ARRAY(description);
			PDATA_DELETE_ARRAY(infoModelFilename);
			PDATA_DELETE_ARRAY(infoImageFilename);
			PDATA_DELETE_ARRAY(bonesFilename);
			PDATA_DELETE_ARRAY(explosionSound);
			PDATA_DELETE_ARRAY(aimBone);
			PDATA_DELETE_ARRAY(haloType);
			PDATA_DELETE_ARRAY(jumpTiming);
			PDATA_DELETE_ARRAY(jumpTimingBackward);
			PDATA_DELETE_ARRAY(jumpTimingSideways);
			PDATA_DELETE_ARRAY(jumpSoundForward);
			PDATA_DELETE_ARRAY(jumpSoundBackward);
			PDATA_DELETE_ARRAY(jumpSoundSideways);
			for (int gor = 0; gor < GORETYPE_AMOUNT; gor++)
			{
				for (int gorenum = 0; gorenum < UNITTYPE_MAX_GORE_AMOUNT; gorenum++)
				{
					PDATA_DELETE_ARRAY(gore[gor][gorenum]);
					PDATA_DELETE_ARRAY(goreSound[gor][gorenum]);
					PDATA_DELETE_ARRAY(gorePart[gor][gorenum]);
				}
			}

			for( unsigned int i = 0; i < walkOnMaterialArray.size(); ++i )
			{
				delete walkOnMaterialArray[ i ];
			}
			delete walkOnMaterialDefault;
			PDATA_DELETE_ARRAY(directControlType);
		}
	};

	extern UnitType *getUnitTypeById(int id);

	extern UnitType *getUnitTypeByName(const char *name);

}

#endif


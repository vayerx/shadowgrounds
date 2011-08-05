
#include "precompiled.h"

#include "gamedefs.h"
#include "scaledefs.h"
#include "unittypes.h"
#include "Unit.h"
#include "UnitType.h"
#include "UnitVisibility.h"
#include "Character.h"
#include "Part.h"
#include "Weapon.h"
#include "WeaponObject.h"
#include "PowerCell.h"
#include "Reactor.h"
#include "AmmoPack.h"
#include "AmmoPackObject.h"
#include "Projectile.h"
#include "Item.h"
#include "Flashlight.h"

#include <string>

#include "../system/Logger.h"
#include "../util/AI_PathFind.h"
#include "../ui/AnimationSet.h"
#include "../ui/VisualEffect.h"

#include "../util/Debug_MemoryManager.h"

#define UNIT_MAX_ARMOR_AMOUNT 100
#define UNIT_MAX_ARMOR_CLASS 3

// at how many tick interval are unit backtrack positions sampled
#define UNIT_BACKTRACK_INTERVAL 8

namespace game
{
	// for survivors snow levels
	float Unit::visualizationOffset = 0;
	bool Unit::visualOffsetInUse = false;

	const int unitDataId = 0;

	SaveData *Unit::getSaveData() const
	{
		// TODO
		return null;
	}

	const char *Unit::getStatusInfo() const
	{
		// WARNING: return value valid only until next call
		static std::string unit_get_status_info_buf;
		unit_get_status_info_buf = std::string("Unit (");
		if (this->idString != NULL)
		{
			unit_get_status_info_buf += this->idString;
		} else {
			unit_get_status_info_buf += "-";
		}
		unit_get_status_info_buf += ", ";
		if (this->owner != NO_UNIT_OWNER)
		{
			unit_get_status_info_buf += int2str(owner);
		} else {
			unit_get_status_info_buf += "no_owner";
		}
		unit_get_status_info_buf += ")";

		return unit_get_status_info_buf.c_str();
	}

	void *Unit::getVisualObjectDataId() const
	{ 
		return (void *)&unitDataId; 
	}

	//	Unit::Unit(int owner)
	Unit::Unit() :
		movingForward(false),
		movingBackward(false),
		movingSideways(false),
		visualizationOffsetInterpolation( 1.0f )
	{
		//this->owner = owner;
		owner = NO_UNIT_OWNER;
		rootPart = NULL;
		character = NULL;

		idNumber = 0;

		path = NULL;
		atPathEnd = true;
		pathIndex = 0;
		pathIsStored = false;

		visualObject = NULL;

		position = VC3(0, 0, 0);
		waypoint = VC3(0, 0, 0);
		finalDestination = VC3(0, 0, 0);
		velocity = VC3(0, 0, 0);
		xAngle = 0;
		yAngle = 0;
		zAngle = 0;
		rotateYSpeed = 0;

		lastXRotation = 0;
		lastZRotation = 0;

		active = false;
		selected = false;
		mode = UNIT_MODE_DEFENSIVE;

		startEnergy = 0;
		maxEnergy = 0;
		energy = 0;
		heat = 0;
		maxHeat = 0;

		hp = 0;
		maxHP = 0;

		poisonResistance = 0;
		criticalHitPercent = -1;
		moveInReverseDir = false;
		turningSound = -1;
		turningSoundStartTime = 0;
		turningSoundVolume = 0;

		hpGainLimit = 0.0f;
		hpGainAmount = 0;
		hpGainDelay = 0;
		hpGainStartTime = -1; // disable hp gain
		hpGainStartDelay = 0;
		hpGainDamageFactor = 0.0f;
		lastTimeDamaged = 0;

		sightBonus = 0;

		rechargingAmount = 0;
		coolingAmount = 0;

		weight = 1;

		lastBoneAimDirection = 0;
		lastBoneAimBetaAngle = 0;
		flashlightDirection = 0;

		runningValue = 0;
		stealthValue = 0;
		reconValue = 0;

		stealthing = false;

		currentAnimation = 0;  // ANIM_NONE!
		for (int a = 0; a < UNIT_MAX_BLEND_ANIMATIONS; a++)
		{
			currentBlendAnimation[a] = 0;  // ANIM_NONE!
		}
		animationSpeedFactor = 1.0f;

		animationTimeLeft = 0;

		walkDelay = 0;
		int i;
		for (i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			fireWaitDelay[i] = 0;
			fireReloadDelay[i] = 0;
			weaponType[i] = NULL;
			weaponPosition[i] = SLOT_POSITION_EXTERNAL_ITEM;
			weaponAmmoNumber[i] = -1;
			weaponActive[i] = false;
			weaponOperable[i] = false;
			weaponVisible[i] = false;
			weaponFireTime[i] = 0;
			weaponCopyProjectile[i] = NULL;
			weaponLoopSoundHandle[i] = -1;
			weaponLoopSoundKey[i] = -1;
			weaponSoundHandle[i] = -1;
			weaponAmmoInClip[i] = 0;
			weaponBarrel[i] = 0;
			weaponEjectBarrel[i] = 0;
		}
		for (i = 0; i < UNIT_MAX_AMMOS; i++)
		{
			ammoAmount[i] = 0;
			maxAmmoAmount[i] = 0;
			ammoType[i] = NULL;
		}
		maxWeaponRange = 0.0f;

		destroyed = false;

		seeingUnit = NULL;
		seeingUnitDistance = 0;
		toBeSeeingUnit = NULL;
		toBeSeeingUnitDistance = 0;

		unitTypeId = 0;

		onGround = true;
		groundFriction = true;

		ai = NULL;
		script = NULL;

		leader = NULL;

		speed = UNIT_SPEED_FAST;
		turning = false;
		turningToAngle = 0;

		moveState = UNIT_MOVE_STATE_NORMAL;
		moveStateCounter = 0;

		hitDelay = 0;
		hitMissDelay = 0;
		pointedDelay = 0;
		hearNoiseDelay = 0;
		spottedDelay = 0;
		hitByUnit = NULL;
		hitByBullet = NULL;
		hitMissByUnit = NULL;
		pointedByUnit = NULL;
		hearNoiseByUnit = NULL;

		hasSpawn = false;
		spawn = VC3(0,0,0);

		idleTime = 0;

		obstacleX = -1;
		obstacleY = -1;
		obstacleExists = false;
		obstacleAngle = 0;
		obstacleOverlaps = false;

		lookBetaAngle = 0;
		directControl = false;
		directControlType = UNIT_DIRECT_CONTROL_TYPE_NONE;

		groupNumber = 0;

		stepNoiseCounter = 0;

		animationSet = NULL;

		stealthVisualInUse = false;

		visibility = UnitVisibility(NO_UNIT_OWNER);

		reconAvailable = false;

		firingSpreadFactor = 1.0f;

		rushDistance = 0.0f;

		armorAmount = 0;
		armorClass = 0;

		spotlight = NULL;
		spotlight2 = NULL;

		muzzleflashVisualEffect = NULL;
		muzzleflashDuration = 0;

		ejectVisualEffect = NULL;
		ejectDuration = 0;

		pointerVisualEffect = NULL;
		pointerHitVisualEffect = NULL;

		deathScriptHasRun = false;

		clipReloading = false;

		lastMoveStrafed = false;

		unitMirrorSide = false;

		forcedAnim = 0; // ANIM_NONE

		items = NULL;

		touchProjectileEnabled = 1;

		selectedWeapon = -1;
		selectedSecondaryWeapon = -1;

		blockedTime = 0;
		nonBlockedTime = 0;

		flashlight = NULL;

		lastPathfindSuccess = true;

		immortal = false;
		immortalWithHitScript = false;
		jumpCounter = 0;
		jumpTotalTime = 0;
		turnedTime = 0;

		deathBleedDelay = 0;

		animated = false;
		firingInProgress = false;

		for (i = 0; i < UNIT_BACKTRACK_AMOUNT; i++)
		{
			backtrackPositions[i] = VC3(0,0,0);
			backtrackMoves[i] = false;
		}
		currentBacktrackSlot = 0;
		backtrackCounter = 0;

		idString = NULL;

		strafeRotationOffset = 0;
		strafeAimOffset = 0;

		this->jumpCamForward = false;
		this->jumpCamBackward = false;
		this->jumpCamLeft = false;
		this->jumpCamRight = false;
		this->jumpUnitForward = false;
		this->jumpUnitBackward = false;
		this->jumpUnitLeft = false;
		this->jumpUnitRight = false;

		this->jumpNotAllowedTime = 0;

		this->pendingWeaponChangeRequest = -1;

		this->ghostOfFuture = false;
		this->ghostTime = 0;

		slowdown = 0.0f;

		effectLayerType = UNIT_EFFECT_LAYER_NONE;
		effectLayerDuration = 0;

		this->idleRequest = 0;

		this->keepFiring = false;
		this->keepReloading = false;

		this->useAIDisableRange = true;

		this->selectedItem = -1;

		this->currentLighting = 1.0f;
		this->fadeLightingTo = 1.0f;
		this->fadeLightingTimeLeft = 0;
		this->fadeLightingTimeTotal = 1;

		this->currentVisibility = 1.0f;
		this->fadeVisibilityTo = 1.0f;
		this->fadeVisibilityTimeLeft = 0;
		this->fadeVisibilityTimeTotal = 1;

		this->lightVisibilityFactor = 0;
		this->forceLightVisibilityFactor = 0;

		this->collisionCheck = false;
		this->collisionBlocksOthers = true;

		this->areaTriggered = false;
		this->areaRange = 0;
		this->areaClipMask = 0;
		this->areaCenter = VC3(0,0,0);
		this->areaCircleId = -1;

		this->followPlayer = false;

		this->lastLightUpdatePosition = VC3(-999999,-999999,-999999);

		this->unitListEntity = NULL;

		this->continuousFireTime = 0;

		this->aliveMarker = true;
		//this->ignoreRespawn = false;
		//this->loseOnRespawn = false;

		this->sweepDirection = 1;

		this->executeTipText = NULL;

		this->diedByPoison = false;

		this->shootAnimStanding = false;

		this->delayedHitProjectileBullet = NULL;
		this->delayedHitProjectileInterval = 0;
		this->delayedHitProjectileAmount = 0;

		this->aniRecordBlendFlag = ANIM_NONE;
		this->aniRecordBlendEndFlag = false;

		this->aniRecordFireFlag = false;
		this->aniRecordFireSourcePosition = VC3(0,0,0);
		this->aniRecordFireDestinationPosition = VC3(0,0,0);

		highlightStyle = -1;
		highlightText = "";

		this->jumpAnim = ANIM_NONE;

		this->weaponCharging = false;
		this->weaponChargeAmount = 0;
		this->weaponChargeMin = 0;
		this->weaponChargePeak = 0;
		this->weaponChargeSteps = 0;

		this->disappearCounter = 0;

		this->hitAnimationCounter = 0;

		this->destroyedTime = 0;

		this->hitAnimationBoneAngle = 0;
		this->hitAnimationVector = VC3();
		this->hitAnimationFactor = 0;

		this->spottable = true;

		this->clipEmptySoundDone = false;
		this->clipEmptyTime = 0;

		this->aimStopCounter = 0;

		this->onFireCounter = 0;

		this->burnedCrispyAmount = 0;

		this->lastRotationDirection = 0;

		this->actCheckCounter = 0;
		this->acted = true;

		this->animationLastPosition = VC3(0,0,0);
		this->onPhysicsObject = false;
		this->onSlope = false;

		this->physicsObject = NULL;
		this->fluidContainmentPhysicsObject = NULL;

		this->ejectRateCounter = 0;

		this->physicsObjectFeedbackEnabled = true;
		this->physicsObjectDifference = 0.0f;
		this->physicsObjectLock = false;

		this->sideways = false;
		this->sideGravityX = 0.0f;
		this->sideGravityZ = 0.0f;
		this->sideVelocityMax = 0.0f;

		this->customTimeFactor = 1.0f;
		this->lastTargetLockTime = 0;
		this->targetLockCounter = 0;
		this->targetLockCounterMax = 0;
		this->targetLockReleaseTime = 0;
		this->targetLockCancelTime = 0;
		this->targetLockSoundPlayed = false;
		this->lastTargetLockUnit = NULL;

		this->speedBeforeFiring = -1;

		this->setForcedAnimation( 0 );

		this->fallenOnBack = false;
		this->turningSoundFrequency = 0;

		this->aiDirectControl = NULL;

		this->launchSpeed = 0.0f;
		this->launchNow = false;

		this->shielded = false;
	}



	Unit::~Unit()
	{
		if (character != NULL) character->setUnit(NULL);
		if (script != NULL)
		{
			delete [] script;
			script = NULL;
		}
		if (idString != NULL)
		{
			delete [] idString;
			idString = NULL;
		}
		if (executeTipText != NULL)
		{
			delete [] executeTipText;
			executeTipText = NULL;
		}
		for (int j = 0; j < UNIT_MAX_WEAPONS; j++)
		{
			if (weaponCopyProjectile[j] != NULL)
			{
				delete weaponCopyProjectile[j];
			}
		}
		if (path != NULL) 
		{
			if (!pathIsStored)
				delete path;
		}

		if (spotlight != NULL)
		{
			assert(!"Spotlight was not properly deleted");
			/*
			delete spotlight;
			spotlight = NULL;
			*/
		}

		if (muzzleflashVisualEffect != NULL)
		{
			muzzleflashVisualEffect->setDeleteFlag();
			muzzleflashVisualEffect->freeReference();
			muzzleflashVisualEffect = NULL;
		}

		if (ejectVisualEffect != NULL)
		{
			ejectVisualEffect->setDeleteFlag();
			ejectVisualEffect->freeReference();
			ejectVisualEffect = NULL;
		}

		if (pointerVisualEffect != NULL)
		{
			pointerVisualEffect->setDeleteFlag();
			pointerVisualEffect->freeReference();
			pointerVisualEffect = NULL;
		}

		if (pointerHitVisualEffect != NULL)
		{
			pointerHitVisualEffect->setDeleteFlag();
			pointerHitVisualEffect->freeReference();
			pointerHitVisualEffect = NULL;
		}

		if (items != NULL)
		{
			for (int i = 0; i < UNIT_MAX_ITEMS; i++)
			{
				if (items[i] != NULL) delete items[i];
			}
			delete [] items;
		}

		if (flashlight != NULL)
		{
			assert(!"Unit still has a flashlight at destructor.");
			delete flashlight;
			flashlight = NULL;
		}

		deleteCustomizedWeaponTypes();

		for (unsigned int i = 0; i < footStepTriggers.size(); i++) {
			delete footStepTriggers[i];
			footStepTriggers[i] = NULL;
		}

		// TODO, delete all parts under this one
	}

	void Unit::setAI(void *ai)
	{
		this->ai = ai;
	}

	void *Unit::getAI() const
	{
		return ai;
	}

	const char *Unit::getIdString() const
	{
		return idString;
	}

	void Unit::setIdString(const char *idstring)
	{
		if (this->idString != NULL)
		{
			delete [] this->idString;
			this->idString = NULL;
		}
		if (idstring != NULL)
		{
			this->idString = new char[strlen(idstring) + 1];
			strcpy(this->idString, idstring);
		}
	}

	int Unit::getIdNumber() const
	{
		return this->idNumber;
	}

	void Unit::setIdNumber(int id)
	{
		this->idNumber = id;
	}

	const char *Unit::getExecuteTipText() const
	{
		return executeTipText;
	}

	void Unit::setExecuteTipText(const char *executeTip)
	{
		if (this->executeTipText != NULL)
		{
			delete [] this->executeTipText;
			this->executeTipText = NULL;
		}
		if (executeTip!= NULL)
		{
			this->executeTipText = new char[strlen(executeTip) + 1];
			strcpy(this->executeTipText, executeTip);
		}
	}

	void Unit::setPointerVisualEffect(ui::VisualEffect *visualEffect)
	{
		if(pointerVisualEffect != NULL)
		{
			pointerVisualEffect->setDeleteFlag();
			pointerVisualEffect->freeReference();
		}

		pointerVisualEffect = visualEffect;

		if(pointerVisualEffect != NULL)
		{
			pointerVisualEffect->addReference();
		}
	}

	ui::VisualEffect *Unit::getPointerVisualEffect(void) const
	{
		return pointerVisualEffect;
	}

	void Unit::setPointerHitVisualEffect(ui::VisualEffect *visualEffect)
	{
		if(pointerHitVisualEffect != NULL)
		{
			pointerHitVisualEffect->setDeleteFlag();
			pointerHitVisualEffect->freeReference();
		}

		pointerHitVisualEffect = visualEffect;

		if(pointerHitVisualEffect != NULL)
		{
			pointerHitVisualEffect->addReference();
		}
	}

	ui::VisualEffect *Unit::getPointerHitVisualEffect(void) const
	{
		return pointerHitVisualEffect;
	}

	void Unit::setMuzzleflashVisualEffect(ui::VisualEffect *visualEffect,
		int muzzleflashDuration)
	{
		//assert(visualEffect != NULL);
			
		if (this->muzzleflashVisualEffect != NULL)
		{
			this->muzzleflashVisualEffect->setDeleteFlag();
			this->muzzleflashVisualEffect->freeReference();
		}
		this->muzzleflashVisualEffect = visualEffect;
		if (visualEffect != NULL)
			visualEffect->addReference();

		if (muzzleflashDuration <= 0 && visualEffect != NULL)
		{
			assert(!"Muzzleflash duration less than 1");
			muzzleflashDuration = 1;
		}
		this->muzzleflashDuration = muzzleflashDuration;
	}


	void Unit::advanceMuzzleflashVisualEffect()
	{
		if (muzzleflashDuration > 0)
		{
			muzzleflashDuration--;
			if (muzzleflashDuration == 0)
			{
				if (muzzleflashVisualEffect != NULL)
				{
					muzzleflashVisualEffect->setDeleteFlag();
					muzzleflashVisualEffect->freeReference();
					muzzleflashVisualEffect = NULL;
				}
			}
		}
	}


	void Unit::resetMuzzleflashDuration(int muzzleflashDuration, int atTime)
	{
		assert(isMuzzleflashVisible());
		if (muzzleflashDuration <= 0)
		{
			assert(!"Unit::resetMuzzleflashDuration - Muzzleflash duration less than 1");
			return;
		}

		if (this->muzzleflashVisualEffect != NULL)
		{
			this->muzzleflashVisualEffect->resetSpotlightFadeout(atTime);
		}
		this->muzzleflashDuration = muzzleflashDuration;
	}


	bool Unit::isMuzzleflashVisible() const
	{
		if (muzzleflashDuration > 0)
		{
			return true;
		} else {
			return false;
		}
	}


	void Unit::setEjectVisualEffect(ui::VisualEffect *visualEffect,
		int ejectDuration)
	{
		//assert(visualEffect != NULL);
			
		if (this->ejectVisualEffect != NULL)
		{
			this->ejectVisualEffect->setDeleteFlag();
			this->ejectVisualEffect->freeReference();
		}
		this->ejectVisualEffect = visualEffect;
		if (visualEffect != NULL)
			visualEffect->addReference();

		if (ejectDuration <= 0 && visualEffect != NULL)
		{
			assert(!"Eject duration less than 1");
			ejectDuration = 1;
		}
		this->ejectDuration = ejectDuration;
	}


	void Unit::advanceEjectVisualEffect()
	{
		if (ejectDuration > 0)
		{
			ejectDuration--;
			if (ejectDuration == 0)
			{
				if (ejectVisualEffect != NULL)
				{
					ejectVisualEffect->setDeleteFlag();
					ejectVisualEffect->freeReference();
					ejectVisualEffect = NULL;
				}
			}
		}
	}


	void Unit::resetEjectDuration(int ejectDuration, int atTime)
	{
		assert(isEjectVisible());
		if (ejectDuration <= 0)
		{
			assert(!"Unit::resetEjectDuration - Eject duration less than 1");
			return;
		}

		if (this->ejectVisualEffect != NULL)
		{
			this->ejectVisualEffect->resetSpotlightFadeout(atTime);
		}
		this->ejectDuration = ejectDuration;
	}


	bool Unit::isEjectVisible() const
	{
		if (ejectDuration > 0)
		{
			return true;
		} else {
			return false;
		}
	}


	void Unit::restoreDefaultSpeed()
	{
		// TODO: cache the unittype DEFAULT_SPEED enum to corresponding UNIT_SPEED enum?
		// (or rely on fast / slow being so common cases, that this if clause performance is not relevant)

		assert(unitType != NULL);

		if (unitType->getDefaultSpeed() == UnitType::DEFAULT_SPEED_FAST)
			this->speed = Unit::UNIT_SPEED_FAST;
		else if (unitType->getDefaultSpeed() == UnitType::DEFAULT_SPEED_SLOW)
			this->speed = Unit::UNIT_SPEED_SLOW;
		else if (unitType->getDefaultSpeed() == UnitType::DEFAULT_SPEED_CRAWL)
			this->speed = Unit::UNIT_SPEED_CRAWL;
		else if (unitType->getDefaultSpeed() == UnitType::DEFAULT_SPEED_SPRINT)
			this->speed = Unit::UNIT_SPEED_SPRINT;
		else 
		{
			assert(!"Unit::restoreDefaultSpeed - unknown default speed type, this should never happen.");
		}
	}

	void Unit::setSpeed(UNIT_SPEED speed)
	{
		// keep up to date
		/*if(speedBeforeFiring != -1)
		{
			speedBeforeFiring = (int) speed;
		}*/
		this->speed = speed;
	}

	void Unit::setOwner(int player)
	{
		this->owner = player;
		// reset the visibility thingy... (for the new owner)
		visibility = game::UnitVisibility(player);
	}

	/*
	// inlined
	Unit::UNIT_SPEED Unit::getSpeed()
	{
		return speed;
	}
	*/

	void Unit::setTurnToAngle(float angle)
	{
		this->turning = true;
		this->turningToAngle = angle;
	}

	void Unit::stopTurning()
	{
		turning = false;
	}

	bool Unit::isTurning() const
	{
		return turning;
	}

	float Unit::getTurnToAngle() const
	{
		return this->turningToAngle;
	}
	
	float Unit::getAngleTo(const VC3 &toPosition) const
	{
		VC2 destFloat = VC2(
			(float)(toPosition.x-position.x), (float)(toPosition.z-position.z));
		float destAngleFloat = destFloat.CalculateAngle();
		float destAngle = -RAD_TO_UNIT_ANGLE(destAngleFloat) + (360+270);
		while (destAngle >= 360) destAngle -= 360;
		return destAngle;
	}

	void Unit::setScript(const char *scriptName)
	{
		if (this->script != NULL)
		{
			delete [] this->script;
			this->script = NULL;
		}
		if (scriptName != NULL)
		{
			this->script = new char[strlen(scriptName) + 1];
			strcpy(this->script, scriptName);
		}
	}

	char *Unit::getScript() const
	{
		return script;
	}

	int Unit::getBlockedTime() const
	{
		return blockedTime;
	}

	void Unit::increaseBlockedTime()
	{
		blockedTime++;
	}

	void Unit::clearBlockedTime()
	{
		blockedTime = 0;
	}

	int Unit::getNonBlockedTime() const
	{
		return nonBlockedTime;
	}

	void Unit::increaseNonBlockedTime()
	{
		nonBlockedTime++;
	}

	void Unit::clearNonBlockedTime()
	{
		nonBlockedTime = 0;
	}

	const VC3 &Unit::getPointerPosition() const
	{
		return position;
	}

	const VC3 Unit::getPointerMiddleOffset() const
	{
		if (speed == Unit::UNIT_SPEED_CRAWL
			|| destroyed || moveState == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
			return VC3(0, unitType->getAimHeightCrawling(), 0);
		else
			return VC3(0, unitType->getAimHeightStanding(), 0);
	}


	Part *Unit::getRootPart() const
	{
		return rootPart;
	}

	void Unit::setRootPart(Part *part)
	{
		rootPart = part;
	}

	Character *Unit::getCharacter() const
	{
		return character;
	}

	void Unit::setCharacter(Character *character)
	{
		if (this->character != NULL) this->character->setUnit(NULL);
		this->character = character;
		if (this->character != NULL) this->character->setUnit(this);
	}

	/*
	// inlined
	bool Unit::isActive()
	{
		return active;
	}
	*/

	void Unit::setActive(bool active)
	{
		this->active = active;
	}

	bool Unit::isSelected() const
	{
		return selected;
	}

	void Unit::setSelected(bool selected)
	{
		this->selected = selected;
	}

	Unit::UNIT_MODE Unit::getMode() const
	{
		return mode;
	}

	void Unit::setMode(UNIT_MODE mode)
	{
		this->mode = mode;
	}

	void Unit::setMoveState(UNIT_MOVE_STATE moveState)
	{
		//assert(this->moveState != UNIT_MOVE_STATE_UNCONSCIOUS
		//	|| moveState == UNIT_MOVE_STATE_UNCONSCIOUS || hp >= 0);
		assert((moveState != UNIT_MOVE_STATE_UNCONSCIOUS && hp >= 0)
			|| (moveState == UNIT_MOVE_STATE_UNCONSCIOUS || hp < 0));

		this->moveState = moveState;
	}

	Unit::UNIT_MOVE_STATE Unit::getMoveState() const
	{
		return moveState;
	}
	
	void Unit::setMoveStateCounter(int moveStateCounter)
	{
		this->moveStateCounter = moveStateCounter;
	}


	int Unit::getMoveStateCounter() const
	{
		return moveStateCounter;
	}

	int Unit::getUnitTypeId() const
	{
		return unitTypeId;
	}

	void Unit::setUnitTypeId(int id)
	{
		unitTypeId = id;
	}

	/*
	UnitType *Unit::getUnitType()
	{
		return unitType;
	}
	*/

	void Unit::setUnitType(UnitType *unitType)
	{
		this->unitType = unitType;
		if (unitType != NULL)
		{
			this->targeting.setTargetLoseTime(unitType->getTargetLoseTime());

			// read defaults from unit type
			poisonResistance = unitType->getPoisonResistance();
			criticalHitPercent = unitType->getCriticalHitPercent();

			const std::vector<FootStepTrigger> &triggers = unitType->getFootStepTriggers();
			footStepTriggers.resize(triggers.size());
			for(unsigned int i = 0; i < footStepTriggers.size(); i++)
			{
				footStepTriggers[i] = new FootStepTrigger;
				(*footStepTriggers[i]) = triggers[i];
			}

			this->restoreDefaultSpeed();
		}
	}

	void Unit::setPosition(const VC3 &position)
	{
		this->position = position;
		if (visualObject != NULL)
		{
			// for the survivors snow levels 
			if( visualOffsetInUse )
			{			
				VC3 pos( this->position );
				pos.y += ( visualizationOffset * visualizationOffsetInterpolation );
				visualObject->setPosition( pos );
			}
			else
			{
				visualObject->setPosition( position );
			}
		}
		/*
		VC3 offset = position - this->position;
		this->position += offset;
		if (visualObject != NULL)
		{
			visualObject->addPosition(offset);
		}
		*/
		//if (rootPart != NULL)
		//{
		//	moveParts(rootPart, offset);
		//}
	}

	bool Unit::isOnGround() const
	{
		return onGround;
	}

	void Unit::setOnGround(bool onGround)
	{
		this->onGround = onGround;
	}

	bool Unit::isGroundFriction() const
	{
		return groundFriction;
	}

	void Unit::setGroundFriction(bool groundFriction)
	{
		this->groundFriction = groundFriction;
	}

	void Unit::setDeathBleedDelay(int bleedDelay)
	{
		this->deathBleedDelay = bleedDelay;
	}

	int Unit::getDeathBleedDelay() const
	{
		return this->deathBleedDelay;
	}

	/*
	void Unit::setVelocity(VC3 &velocity)
	{
		this->velocity = velocity;
	}

	void Unit::setWaypoint(VC3 &waypoint)
	{
		this->waypoint = waypoint;
	}

	void Unit::setFinalDestination(VC3 &finalDestination)
	{
		this->finalDestination = finalDestination;
	}
	*/

	void Unit::setRotateYSpeed(int rotateYSpeed)
	{
		this->rotateYSpeed = rotateYSpeed;
	}

	int Unit::getRotateYSpeed() const
	{
		return rotateYSpeed;
	}

	void Unit::setRotation(float xAngle, float yAngle, float zAngle)
	{
		assert(yAngle >= 0 && yAngle <= 360);

		this->xAngle = xAngle;
		this->yAngle = yAngle;
		this->zAngle = zAngle;
		/*
		QUAT rotation = QUAT(
			(float)xAngle * 3.1415f/180, 
			(float)yAngle * 3.1415f/180, 
			(float)zAngle * 3.1415f/180);
		*/
		if (visualObject != NULL)
		{
			float visYAngle = yAngle + strafeRotationOffset;
			if (visYAngle >= 360.0f) visYAngle -= 360.0f;
			if (visYAngle < 0.0f) visYAngle += 360.0f;

			visualObject->setRotation(xAngle, visYAngle, zAngle);
		}
		//if (rootPart != NULL)
		//{
		//	rotateParts(rootPart, xAngle, yAngle, zAngle);
		//}
	}

	/*
	VC3 Unit::getPosition()
	{
		return position;
	}

	VC3 Unit::getVelocity()
	{
		return velocity;
	}

	VC3 Unit::getWaypoint()
	{
		return waypoint;
	}

	VC3 Unit::getFinalDestination()
	{
		return finalDestination;
	}

	VC3 Unit::getRotation()
	{
		VC3 ret = VC3(xAngle, yAngle, zAngle);
		return ret;
	}
	*/

	void Unit::setVisualObject(ui::VisualObject *visualObject)
	{
		this->visualObject = visualObject;
		// recreate pointer visual
		setPointerVisualEffect(NULL);
		setPointerHitVisualEffect(NULL);
	}

	/*
	ui::VisualObject *Unit::getVisualObject()
	{
		return visualObject;
	}
	*/

	/*
	void Unit::moveParts(Part *part, VC3 position)
	{
		if (part->getVisualObject() != NULL)
		{
			part->getVisualObject()->addPosition(position);
		}

		int slots = part->getType()->getSlotAmount();
		for (int i = 0; i < slots; i++)
		{
			Part *subp;
			if ((subp = part->getSubPart(i)) != NULL)
			{
				moveParts(subp, position);
			}
		}
	}

	void Unit::rotateParts(Part *part, float xAngle, float yAngle, float zAngle)
	{
		if (part->getVisualObject() != NULL)
			part->getVisualObject()->setRotation(xAngle, yAngle, zAngle);
		// TODO...
		// ... kinda like the move above
	}
	*/

	bool Unit::isStealthVisualInUse() const
	{
		return stealthVisualInUse;
	}

	void Unit::setStealthVisualInUse(bool useStealth)
	{
		assert(stealthValue > 0);
		if (visualObject != NULL
			&& ((!stealthVisualInUse && useStealth)
			|| (stealthVisualInUse && !useStealth)))
		{
			if (useStealth)
			{
				visualObject->setEffect(VISUALOBJECTMODEL_EFFECT_ADDITIVE);
				visualObject->setEffect(VISUALOBJECTMODEL_EFFECT_LESS_DIFFUSE);
			} else {
				visualObject->clearEffects();
			}
			stealthVisualInUse = useStealth;
		} 
	}

	int Unit::getHeat() const
	{
		return heat;
	}

	int Unit::getMaxHeat() const
	{
		return maxHeat;
	}

	int Unit::getEnergy() const
	{
		return energy;
	}

	int Unit::getMaxEnergy() const
	{
		return maxEnergy;
	}

	int Unit::getStartEnergy() const
	{
		return startEnergy;
	}

	int Unit::getRechargingAmount() const
	{
		return rechargingAmount;
	}

	int Unit::getCoolingAmount() const
	{
		return coolingAmount;
	}

	int Unit::getWeight() const
	{
		return weight;
	}

	int Unit::getRunningValue() const
	{
		return runningValue;
	}

	int Unit::getStealthValue() const
	{
		return stealthValue;
	}

	int Unit::getReconValue() const
	{
		return reconValue;
	}

	void Unit::calculateMaxEnergy()
	{
		//UnitType *ut = getUnitTypeById(unitTypeId);
		UnitType *ut = unitType;
		assert(ut != NULL);
		maxEnergy = ut->getEnergy();

		PartType *reactorParentType = getPartTypeById(
			PARTTYPE_ID_STRING_TO_INT("Reac"));

		PartType *powerParentType = getPartTypeById(
			PARTTYPE_ID_STRING_TO_INT("Powe"));

		if (reactorParentType == NULL)
		{
			Logger::getInstance()->warning("Unit::calculateMaxEnergy - Unable to solve \"Reac\" base part type.");
			return;
		}

		if (powerParentType == NULL)
		{
			Logger::getInstance()->warning("Unit::calculateMaxEnergy - Unable to solve \"Powe\" base part type.");
			return;
		}

		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				PartType *pt = p->getType();
				if (pt->isInherited(powerParentType))
				{
					// WARNING: UNSAFE CAST (based on inheritance check above).
					PowerCell *pc = (PowerCell *)p->getType();
					maxEnergy += pc->getAmount();
				}
				if (pt->isInherited(reactorParentType))
				{
					// WARNING: UNSAFE CAST (based on inheritance check above).
					Reactor *r = (Reactor *)p->getType();
					maxEnergy += r->getEnergyAmount();
				}
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}
	}

	void Unit::calculateCoolingAmount()
	{
		//UnitType *ut = getUnitTypeById(unitTypeId);
		UnitType *ut = unitType;
		assert(ut != NULL);
		coolingAmount = ut->getCoolAmount();

		/*
		PartType *coolerParentType = getPartTypeById(
			PARTTYPE_ID_STRING_TO_INT("Cool"));

		if (coolerParentType == NULL)
		{
			Logger::getInstance()->warning("Unit::calculateCoolingAmount - Unable to solve \"Cool\" base part type.");
			return;
		}

		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				PartType *pt = p->getType();
				if (pt->isInherited(coolerParentType))
				{
					// WARNING: UNSAFE CAST (based on inheritance check above).
					ItemPack *pc = (ItemPack *)p->getType();
					coolingAmount += pc->getAmount();
				}
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}
		*/
	}

	void Unit::calculateRechargingAmount()
	{
		//UnitType *ut = getUnitTypeById(unitTypeId);
		UnitType *ut = unitType;
		assert(ut != NULL);
		rechargingAmount = ut->getRechargeAmount();

		/*
		PartType *pextParentType = getPartTypeById(
			PARTTYPE_ID_STRING_TO_INT("PExt"));

		if (pextParentType == NULL)
		{
			Logger::getInstance()->warning("Unit::calculateRechargingAmount - Unable to solve \"PExt\" base part type.");
			return;
		}

		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				PartType *pt = p->getType();
				if (pt->isInherited(pextParentType))
				{
					// WARNING: UNSAFE CAST (based on inheritance check above).
					ItemPack *pc = (ItemPack *)p->getType();
					rechargingAmount += pc->getAmount();
				}
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}
		*/
	}

	void Unit::calculateWeight()
	{
		weight = 0;

		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				weight += p->getType()->getWeight();
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}

		if (weight <= 0) weight = 1;
	}


	void Unit::calculateRunningValue()
	{
		runningValue = 0;

		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				runningValue += p->getType()->getRunningEffect();
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}

		if (runningValue < 0) runningValue = 0;
	}


	void Unit::calculateStealthValue()
	{
		stealthValue = 0;

		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				stealthValue += p->getType()->getStealthEffect();
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}

		if (stealthValue < 0) stealthValue = 0;
	}


	void Unit::calculateReconValue()
	{
		reconValue = 0;

		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				reconValue += p->getType()->getReconEffect();
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}

		if (reconValue < 0) reconValue = 0;
	}


	void Unit::calculateMaxHeat()
	{
		//UnitType *ut = getUnitTypeById(unitTypeId);
		UnitType *ut = unitType;
		assert(ut != NULL);
		maxHeat = ut->getMaxHeat();

		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				maxHeat += p->getType()->getMaxHeat();
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}
	}

	void Unit::setStartEnergy(int startEnergy)
	{
		this->startEnergy = startEnergy;
	}

	void Unit::setEnergy(int energy)
	{
		if (energy < 0) energy = 0;
		if (energy > maxEnergy) energy = maxEnergy;
		this->energy = energy;
	}

	void Unit::setHeat(int heat)
	{
		if (heat < 0) heat = 0;
		if (heat > maxHeat) heat = maxHeat;
		this->heat = heat;
	}

	/*
	void Unit::setMaxHeat(int maxHeat)
	{
		this->maxHeat = maxHeat;
	}
	*/

	bool Unit::atWaypoint() const
	{
		// todo, optimize, make one bool variable of this...
		if (position.x == waypoint.x && position.z == waypoint.z)
			return true;
		else
			return false;
	}

	bool Unit::atFinalDestination() const
	{
		// todo, optimize, make one bool variable of this...
		if (position.x == finalDestination.x && position.z == finalDestination.z)
			return true;
		else
			return false;
	}


	void Unit::setLeader(Unit *unit)
	{
		leader = unit;
	}

	Unit *Unit::getLeader() const
	{
		return leader;
	}


	void Unit::prepareAnimationSet()
	{
		if (unitType->getAnimation() != NULL)
			animationSet = ui::AnimationSet::getSetByName(unitType->getAnimation());
	}


	ui::AnimationSet *Unit::getAnimationSet() const
	{
		return animationSet;
	}


	void Unit::setAnimation(int animation)
	{
		currentAnimation = animation;
	}

	int Unit::getAnimation() const
	{
		return currentAnimation;		
	}

	void Unit::setBlendAnimation(unsigned int num, int animation)
	{
		assert(num < UNIT_MAX_BLEND_ANIMATIONS);
		currentBlendAnimation[num] = animation;
	}

	int Unit::getBlendAnimation(unsigned int num) const
	{
		assert(num < UNIT_MAX_BLEND_ANIMATIONS);
		return currentBlendAnimation[num];
	}

	void Unit::setAnimationSpeedFactor(float speedFactor)
	{
		this->animationSpeedFactor = speedFactor;
	}

	float Unit::getAnimationSpeedFactor() const
	{
		return this->animationSpeedFactor;
	}

	void Unit::setAnimationTimeLeft(int animationTime)
	{
		animationTimeLeft = animationTime;
	}

	int Unit::getAnimationTimeLeft() const
	{
		return animationTimeLeft;
	}

	void Unit::setWalkDelay(int delayTicks)
	{
		walkDelay = delayTicks;
	}

	/*
	int Unit::getWalkDelay()
	{
		return walkDelay; 
	}
	*/

	void Unit::setFireReloadDelay(unsigned int weapon, int delayTicks)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		fireReloadDelay[weapon] = delayTicks;
		if (delayTicks == 0)
			clipReloading = false;
	}

	/*
	int Unit::getFireReloadDelay(int weapon)
	{
		return fireReloadDelay[weapon]; 
	}
	*/

	void Unit::setFireWaitDelay(unsigned int weapon, int delayTicks)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		fireWaitDelay[weapon] = delayTicks;
	}

	/*
	int Unit::getFireWaitDelay(int weapon)
	{
		return fireWaitDelay[weapon]; 
	}
	*/

	void Unit::setWeaponFireTime(unsigned int weapon, int delayTicks)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		weaponFireTime[weapon] = delayTicks;
	}

	/*
	int Unit::getWeaponFireTime(int weapon)
	{
		return weaponFireTime[weapon];
	}
	*/


	bool Unit::useWeaponAmmoImpl(unsigned int weapon, int usageWeapon)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		// usageWeapon defines the weapon to use for usage amounts
		// weapon is the weapon the ammo of which is used.

		// use the ammo in the actual clip...
		// (may not be the same as the weapon clip in case of attachments)

		if (weaponAmmoNumber[weapon] != -1)
		{
			assert(usageWeapon >= 0 && usageWeapon < UNIT_MAX_WEAPONS);
			assert(weaponType[weapon] != NULL);
			assert(weaponType[usageWeapon] != NULL);

			int minAmmoReq = weaponType[usageWeapon]->getMinAmmoForUsage();
			// HACK: for shotgun, shoot even the 4 last bullets if in rapid mode
			if (keepFiring > 0)
			{
				minAmmoReq = 1;
			}

			if (ammoAmount[weaponAmmoNumber[weapon]] >= minAmmoReq
				&& (weaponType[weapon]->getClipSize() == 0
				|| weaponAmmoInClip[weapon] >= minAmmoReq))
			{
				ammoAmount[weaponAmmoNumber[weapon]] -= weaponType[usageWeapon]->getAmmoUsage();
				if (ammoAmount[weaponAmmoNumber[weapon]] < 0)
				{
					ammoAmount[weaponAmmoNumber[weapon]] = 0; 				 
				}

				// HACK: for gaso trap, keep at least 5 fuel in any case...
				if (ammoAmount[weaponAmmoNumber[weapon]] < 3)
				{
					if (weaponType[usageWeapon]->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_GasoTr"))
					{
						ammoAmount[weaponAmmoNumber[weapon]] = 3; 				 
					}
				}

				weaponAmmoInClip[weapon] -= weaponType[usageWeapon]->getAmmoUsage();
				if (weaponAmmoInClip[weapon] < 0)
				{
					weaponAmmoInClip[weapon] = 0;
				}
			} else {
				// no ammo...
				return false;
			}
		}
		return true;
	}


	int Unit::getWeaponForSharedClip(unsigned int weapon) const
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		assert(weaponType[weapon]->isSharedClipAttachment());

		int sharedClipWeap = -1;

		// find the weapon to which this weapon is attached,
		// and sharing clip with it.
		for (unsigned int i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			if (weaponType[i] != NULL)
			{
				if (weaponType[i]->getAttachedWeaponType() != NULL
					&& weaponType[i]->getAttachedWeaponType()->getPartTypeId() == weaponType[weapon]->getPartTypeId())
				{
					assert(weaponType[i] != weaponType[weapon]);
					sharedClipWeap = i;
					break;
				}
			}
		}

		if (sharedClipWeap == -1)
		{
			assert(!"Unit::getWeaponForSharedClip - Unable to solve proper shared clip weapon number.");
		}

		return sharedClipWeap;
	}

	int Unit::getAttachedWeapon(int weapon) const
	{
		if(weapon < 0 || weapon > UNIT_MAX_WEAPONS) return -1;

		Weapon *w = weaponType[weapon]->getAttachedWeaponType();
		if(w)
		{
			for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
			{
				if (weaponType[i] && weaponType[i]->getPartTypeId() == w->getPartTypeId())
				{
					return i;
				}
			}
		}
		return -1;
	}


	bool Unit::useWeaponAmmo(unsigned int weapon)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		assert(weaponType[weapon] != NULL);

		if (!weaponOperable[weapon])
			return false;

		// check if this weapon has a shared clip 
		// (some attachment weapons share clip with "parent" weapon)
		if (weaponType[weapon]->isSharedClipAttachment())
		{
			int sharedClipWeap = getWeaponForSharedClip(weapon);
			if (sharedClipWeap >= 0)
				return useWeaponAmmoImpl(sharedClipWeap, weapon);
			else
				return false;
		} else {
			return useWeaponAmmoImpl(weapon, weapon);
		}
	}


	bool Unit::addWeaponAmmo(AmmoPack *ammoPackType, int amount)
	{
		for (int i = 0; i < UNIT_MAX_AMMOS; i++)
		{
			if (ammoType[i] == ammoPackType
				&& ammoAmount[i] < maxAmmoAmount[i])
			{
				ammoAmount[i] += amount;
				if (ammoAmount[i] > maxAmmoAmount[i])
				{
					ammoAmount[i] = maxAmmoAmount[i];
				}
				return true;
			}
		}
		return false;
	}

	bool Unit::setWeaponAmmo(AmmoPack *ammoPackType, int amount)
	{
		for (int i = 0; i < UNIT_MAX_AMMOS; i++)
		{
			if (ammoType[i] == ammoPackType)
			{
				ammoAmount[i] = amount;
				if (ammoAmount[i] > maxAmmoAmount[i])
				{
					ammoAmount[i] = maxAmmoAmount[i];
				}
				return true;
			}
		}
		return false;
	}

	void Unit::setWeaponType(int weapon, Weapon *weaponType)
	{
		// find customization
		for(unsigned int i = 0; i < weaponCustomizations.size(); i++)
		{
			if(weaponCustomizations[i].originalId == weaponType->getPartTypeId())
			{
				weaponType = weaponCustomizations[i].weapon;
			}
		}

		this->weaponType[weapon] = weaponType;
	}

	Weapon *Unit::getCustomizedWeaponType(const Weapon *weapon) const
	{
		if(weapon == NULL)
			return NULL;

		int id = weapon->getPartTypeId();
		for(unsigned int i = 0; i < weaponCustomizations.size(); i++)
		{
			if(weaponCustomizations[i].originalId == id	|| weaponCustomizations[i].weapon->getPartTypeId() == id)
			{
				return weaponCustomizations[i].weapon;
			}
		}
		return NULL;
	}

	Weapon *Unit::createCustomizedWeaponType(Weapon *oldweap)
	{
		Weapon *newweap = new Weapon();
		oldweap->prepareNewForInherit(newweap);
		newweap->setPartTypeId(oldweap->getPartTypeId(), false);
		newweap->setPartTypeIdString(oldweap->getPartTypeIdString());

		WeaponCustomization wc;
		wc.originalId = oldweap->getPartTypeId();
		wc.weapon = newweap;
		weaponCustomizations.push_back(wc);

		int wnum = getWeaponByWeaponType(wc.originalId);
		if(wnum != -1)
		{
			this->weaponType[wnum] = newweap;
		}
		return newweap;
	}

	void Unit::deleteCustomizedWeaponTypes()
	{
		for(unsigned int i = 0; i < weaponCustomizations.size(); i++)
		{
			delete weaponCustomizations[i].weapon;
		}
		weaponCustomizations.clear();
	}

	/*
	Weapon *Unit::getWeaponType(int weapon)
	{
		return weaponType[weapon];
	}
	*/

	bool Unit::hasAnyWeaponReady() const
	{
		for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			if (weaponType[i] != NULL && fireWaitDelay[i] == 0 
				&& fireReloadDelay[i] == 0 && weaponActive[i])
				return true;
		}
		return false;
	}

	/*
	bool Unit::isDestroyed()
	{
		return destroyed;
	}
	*/

	void Unit::setDestroyed(bool destroyed)
	{
		if (this->destroyed != destroyed)
		{
			this->destroyedTime = 0;
			if( getForcedAnimation() && !destroyed )
			{
				setForcedAnimation( 0 );
			}
		}
		this->destroyed = destroyed;
		visibility.setDestroyed(destroyed);
		this->deathScriptHasRun = false;
		if(this->destroyed)
		{
			// reset target lock counter
			setTargetLockCounter(0);
		}
	}


	// CALL THIS _ONLY ONCE_ WHEN STARTING THE COMBAT!!!
	void Unit::initWeapons()
	{
		// clear ammo array
		int i;
		for (i = 0; i < UNIT_MAX_AMMOS; i++)
		{
			ammoType[i] = NULL;
			ammoAmount[i] = 0;
			maxAmmoAmount[i] = 0;
		}

		// first create an ammo array...
		AmmoPackObject *ammoArr[UNIT_MAX_AMMOS]; 
		int num = 0;
		if (rootPart != NULL)
			num = getAmmoArray(ammoArr, UNIT_MAX_AMMOS, rootPart, 0);

		int j;

		for (i = 0; i < num; i++)
		{
			for (j = 0; j < num; j++)
			{
				if (ammoType[j] == ammoArr[i]->getType()
					|| ammoType[j] == NULL)
				{
					// WARNING: unsafe cast!
					// if ammo array contained some other shit, we're fucked. 		 
					ammoType[j] = (AmmoPack *)ammoArr[i]->getType();
					ammoAmount[j] += ammoArr[i]->getAmount();
					maxAmmoAmount[j] += ammoArr[i]->getMaxAmount();
					ammoArr[i]->setAmount(0);
					break;
				}
			}
		}

		// then create a weapon array...
		WeaponObject *weapArr[UNIT_MAX_WEAPONS]; 
		num = 0;
		if (rootPart != NULL)
			num = getWeaponArray(weapArr, UNIT_MAX_WEAPONS, rootPart, 0);

		maxWeaponRange = 0.0f;
		for (i = 0; i < num; i++)
		{
			// WARNING: unsafe cast!
			// if weapon array contained some other shit, we're fucked.
			setWeaponType(i, (Weapon *)weapArr[i]->getType());
			
			// find out weapon position, kinda hack here...
			if (weapArr[i]->getParent() != NULL
				&& weapArr[i]->getParent()->getParent() != NULL)
			{
				Part *gparent = weapArr[i]->getParent()->getParent();
				int gparentSlots = gparent->getType()->getSlotAmount();
				for (j = 0; j < gparentSlots; j++)
				{
					if (gparent->getSubPart(j) == weapArr[i]->getParent())
					{
						weaponPosition[i] = gparent->getType()->getSlotPosition(j);
					}
				}
			}

			fireReloadDelay[i] = 0;
			fireWaitDelay[i] = 0;
			weaponActive[i] = true;
			weaponOperable[i] = true;
			weaponVisible[i] = true;
			weaponFireTime[i] = 0;
			weaponCopyProjectile[i] = NULL;
			weaponLoopSoundHandle[i] = -1;
			weaponLoopSoundKey[i] = -1;
			weaponSoundHandle[i] = -1;
			weaponAmmoInClip[i] = 0;
			if (weaponType[i] != NULL)
			{
				weaponBarrel[i] = weaponType[i]->getBarrelNumber();
				weaponEjectBarrel[i] = weaponType[i]->getBarrelNumber();
			} else {
				weaponBarrel[i] = 0;
				weaponEjectBarrel[i] = 0;
			}

			if (weaponType[i]->getRange() > maxWeaponRange)
				maxWeaponRange = weaponType[i]->getRange();
			
			weaponAmmoNumber[i] = -1;
			if (weaponType[i]->getAmmoType() != NULL)
			{
				for (int j = 0; j < UNIT_MAX_AMMOS; j++)
				{
					if (ammoType[j] == NULL 
						|| ammoType[j] == weaponType[i]->getAmmoType())
					{
						if (ammoType[j] == NULL)
						{
							// no ammotype was found, create a new one.
							ammoType[j] = (AmmoPack *)weaponType[i]->getAmmoType();
							ammoAmount[j] = 0;
							maxAmmoAmount[j] = 0;
						}
						weaponAmmoNumber[i] = j;
						ammoAmount[j] += weapArr[i]->getAmmoAmount();
						maxAmmoAmount[j] += weapArr[i]->getMaxAmmoAmount();
						weapArr[i]->setAmmoAmount(0);
						break;
					}
				}
				if (weaponAmmoNumber[i] == -1)
				{
					// should never happen(?)
					Logger::getInstance()->warning("Unit::initWeapons - Could not solve ammo for weapon.");
				}
			}
		}
		for (i = num; i < UNIT_MAX_WEAPONS; i++)
		{
			weaponType[i] = NULL;
			fireReloadDelay[i] = 0;
			fireWaitDelay[i] = 0;
			weaponAmmoNumber[i] = -1;
			weaponActive[i] = false;
			weaponPosition[i] = SLOT_POSITION_EXTERNAL_ITEM;
			weaponOperable[i] = true;
			weaponVisible[i] = true;
			weaponFireTime[i] = 0;
			weaponCopyProjectile[i] = NULL;
			weaponLoopSoundHandle[i] = -1;
			weaponLoopSoundKey[i] = -1;
			weaponSoundHandle[i] = -1;
			weaponAmmoInClip[i] = 0;
			weaponBarrel[i] = 0;
			weaponEjectBarrel[i] = 0;
		}

		turningSound = -1;
		turningSoundStartTime = 0;
		turningSoundVolume = 0;
	}

	Part *Unit::seekPartOfPartType(const PartType *partType) const
	{
		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				PartType *pt = p->getType();
				if (pt == partType)
				{
					while (!tempStack.isEmpty())
					{
						tempStack.popLast();
					}
					return p;
				}
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}
		return NULL;
	}

	void Unit::uninitWeapons()
	{
		// distribute ammo to proper parts...

		PartType *weapParent = getPartTypeById(
			PARTTYPE_ID_STRING_TO_INT("Weap"));

		PartType *ammoParent = getPartTypeById(
			PARTTYPE_ID_STRING_TO_INT("Ammo"));

		assert(weapParent != NULL);
		assert(ammoParent != NULL);

		if (rootPart != NULL)
		{
			LinkedList tempStack = LinkedList();
			tempStack.append(rootPart);
			while (!tempStack.isEmpty())
			{
				Part *p = (Part *)tempStack.popFirst();
				PartType *pt = p->getType();
				if (pt->isInherited(weapParent))
				{
					// WARNING: UNSAFE CAST (based on inheritance check above).
					WeaponObject *wo = (WeaponObject *)p;
					Weapon *w = (Weapon *)p->getType();
					for (int i = 0; i < UNIT_MAX_AMMOS; i++)
					{
						if (ammoType[i] != NULL
							&& ammoAmount[i] > 0
							&& w->getAmmoType() == ammoType[i])
						{
							assert(wo->getAmmoAmount() == 0);

							int minVal = wo->getMaxAmmoAmount() - wo->getAmmoAmount();
							if (ammoAmount[i] < minVal) minVal = ammoAmount[i];

							ammoAmount[i] -= minVal;
							wo->setAmmoAmount(wo->getAmmoAmount() + minVal);
							break;
						}
					}
				}
				if (pt->isInherited(ammoParent))
				{
					// WARNING: UNSAFE CAST (based on inheritance check above).
					AmmoPackObject *apo = (AmmoPackObject *)p;
					AmmoPack *ap = (AmmoPack *)p->getType();
					for (int i = 0; i < UNIT_MAX_AMMOS; i++)
					{
						if (ammoType[i] != NULL
							&& ammoAmount[i] > 0
							&& ap == ammoType[i])
						{
							assert(apo->getAmount() == 0);

							int minVal = apo->getMaxAmount() - apo->getAmount();
							if (ammoAmount[i] < minVal) minVal = ammoAmount[i];

							ammoAmount[i] -= minVal;
							apo->setAmount(apo->getAmount() + minVal);
							break;
						}
					}
				}
				int slotAmount = p->getType()->getSlotAmount();
				for (int i = 0; i < slotAmount; i++)
				{
					if (p->getSubPart(i) != NULL)
						tempStack.append(p->getSubPart(i));
				}
			}
		}
	}


	void Unit::inactivateAntiVehicleWeapons()
	{
		for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			if (weaponType[i] == NULL) continue;

			if (weaponType[i]->isAntiVehicleWeapon())
				setWeaponActive(i, false);
		}
	}


	void Unit::setWeaponsActiveByFiretype(FireType fireType)
	{ 	
		for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			bool act = false;

			if (weaponType[i] == NULL) continue;

			// shooting primary?
			if (fireType == FireTypePrimary && i == 0) act = true;
			// shooting secondary?
			if (fireType == FireTypeSecondary && i == 1) act = true;
			// shooting a basic weapon? (not heavy, drop or manual)
			if (fireType == FireTypeBasic
				&& !weaponType[i]->isHeavyWeapon()
				&& !weaponType[i]->isDropWeapon() 
				&& !weaponType[i]->isManualWeapon()) 
				act = true;
			// shooting a heavy weapon? (but not drop or manual)
			if (fireType == FireTypeHeavy
				&& weaponType[i]->isHeavyWeapon()
				&& !weaponType[i]->isDropWeapon() 
				&& !weaponType[i]->isManualWeapon()) 
				act = true; 		
			// shooting all? (not drop or manual)
			if (fireType == FireTypeAll
				&& !weaponType[i]->isDropWeapon() 
				&& !weaponType[i]->isManualWeapon()) 
				act = true; 		

			setWeaponActive(i, act);
		}
	}

	
	bool Unit::hasWeaponsForFiretype(FireType fireType) const
	{ 	
		bool act = false;

		for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			if (weaponType[i] == NULL) continue;

			// shooting primary?
			if (fireType == FireTypePrimary && i == 0) act = true;
			// shooting secondary?
			if (fireType == FireTypeSecondary && i == 1) act = true;
			// shooting a basic weapon? (not heavy, drop or manual)
			if (fireType == FireTypeBasic
				&& !weaponType[i]->isHeavyWeapon()
				&& !weaponType[i]->isDropWeapon() 
				&& !weaponType[i]->isManualWeapon()) 
				act = true;
			// shooting a heavy weapon? (but not drop or manual)
			if (fireType == FireTypeHeavy
				&& weaponType[i]->isHeavyWeapon()
				&& !weaponType[i]->isDropWeapon() 
				&& !weaponType[i]->isManualWeapon()) 
				act = true; 		
			// shooting all? (not drop or manual)
			if (fireType == FireTypeAll
				&& !weaponType[i]->isDropWeapon() 
				&& !weaponType[i]->isManualWeapon()) 
				act = true; 		
		}

		return act;
	}

	
	bool Unit::isWeaponActive(int weapon) const
	{
		return weaponActive[weapon];
	}

	void Unit::setWeaponActive(int weapon, bool active)
	{
		weaponActive[weapon] = active;
	}

	bool Unit::isWeaponOperable(int weapon) const
	{
		return weaponOperable[weapon];
	}

	void Unit::setWeaponOperable(int weapon, bool operable)
	{
		weaponOperable[weapon] = operable;
		if (!operable)
		{
			weaponActive[weapon] = false;
		}
	}

	bool Unit::isWeaponVisible(int weapon) const
	{
		if(weapon < 0 || weapon > UNIT_MAX_WEAPONS) return false;
		return weaponVisible[weapon];
	}

	void Unit::setWeaponVisible(int weapon, bool visible)
	{
		weaponVisible[weapon] = visible;
		if (!visible)
		{
			weaponActive[weapon] = false;
		}
	}

	float Unit::getMaxWeaponRange() const
	{
		return maxWeaponRange;
	}

	int Unit::getWeaponAmmoAmount(unsigned int weapon) const
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		if (weaponAmmoNumber[weapon] != -1)
			return ammoAmount[weaponAmmoNumber[weapon]];
		else
			return 0;
	}

	int Unit::getWeaponMaxAmmoAmount(unsigned int weapon) const
	{
        assert(weapon < UNIT_MAX_WEAPONS);
		if (weaponAmmoNumber[weapon] != -1)
			return maxAmmoAmount[weaponAmmoNumber[weapon]];
		else
			return 0;
	}

	AmmoPack *Unit::getWeaponAmmoType(unsigned int weapon) const
	{
        assert(weapon < UNIT_MAX_WEAPONS);
		if (weaponAmmoNumber[weapon] != -1)
			return ammoType[weaponAmmoNumber[weapon]];
		else
			return NULL;
	}

	int Unit::getWeaponPosition(unsigned int weapon) const
	{
        assert(weapon < UNIT_MAX_WEAPONS);
		return weaponPosition[weapon];
	}

	
	int Unit::getWeaponArray(WeaponObject **weaponArray, int arraySize, Part *startPart, int startNumber) const
	{
		if (weaponArray == NULL)
		{
			Logger::getInstance()->error("Unit::getWeaponArray - Null weaponArray given.");
			assert(!"Unit::getWeaponArray - Null weaponArray given.");
			return 0;
		}
		int weapsAdded = 0;
		if (startPart != NULL)
		{
			// TODO: should actually be a call to isInherited(...), 
			// but it would be slow
			// TODO: optimize...
			if (startNumber < arraySize)
			{
				if (startPart->getType()->getParentType() != NULL)
				{
					if (startPart->getType()->getParentType()->getPartTypeId()
						== PARTTYPE_ID_STRING_TO_INT("Weap")
						|| (startPart->getType()->getParentType()->getParentType() != NULL
						&& startPart->getType()->getParentType()->getParentType()->getPartTypeId()
						== PARTTYPE_ID_STRING_TO_INT("Weap")))
					{
						// WARNING: unsafe cast! (based on check above)
						weaponArray[startNumber] = (WeaponObject *)startPart;
						startNumber++;
						weapsAdded++;
					}
				}
			} else {
				Logger::getInstance()->warning("Unit::getWeaponArray - Unit has too many weapon parts.");
			}
			int slotAmount = startPart->getType()->getSlotAmount();
			for (int i = 0; i < slotAmount; i++)
			{
				if (startPart->getSubPart(i) != NULL)
				{
					int ret = getWeaponArray(weaponArray, arraySize, startPart->getSubPart(i), startNumber);
					startNumber += ret;
					weapsAdded += ret;
				}
			}
		}
		return weapsAdded;
	}


	int Unit::getAmmoArray(AmmoPackObject **ammoArray, int arraySize, Part *startPart, int startNumber) const
	{
		if (ammoArray == NULL)
		{
			Logger::getInstance()->error("Unit::getAmmoArray - Null ammoArray given.");
			abort();
		}
		int ammosAdded = 0;
		if (startPart != NULL)
		{
			// TODO: should actually be a call to isInherited(...), 
			// but it would be slow
			// TODO: optimize...
			if (startNumber < arraySize)
			{
				if (startPart->getType()->getParentType() != NULL)
				{
					if (startPart->getType()->getParentType()->getPartTypeId()
						== PARTTYPE_ID_STRING_TO_INT("Ammo")
						|| (startPart->getType()->getParentType()->getParentType() != NULL
						&& startPart->getType()->getParentType()->getParentType()->getPartTypeId()
						== PARTTYPE_ID_STRING_TO_INT("Ammo")))
					{
						// WARNING: unsafe cast! (based on check above)
						ammoArray[startNumber] = (AmmoPackObject *)startPart;
						startNumber++;
						ammosAdded++;
					}
				}
			}
			int slotAmount = startPart->getType()->getSlotAmount();
			for (int i = 0; i < slotAmount; i++)
			{
				if (startPart->getSubPart(i) != NULL)
				{
					int ret = getAmmoArray(ammoArray, arraySize, startPart->getSubPart(i), startNumber);
					startNumber += ret;
					ammosAdded += ret;
				}
			}
		}
		return ammosAdded;
	}

	
	void Unit::setToBeSeenUnit(Unit *seeUnit)
	{
		toBeSeeingUnit = seeUnit;
	}

	Unit *Unit::getToBeSeenUnit() const
	{
		return toBeSeeingUnit;
	}

	void Unit::setToBeSeenUnitDistance(float distance)
	{
		toBeSeeingUnitDistance = distance;
	}

	void Unit::useToBeSeenUnit()
	{
		seeingUnit = toBeSeeingUnit;
		seeingUnitDistance = toBeSeeingUnitDistance;
		toBeSeeingUnit = NULL;
	}

	/*
	Unit *Unit::getSeeUnit()
	{
		return seeingUnit;
	}
	*/

	float Unit::getSeeUnitDistance() const
	{
		return seeingUnitDistance;
	}

	float Unit::getToBeSeenUnitDistance() const
	{
		return toBeSeeingUnitDistance;
	}


	// if the path is changed, must call updatePath next.
	frozenbyte::ai::Path *Unit::getPath() const
	{
		return path;
	}

	void Unit::setPath(frozenbyte::ai::Path *path)
	{
		if (this->path != NULL)
		{
			if (!pathIsStored)
				delete this->path;
		}
		pathIsStored = false;
		this->path = path;
		if (path != NULL)
		{
			pathIndex = 0;
			if (pathIndex == path->getSize())
				atPathEnd = true;
			else
				atPathEnd = false;
		} else {
			atPathEnd = true;
		}
	}

	bool Unit::isAtPathEnd() const
	{
		return atPathEnd;
	}

	int Unit::getPathIndex() const
	{
		return pathIndex;
	}

	void Unit::setPathIndex(int pathIndex)
	{
		assert(path != NULL);
		this->pathIndex = pathIndex;
		if (pathIndex >= path->getSize()) atPathEnd = true;
	}

	void Unit::useStoredPath(int pathNumber)
	{
		frozenbyte::ai::Path *p = scriptPaths.getStoredPathForUse(pathNumber);
		if (p != NULL)
		{
			setPath(p);
			pathIsStored = true; 
		}
	}

	void Unit::setStoredPath(int pathNumber, frozenbyte::ai::Path *path,
		const VC3 &startPosition, const VC3 &endPosition)
	{
		scriptPaths.setStoredPath(pathNumber, path, 
			startPosition, endPosition, this);
	}



	void Unit::setSpottedScriptDelay(int delayTicks)
	{
		spottedDelay = delayTicks;
	}

	int Unit::getSpottedScriptDelay() const
	{
		return spottedDelay;
	}

	void Unit::setHitScriptDelay(int delayTicks)
	{
		hitDelay = delayTicks;
	}

	int Unit::getHitScriptDelay() const
	{
		return hitDelay;
	}

	void Unit::setHitByUnit(Unit *unit, Bullet *hitBullet)
	{
		if (!this->isDestroyed())
		{
			hitByUnit = unit;
			hitByBullet = hitBullet;
		} else {
			if (this->isDestroyed() && !deathScriptHasRun
				// really bad hack to allow hitscript from terrain object explosions
				&& (unit != NULL || getUnitType()->doesRunHitScriptWithoutShooter()))
			{
				hitByUnit = unit;
				hitByBullet = hitBullet;
				hitDelay = 0;
				deathScriptHasRun = true;
			} else {
				hitByUnit = NULL;
				hitByBullet = NULL;
			}
		}
	}

	Unit *Unit::getHitByUnit() const
	{
		return hitByUnit;
	}

	Bullet *Unit::getHitByBullet() const
	{
		return hitByBullet;
	}

	void Unit::setHitMissScriptDelay(int delayTicks)
	{
		hitMissDelay = delayTicks;
	}

	int Unit::getHitMissScriptDelay() const
	{
		return hitMissDelay;
	}

	void Unit::setHitMissByUnit(Unit *unit)
	{
		hitMissByUnit = unit;
	}

	Unit *Unit::getHitMissByUnit() const
	{
		return hitMissByUnit;
	}

	void Unit::setPointedScriptDelay(int delayTicks)
	{
		pointedDelay = delayTicks;
	}

	int Unit::getPointedScriptDelay() const
	{
		return pointedDelay;
	}

	void Unit::setPointedByUnit(Unit *unit)
	{
		pointedByUnit = unit;
	}

	Unit *Unit::getPointedByUnit() const
	{
		return pointedByUnit;
	}

	void Unit::setHearNoiseScriptDelay(int delayTicks)
	{
		hearNoiseDelay = delayTicks;
	}

	int Unit::getHearNoiseScriptDelay() const
	{
		return hearNoiseDelay;
	}

	void Unit::setHearNoiseByUnit(Unit *unit)
	{
		hearNoiseByUnit = unit;
	}

	Unit *Unit::getHearNoiseByUnit() const
	{
		return hearNoiseByUnit;
	}


	void Unit::setSpawnCoordinates(VC3 &spawn)
	{
		// NOTICE: spawn coordinates changed from config to scaled scale.
		// they used to be config (= heightmap) coordinates, but no longer.
		this->spawn = spawn;
		this->hasSpawn = true;
	}

	void Unit::clearSpawnCoordinates()
	{
		hasSpawn = false;
	}

	VC3 Unit::getSpawnCoordinates() const
	{
		return this->spawn;
	}

	bool Unit::hasSpawnCoordinates() const
	{
		return hasSpawn;
	}

	void Unit::setIdleTime(int idleTime)
	{
		this->idleTime = idleTime;
	}

	int Unit::getIdleTime() const
	{
		return this->idleTime;
	}

	void Unit::setLookBetaAngle(float lookBetaAngle)
	{
		this->lookBetaAngle = lookBetaAngle;
	}

	float Unit::getLookBetaAngle() const
	{
		return lookBetaAngle;
	}

	void Unit::setDirectControl(bool directControl)
	{
		this->directControl = directControl;
	}

	bool Unit::isDirectControl() const
	{
		return directControl;
	}

	void Unit::setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE directControlType)
	{
		this->directControlType = directControlType;
	}

	Unit::UNIT_DIRECT_CONTROL_TYPE Unit::getDirectControlType() const
	{
		return directControlType;
	}

	int Unit::getGroupNumber() const
	{
		return groupNumber;
	}

	void Unit::setGroupNumber(int groupNumber)
	{
		this->groupNumber = groupNumber;
	}

	int Unit::getHP() const
	{
		return hp;
	}

	void Unit::setHP(int hp)
	{
//		assert( getOwner() != NO_UNIT_OWNER );
		if (hp > maxHP)
			this->hp = maxHP;
		else
			this->hp = hp;
	}

	int Unit::getMaxHP() const
	{
		return maxHP;
	}

	void Unit::setMaxHP(int maxHP)
	{
		this->maxHP = maxHP;
	}

	float Unit::getPoisonResistance() const
	{
		return poisonResistance;
	}

	void Unit::setPoisonResistance(float value)
	{
		poisonResistance = value;
	}

	float Unit::getCriticalHitPercent() const
	{
		return criticalHitPercent;
	}

	void Unit::setCriticalHitPercent(float value)
	{
		criticalHitPercent = value;
	}

	int Unit::calculateArmorRating() const
	{
		// TODO: proper implementation!
		// this only calculates to depth 1 (rootpart and parts attached to it)
		int ret = 0;

		if (rootPart != NULL)
		{
			// first add root part's AR
			// part's damage affects AR...
			// damage affects by halving the part's AR if totally damaged
			ret += rootPart->getType()->getArmorRating() 
				* (100 - (50 * rootPart->getDamage() / rootPart->getType()->getMaxDamage())) / 100;
			int slotAmount = rootPart->getType()->getSlotAmount();
			// then add its subpart's ARs
			for (int i = 0; i < slotAmount; i++)
			{
				Part *p = rootPart->getSubPart(i);
				if (p != NULL)
				{
					if (p->getType()->getMaxDamage() == 0)
						ret += p->getType()->getArmorRating();
					else
						ret += p->getType()->getArmorRating()
							* (100 - (50 * p->getDamage() / p->getType()->getMaxDamage())) / 100;
				}
			}
		}

		return ret;
	}

	/*
	Projectile *Unit::getWeaponCopyProjectile(int weapon)
	{
		return weaponCopyProjectile[weapon];
	}
	*/

	int Unit::getWeaponLoopSoundHandle(unsigned int weapon) const
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		return weaponLoopSoundHandle[weapon];
	}

	int Unit::getWeaponLoopSoundKey(unsigned int weapon) const
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		return weaponLoopSoundKey[weapon];
	}

	int Unit::getWeaponSoundHandle(unsigned int weapon) const
	{
		return weaponSoundHandle[weapon];
	}

	void Unit::setWeaponLoopSoundHandle(unsigned int weapon, int handle, int key)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		weaponLoopSoundHandle[weapon] = handle;
		weaponLoopSoundKey[weapon] = key;
	}

	void Unit::setWeaponSoundHandle(unsigned int weapon, int handle)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		weaponSoundHandle[weapon] = handle;
	}

	void Unit::setWeaponCopyProjectile(unsigned int weapon, Projectile *projectile)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		this->weaponCopyProjectile[weapon] = projectile;
	}

	void Unit::setSightBonus(bool sightBonus)
	{
		this->sightBonus = sightBonus;
	}

	bool Unit::hasSightBonus() const
	{
		return sightBonus;
	}

	bool Unit::makeStepNoise()
	{
		stepNoiseCounter++;
		if (stepNoiseCounter > 50)
		{
			stepNoiseCounter = 0;
			return true;
		} else {
			return false;
		}
	}

	void Unit::setLastBoneAimDirection(float angle)
	{
		lastBoneAimDirection = angle;
	}

	float Unit::getLastBoneAimDirection() const
	{
		return lastBoneAimDirection;
	}

	void Unit::setLastBoneAimBetaAngle(float angle)
	{
		lastBoneAimBetaAngle = angle;
	}
	
	float Unit::getLastBoneAimBetaAngle() const
	{
		return lastBoneAimBetaAngle;
	}

	void Unit::setFlashlightDirection(float angle)
	{
		lastBoneAimDirection = angle;
	}

	float Unit::getFlashlightDirection() const
	{
		return lastBoneAimDirection;
	}

	void Unit::setStealthing(bool stealthing)
	{
		this->stealthing = stealthing;
	}

	bool Unit::isStealthing() const
	{
		return stealthing;
	}


	void Unit::setReconAvailableFlag(bool reconAvailable)
	{
		this->reconAvailable = reconAvailable;
	}

	bool Unit::isReconAvailableFlag() const
	{
		return reconAvailable;
	}


	void Unit::setFallenOnBack(bool fallenOnBack)
	{
		this->fallenOnBack = fallenOnBack;
	}

	bool Unit::hasFallenOnBack() const
	{
		return fallenOnBack;
	}


	float Unit::getFiringSpreadFactor() const
	{
		return firingSpreadFactor;
	}

	void Unit::setFiringSpreadFactor(float firingSpreadFactor)
	{
		this->firingSpreadFactor = firingSpreadFactor;
	}

	int Unit::getWeaponClipSize(unsigned int weapon) const
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		if (weaponType[weapon] == NULL)
			return 0;

		// TODO: what about shared clips, etc.?

		return weaponType[weapon]->getClipSize();
	}

	bool Unit::reloadWeaponAmmoClip(unsigned int weapon, bool instantReload)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		if (weaponType[weapon] == NULL)
			return false;

		if (!instantReload && fireReloadDelay[weapon] > 0)
			return false;

		int ammo = getWeaponAmmoAmount(weapon);

		// is the clip not full and do we have some more ammo to reload?
		if (weaponAmmoInClip[weapon] < weaponType[weapon]->getClipSize()
			&& ammo > weaponAmmoInClip[weapon])
		{
			if (weaponType[weapon]->isSingleReloading() && !instantReload)
			{
				// reload single ammo per call
				weaponAmmoInClip[weapon]++;
			} else {
				// reload whole clip at once
				if (ammo < weaponType[weapon]->getClipSize())
					weaponAmmoInClip[weapon] = ammo;
				else
					weaponAmmoInClip[weapon] = weaponType[weapon]->getClipSize();
			}
			//if (weaponAmmoInClip[weapon] == weaponType[weapon]->getClipSize()
			//	|| weaponAmmoInClip[weapon] == ammo)
			//{
			//	this->setKeepReloading(false);
			//}
			if (!instantReload)
			{
				fireReloadDelay[weapon] = weaponType[weapon]->getClipReloadTime();

				// NEW: cause any shared clip weapon to be delayed too...
				Weapon *w = weaponType[weapon];
				if (w->getAttachedWeaponType() != NULL)
				{
					if (!w->usesFireDelayHack() && !w->getAttachedWeaponType()->hasIndependentFireDelay())
					{
						for (int attach = 0; attach < UNIT_MAX_WEAPONS; attach++)
						{
							if (this->getWeaponType(attach) != NULL
								&& w->getAttachedWeaponType()->getPartTypeId() == this->getWeaponType(attach)->getPartTypeId())
							{
								if (this->getFireReloadDelay(attach) < weaponType[weapon]->getClipReloadTime())
								{
									this->setFireReloadDelay(attach, weaponType[weapon]->getClipReloadTime());
								}
							}
						}
					}
				}
				clipReloading = true;
			} else {
				fireWaitDelay[weapon] = 0;
				fireReloadDelay[weapon] = 1;
				clipReloading = true;
			}
			return true;
		} else {
			return false; 	
		}
	}

	int Unit::getWeaponAmmoInClip(unsigned int weapon) const
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		return weaponAmmoInClip[weapon];
	}

	void Unit::clearWeaponAmmoInClip(unsigned int weapon)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		weaponAmmoInClip[weapon] = 0;
	}


	void Unit::setMovingForward() 
	{ 
		movingForward = true; 
		movingSideways = false; 
		movingBackward = false; 
	}
	void Unit::setMovingBackward()
	{ 
		movingForward = false; 
		movingSideways = false; 
		movingBackward = true; 
	}
	void Unit::setMovingSideways()
	{ 
		movingForward = false; 
		movingSideways = true; 
		movingBackward = false; 
	}

	bool Unit::wasMovingForward() const { return movingForward; }
	bool Unit::wasMovingBackward() const { return movingBackward; }
	bool Unit::wasMovingSideways() const { return movingSideways; }

	float Unit::getRushDistance() const
	{
		return rushDistance;
	}

	void Unit::setRushDistance(float rushDistance)
	{
		this->rushDistance = rushDistance;
	}

	void Unit::retireFromCombat()
	{
		clearSpawnCoordinates();
		targeting.targetUnit = NULL;
		targeting.clearLastTargetPosition();
	}

	void Unit::setArmorAmount(int value)
	{
		if (value <= 0) value = 0;
		if (value >= UNIT_MAX_ARMOR_AMOUNT) value = UNIT_MAX_ARMOR_AMOUNT;
		armorAmount = value;
	}

	int Unit::getArmorAmount() const
	{
		return armorAmount;
	}

	void Unit::setArmorClass(int value)
	{
//Logger::getInstance()->error("Unit::setArmorAmount - should not be used.");
//abort();
		if (value <= 0) value = 0;
		if (value >= UNIT_MAX_ARMOR_CLASS) value = UNIT_MAX_ARMOR_CLASS;
		armorClass = value;
	}

	int Unit::getArmorClass() const
	{
//Logger::getInstance()->error("Unit::getArmorAmount - should not be used.");
//abort();
		return armorClass;
	}

	/*
	void Unit::setSpotlight(ui::Spotlight *spotlight)
	{
		this->spotlight = spotlight;
	}

	ui::Spotlight *Unit::getSpotlight()
	{
		return spotlight;
	}
	*/

	void Unit::setFlashlight(Flashlight *flashlight)
	{
		this->flashlight = flashlight;
	}

	Flashlight *Unit::getFlashlight() const
	{
		return flashlight;
	}

	void Unit::setSecondarySpotlight(ui::Spotlight *spotlight)
	{
		this->spotlight2 = spotlight;
	}

	ui::Spotlight *Unit::getSecondarySpotlight() const
	{
		return spotlight2;
	}

	bool Unit::isClipReloading() const
	{
		return clipReloading;
	}

	bool Unit::isLastMoveStrafed() const
	{
		return this->lastMoveStrafed;
	}

	void Unit::setLastMoveStrafed(bool lastMoveStrafed)
	{
		this->lastMoveStrafed = lastMoveStrafed;
	}

	bool Unit::isUnitMirrorSide() const
	{
		return this->unitMirrorSide;
	}

	void Unit::setUnitMirrorSide(bool mirrorSide)
	{
		this->unitMirrorSide = mirrorSide;
	}

	void Unit::setForcedAnimation(int anim)
	{
		this->forcedAnim = anim;
	}

	int Unit::getForcedAnimation() const
	{
		return forcedAnim;
	}

	void Unit::addItem(int number, Item *item)
	{
		if (number < 0 || number >= UNIT_MAX_ITEMS)
		{
			Logger::getInstance()->error("Unit::addItem - Item number out of range.");
			return;
		}

		if (items == NULL)
		{
			items = new Item *[UNIT_MAX_ITEMS];
			for (int i = 0; i < UNIT_MAX_ITEMS; i++)
			{
				items[i] = NULL;
			}
		}

		if (items[number] == NULL)
		{
			items[number] = item;
			return;
		}

		if (items[UNIT_MAX_ITEMS - 1] != NULL)
		{
			Logger::getInstance()->error("Unit::addItem - Too many items.");
			return;
		}

		for (int j = UNIT_MAX_ITEMS - 1; j > number; j--)
		{
			items[j] = items[j - 1];
		}
		items[number] = item;

	}

	Item *Unit::getItem(int number) const
	{
		if (number < 0 || number >= UNIT_MAX_ITEMS)
		{
			Logger::getInstance()->error("Unit::getItem - Item number out of range.");
			return NULL;
		}

		if (items == NULL)
			return NULL;

		return items[number];
	}

	Item *Unit::removeItem(int number)
	{
		if (number < 0 || number >= UNIT_MAX_ITEMS)
		{
			Logger::getInstance()->error("Unit::removeItem - Item number out of range.");
			return NULL;
		}

		if (items == NULL)
			return NULL;

		Item *item = items[number];
		items[number] = NULL;
		return item;
	}

	int Unit::getItemNumberByTypeId(int itemTypeId) const
	{
		if (items == NULL)
			return -1;

		for (int i = 0; i < UNIT_MAX_ITEMS; i++)
		{
			if (items[i] != NULL)
			{
				if (items[i]->getItemTypeId() == itemTypeId)
					return i;
			}
		}
		return -1;
	}

	void Unit::setSelectedWeapon(int weaponNumber)
	{
		this->selectedWeapon = weaponNumber;
	}

	int Unit::getSelectedWeapon() const
	{
		return selectedWeapon;
	}

	void Unit::setSelectedSecondaryWeapon(int weaponNumber)
	{
		this->selectedSecondaryWeapon = weaponNumber;
	}

	int Unit::getSelectedSecondaryWeapon() const
	{
		return selectedSecondaryWeapon;
	}

	int Unit::getWeaponByWeaponType(int weaponTypeId) const
	{
		for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			if (weaponType[i] != NULL
				&& weaponType[i]->getPartTypeId() == weaponTypeId)
			{
				return i;
			}
		} 	
		return -1;
	}

	bool Unit::wasLastPathfindSuccess() const
	{
		return lastPathfindSuccess;
	}

	void Unit::setLastPathfindSuccess(bool pathfindSuccess)
	{
		this->lastPathfindSuccess = pathfindSuccess;
	}

	bool Unit::isImmortal() const
	{
		return immortal;
	}

	bool Unit::isImmortalWithHitScript() const
	{
		return immortalWithHitScript;
	}

	void Unit::setImmortal(bool immortal, bool run_hitscript)
	{
		this->immortal = immortal;
		this->immortalWithHitScript = run_hitscript;
	}

	void Unit::setJumpCounter(int jumpTickLength)
	{
		this->jumpCounter = jumpTickLength;
	}

	int Unit::getJumpCounter() const
	{
		return jumpCounter;
	}

	void Unit::setJumpTotalTime(int jumpTickLength)
	{
		this->jumpTotalTime = jumpTickLength;
	}

	int Unit::getJumpTotalTime() const
	{
		return jumpTotalTime;
	}

	void Unit::setTurnedTime(int turnedTime)
	{
		this->turnedTime = turnedTime;
	}

	int Unit::getTurnedTime() const
	{
		return turnedTime;
	}

	bool Unit::isAnimated() const
	{
		return animated;
	}

	void Unit::setAnimated(bool animated)
	{
		this->animated = animated;
	}

	void Unit::setFiringInProgress(bool firingInProgress)
	{
		this->firingInProgress = firingInProgress;
	}

	bool Unit::isFiringInProgress() const
	{
		return this->firingInProgress;
	}

	bool Unit::isAnyWeaponFiring() const
	{
		for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			if (weaponType[i] != NULL
				&& (weaponFireTime[i] > 0
				|| fireWaitDelay[i] > 0))
			{
				return true;
			}
		}
		return false;
	}

	int Unit::getFireSweepDirection() const
	{
		return sweepDirection;
	}

	void Unit::setFireSweepDirection(int sweepDirection)
	{
		this->sweepDirection = sweepDirection;
	}

	void Unit::addPositionToBacktrack(const VC3 &position, const VC3 &oldPosition)
	{
		backtrackCounter++;
		if (backtrackCounter >= UNIT_BACKTRACK_INTERVAL)
		{
			backtrackCounter = 0;

			assert(currentBacktrackSlot >= 0
				&& currentBacktrackSlot < UNIT_BACKTRACK_AMOUNT);

			backtrackPositions[currentBacktrackSlot] = position;
			if (position.x != oldPosition.x || position.z != oldPosition.z
				|| position.y != oldPosition.y)
			{
				backtrackMoves[currentBacktrackSlot] = true;
			} else {
				backtrackMoves[currentBacktrackSlot] = false;
			}

			currentBacktrackSlot++;
			if (currentBacktrackSlot >= UNIT_BACKTRACK_AMOUNT)
			{
				currentBacktrackSlot = 0;
			}
		}
	}

	bool Unit::hasMovedSinceOldestBacktrack(float movementTreshold) const
	{
		for (int i = 0; i < UNIT_BACKTRACK_AMOUNT/2; i++)
		{
			int checkSlot = (currentBacktrackSlot + i) % UNIT_BACKTRACK_AMOUNT;

			assert(checkSlot >= 0
				&& checkSlot < UNIT_BACKTRACK_AMOUNT);

			VC3 diffVec = this->position - backtrackPositions[checkSlot];
			if (diffVec.GetSquareLength() > movementTreshold * movementTreshold)
			{
				return true;
			}
		}
		return false;
	}

	bool Unit::hasAttemptedToMoveAllBacktrackTime() const
	{
		for (int i = 0; i < UNIT_BACKTRACK_AMOUNT; i++)
		{
			if (!backtrackMoves[i])
				return false;
		}
		return true;
	}


	void Unit::setStrafeRotationOffset(float rotationOffset)
	{
		this->strafeRotationOffset = rotationOffset;
	}

	float Unit::getStrafeRotationOffset() const
	{
		return this->strafeRotationOffset;
	}

	void Unit::setStrafeAimOffset(float aimOffset)
	{
		this->strafeAimOffset = aimOffset;
	}

	float Unit::getStrafeAimOffset() const
	{
		return this->strafeAimOffset;
	}

	void Unit::setCameraRelativeJumpDirections(bool forward, bool backward, bool left, bool right)
	{
		this->jumpCamForward = forward;
		this->jumpCamBackward = backward;
		this->jumpCamLeft = left;
		this->jumpCamRight = right;
	}

	void Unit::getCameraRelativeJumpDirections(bool *forward, bool *backward, bool *left, bool *right) const
	{
		*forward = this->jumpCamForward;
		*backward = this->jumpCamBackward;
		*left = this->jumpCamLeft;
		*right = this->jumpCamRight;
	}

	void Unit::setUnitRelativeJumpDirections(bool forward, bool backward, bool left, bool right)
	{
		this->jumpUnitForward = forward;
		this->jumpUnitBackward = backward;
		this->jumpUnitLeft = left;
		this->jumpUnitRight = right;
	}

	void Unit::getUnitRelativeJumpDirections(bool *forward, bool *backward, bool *left, bool *right) const
	{
		*forward = this->jumpUnitForward;
		*backward = this->jumpUnitBackward;
		*left = this->jumpUnitLeft;
		*right = this->jumpUnitRight;
	}

	int Unit::getPendingWeaponChangeRequest() const
	{
		return this->pendingWeaponChangeRequest;
	}

	void Unit::setPendingWeaponChangeRequest(int pendingWeaponId)
	{
		this->pendingWeaponChangeRequest = pendingWeaponId;
	}

	int Unit::getJumpNotAllowedTime() const
	{
		return this->jumpNotAllowedTime;
	}

	void Unit::setJumpNotAllowedTime(int jumpNotAllowedTime)
	{
		this->jumpNotAllowedTime = jumpNotAllowedTime;
	}

	void Unit::makeGhostOfFuture()
	{
		assert(!this->ghostOfFuture);
		this->ghostOfFuture = true;
	}

	bool Unit::isGhostOfFuture() const
	{
		return this->ghostOfFuture;
	}

	int Unit::getGhostTime() const
	{
		return this->ghostTime;
	}

	void Unit::setGhostTime(int ghostTime)
	{
		this->ghostTime = ghostTime;
	}

	void Unit::addSlowdown(float slowdownFactor)
	{
		this->slowdown += slowdownFactor * unitType->getSlowdownAddFactor();
		if (this->slowdown < -50.0f) 
			this->slowdown = -50.0f;
		if (this->slowdown > unitType->getMaxSlowdown()) 
			this->slowdown = unitType->getMaxSlowdown();
	}

	void Unit::setSlowdown(float slowdownFactor)
	{
		this->slowdown = slowdownFactor;
	}

	float Unit::getSlowdown() const
	{
		return this->slowdown;
	}

	void Unit::wearOffSlowdown()
	{
		if (this->slowdown < 0.0f)
		{
			this->slowdown += unitType->getSlowdownWearOff();
			if (this->slowdown > 0.0f)
				this->slowdown = 0.0f;
		} else {
			if (this->slowdown > 0.0f)
			{
				this->slowdown -= unitType->getSlowdownWearOff();
				if (this->slowdown < 0.0f)
					this->slowdown = 0.0f;
			}
		}
	}

	void Unit::setUnitEffectLayerType(UNIT_EFFECT_LAYER layerType, int duration)
	{
		if (this->unitType->doesDisableEffectLayer())
			return;

		// No effect is allowed to override cloak hit effect (specifically cloak wo/ hit)
		// NOTE: duration 1 probably means that have called the deleteUnitEffectLayer script command
		// and are intending to get rid of the effect, thus, treat duration 1 as duration 0
		if (this->effectLayerType == UNIT_EFFECT_LAYER_CLOAKHIT
			&& this->effectLayerDuration > 1)
		{
			return;		
		}

		this->effectLayerType = layerType;
		this->effectLayerDuration = duration;
	}

	void Unit::setUnitEffectLayerDuration(int duration)
	{
		if (this->unitType->doesDisableEffectLayer())
			return;

		this->effectLayerDuration = duration;
	}


	Unit::UNIT_EFFECT_LAYER Unit::getUnitEffectLayerType() const
	{
		return this->effectLayerType;
	}

	int Unit::getUnitEffectLayerDuration() const
	{
		return this->effectLayerDuration;
	}

	void Unit::setIdleRequest(int idleRequestAnimNumber)
	{
		assert(idleRequestAnimNumber >= 1 && idleRequestAnimNumber <= 8);
		this->idleRequest = idleRequestAnimNumber;
	}

	int Unit::getIdleRequest() const
	{
		return this->idleRequest;
	}

	void Unit::clearIdleRequest()
	{
		this->idleRequest = 0;
	}

	bool Unit::doesKeepReloading() const
	{
		return this->keepReloading;
	}
	
	int Unit::getKeepFiringCount() const
	{
		return this->keepFiring;
	}
	
	void Unit::setKeepReloading(bool keepReloading)
	{
		this->keepReloading = keepReloading;
	}

	void Unit::setKeepFiringCount(int keepFiringCount)
	{
		this->keepFiring = keepFiringCount;
	}

	bool Unit::doesUseAIDisableRange() const
	{
		return this->useAIDisableRange;
	}

	void Unit::setUseAIDisableRange(bool aiDisableRange)
	{
		this->useAIDisableRange = aiDisableRange;
	}

	void Unit::setSelectedItem(int selectedItem)
	{
		assert(selectedItem == -1 || (selectedItem >= 0 && selectedItem < UNIT_MAX_ITEMS));
		this->selectedItem = selectedItem;
	}

	int Unit::getSelectedItem() const
	{
		return selectedItem;
	}

	void Unit::fadeLighting(float fadeToValue, int fadeTime)
	{
		if (fadeTime < 0) fadeTime = 1;
		this->currentLighting = getCurrentLightingFadeValue();
		this->fadeLightingTo = fadeToValue;
		this->fadeLightingTimeLeft = fadeTime;
		this->fadeLightingTimeTotal = fadeTime;
	}

	float Unit::getCurrentLightingFadeValue() const
	{
		float srcFactor = (float)fadeLightingTimeLeft / (float)fadeLightingTimeTotal;
		float targFactor = 1.0f - srcFactor;
		float val = (this->currentLighting * srcFactor) + (this->fadeLightingTo * targFactor);
		if (val < 0.0f) val = 0.0f;
		if (val > 1.0f) val = 1.0f;
		return val;
	}

	void Unit::setFadeVisibilityImmediately(float fadeValue)
	{
		this->currentVisibility = fadeValue;
		this->fadeVisibilityTo = fadeValue;
		this->fadeVisibilityTimeLeft = 0;
		this->fadeVisibilityTimeTotal = 1;
		this->lastLightUpdatePosition = VC3(-99999, -99999, -99999);
	}

	void Unit::fadeVisibility(float fadeToValue, int fadeTime)
	{
		if (fadeTime < 0) fadeTime = 1;
		this->currentVisibility = getCurrentVisibilityFadeValue();
		this->fadeVisibilityTo = fadeToValue;
		this->fadeVisibilityTimeLeft = fadeTime;
		this->fadeVisibilityTimeTotal = fadeTime;
	}

	float Unit::getCurrentVisibilityFadeValue() const
	{
		if (fadeVisibilityTimeTotal <= 1)
		{
			return this->currentVisibility;
		}
		float srcFactor = (float)fadeVisibilityTimeLeft / (float)fadeVisibilityTimeTotal;
		float targFactor = 1.0f - srcFactor;
		float val = (this->currentVisibility * srcFactor) + (this->fadeVisibilityTo * targFactor);
		if (val < 0.0f) val = 0.0f;
		if (val > 1.0f) val = 1.0f;
		return val;
	}

	void Unit::advanceFade()
	{
		if (this->fadeLightingTimeLeft > 0)
		{
			this->fadeLightingTimeLeft--;
			this->lastLightUpdatePosition = VC3(-99999, -99999, -99999);
		}
		if (this->fadeVisibilityTimeLeft > 0)
		{
			this->fadeVisibilityTimeLeft--;
			this->lastLightUpdatePosition = VC3(-99999, -99999, -99999);
		}
	}

	bool Unit::doesCollisionCheck() const
	{
		return this->collisionCheck;
	}

	void Unit::setCollisionCheck(bool collCheck)
	{
		this->collisionCheck = collCheck;
	}

	bool Unit::doesCollisionBlockOthers() const
	{
		return this->collisionBlocksOthers;
	}

	void Unit::setCollisionBlocksOthers(bool collCheck)
	{
		this->collisionBlocksOthers = collCheck;
	}

	bool Unit::hasAreaTriggered() const
	{
		return this->areaTriggered;
	}

	void Unit::setAreaTriggered(bool areaTriggered)
	{
		this->areaTriggered = areaTriggered;
	}

	VC3 Unit::getTrackablePosition() const
	{
		return this->position;
	}

	float Unit::getTrackableRadius2d() const
	{
		return 0;
	}

	const VC3 &Unit::getAreaCenter() const
	{
		return this->areaCenter;
	}

	void Unit::setAreaCenter(const VC3 &position)
	{
		this->areaCenter = position;
	}

	float Unit::getAreaRange() const
	{
		return this->areaRange;
	}

	void Unit::setAreaRange(float range)
	{
		this->areaRange = range;
	}

	int Unit::getAreaClipMask() const
	{
		return this->areaClipMask;
	}

	void Unit::setAreaClipMask(int clipMask)
	{
		//assert(clipMask >= 1 && clipMask <= 15);
		this->areaClipMask = clipMask;
	}

	int Unit::getAreaCircleId() const
	{
		return this->areaCircleId;
	}

	void Unit::setAreaCircleId(int areaCircleId)
	{
		this->areaCircleId = areaCircleId;
	}

	bool Unit::isFollowPlayer() const
	{
		return this->followPlayer;
	}

	void Unit::setFollowPlayer(bool followPlayer)
	{
		this->followPlayer = followPlayer;
	}

	void Unit::setLastLightUpdatePosition(const VC3 &position)
	{
		this->lastLightUpdatePosition = position;
	}

	VC3 Unit::getLastLightUpdatePosition() const
	{
		return this->lastLightUpdatePosition;
	}

	void Unit::setUnitListEntity(UnitListEntity *listEntity)
	{
		assert((this->unitListEntity == NULL && listEntity != NULL)
			|| (this->unitListEntity != NULL && listEntity == NULL));
		this->unitListEntity = listEntity;
	}

	UnitListEntity *Unit::getUnitListEntity() const
	{
		return this->unitListEntity;
	}

	ui::VisualEffect *Unit::getMuzzleflashVisualEffect() const
	{
		return this->muzzleflashVisualEffect;
	}

	ui::VisualEffect *Unit::getEjectVisualEffect() const
	{
		return this->ejectVisualEffect;
	}

	void Unit::increaseContinuousFireTime()
	{
		this->continuousFireTime++;
	}

	void Unit::clearContinuousFireTime()
	{
		this->continuousFireTime = 0;
	}

	int Unit::getContinuousFireTime() const
	{
		return this->continuousFireTime;
	}

	bool Unit::isOnFire() const
	{
		if (onFireCounter > 0)
			return true;
		else
			return false;
	}

	int Unit::getOnFireCounter() const
	{
		return onFireCounter;
	}

	void Unit::setOnFireCounter(int value)
	{
		this->onFireCounter = value;
	}

	bool Unit::hasAliveMarker() const
	{
		return this->aliveMarker;
	}

	void Unit::setAliveMarker(bool alive)
	{
		this->aliveMarker = alive;
	}

	bool Unit::hasDiedByPoison() const
	{
		return this->diedByPoison;
	}

	void Unit::setDiedByPoison(bool diedByPoison)
	{
		this->diedByPoison = diedByPoison;
	}

	void Unit::rotateWeaponBarrel(unsigned int weapon)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		assert(weaponType[weapon] != NULL);

		if (weaponType[weapon] != NULL)
		{
			this->weaponBarrel[weapon]++;
			if (this->weaponBarrel[weapon] > weaponType[weapon]->getBarrelRotateToNumber())
				this->weaponBarrel[weapon] = weaponType[weapon]->getBarrelRotateFromNumber();
		}
	}

	int Unit::getRotatedWeaponBarrel(unsigned int weapon) const
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		return this->weaponBarrel[weapon];
	}

	void Unit::rotateWeaponEjectBarrel(unsigned int weapon)
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		assert(weaponType[weapon] != NULL);

		if (weaponType[weapon] != NULL)
		{
			this->weaponEjectBarrel[weapon]++;
			if (this->weaponEjectBarrel[weapon] > weaponType[weapon]->getBarrelRotateToNumber())
				this->weaponEjectBarrel[weapon] = weaponType[weapon]->getBarrelRotateFromNumber();
		}
	}

	int Unit::getRotatedWeaponEjectBarrel(unsigned int weapon) const
	{
		assert(weapon < UNIT_MAX_WEAPONS);
		return this->weaponEjectBarrel[weapon];
	}

	void Unit::setLightVisibilityFactor(float lightVisibility, bool forceLightVisibilityFactor)
	{
		this->lightVisibilityFactor = lightVisibility;
		this->forceLightVisibilityFactor = forceLightVisibilityFactor;
	}

	float Unit::getLightVisibilityFactor() const
	{
		return this->lightVisibilityFactor;
	}

	bool Unit::isForcedLightVisibilityFactor() const
	{
		return this->forceLightVisibilityFactor;
	}

	bool Unit::isShootAnimStanding() const
	{
		return this->shootAnimStanding;
	}

	void Unit::setShootAnimStanding(bool shootAnimStanding)
	{
		this->shootAnimStanding = shootAnimStanding;
	}

	bool Unit::isRushing() const
	{
		return this->rushing;
	}

	void Unit::setRushing(bool rushing)
	{
		this->rushing = rushing;
	}

	/*
	void Unit::setDelayedHitProjectileBullet(Bullet *bullet, int interval, int amount)
	{
		this->delayedHitProjectileBullet = bullet;
		this->delayedHitProjectileInterval = interval;
		this->delayedHitProjectileAmount = amount;
	}
	*/

	void Unit::setDelayedHitProjectileBullet(Bullet *bullet)
	{
		this->delayedHitProjectileBullet = bullet;
	}

	void Unit::setDelayedHitProjectileInterval(int interval)
	{
		if (interval < GAME_TICK_MSEC)
			interval = GAME_TICK_MSEC;
		this->delayedHitProjectileInterval = interval;
	}

	void Unit::setDelayedHitProjectileAmount(int amount)
	{
		this->delayedHitProjectileAmount = amount;
	}

	Bullet *Unit::getDelayedHitProjectileBullet() const
	{
		return this->delayedHitProjectileBullet;
	}

	int Unit::getDelayedHitProjectileInterval() const
	{
		return this->delayedHitProjectileInterval;
	}

	int Unit::getDelayedHitProjectileAmount() const
	{
		return this->delayedHitProjectileAmount;
	}

	void Unit::setJumpAnim(int anim)
	{
		this->jumpAnim = anim;
	}

	int Unit::getJumpAnim() const
	{
		return this->jumpAnim;
	}

	void Unit::setAniRecordBlendEndFlag(bool blendEndFlag)
	{
		this->aniRecordBlendEndFlag = blendEndFlag;
	}

	void Unit::setAniRecordBlendFlag(int blendAnim)
	{
		this->aniRecordBlendFlag = blendAnim;
	}

	bool Unit::getAniRecordBlendEndFlag() const
	{
		return this->aniRecordBlendEndFlag;
	}

	int Unit::getAniRecordBlendFlag() const
	{
		return this->aniRecordBlendFlag;
	}

	bool Unit::hasAniRecordFireFlag() const
	{
		return this->aniRecordFireFlag;
	}

	VC3 Unit::getAniRecordFireSourcePosition() const
	{
		return this->aniRecordFireSourcePosition;
	}

	VC3 Unit::getAniRecordFireDestinationPosition() const
	{
		return this->aniRecordFireDestinationPosition;
	}

	void Unit::setAniRecordFirePosition(const VC3 &sourcePosition, const VC3 &destinationPosition)
	{
		this->aniRecordFireFlag = true;
		this->aniRecordFireSourcePosition = sourcePosition;
		this->aniRecordFireDestinationPosition = destinationPosition;
	}

	void Unit::clearAniRecordFireFlag()
	{
		this->aniRecordFireFlag = false;
		this->aniRecordFireSourcePosition = VC3(0,0,0);
		this->aniRecordFireDestinationPosition = VC3(0,0,0);
	}
	
	int Unit::getHighlightStyle() const
	{
		return highlightStyle;
	}

	void Unit::setHighlightStyle( int style )
	{
		highlightStyle = style;
	}

	bool Unit::hasHighlightText() const
	{
		return !highlightText.empty();
	}
	
	std::string Unit::getHighlightText() const
	{
		return highlightText;
	}

	void Unit::setHighlightText( const std::string& styletext )
	{
		highlightText = styletext;
	}

	void Unit::setWeaponRechargeAmount(int chargeAmount)
	{
		this->weaponChargeAmount = chargeAmount;
	}

	void Unit::setWeaponRechargeRange(int chargeMin, int chargePeak, int steps)
	{
		this->weaponChargeMin = chargeMin;
		this->weaponChargePeak = chargePeak;
		this->weaponChargeSteps = steps;
	}

	int Unit::getWeaponRechargeAmount() const
	{
		return this->weaponChargeAmount;
	}

	int Unit::getWeaponRechargeMin() const
	{
		return this->weaponChargeMin;
	}

	int Unit::getWeaponRechargePeak() const
	{
		return this->weaponChargePeak;
	}

	int Unit::getWeaponRechargeSteps() const
	{
		return this->weaponChargeSteps;
	}

	bool Unit::isWeaponRecharging() const
	{
		return this->weaponCharging;
	}

	void Unit::setWeaponRecharging(bool charging)
	{
		this->weaponCharging = charging;
	}

	// THIS IS A TOTAL HACK METHOD, SHOULD BE REMOVED!
	void Unit::setWeaponAmmoInClip(int weapon, int ammoInClip)
	{
		this->weaponAmmoInClip[weapon] = ammoInClip;
	}

	bool Unit::increaseEjectRateCounter(int maxValue)
	{
		this->ejectRateCounter++;
		if (this->ejectRateCounter >= maxValue)
		{
			this->ejectRateCounter = 0;
			return true;
		}
		return false;
	}

	void Unit::setVisualizationOffset( float offset )
	{ 
		if( offset != 0 ) 
		{
			visualOffsetInUse = true; 
			visualizationOffset = offset; 
		}
		else
		{
			visualOffsetInUse = false;
		}
	}

	void Unit::enableHPGain(float limit, int amount, int delay, int startdelay, float damagefactor)
	{
		hpGainLimit = limit;
		hpGainAmount = amount;
		hpGainDelay = delay;
		hpGainStartDelay = startdelay;
		hpGainStartTime = 0;
		hpGainDamageFactor = damagefactor;
	}

	void Unit::disableHPGain(void)
	{
		hpGainStartTime = -1;
	}

	float Unit::getHPGainLimit(void) const
	{
		return hpGainLimit;
	}

	int Unit::getHPGainAmount(void) const
	{
		return hpGainAmount;
	}

	int Unit::getHPGainDelay(void) const
	{
		return hpGainDelay;
	}

	int Unit::getHPGainStartDelay(void) const
	{
		return hpGainStartDelay;
	}

	int Unit::getHPGainStartTime(void) const
	{
		return hpGainStartTime;
	}

	float Unit::getHPGainDamageFactor(void) const
	{
		return hpGainDamageFactor;
	}

	int Unit::getLastTimeDamaged(void) const
	{
		return lastTimeDamaged;
	}

	void Unit::setHPGainStartTime(int time)
	{
		hpGainStartTime = time;
	}

	void Unit::setLastTimeDamaged(int time)
	{
		lastTimeDamaged = time;
	}

	void Unit::setTurningSound(int handle, int timeNow)
	{
		turningSound = handle;
		turningSoundStartTime = timeNow;
	}

	int Unit::getTurningSound() const
	{
		return turningSound;
	}

	int Unit::getTurningSoundStartTime() const
	{
		return turningSoundStartTime;
	}

	int Unit::getTurningSoundVolume() const
	{
		return turningSoundVolume;
	}

	void Unit::setTurningSoundVolume(int vol)
	{
		turningSoundVolume = vol;
	}

	int Unit::getTurningSoundFrequency() const
	{
		return turningSoundFrequency;
	}

	void Unit::setTurningSoundFrequency(int freq)
	{
		turningSoundFrequency = freq;
	}

	const std::vector<FootStepTrigger *> &Unit::getFootStepTriggers() const
	{
		return footStepTriggers;
	}

	void Unit::updateUnitTargetLock(int time, bool being_locked)
	{
		if(being_locked)
		{
			if(time > lastTargetLockTime)
			{
				// increase counter
				if(targetLockCounter < targetLockCounterMax)
				{
					targetLockCounter++;
				}
			}
			lastTargetLockTime = time;
		}
		else
		{
			// locking is finished
			if(targetLockCounter >= targetLockCounterMax)
			{
				// not locked for a longer while
				if(time - lastTargetLockTime > targetLockReleaseTime)
				{
					// decrease counter
					targetLockCounter--;
				}
			}
			else if(targetLockCounter > 0)
			{
				// not locked for a while
				int delta = time - lastTargetLockTime;
				if(delta > targetLockCancelTime || delta > targetLockReleaseTime)
				{
					// decrease counter
					targetLockCounter = (targetLockCounter * 3) / 4;
				}
			}
		}
		
		if(targetLockCounter < targetLockCounterMax)
		{
			targetLockSoundPlayed = false;
		}
	}

	void Unit::setSpeedWhileFiring(Unit::UNIT_SPEED speed)
	{
		if(speedBeforeFiring == -1)
		{
			speedBeforeFiring = (int) getSpeed();
		}
		setSpeed(speed);
	}

	void Unit::resetSpeedAfterFiring(void)
	{
		if(speedBeforeFiring != -1)
		{
			setSpeed((Unit::UNIT_SPEED)speedBeforeFiring);
			speedBeforeFiring = -1;
		}
	}
}


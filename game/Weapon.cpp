
#include "precompiled.h"

#include "../convert/str2int.h"
#include "gamedefs.h"
#include "Weapon.h"
#include "WeaponObject.h"
#include "../system/SystemRandom.h"

#include "../util/Debug_MemoryManager.h"

#define WEAPON_SLOTS 0

#define WEAPON_CPYINHERITSTRING(strname) \
  if (strname != NULL) \
  { \
	  if (ret->strname != NULL) \
		{ \
		  delete [] ret->strname; \
		  ret->strname = NULL; \
		} \
    ret->strname = new char[strlen(strname) + 1]; \
    strcpy(ret->strname, strname); \
  }

namespace game
{

	Weapon::Weapon()
	{
		image = NULL;
		parentType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Tool"));
		slotAmount = WEAPON_SLOTS;
		slotTypes = NULL;
		slotPositions = NULL;
		maxDamage = 1;
		maxHeat = 1;

		for (int dmg = 0; dmg < DAMAGE_TYPES_AMOUNT; dmg++)
		{
			resistance[dmg] = 0;
			damagePass[dmg] = 0;
			damageAbsorb[dmg] = 0;
		}

		bulletType = NULL;
		ammoType = NULL;
		ammoUsage = 0;
		minAmmoForUsage = 1;
		powerUsage = 0;
		heatGeneration = 0;
		fireWaitTime = 0;
		fireWaitVary = 0;
		fireReloadTime = 0;
		fireReloadVary = 0;
		walkStopTime = 0;
		walkStopAtPhase = 0;
		range = 0.0f;
		accuracy = 100;
		lowAccuracy = 100;
		initialAmmo = 0;
		fireSound = NULL;
		repeatSound = NULL;
		prepareFireSound = NULL;
		raytraceAmount = 1;
		repeatAmount = 1;
		repeatDelay = 0;
		repeatSpread = 0;
		heavyWeapon = false;
		dropWeapon = false;
		manualWeapon = false;
		antiVehicleWeapon = false;
		needsRecon = false;
		kickback = 1000 / GAME_TICK_MSEC;
		clipSize = 0;
		clipReloadTime = 0;
		clipReloadSound = NULL;
		clipFirstReloadSound = NULL;
		clipEmptySound = NULL;
		muzzleflashEffect = NULL;
		muzzleflashEffect2 = NULL;
		muzzleflashEffect3 = NULL;
		minSpread = 1.0f;
		maxSpread = 5.0f;
		fireFromWeaponBarrel = true;
		raytraceFromWeaponBarrel = false;

		lastMuzzleflashNumber = 0;

		weaponAnimationType = 0;

		singleShot = false;
		firingRequireWalk = false;

		delayAllWeapons = false;
		throwable = false;

		attachedWeaponType = NULL;
		sharedClipAttachment = false;

		singleReloading = false;
		burstFireAmount = 0;

		projectileRange = -1.0f;

		flattenShootDirection = 0.0f;
		weaponRayHeightFromBarrel = false;

		independentFireDelay = false;

    continuousFireBulletType = NULL;
		continuousFireTime = 0;

		clipFirstReloadTime = 0;

		sweep = false;
		noAutoAim = false;

		barrelNumber = 1;
		barrelRotateFromNumber = 1;
    barrelRotateToNumber = 1;

		sweepAngle = 20;

		rechargeMinTime = 0;
		rechargePeakTime = 0;
		rechargeMaxTime = 0;
		rechargeBulletSteps = 0;
		rechargeSound = NULL;
		rechargeAmmoRate = 0;
		rechargeAmmoPeakHoldRate = 0;

		fireByClick = false;

		shootDirectionLimit = 180.0f;

		rechargeEffect = NULL;

		ejectEffect = NULL;
		clipreloadEffect = NULL;

		ejectRate = 1;
		fireWaitReset = false;

		barrelEjectAngles = NULL;
		numBarrelEjectAngles = 0;

		useCustomTimeFactor = false;

		remoteTrigger = false;
		remoteTriggerSound = NULL;

		targetLock = false;
		targetLockTime = 0;
		targetLockReleaseTime = 0;
		targetLockCancelTime = 0;
		targetLockSound = NULL;

		pointerHelper = NULL;
		pointerVisualEffect = NULL;
		pointerHitVisualEffect = NULL;

		cursor = -1;
		reloadCursor = -1;

		attachedWeaponReload = false;
		fireDelayHack = false;

		launchSpeed = false;
		launchSpeedAnimation = false;
		launchSpeedMax = 0.0f;
		launchSpeedAdd = 0.0f;

		customCamera = false;
		customCameraAngle = 0.0f;
		customCameraZoom = 0.0f;

		selectableWithoutAmmo = true;
		
		aimEndDelay = 100;

		criticalHitDamageMax = 10000;
		criticalHitDamageMultiplier = 10.0f;
		criticalHitProbabilityMultiplier = 1.0f;

		reloadFinishedSound = NULL;
		customWeaponBarrelHelper = NULL;
		// NOTICE!!!
		// any variable added needs to be handled below (inherit method)
	}

  void Weapon::prepareNewForInherit(PartType *partType)
  {
    this->PartType::prepareNewForInherit(partType);

    // WARNING!
    // TODO: should really check that the given parameter is of class!
    Weapon *ret = (Weapon *)partType;

    ret->bulletType = bulletType;
    ret->ammoType = ammoType;
    ret->ammoUsage= ammoUsage;
    ret->powerUsage = powerUsage;
    ret->heatGeneration = heatGeneration;
    ret->fireWaitTime = fireWaitTime;
    ret->fireReloadTime = fireReloadTime;
    ret->fireWaitVary = fireWaitVary;
    ret->fireReloadVary = fireReloadVary;
    ret->walkStopTime = walkStopTime;
    ret->walkStopAtPhase = walkStopAtPhase;
    ret->range = range;
    ret->accuracy = accuracy;
    ret->lowAccuracy = lowAccuracy;
    ret->initialAmmo = initialAmmo;
    ret->raytraceAmount = raytraceAmount;
    ret->repeatAmount = repeatAmount;
    ret->repeatDelay = repeatDelay;
    ret->repeatSpread = repeatSpread;
    ret->heavyWeapon = heavyWeapon;
    ret->dropWeapon = dropWeapon;
    ret->manualWeapon = manualWeapon;
    ret->antiVehicleWeapon = antiVehicleWeapon;
		ret->needsRecon = needsRecon;
		ret->kickback = kickback;
		ret->clipSize = clipSize;
		ret->clipReloadTime = clipReloadTime;
    WEAPON_CPYINHERITSTRING(fireSound);
    WEAPON_CPYINHERITSTRING(prepareFireSound);
    WEAPON_CPYINHERITSTRING(repeatSound);
    WEAPON_CPYINHERITSTRING(clipReloadSound);
    WEAPON_CPYINHERITSTRING(clipFirstReloadSound);
    WEAPON_CPYINHERITSTRING(clipEmptySound);
    WEAPON_CPYINHERITSTRING(muzzleflashEffect);
    WEAPON_CPYINHERITSTRING(muzzleflashEffect2);
    WEAPON_CPYINHERITSTRING(muzzleflashEffect3);
		ret->minSpread = minSpread;
		ret->maxSpread = maxSpread;
		ret->fireFromWeaponBarrel = fireFromWeaponBarrel;
		ret->raytraceFromWeaponBarrel = raytraceFromWeaponBarrel;
		ret->weaponAnimationType = weaponAnimationType;
		//ret->lastMuzzleflashNumber = lastMuzzleflashNumber;
		ret->singleShot = singleShot;
		ret->firingRequireWalk = firingRequireWalk;
		ret->delayAllWeapons = delayAllWeapons;
		ret->throwable = throwable;
		ret->attachedWeaponType = attachedWeaponType;
		ret->sharedClipAttachment = sharedClipAttachment;
		ret->singleReloading = singleReloading;
		ret->burstFireAmount = burstFireAmount;
		ret->projectileRange = projectileRange;
		ret->flattenShootDirection = flattenShootDirection;
		ret->weaponRayHeightFromBarrel = weaponRayHeightFromBarrel;
		ret->independentFireDelay = independentFireDelay;
		ret->continuousFireBulletType = continuousFireBulletType;
		ret->continuousFireTime = continuousFireTime;
		ret->clipFirstReloadTime = clipFirstReloadTime;
		ret->minAmmoForUsage = minAmmoForUsage;
		ret->sweep = sweep;
		ret->noAutoAim = noAutoAim;
		ret->barrelNumber = barrelNumber;
		ret->barrelRotateFromNumber = barrelRotateFromNumber;
		ret->barrelRotateToNumber = barrelRotateToNumber;
		ret->sweepAngle = sweepAngle;

		ret->rechargeMinTime = rechargeMinTime;
		ret->rechargePeakTime = rechargePeakTime;
		ret->rechargeMaxTime = rechargeMaxTime;
		ret->rechargeBulletSteps = rechargeBulletSteps;
    WEAPON_CPYINHERITSTRING(rechargeSound);
		ret->rechargeAmmoRate = rechargeAmmoRate;
		ret->rechargeAmmoPeakHoldRate = rechargeAmmoPeakHoldRate;
		ret->fireByClick = fireByClick;
		ret->shootDirectionLimit = shootDirectionLimit;
    WEAPON_CPYINHERITSTRING(rechargeEffect);
    WEAPON_CPYINHERITSTRING(ejectEffect);
    WEAPON_CPYINHERITSTRING(clipreloadEffect);
		ret->ejectRate = ejectRate;
		ret->fireWaitReset = fireWaitReset;
		ret->barrelEjectAngles = (int *)realloc(ret->barrelEjectAngles, sizeof(int) * numBarrelEjectAngles);
		memcpy(ret->barrelEjectAngles, barrelEjectAngles, sizeof(int) * numBarrelEjectAngles);
		ret->numBarrelEjectAngles = numBarrelEjectAngles;
		ret->useCustomTimeFactor = useCustomTimeFactor;
		ret->remoteTrigger = remoteTrigger;
		WEAPON_CPYINHERITSTRING(remoteTriggerSound);
		ret->targetLock = targetLock;
		ret->targetLockTime = targetLockTime;
		ret->targetLockReleaseTime = targetLockReleaseTime;
		ret->targetLockCancelTime = targetLockCancelTime;
		WEAPON_CPYINHERITSTRING(targetLockSound);
		WEAPON_CPYINHERITSTRING(pointerHelper);
		WEAPON_CPYINHERITSTRING(pointerVisualEffect);
		WEAPON_CPYINHERITSTRING(pointerHitVisualEffect);
		ret->cursor = cursor;
		ret->reloadCursor = reloadCursor;
		ret->attachedWeaponReload = attachedWeaponReload;
		ret->fireDelayHack = fireDelayHack;

		ret->launchSpeed = launchSpeed;
		ret->launchSpeedAnimation = launchSpeedAnimation;
		ret->launchSpeedMax = launchSpeedMax;
		ret->launchSpeedAdd = launchSpeedAdd;

		ret->customCamera = customCamera;
		ret->customCameraAngle = customCameraAngle;
		ret->customCameraZoom = customCameraZoom;

		ret->selectableWithoutAmmo = selectableWithoutAmmo;

		ret->aimEndDelay = aimEndDelay;

		ret->criticalHitDamageMax = criticalHitDamageMax;
		ret->criticalHitDamageMultiplier = criticalHitDamageMultiplier;
		ret->criticalHitProbabilityMultiplier = criticalHitProbabilityMultiplier;
		WEAPON_CPYINHERITSTRING(reloadFinishedSound);
		WEAPON_CPYINHERITSTRING(customWeaponBarrelHelper);
  }

	void Weapon::saveOriginals()
	{
		if (originals == NULL)
		{
			originals = new Weapon();
		} else {
			assert(!"Weapon::saveOriginals - Attempt to save originals multiple times.");
		}

		// FIXME: this may not be correct way to do this!
		this->prepareNewForInherit(originals);
	}

/*
	Weapon::Weapon(int id)
	{
		parentType = getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Tool"));
		setPartTypeId(id);
	}
*/

	Weapon::~Weapon()
	{
		free(barrelEjectAngles);
		if (fireSound != NULL) { delete [] fireSound; fireSound = NULL; }
		if (prepareFireSound != NULL) { delete [] prepareFireSound; prepareFireSound = NULL; }
		if (repeatSound != NULL) { delete [] repeatSound; repeatSound = NULL; }
		if (clipReloadSound != NULL) { delete [] clipReloadSound; clipReloadSound = NULL; }
		if (clipFirstReloadSound != NULL) { delete [] clipFirstReloadSound; clipFirstReloadSound = NULL; }
		if (clipEmptySound != NULL) { delete [] clipEmptySound; clipEmptySound = NULL; }
		if (muzzleflashEffect != NULL) { delete [] muzzleflashEffect; muzzleflashEffect = NULL; }
		if (muzzleflashEffect2 != NULL) { delete [] muzzleflashEffect2; muzzleflashEffect2 = NULL; }
		if (muzzleflashEffect3 != NULL) { delete [] muzzleflashEffect3; muzzleflashEffect3 = NULL; }
		if (rechargeSound != NULL) { delete [] rechargeSound; rechargeSound = NULL; }
		if (remoteTriggerSound != NULL) { delete [] remoteTriggerSound; remoteTriggerSound = NULL; }
		if (pointerHelper != NULL) { delete [] pointerHelper; pointerHelper = NULL; };
		if (pointerVisualEffect != NULL) { delete [] pointerVisualEffect; pointerVisualEffect = NULL; }
		if (pointerHitVisualEffect != NULL) { delete [] pointerHitVisualEffect; pointerHitVisualEffect = NULL; }
		if (targetLockSound != NULL) { delete [] targetLockSound; targetLockSound = NULL; }
		if (reloadFinishedSound != NULL) { delete [] reloadFinishedSound; reloadFinishedSound = NULL; }
		if (customWeaponBarrelHelper != NULL) { delete [] customWeaponBarrelHelper; customWeaponBarrelHelper = NULL; }
	}


	bool Weapon::setData(char *key, char *value)
	{
		if (strcmp(key, "bullettype") == 0)
		{
			if (!PARTTYPE_ID_STRING_VALID(value)) return false;

			int val = PARTTYPE_ID_STRING_TO_INT(value);
			PartType *bullet = getPartTypeById(val);
			if (bullet == NULL) return false;
			if (!bullet->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
				return false;
			// WARNING: Unsafe cast! (although checked above)
			bulletType = (Bullet *)bullet;
			return true;
		}
		if (strcmp(key, "ammotype") == 0)
		{
			if (!PARTTYPE_ID_STRING_VALID(value)) return false;

			int val = PARTTYPE_ID_STRING_TO_INT(value);
			PartType *ammotype = getPartTypeById(val);
			if (ammotype == NULL) return false;
			if (!ammotype->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Ammo"))))
				return false;
			// WARNING: Unsafe cast! (although checked above)
			ammoType = (AmmoPack *)ammotype;
			return true;
		}
		if (strcmp(key, "initialammo") == 0)
		{
			initialAmmo = str2int(value);
			return true;
		}
		if (strcmp(key, "ammousage") == 0)
		{
			ammoUsage = str2int(value);
			return true;
		}
		if (strcmp(key, "minammoforusage") == 0)
		{
			minAmmoForUsage = str2int(value);
			return true;
		}
		if (strcmp(key, "powerusage") == 0)
		{
			powerUsage = str2int(value);
			return true;
		}
		if (strcmp(key, "accuracy") == 0)
		{
			accuracy = str2int(value);
			return true;
		}
		if (strcmp(key, "lowaccuracy") == 0)
		{
			lowAccuracy = str2int(value);
			return true;
		}
		if (strcmp(key, "heatgen") == 0)
		{
			heatGeneration = str2int(value);
			return true;
		}
		if (strcmp(key, "firewaittime") == 0)
		{
			fireWaitTime = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && fireWaitTime == 0) 
				fireWaitTime = 1;
			return true;
		}
		if (strcmp(key, "firereloadtime") == 0)
		{
			fireReloadTime = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && fireReloadTime == 0) 
				fireReloadTime = 1;
			return true;
		}
		if (strcmp(key, "firewaitvary") == 0)
		{
			fireWaitVary = str2int(value);
			return true;
		}
		if (strcmp(key, "firereloadvary") == 0)
		{
			fireReloadVary = str2int(value);
			return true;
		}
		if (strcmp(key, "walkstoptime") == 0)
		{
			walkStopTime = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && walkStopTime == 0) 
				walkStopTime = 1;
			return true;
		}
		if (strcmp(key, "walkstopatphase") == 0)
		{
			walkStopAtPhase = str2int(value);
			if (walkStopAtPhase < 0 || walkStopAtPhase > 2)
			{
				walkStopAtPhase = 0;
				return false;
			} else {
				return true;
			}
		}
		if (strcmp(key, "range") == 0)
		{
			range = (float)atof(value);
			return true;
		}
		if (strcmp(key, "projectilerange") == 0)
		{
			projectileRange = (float)atof(value);
			return true;
		}
		if (strcmp(key, "flattenshootdirection") == 0)
		{
			flattenShootDirection = (float)atof(value);
			return true;
		}
		if (strcmp(key, "shootdirectionlimit") == 0)
		{
			shootDirectionLimit = (float)atof(value);
			return true;
		}
		if (strcmp(key, "weaponrayheightfrombarrel") == 0)
		{
			if (str2int(value) != 0) 
				weaponRayHeightFromBarrel = true;
			else
				weaponRayHeightFromBarrel = false;
			return true;
		}
		if (strcmp(key, "firesound") == 0)
		{
			if (fireSound != NULL) delete [] fireSound;
			fireSound = new char[strlen(value) + 1];
			strcpy(fireSound, value);
			return true;
		}
		if (strcmp(key, "preparefiresound") == 0)
		{
			if (prepareFireSound != NULL) delete [] prepareFireSound;
			prepareFireSound = new char[strlen(value) + 1];
			strcpy(prepareFireSound, value);
			return true;
		}
		if (strcmp(key, "repeatsound") == 0)
		{
			if (repeatSound != NULL) delete [] repeatSound;
			repeatSound = new char[strlen(value) + 1];
			strcpy(repeatSound, value);
			return true;
		}
		if (strcmp(key, "raytraceamount") == 0)
		{
			raytraceAmount = str2int(value);
			return true;
		}
		if (strcmp(key, "repeatamount") == 0)
		{
			repeatAmount = str2int(value);
			return true;
		}
		if (strcmp(key, "repeatdelay") == 0)
		{
			repeatDelay = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && repeatDelay == 0) 
				repeatDelay = 1;
			return true;
		}
		if (strcmp(key, "repeatspread") == 0)
		{
			repeatSpread = str2int(value);
			return true;
		}
		if (strcmp(key, "minspread") == 0)
		{
			minSpread = (float)atof(value);
			return true;
		}
		if (strcmp(key, "maxspread") == 0)
		{
			maxSpread = (float)atof(value);
			return true;
		}
		if (strcmp(key, "rechargemintime") == 0)
		{
			rechargeMinTime = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && rechargeMinTime == 0) 
				rechargeMinTime = 1;
			return true;
		}
		if (strcmp(key, "rechargemaxtime") == 0)
		{
			rechargeMaxTime = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && rechargeMaxTime == 0) 
				rechargeMaxTime = 1;
			return true;
		}
		if (strcmp(key, "rechargepeaktime") == 0)
		{
			rechargePeakTime = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && rechargePeakTime == 0) 
				rechargePeakTime = 1;
			return true;
		}
		if (strcmp(key, "rechargebulletsteps") == 0)
		{
			rechargeBulletSteps = str2int(value);
			return true;
		}
		if (strcmp(key, "rechargesound") == 0)
		{
			if (rechargeSound != NULL) delete [] rechargeSound;
			if (strcmp(value, "null") == 0)
			{
				rechargeSound = NULL;
			} else {
				rechargeSound = new char[strlen(value) + 1];
				strcpy(rechargeSound, value);
			}
			return true;
		}
		if (strcmp(key, "rechargeammorate") == 0)
		{
			rechargeAmmoRate = str2int(value);
			return true;
		}
		if (strcmp(key, "rechargeammopeakholdrate") == 0)
		{
			rechargeAmmoPeakHoldRate = str2int(value);
			return true;
		}
		if (strcmp(key, "heavyweapon") == 0)
		{
			if (str2int(value) == 1)
			{
				heavyWeapon = true;
			} else {
				heavyWeapon = false;
			}
			return true;
		}
		if (strcmp(key, "dropweapon") == 0)
		{
			if (str2int(value) == 1)
			{
				dropWeapon = true;
			} else {
				dropWeapon = false;
			}
			return true;
		}
		if (strcmp(key, "sweep") == 0)
		{
			if (str2int(value) == 1)
			{
				sweep = true;
			} else {
				sweep = false;
			}
			return true;
		}
		if (strcmp(key, "firebyclick") == 0)
		{
			if (str2int(value) == 1)
			{
				fireByClick = true;
			} else {
				fireByClick = false;
			}
			return true;
		}
		if (strcmp(key, "sweepangle") == 0)
		{
			sweepAngle = str2int(value);
			return true;
		}
		if (strcmp(key, "noautoaim") == 0)
		{
			if (str2int(value) == 1)
			{
				noAutoAim = true;
			} else {
				noAutoAim = false;
			}
			return true;
		}
		if (strcmp(key, "manualweapon") == 0)
		{
			if (str2int(value) == 1)
			{
				manualWeapon = true;
			} else {
				manualWeapon = false;
			}
			return true;
		}
		if (strcmp(key, "antivehicleweapon") == 0)
		{
			if (str2int(value) == 1)
			{
				antiVehicleWeapon = true;
			} else {
				antiVehicleWeapon = false;
			}
			return true;
		}
		if (strcmp(key, "needsrecon") == 0)
		{
			if (str2int(value) == 1)
				needsRecon = true;
			else
				needsRecon = false;
			return true;
		}
		if (strcmp(key, "kickback") == 0)
		{
			kickback = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && kickback == 0) 
				kickback = 1;
			return true;
		}
		if (strcmp(key, "clipsize") == 0)
		{
			clipSize = str2int(value);
			return true;
		}
		if (strcmp(key, "clipreloadtime") == 0)
		{
			clipReloadTime = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && clipReloadTime == 0) 
				clipReloadTime = 1;
			return true;
		}
		if (strcmp(key, "clipfirstreloadtime") == 0)
		{
			clipFirstReloadTime = str2int(value) / GAME_TICK_MSEC;
			if (str2int(value) > 0 && clipFirstReloadTime == 0) 
				clipFirstReloadTime = 1;
			return true;
		}
		if (strcmp(key, "singlereloading") == 0)
		{
			if (str2int(value) == 1)
			{
				singleReloading = true;
			} else {
				singleReloading = false;
			}
			return true;
		}
		if (strcmp(key, "burstfireamount") == 0)
		{
			burstFireAmount = str2int(value);
			return true;
		}
		if (strcmp(key, "clipreloadsound") == 0)
		{
			if (clipReloadSound != NULL) delete [] clipReloadSound;
		  clipReloadSound = new char[strlen(value) + 1];
			strcpy(clipReloadSound, value);
			return true;
		}
		if (strcmp(key, "clipfirstreloadsound") == 0)
		{
			if (clipFirstReloadSound != NULL) delete [] clipFirstReloadSound;
		  clipFirstReloadSound = new char[strlen(value) + 1];
			strcpy(clipFirstReloadSound, value);
			return true;
		}
		if (strcmp(key, "clipemptysound") == 0)
		{
			if (clipEmptySound != NULL) delete [] clipEmptySound;
		  clipEmptySound = new char[strlen(value) + 1];
			strcpy(clipEmptySound, value);
			return true;
		}
		if (strcmp(key, "muzzleflasheffect") == 0)
		{
			if (muzzleflashEffect != NULL) delete [] muzzleflashEffect;
		  muzzleflashEffect = new char[strlen(value) + 1];
			strcpy(muzzleflashEffect, value);
			return true;
		}
		if (strcmp(key, "muzzleflasheffect2") == 0)
		{
			if (muzzleflashEffect2 != NULL) delete [] muzzleflashEffect2;
		  muzzleflashEffect2 = new char[strlen(value) + 1];
			strcpy(muzzleflashEffect2, value);
			return true;
		}
		if (strcmp(key, "muzzleflasheffect3") == 0)
		{
			if (muzzleflashEffect3 != NULL) delete [] muzzleflashEffect3;
		  muzzleflashEffect3 = new char[strlen(value) + 1];
			strcpy(muzzleflashEffect3, value);
			return true;
		}
		if (strcmp(key, "rechargeeffect") == 0)
		{
			if (rechargeEffect != NULL) delete [] rechargeEffect;
		  rechargeEffect = new char[strlen(value) + 1];
			strcpy(rechargeEffect, value);
			return true;
		}
		if (strcmp(key, "weaponanimationtype") == 0)
		{
			weaponAnimationType = str2int(value);
			return true;
		}
		if (strcmp(key, "barrelnumber") == 0)
		{
			barrelNumber = str2int(value);
			barrelRotateFromNumber = barrelNumber;
			return true;
		}
		if (strcmp(key, "barrelrotatetonumber") == 0)
		{
			barrelRotateToNumber = str2int(value);
			return true;
		}
		if (strcmp(key, "firefromweaponbarrel") == 0)
		{
			if (str2int(value) == 1)
			{
				fireFromWeaponBarrel = true;
			} else {
				fireFromWeaponBarrel = false;
			}
			return true;
		}
		if (strcmp(key, "raytracefromweaponbarrel") == 0)
		{
			if (str2int(value) == 1)
			{
				raytraceFromWeaponBarrel = true;
			} else {
				raytraceFromWeaponBarrel = false;
			}
			return true;
		}
		if (strcmp(key, "singleshot") == 0)
		{
			if (str2int(value) == 1)
			{
				singleShot = true;
			} else {
				singleShot = false;
			}
			return true;
		}
		if (strcmp(key, "firingrequirewalk") == 0)
		{
			if (str2int(value) == 1)
			{
				firingRequireWalk = true;
			} else {
				firingRequireWalk = false;
			}
			return true;
		}
		if (strcmp(key, "delayallweapons") == 0)
		{
			if (str2int(value) == 1)
			{
				delayAllWeapons = true;
			} else {
				delayAllWeapons = false;
			}
			return true;
		}
		if (strcmp(key, "independentfiredelay") == 0)
		{
			int val = str2int(value);
			if (val == 1)
			{
				independentFireDelay = true;
				fireDelayHack = false;
			}
			else if(val == 2)
			{
				// hack to delay secondary weapon but not attached weapon
				independentFireDelay = false;
				fireDelayHack = true;
			}
			else
			{
				independentFireDelay = false;
				fireDelayHack = false;
			}
			return true;
		}
		if (strcmp(key, "throwable") == 0)
		{
			if (str2int(value) == 1)
			{
				throwable = true;
			} else {
				throwable = false;
			}
			return true;
		}
		if (strcmp(key, "sharedclipattachment") == 0)
		{
			if (str2int(value) == 1)
			{
				sharedClipAttachment = true;
			} else {
				sharedClipAttachment = false;
			}
			return true;
		}
		if (strcmp(key, "attachedweapontype") == 0)
		{
			if (!PARTTYPE_ID_STRING_VALID(value)) return false;

			int val = PARTTYPE_ID_STRING_TO_INT(value);
			PartType *wtype = getPartTypeById(val);
			if (wtype == NULL) return false;
			if (!wtype->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Weap"))))
				return false;
			// WARNING: Unsafe cast! (although checked above)
			attachedWeaponType = (Weapon *)wtype;
			return true;
		}
		if (strcmp(key, "continuousfiretime") == 0)
		{
			continuousFireTime = str2int(value) / GAME_TICK_MSEC;
			return true;
		}
		if (strcmp(key, "continuousfirebullettype") == 0)
		{
			if (!PARTTYPE_ID_STRING_VALID(value)) return false;

			int val = PARTTYPE_ID_STRING_TO_INT(value);
			PartType *bullet = getPartTypeById(val);
			if (bullet == NULL) return false;
			if (!bullet->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
				return false;
			// WARNING: Unsafe cast! (although checked above)
			continuousFireBulletType = (Bullet *)bullet;
			return true;
		}
		if (strcmp(key, "ejecteffect") == 0)
		{
			if (ejectEffect != NULL) delete [] ejectEffect;
		  ejectEffect = new char[strlen(value) + 1];
			strcpy(ejectEffect, value);
			return true;
		}
		if (strcmp(key, "clipreloadeffect") == 0)
		{
			if (clipreloadEffect != NULL) delete [] clipreloadEffect;
		  clipreloadEffect = new char[strlen(value) + 1];
			strcpy(clipreloadEffect, value);
			return true;
		}
		if (strcmp(key, "ejectrate") == 0)
		{
			ejectRate = str2int(value);
			return true;
		}
		if (strcmp(key, "firewaitreset") == 0)
		{
			if (str2int(value) == 1)
			{
				fireWaitReset = true;
			} else {
				fireWaitReset = false;
			}
			return true;
		}
		if (strcmp(key, "barrelejectangles") == 0)
		{
			// count number of entries
			{
				numBarrelEjectAngles = 0;
				int i = 0;
				while(value[i] != 0)
				{
					if(value[i] == ',')
						numBarrelEjectAngles++;
					i++;
				}
				numBarrelEjectAngles++;
			}

			// allocate array
			barrelEjectAngles = (int *)realloc(barrelEjectAngles, sizeof(int) * numBarrelEjectAngles);

			// read entries
			int i = 0;
			int lastpos = 0;
			int entry = 0;
			while(true)
			{
				if(value[i] == ',' || value[i] == 0)
				{
					if(sscanf(value + lastpos, "%i", &barrelEjectAngles[entry]) == 1)
					{
						entry++;
						lastpos = i+1;
					}
				}
				if(value[i] == 0)
					break;
				i++;
			}
			numBarrelEjectAngles = entry;
			return true;
		}

		if( strcmp(key, "usecustomtimefactor") == 0)
		{
			useCustomTimeFactor = str2int(value) != 0 ? true : false;
			return true;
		}

		if(strcmp(key, "remotetrigger") == 0)
		{
			remoteTrigger = str2int(value) != 0 ? true : false;
			return true;
		}

		if (strcmp(key, "remotetriggersound") == 0)
		{
			if (remoteTriggerSound != NULL) delete [] remoteTriggerSound;
		  remoteTriggerSound = new char[strlen(value) + 1];
			strcpy(remoteTriggerSound, value);
			return true;
		}

		if( strcmp(key, "targetlock") == 0)
		{
			targetLock = str2int(value) != 0 ? true : false;
			return true;
		}

		if( strcmp(key, "targetlocktime") == 0)
		{
			targetLockTime = (str2int(value) * GAME_TICKS_PER_SECOND) / 1000;
			return true;
		}

		if( strcmp(key, "targetlockreleasetime") == 0)
		{
			targetLockReleaseTime = (str2int(value) * GAME_TICKS_PER_SECOND) / 1000;
			return true;
		}

		if( strcmp(key, "targetlockcanceltime") == 0)
		{
			targetLockCancelTime = (str2int(value) * GAME_TICKS_PER_SECOND) / 1000;
			return true;
		}

		if (strcmp(key, "targetlocksound") == 0)
		{
			if (targetLockSound != NULL) delete [] targetLockSound;
			targetLockSound = new char[strlen(value) + 1];
			strcpy(targetLockSound, value);
			return true;
		}

		if( strcmp(key, "pointerhelper" ) == 0)
		{
			if (pointerHelper != NULL) delete [] pointerHelper;
		  pointerHelper = new char[strlen(value) + 1];
			strcpy(pointerHelper, value);
			return true;
		}

		if( strcmp(key, "pointervisualeffect" ) == 0)
		{
			if (pointerVisualEffect != NULL) delete [] pointerVisualEffect;
		  pointerVisualEffect = new char[strlen(value) + 1];
			strcpy(pointerVisualEffect, value);
			return true;
		}

		if( strcmp(key, "pointerhitvisualeffect" ) == 0)
		{
			if (pointerHitVisualEffect != NULL) delete [] pointerHitVisualEffect;
		  pointerHitVisualEffect = new char[strlen(value) + 1];
			strcpy(pointerHitVisualEffect, value);
			return true;
		}

		if(strcmp(key, "cursor") == 0)
		{
			cursor = str2int(value);
			return true;
		}

		if(strcmp(key, "reloadcursor") == 0)
		{
			reloadCursor = str2int(value);
			return true;
		}

		if( strcmp(key, "attachedweaponreload") == 0)
		{
			attachedWeaponReload = str2int(value) != 0 ? true : false;
			return true;
		}

		if(strcmp(key, "launchspeedmultiplier") == 0)
		{
			launchSpeed = true;
			launchSpeedMax = (float)atof(value) - 1.0f;
			if(launchSpeedMax < 0.0f)
			{
				return false;
			}
			return true;
		}

		if(strcmp(key, "launchspeedadd") == 0)
		{
			launchSpeed = true;
			launchSpeedAdd = (float)atof(value);
			return true;
		}

		if(strcmp(key, "launchspeedanimation") == 0)
		{
			launchSpeedAnimation = atoi(value) == 0 ? false : true;
			return true;
		}

		if(strcmp(key, "customcameraangle") == 0)
		{
			customCamera = true;
			customCameraAngle = (float)atof(value);
			return true;
		}

		if(strcmp(key, "customcamerazoom") == 0)
		{
			customCamera = true;
			customCameraZoom = (float)atof(value);
			return true;
		}

		if(strcmp(key, "selectablewithoutammo") == 0)
		{
			selectableWithoutAmmo = atoi(value) == 0 ? false : true;
			return true;
		}

		if(strcmp(key, "aimenddelay") == 0)
		{
			aimEndDelay = atoi(value);
			return true;
		}

		if(strcmp(key, "criticalhitdamagemax") == 0)
		{
			aimEndDelay = atoi(value);
			return true;
		}
		if(strcmp(key, "criticalhitdamagemultiplier") == 0)
		{
			criticalHitDamageMultiplier = (float)atof(value);
			return true;
		}
		if(strcmp(key, "criticalhitprobabilitymultiplier") == 0)
		{
			criticalHitProbabilityMultiplier = (float)atof(value);
			return true;
		}
		if (strcmp(key, "reloadfinishedsound") == 0)
		{
			if (reloadFinishedSound != NULL) delete [] reloadFinishedSound;
			reloadFinishedSound = new char[strlen(value) + 1];
			strcpy(reloadFinishedSound, value);
			return true;
		}
		if (strcmp(key, "customweaponbarrelhelper") == 0)
		{
			if (customWeaponBarrelHelper != NULL) delete [] customWeaponBarrelHelper;
			customWeaponBarrelHelper = new char[strlen(value) + 1];
			strcpy(customWeaponBarrelHelper, value);
			return true;
		}
		return setRootData(key, value);
	}


	Bullet *Weapon::getBulletType() const 
	{
		return bulletType;
	}


	AmmoPack *Weapon::getAmmoType() const
	{
		return ammoType;
	}


	const char *Weapon::getPrepareFireSound() const
	{
		return prepareFireSound;
	}


	const char *Weapon::getFireSound() const
	{
		return fireSound;
	}


	const char *Weapon::getRepeatSound() const
	{
		return repeatSound;
	}


	int Weapon::getHeatGeneration() const
	{
		return heatGeneration;
	}


	int Weapon::getPowerUsage() const
	{
		return powerUsage;
	}


	int Weapon::getAccuracy() const
	{
		return accuracy;
	}

	
	int Weapon::getLowAccuracy() const
	{
		return lowAccuracy;
	}


	int Weapon::getAmmoUsage() const
	{
		return ammoUsage;
	}


	int Weapon::getWalkStopTime() const
	{
		return walkStopTime;
	}


	int Weapon::getWalkStopAtPhase() const
	{
		return walkStopAtPhase;
	}


	int Weapon::getFireReloadTime() const
	{
		return fireReloadTime;
	}


	int Weapon::getFireWaitTime() const
	{
		return fireWaitTime;
	}


	int Weapon::getFireReloadVary() const
	{
		return fireReloadVary;
	}


	int Weapon::getFireWaitVary() const
	{
		return fireWaitVary;
	}


	float Weapon::getRange() const
	{
		return range;
	}


	int Weapon::getInitialAmmoAmount() const
	{
		return initialAmmo;
	}


	Part *Weapon::getNewPartInstance()
	{
		WeaponObject *ret = new WeaponObject();
		ret->setType(this);
		ret->setAmmoAmount(initialAmmo);
		ret->setMaxAmmoAmount(initialAmmo);
		return ret;
	} 


	int Weapon::getRaytraceAmount() const
	{
		return raytraceAmount;
	}

	int Weapon::getRepeatAmount() const
	{
		return repeatAmount;
	}

	int Weapon::getRepeatDelay() const
	{
		return repeatDelay;
	}

	int Weapon::getRepeatSpread() const
	{
		return repeatSpread;
	}

	bool Weapon::isHeavyWeapon() const
	{
		return heavyWeapon;
	}

	bool Weapon::isDropWeapon() const
	{
		return dropWeapon;
	}

	bool Weapon::isManualWeapon() const
	{
		return manualWeapon;
	}

	bool Weapon::isAntiVehicleWeapon() const
	{
		return antiVehicleWeapon;
	}

	bool Weapon::doesNeedRecon() const
	{
		return needsRecon;
	}

	int Weapon::getKickback() const
	{
		return kickback;
	}


	const char *Weapon::getClipReloadSound() const
	{
		return clipReloadSound;
	}


	const char *Weapon::getClipFirstReloadSound() const
	{
		return clipFirstReloadSound;
	}


	const char *Weapon::getClipEmptySound() const
	{
		return clipEmptySound;
	}


	int Weapon::getClipSize() const
	{
		return clipSize;
	}


	int Weapon::getClipReloadTime() const
	{
		return clipReloadTime;
	}


	int Weapon::getClipFirstReloadTime() const
	{
		return clipFirstReloadTime;
	}


	float Weapon::getMinSpread() const
	{
		return minSpread;
	}


	float Weapon::getMaxSpread() const
	{
		return maxSpread;
	}


	const char *Weapon::getMuzzleflashEffect()
	{		
		// HACK: return a random muzzleflash effect name

		if (muzzleflashEffect == NULL)
			return NULL;

		if (muzzleflashEffect3 != NULL 
			&& muzzleflashEffect2 != NULL)
		{
			lastMuzzleflashNumber = (lastMuzzleflashNumber 
				+ 1 + (SystemRandom::getInstance()->nextInt() % 2)) % 3;

			if (lastMuzzleflashNumber == 0)
				return muzzleflashEffect;
			else
				if (lastMuzzleflashNumber == 1)
					return muzzleflashEffect2;
				else
					return muzzleflashEffect3;
		} else {
			if (muzzleflashEffect2 != NULL)
			{
				if ((SystemRandom::getInstance()->nextInt() % 2) == 0)
					return muzzleflashEffect;				
				else
					return muzzleflashEffect2;
			} else {
				return muzzleflashEffect;				
			}
		}
	}

	const char *Weapon::getRechargeEffect() const
	{		
		return rechargeEffect;
	}

	const char *Weapon::getEjectEffect() const
	{		
		return ejectEffect;
	}

	int Weapon::getEjectRate() const
	{		
		return ejectRate;
	}

	const char *Weapon::getClipreloadEffect() const
	{		
		return clipreloadEffect;
	}

	int Weapon::getWeaponAnimationType() const
	{
		return weaponAnimationType;
	}

	bool Weapon::doesFireFromWeaponBarrel() const
	{
		return fireFromWeaponBarrel;
	}

	bool Weapon::doesRaytraceFromWeaponBarrel() const
	{
		return raytraceFromWeaponBarrel;
	}

	bool Weapon::isSingleShot() const
	{
		return singleShot;
	}

	bool Weapon::doesFiringRequireWalk() const
	{
		return firingRequireWalk;
	}

	bool Weapon::doesDelayAllWeapons() const
	{
		return delayAllWeapons;
	}

	bool Weapon::isThrowable() const
	{
		return throwable;
	}

	Weapon *Weapon::getAttachedWeaponType() const
	{
		return attachedWeaponType;
	}

	bool Weapon::isSharedClipAttachment() const
	{
		return sharedClipAttachment;
	}

	bool Weapon::isSingleReloading() const
	{
		return singleReloading;
	}

	int Weapon::getBurstFireAmount() const
	{
		return burstFireAmount;
	}

	float Weapon::getFlattenShootDirection() const
	{
		return flattenShootDirection;
	}

	bool Weapon::isWeaponRayHeightFromBarrel() const
	{
		return weaponRayHeightFromBarrel;
	}

	float Weapon::getProjectileRange() const
	{
		if (projectileRange < 0.0f)
			return range;
		else
			return projectileRange;
	}

	bool Weapon::hasIndependentFireDelay() const
	{
		return independentFireDelay;
	}

	int Weapon::getContinuousFireTime() const
	{
		return this->continuousFireTime;
	}

	Bullet *Weapon::getContinuousFireBulletType() const
	{
		return this->continuousFireBulletType;
	}

	int Weapon::getMinAmmoForUsage() const
	{
		return this->minAmmoForUsage;
	}

	bool Weapon::doesSweep() const
	{
		return this->sweep;
	}

	int Weapon::getSweepAngle() const
	{
		return this->sweepAngle;
	}

	bool Weapon::hasNoAutoAim() const
	{
		return this->noAutoAim;
	}

	int Weapon::getBarrelNumber() const
	{
		return this->barrelNumber;
	}

	int Weapon::getBarrelRotateFromNumber() const
	{
		return this->barrelRotateFromNumber;
	}

	int Weapon::getBarrelRotateToNumber() const
	{
		return this->barrelRotateToNumber;
	}

	bool Weapon::doesFireByClick() const
	{
		return this->fireByClick;
	}

	float Weapon::getShootDirectionLimit() const
	{
		return shootDirectionLimit;
	}

	bool Weapon::getFireWaitReset() const
	{
		return fireWaitReset;
	}

	int Weapon::getBarrelEjectAngle(int i) const
	{
		if(i-1 < 0 || i-1 >= numBarrelEjectAngles)
		{
			return 0;
		}
		return barrelEjectAngles[i-1];
	}
}



#ifndef WEAPON_H
#define WEAPON_H

#include "Tool.h"
#include "Bullet.h"
#include "AmmoPack.h"

#define WEAPON_WALK_STOP_AT_PHASE_PREFIRE 0
#define WEAPON_WALK_STOP_AT_PHASE_FIRE 1
// NOTE: this last phase not yet supported
//#define WEAPON_WALK_STOP_AT_PHASE_RELOAD 2

namespace game
{

  class Weapon : public Tool
  {
  public:
    Weapon();
    Weapon(int id);
    virtual ~Weapon();

    virtual bool setData(char *key, char *value);

    virtual Part *getNewPartInstance();

    Bullet *getBulletType() const;
  
    AmmoPack *getAmmoType() const;

    const char *getFireSound() const;

    const char *getRepeatSound() const;

    const char *getPrepareFireSound() const;
 
    int getHeatGeneration() const;
  
    int getPowerUsage() const;
  
    int getAmmoUsage() const;
  
    int getMinAmmoForUsage() const;

    int getWalkStopTime() const;
  
    int getWalkStopAtPhase() const;
  
    int getFireReloadTime() const;

    int getFireWaitTime() const;
		bool getFireWaitReset() const;

    int getFireReloadVary() const;

    int getFireWaitVary() const;

		// NOTE: these should be named "charge" not "recharge"
    int getRechargeMinTime() const { return this->rechargeMinTime; }
    int getRechargeMaxTime() const { return this->rechargeMaxTime; }
    int getRechargePeakTime() const { return this->rechargePeakTime; }
    int getRechargeBulletSteps() const { return this->rechargeBulletSteps; }
    const char *getRechargeSound() const { return this->rechargeSound; }
    int getRechargeAmmoRate() const { return this->rechargeAmmoRate; }
    int getRechargeAmmoPeakHoldRate() const { return this->rechargeAmmoPeakHoldRate; }

    float getRange() const;

    int getAccuracy() const;

    int getLowAccuracy() const;

    int getInitialAmmoAmount() const;

    int getRaytraceAmount() const;

    int getRepeatAmount() const;

    int getRepeatDelay() const;

    int getRepeatSpread() const;

    bool isHeavyWeapon() const;

    bool isDropWeapon() const;

    bool isManualWeapon() const;

    bool isAntiVehicleWeapon() const;

    bool doesNeedRecon() const;

    int getKickback() const;

		int getClipSize() const;

		int getClipReloadTime() const;

		int getClipFirstReloadTime() const;

		const char *getClipReloadSound() const;

		const char *getClipFirstReloadSound() const;

		const char *getClipEmptySound() const;

		const char *getMuzzleflashEffect();

		float getMinSpread() const;

		float getMaxSpread() const;

		int getWeaponAnimationType() const;

    bool doesFireFromWeaponBarrel() const;
    bool doesRaytraceFromWeaponBarrel() const;

    bool isSingleShot() const;

    bool doesFiringRequireWalk() const;

    bool doesDelayAllWeapons() const;

    bool isThrowable() const;

    Weapon *getAttachedWeaponType() const;
		void setAttachedWeaponType(Weapon *w) { attachedWeaponType = w; }

		bool isSharedClipAttachment() const;

		bool isSingleReloading() const;

		int getBurstFireAmount() const;

		float getProjectileRange() const;

		bool isWeaponRayHeightFromBarrel() const;

		float getFlattenShootDirection() const;

		bool hasIndependentFireDelay() const;

		bool doesSweep() const;
		int getSweepAngle() const;

		int getContinuousFireTime() const;
		Bullet *getContinuousFireBulletType() const;

		bool hasNoAutoAim() const;

		int getBarrelNumber() const;
		int getBarrelRotateFromNumber() const;
		int getBarrelRotateToNumber() const;

    virtual void prepareNewForInherit(PartType *partType);
		virtual void saveOriginals();

		bool doesFireByClick() const;

		float getShootDirectionLimit() const;

		const char *getRechargeEffect() const;
		const char *getEjectEffect() const;
		const char *getClipreloadEffect() const;

		int getEjectRate() const;

		int getBarrelEjectAngle(int i) const;

		bool usesCustomTimeFactor(void) const { return useCustomTimeFactor; }

		bool isRemoteTrigger(void) const { return remoteTrigger; }

		const char *getRemoteTriggerSound() const { return remoteTriggerSound; }

		bool hasTargetLock(void) const { return targetLock; }
		int getTargetLockTime(void) const { return targetLockTime; }
		int getTargetLockReleaseTime(void) const { return targetLockReleaseTime; }
		int getTargetLockCancelTime(void) const { return targetLockCancelTime; }
		const char *getTargetLockSound(void) const { return targetLockSound; }


		const char *getPointerHelper() const { return pointerHelper; }
		const char *getPointerVisualEffect() const { return pointerVisualEffect; }
		const char *getPointerHitVisualEffect() const { return pointerHitVisualEffect; }

		int getCursor() const { return cursor; }
		int getReloadCursor() const { return reloadCursor; }

		bool allowsAttachedWeaponReload() const { return attachedWeaponReload; }

		// hack to delay secondary weapon but not attached weapon
		bool usesFireDelayHack() const { return fireDelayHack; }

		bool usesLaunchSpeed() const { return launchSpeed; }
		bool usesLaunchSpeedAnimation() const { return launchSpeed && launchSpeedAnimation; }
		float getLaunchSpeedMax() const { return launchSpeedMax; }
		float getLaunchSpeedAdd() const { return launchSpeedAdd; }

		bool usesCustomCamera() const { return customCamera; }
		float getCustomCameraAngle() const { return customCameraAngle; }
		float getCustomCameraZoom() const { return customCameraZoom; }

		bool isSelectableWithoutAmmo() const { return selectableWithoutAmmo; }

		int getAimEndDelay() const { return aimEndDelay; }

		int getCriticalHitDamageMax() const { return criticalHitDamageMax; }
		float getCriticalHitDamageMultiplier() const { return criticalHitDamageMultiplier; }
		float getCriticalHitProbabilityMultiplier() const { return criticalHitProbabilityMultiplier; }

		const char *getReloadFinishedSound() const { return reloadFinishedSound; }

		const char *getCustomWeaponBarrelHelper() const { return customWeaponBarrelHelper; }

	public:
		int *barrelEjectAngles;
		int numBarrelEjectAngles;

  protected:
    Bullet *bulletType;
    AmmoPack *ammoType;
    int ammoUsage;
		int minAmmoForUsage;
    int powerUsage;
    int heatGeneration;
    int fireWaitTime;
    int fireReloadTime;
    int fireWaitVary;
    int fireReloadVary;
    int walkStopTime;
    int walkStopAtPhase;
    float range;
    int accuracy;
    int lowAccuracy;
    int initialAmmo;
    char *fireSound;
    char *prepareFireSound;
    char *repeatSound;
    int raytraceAmount;
    int repeatAmount;
    int repeatDelay;
    int repeatSpread;
    bool heavyWeapon;
    bool dropWeapon;
    bool manualWeapon;
    bool antiVehicleWeapon;
		bool needsRecon;
		int kickback;
		int clipSize;
		int clipReloadTime;
		char *clipReloadSound;
		char *clipFirstReloadSound;
		char *clipEmptySound;
		char *muzzleflashEffect;
		char *muzzleflashEffect2;
		char *muzzleflashEffect3;
		float minSpread;
		float maxSpread;
		int weaponAnimationType;
		bool fireFromWeaponBarrel;
		bool raytraceFromWeaponBarrel;
		bool singleShot;
		bool firingRequireWalk;
		bool delayAllWeapons;
		bool throwable;
    Weapon *attachedWeaponType;
		bool attachedWeaponReload;
		bool sharedClipAttachment;
		float projectileRange;

		bool singleReloading;
		int burstFireAmount;

		bool weaponRayHeightFromBarrel;
		float flattenShootDirection;

		int lastMuzzleflashNumber;

		bool independentFireDelay;

		int clipFirstReloadTime;

		bool sweep;
		bool noAutoAim;

		int barrelNumber;
		int barrelRotateFromNumber;
    int barrelRotateToNumber;

    Bullet *continuousFireBulletType;
		int continuousFireTime;

		int sweepAngle; 

		int rechargeMinTime;
		int rechargePeakTime;
		int rechargeMaxTime;
		int rechargeBulletSteps;
		char *rechargeSound;
		int rechargeAmmoRate;
		int rechargeAmmoPeakHoldRate;

		bool fireByClick;

		float shootDirectionLimit;

		char *rechargeEffect;
		char *ejectEffect;
		char *clipreloadEffect;

		int ejectRate;
		// resets the wait time when the target moves out of field of fire
		bool fireWaitReset;

		bool useCustomTimeFactor;

		bool targetLock;
		int targetLockTime;
		int targetLockReleaseTime;
		int targetLockCancelTime;
		char *targetLockSound;

		bool remoteTrigger;
		char *remoteTriggerSound;

		char *pointerHelper;
		char *pointerVisualEffect;
		char *pointerHitVisualEffect;

		int cursor;
		int reloadCursor;

		// hack to delay secondary weapon but not attached weapon
		bool fireDelayHack;

		bool launchSpeed;
		bool launchSpeedAnimation;
		float launchSpeedMax;
		float launchSpeedAdd;

		bool customCamera;
		float customCameraAngle;
		float customCameraZoom;

		bool selectableWithoutAmmo;

		int aimEndDelay;

		int criticalHitDamageMax;
		float criticalHitDamageMultiplier;
		float criticalHitProbabilityMultiplier;

		char *reloadFinishedSound;
		char *customWeaponBarrelHelper;
  };

  //extern Weapon weapon;

}

#endif

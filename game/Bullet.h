
#ifndef BULLET_H
#define BULLET_H

#include "PartType.h"
#include "hitchaindefs.h"
#include "goretypedefs.h"

#include <DatatypeDef.h>


namespace game
{

	extern const char *hitChainName[HITCHAIN_AMOUNT];

	class Bullet : public PartType
	{
	public:
		enum FLYPATH
		{
			FLYPATH_RAY = 1,
			FLYPATH_DIRECT = 2,
			FLYPATH_GRAVITY = 3,
			FLYPATH_PARABOLIC = 4,
			FLYPATH_STATIC = 5,
			FLYPATH_GOTOTARGET = 6
		};

		enum TERRAIN_HOLE_TYPE
		{
			TERRAIN_HOLE_TYPE_CIRCLE = 1,
			TERRAIN_HOLE_TYPE_SPHERE = 2
			//TERRAIN_HOLE_TYPE_CRACK = 3
		};

		Bullet();
		Bullet(int id);
		virtual ~Bullet();

		virtual bool setSub(const char *key);
		// FIXME: making key const breaks stuff, WHY?
		virtual bool setData(char *key, char *value);

		FLYPATH getFlyPath();
		float getVelocity();

		int createDamageTo(Part *part, float factor);
		
		int getHeatTo(Part *part);

		int getVisualEffectNumber();

		int getDamageRange();

		int getPlayerDamageRange();

		int getHPDamage();

		int getLifeTime();

		int getAfterLifeTime();

		int getHitDamage(int damageType);

		/**
		 * Get the amount with which this bullet pushes a unit on impact.
		 * This value should be added to unit's velocity with proper
		 * direction (projectiles velocity direction or direction from
		 * an explosion's center).
		 * @return float, impact push amount in meters per game tick (10ms)
		 */
		float getImpactPush();

		Bullet *getChainBullet(int hitChainType);

		const char *getChainScript(int hitChainType);

		const char *getChainSound(int hitChainType, int soundNum);
		int getChainSoundAmount(int hitChainType);
		int getChainSoundProbability(int hitChainType);

		int getGoreProbability(int goreType) const;


		/**
		 * Get amount of terrain blending caused by this bullet when hitting
		 * the ground or hitting a unit near ground (within damagerange).
		 * @return int, the amount of blending caused by this bullet 
		 * (0 is no blending, 255 is maximum blend).
		 */
		int getTerrainBlendAmount();

		/**
		 * Get the maximum amount of terrain blending caused by this bullet.
		 */
		int getTerrainBlendMax();

		/**
		 * Get terrain blend radius caused by this bullet.
		 * @return float, hole radius in meters.
		 */
		float getTerrainBlendRadius();

		/**
		 * Get terrain hole depth caused by this bullet.
		 * @return float, hole depth in meters.
		 */
		float getTerrainHoleDepth();

		/**
		 * Get terrain hole radius caused by this bullet.
		 * @return float, hole radius in meters.
		 */
		float getTerrainHoleRadius();

		/**
		 * Check if hole has edges or not (crater or just plain hole)
		 * @return bool, true if edged hole (crater), false if not (just a hole)
		 */
		bool hasTerrainHoleEdges();

		/**
		 * Get the type of terrain hole caused by this bullet.
		 */
		TERRAIN_HOLE_TYPE getTerrainHoleType();

		// shotty :)
		bool isRadicalDistanceRatio();

		virtual void prepareNewForInherit(PartType *partType);
		virtual void saveOriginals();

		float getStartSkipAmount();

		bool isEndPadToHit();

		int getSplitRaytrace();

		int getSlowdown();

		int getChainSoundPriority(int chain) const;
		float getChainSoundRange(int chain) const;

		bool hasNoRotation() const;

		int getParabolicPathHeight() const;

		bool doesConnectToParent() const;
		bool doesParentToNextBullet() const;

		Bullet *getHitByProjectileBullet();

		bool doesPoisonDamage() const;

		Bullet *getDelayedHitProjectileBullet() const;
		int getDelayedHitProjectileInterval() const;
		int getDelayedHitProjectileAmount() const;
		bool hasNoDelayedHitProjectileForPlayer() const;

		bool hasNoDifficultyEffectOnDamageAmount() const;
		bool doesNoTerrainObjectBreaking() const;

		float getProximityRange() const { return this->proximityRange; }
		int getProximityCheckRate() const { return this->proximityCheckRate; }
		// should be named "dropped" life time (not drop lifetime)
		int getProximityDropLifeTime() const { return this->proximityDropLifeTime; }

		bool doesNoSelfDamage() const { return this->noSelfDamage; }

		bool doesNoPlayerSlowdown() const { return this->noPlayerSlowdown; }

		float getFluidPushRadius() const { return this->fluidPushRadius; }
		int getFluidPushTime() const { return this->fluidPushTime; }

		float getPhysicsImpulseRadius() const { return this->physicsImpulseRadius; }
		bool doesOverridePhysicsImpulseRadius() const { return this->overridePhysicsImpulseRadius; }
		float getPhysicsImpulseFactor() const { return this->physicsImpulseFactor; }

		int getTerrainObjectRadiusDamageAmount() const { return this->terrainObjectRadiusDamageAmount; }
		int getTerrainObjectDirectDamageAmount() const { return this->terrainObjectDirectDamageAmount; }
		int getTerrainObjectDamageProbability() const { return this->terrainObjectDamageProbability; }

		VC3 getPathGravity() const { return this->pathGravity; }

		int getAccuracyChange() const { return this->accuracyChange; }

		float getCriticalHitPercent() const { return this->criticalHitPercent; }
		bool allowCriticalHits() const { return this->criticalHitAllowed; }

		unsigned int getForcewearEffect() const { return this->forcewearEffect; }

		bool isRemoteExplosive() const { return this->remoteExplosive; }
		bool isSticky() const { return this->sticky; }

		float getPlayerDamageFactor() const { return this->playerDamageFactor; }

	protected:
		int createDamageToImpl(Part *part, int *damages, float factor);

		int hitDamage[DAMAGE_TYPES_AMOUNT];
		int hpDamage;
		float velocity;
		int damageRange;
		int playerDamageRange;
		enum FLYPATH flyPath;
		float impactPush;

		int terrainBlendAmount;
		int terrainBlendMax;
		float terrainBlendRadius;
		float terrainHoleDepth;
		float terrainHoleRadius;
		bool terrainHoleEdges;
		enum TERRAIN_HOLE_TYPE terrainHoleType;

		int visualEffect;

		int atChain; // for parsing

		int lifeTime;
		int afterLifeTime;

		int splitRaytrace;

		// skip distance at origin and possibly "padding" near end to 
		// hit the target exactly
		float startSkipAmount;
		bool endPadToHit;

		bool radicalDistanceRatio;

		bool chainDefined[HITCHAIN_AMOUNT];
		int hitSoundProbability[HITCHAIN_AMOUNT];
		char *chainSound[HITCHAIN_AMOUNT][HITCHAIN_SOUNDS];
		Bullet *chainBullet[HITCHAIN_AMOUNT];
		char *chainScript[HITCHAIN_AMOUNT];
		int chainSoundPriority[HITCHAIN_AMOUNT];
		float chainSoundRange[HITCHAIN_AMOUNT];

		int goreProbability[GORETYPE_AMOUNT];

		int slowdown;

		bool noRotation;

		int parabolicPathHeight;

		bool connectToParent;
		bool parentToNextBullet;

		bool poisonDamage;

		Bullet *hitByProjectileBullet;

    Bullet *delayedHitProjectileBullet;
    int delayedHitProjectileInterval;
    int delayedHitProjectileAmount;
		bool noDelayedHitProjectileForPlayer;

		bool noDifficultyEffectOnDamageAmount;
		bool noTerrainObjectBreaking;

		float proximityRange;
		int proximityCheckRate;
		int proximityDropLifeTime;

		bool noSelfDamage;
		bool noPlayerSlowdown;

		float fluidPushRadius;
		int fluidPushTime;

		float physicsImpulseRadius;
		bool overridePhysicsImpulseRadius;
		float physicsImpulseFactor;

		int terrainObjectDirectDamageAmount;
		int terrainObjectRadiusDamageAmount;
		int terrainObjectDamageProbability;

		int forcewearEffect;

		int accuracyChange;

		VC3 pathGravity;

		float criticalHitPercent;
		bool criticalHitAllowed;

		bool remoteExplosive;
		bool sticky;

		float playerDamageFactor;
	};

}

#endif

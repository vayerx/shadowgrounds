
#include "precompiled.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "Bullet.h"

#include "gamedefs.h"
#include "goretypedefs.h"
#include "../convert/str2int.h"
//#include "projectilevisdefs.h"
//#include "hitscriptdefs.h"
#include "Part.h"
#include "../ui/VisualEffectManager.h"
#include "../util/Debug_MemoryManager.h"
#include "../system/Logger.h"
#include "../sound/sounddefs.h"
#include "Forcewear.h"
#include <string>
#include <boost/lexical_cast.hpp>


#define BULLET_SLOTS 0

// must not overlap subs defined in PartType
#define PARTTYPE_SUB_HIT 1368
#define PARTTYPE_SUB_CHAIN 2817



namespace game
{

#ifdef PROJECT_SHADOWGROUNDS
	const char *hitChainName[HITCHAIN_AMOUNT] =
	{
		"nothing",
		"unit",
		"ground",
		"building",
		"terrainobject",
		"indirect",
		"metallic",
		"material_sand",
		"material_rock",
		"material_concrete",
		"material_metal_hard",
		"material_metal_tin",
		"material_glass",
		"material_wood",
		"material_metal_grate",
		"unit_type2",
		"indirect_type2",
		"indirect_metallic"
	};
#else
	const char *hitChainName[HITCHAIN_AMOUNT] =
	{
		"nothing",
		"unit",
		"ground",
		"building",
		"terrainobject",
		"indirect",
		"metallic",

		"material_sand",
		"material_rock",
		"material_concrete",
		"material_metal_hard",
		"material_metal_tin",
		"material_wood",
		"material_glass",
		"material_plastic",
		"material_leaves",
		"material_liquid",
		"material_metal_grate",
		"material_soil",
		"material_grass",
		"material_snow_shallow",
		"material_snow_deep",
		"material_reserved_15",

		"unit_type2",
		"indirect_type2",
		"indirect_metallic"
	};
#endif


	Bullet::Bullet()
	{
		image = NULL;
		parentType = &partType;
		slotAmount = BULLET_SLOTS;
		slotTypes = NULL;
		slotPositions = NULL;
		maxDamage = 0;
		maxHeat = 0;
		for (int dmg = 0; dmg < DAMAGE_TYPES_AMOUNT; dmg++)
		{
			resistance[dmg] = 0;
			damagePass[dmg] = 0;
			damageAbsorb[dmg] = 0;
		}
		for (int dmg2 = 0; dmg2 < DAMAGE_TYPES_AMOUNT; dmg2++)
		{
			hitDamage[dmg2] = 0;
		}
		hpDamage = 0;
		damageRange = 0;
		playerDamageRange = -1;
		flyPath = FLYPATH_STATIC;
		velocity = 0;
		impactPush = 0.0f;
		lifeTime = 1;
		afterLifeTime = 0;
		terrainBlendAmount = 0;
		terrainBlendMax = 0;
		terrainHoleDepth = 0.0f;
		terrainHoleRadius = 0.0f;
		terrainBlendRadius = 0.0f;
		terrainHoleEdges = false;
		visualEffect = 0; // PROJECTILE_VIS_NONE;

		radicalDistanceRatio = false;

		for (int i = 0; i < HITCHAIN_AMOUNT; i++)
		{
			chainDefined[i] = false;
			chainBullet[i] = NULL;
			chainScript[i] = NULL;
			chainSoundPriority[i] = DEFAULT_SOUND_PRIORITY_NORMAL;
			chainSoundRange[i] = DEFAULT_SOUND_RANGE;

			for (int snum = 0; snum < HITCHAIN_SOUNDS; snum++)
			{
				chainSound[i][snum] = NULL;
			}
			//chainScript[i] = NULL;
			hitSoundProbability[i] = 100;
		}
#if defined(PROJECT_SHADOWGROUNDS) || defined(PROJECT_SURVIVOR) 
		// default start skip 1.5 meters
		startSkipAmount = 1.5f;
#else
		startSkipAmount = 0.0f;
#endif
		// default do NOT pad last position to hit exactly
		endPadToHit = false;

		splitRaytrace = 1;
		slowdown = 0;

		noSelfDamage = false;

		for (int gor = 0; gor < GORETYPE_AMOUNT; gor++)
		{
			goreProbability[gor] = 0;
		}

		noRotation = false;

		parabolicPathHeight = 1;

		connectToParent = false;
		parentToNextBullet = false;

		hitByProjectileBullet = NULL;

		poisonDamage = false;

		delayedHitProjectileBullet = NULL;
		delayedHitProjectileInterval = 0;
		delayedHitProjectileAmount = 0;
		noDelayedHitProjectileForPlayer = false;

		noDifficultyEffectOnDamageAmount = false;
		noTerrainObjectBreaking = false;

		proximityRange = 0.0f;
		proximityCheckRate = 4;
		proximityDropLifeTime = 1;

		noPlayerSlowdown = false;

		fluidPushRadius = 0.0f;
		fluidPushTime = 0;

		physicsImpulseRadius = 0.0f;
		overridePhysicsImpulseRadius = false;
		physicsImpulseFactor = 1.0f;

		terrainObjectRadiusDamageAmount = 15;
		terrainObjectDirectDamageAmount = 1;
		terrainObjectDamageProbability = 100;

		forcewearEffect = 0;

		accuracyChange = 0;

		pathGravity = VC3(0,0,0);

		criticalHitPercent = -1.0f;
		criticalHitAllowed = true;
		remoteExplosive = false;
		sticky = false;
		playerDamageFactor = 1.0f;


		// NOTICE!!!
		// any variable added needs to be handled below (inherit method)
	}


	/*
	Bullet::Bullet(int id)
	{
		parentType = &partType;
		setPartTypeId(id);
	}
	*/

	void Bullet::prepareNewForInherit(PartType *partType)
	{
		this->PartType::prepareNewForInherit(partType);

		// FIXME: this one leaks memory if save/restoreOriginals is used!

		// WARNING!
		// TODO: should really check that the given parameter is of class!
		Bullet *ret = (Bullet *)partType;

		int i;
		for (i = 0; i < DAMAGE_TYPES_AMOUNT; i++)
		{
			ret->hitDamage[i] = hitDamage[i];
		}
		ret->hpDamage = hpDamage;
		ret->velocity = velocity;
		ret->damageRange = damageRange;
		ret->playerDamageRange = playerDamageRange;
		ret->flyPath = flyPath;
		ret->impactPush = impactPush;

		ret->terrainBlendAmount = terrainBlendAmount;
		ret->terrainBlendMax = terrainBlendMax;
		ret->terrainBlendRadius = terrainBlendRadius;
		ret->terrainHoleDepth = terrainHoleDepth;
		ret->terrainHoleRadius = terrainHoleRadius;
		ret->terrainHoleEdges = terrainHoleEdges;
		ret->terrainHoleType = terrainHoleType;

		ret->visualEffect = visualEffect;

		ret->terrainObjectRadiusDamageAmount = terrainObjectRadiusDamageAmount;
		ret->terrainObjectDirectDamageAmount = terrainObjectDirectDamageAmount;
		ret->terrainObjectDamageProbability = terrainObjectDamageProbability;

		ret->lifeTime = lifeTime;
		ret->afterLifeTime = afterLifeTime;

		ret->radicalDistanceRatio = radicalDistanceRatio;

		for (i = 0; i < HITCHAIN_AMOUNT; i++)
		{
			ret->chainDefined[i] = chainDefined[i];
			ret->hitSoundProbability[i] = hitSoundProbability[i];
			ret->chainBullet[i] = chainBullet[i];
			ret->chainSoundPriority[i] = chainSoundPriority[i];
			ret->chainSoundRange[i] = chainSoundRange[i];
			for (int j = 0; j < HITCHAIN_SOUNDS; j++)
			{
				if (chainSound[i][j] != NULL)
				{
					delete[] ret->chainSound[i][j];
					ret->chainSound[i][j] = new char[strlen(chainSound[i][j]) + 1];
					strcpy(ret->chainSound[i][j], chainSound[i][j]);
				} else {
					ret->chainSound[i][j] = NULL;
				}
			}

			/*
			ret->chainScript[i] = chainScript[i];
			*/
			if (chainScript[i] != NULL)
			{
				delete[] ret->chainScript[i];
				ret->chainScript[i] = new char[strlen(chainScript[i]) + 1];
				strcpy(ret->chainScript[i], chainScript[i]);
			} else {
				ret->chainScript[i] = NULL;
			}
		}

		ret->startSkipAmount = startSkipAmount;
		ret->endPadToHit = endPadToHit;

		ret->splitRaytrace = splitRaytrace;
		ret->slowdown = slowdown;

		for (int gor = 0; gor < GORETYPE_AMOUNT; gor++)
		{
			ret->goreProbability[gor] = goreProbability[gor];
		}

		ret->noRotation = noRotation;
		ret->parabolicPathHeight = parabolicPathHeight;

		ret->connectToParent = connectToParent;
		ret->parentToNextBullet = parentToNextBullet;

		ret->poisonDamage = poisonDamage;

		ret->delayedHitProjectileBullet = delayedHitProjectileBullet;
		ret->delayedHitProjectileInterval = delayedHitProjectileInterval;
		ret->delayedHitProjectileAmount = delayedHitProjectileAmount;
		ret->noDelayedHitProjectileForPlayer = noDelayedHitProjectileForPlayer;

		ret->noDifficultyEffectOnDamageAmount = noDifficultyEffectOnDamageAmount;
		ret->noTerrainObjectBreaking = noTerrainObjectBreaking;

		ret->proximityRange = proximityRange;
		ret->proximityCheckRate = proximityCheckRate;
		ret->proximityDropLifeTime = proximityDropLifeTime;

		ret->noSelfDamage = noSelfDamage;

		ret->noPlayerSlowdown = noPlayerSlowdown;

		ret->fluidPushRadius = fluidPushRadius;
		ret->fluidPushTime = fluidPushTime;

		ret->physicsImpulseRadius = physicsImpulseRadius;
		ret->overridePhysicsImpulseRadius = overridePhysicsImpulseRadius;
		ret->physicsImpulseFactor = physicsImpulseFactor;

		ret->pathGravity = pathGravity;

		ret->accuracyChange = accuracyChange;
		ret->criticalHitPercent = criticalHitPercent;
		ret->criticalHitAllowed = criticalHitAllowed;
		ret->remoteExplosive = remoteExplosive;
		ret->sticky = sticky;
		ret->playerDamageFactor = playerDamageFactor;
	}

	void Bullet::saveOriginals()
	{
		if (originals == NULL)
		{
			originals = new Bullet();
		} else {
			assert(!"Bullet::saveOriginals - Attempt to save originals multiple times.");
		}

		// FIXME: this may not be correct way to do this!
		this->prepareNewForInherit(originals);
	}


	Bullet::~Bullet()
	{
		// TODO!!

		// (..probably more than just this cleanup missing..)
		for (int i = 0; i < HITCHAIN_AMOUNT; i++)
		{
			for (int j = 0; j < HITCHAIN_SOUNDS; j++)
			{
				if (chainSound[i][j] != NULL)
				{
					delete [] chainSound[i][j];
					chainSound[i][j] = NULL;
				}
			}
			if (chainScript[i] != NULL)
			{
				delete [] chainScript[i];
				chainScript[i] = NULL;
			}
		}
	}

	/*
	Part *Bullet::getNewPartInstance()
	{
		BulletObject *ret = new BulletObject();
		ret->setType(this);
		return ret;
	} 
	*/

	bool Bullet::setSub(const char *key)
	{
		if (key != NULL && strcmp(key, "hit") == 0)
		{
			atSub = PARTTYPE_SUB_HIT;
			return true;
		}
		if (key != NULL && strcmp(key, "chain") == 0)
		{
			atSub = PARTTYPE_SUB_CHAIN;
			atChain = -1;
			return true;
		}
		return setRootSub(key);
	}

	bool Bullet::setData(char *key, char *value)
	{

		// Delete possible whitespaces/tabs from front of the key.
		while( key[0] == ' ' || key[0] == 9 )
			key++;

		if (atSub == PARTTYPE_SUB_HIT)
		{
			if (strcmp(key, "damagerange") == 0)
			{ 			 
				damageRange = str2int(value);
				return true;
			}
			if (strcmp(key, "playerdamagerange") == 0)
			{ 			 
				playerDamageRange = str2int(value);
				return true;
			}
			if(strcmp(key, "playerdamagefactor") == 0)
			{
				playerDamageFactor = (float)atof(value);
				return true;
			}
			if (strcmp(key, "push") == 0)
			{
				// convert from m/s to something else... :)
				impactPush = (float)str2int(value) / 100.0f;
				return true;
			}
			if (strcmp(key, "hp") == 0)
			{
				hpDamage = str2int(value);
				return true;
			}
			if (strcmp(key, "noselfdamage") == 0)
			{
				if (str2int(value) == 1)
				{
					noSelfDamage = true;
					//Logger::getInstance()->warning("Bullet::setData - noselfdamage used, but not properly implemented.");
					// done?
				} else {
					noSelfDamage = false;
				}
				return true;
			}
			if (strcmp(key, "noplayerslowdown") == 0)
			{
				if (str2int(value) == 1)
				{
					noPlayerSlowdown = true;
				} else {
					noPlayerSlowdown = false;
				}
				return true;
			}
			if (strcmp(key, "no_terrain_object_breaking") == 0)
			{
				if (str2int(value) == 1)
					noTerrainObjectBreaking = true;
				else
					noTerrainObjectBreaking = false;
				return true;
			}
			if (strcmp(key, "delayedhitprojectileinterval") == 0)
			{
				delayedHitProjectileInterval = str2int(value);
				return true;
			}
			if (strcmp(key, "delayedhitprojectileamount") == 0)
			{
				delayedHitProjectileAmount = str2int(value);
				return true;
			}
			if (strcmp(key, "delayedhitprojectilebullet") == 0)
			{
				if (!PARTTYPE_ID_STRING_VALID(value)) return false;

				int val = PARTTYPE_ID_STRING_TO_INT(value);
				PartType *b = getPartTypeById(val);
				if (b == NULL) return false;
				if (b == this) return false; // doh?

				// make sure it's a bullet
				if (!b->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull")))) 
					return false;

				// WARNING: unsafe cast (based on check above)
				delayedHitProjectileBullet = (Bullet *)b;

				return true;
			}
			if (strcmp(key, "nodelayedhitprojectileforplayer") == 0)
			{
				if (str2int(value) == 1)
					noDelayedHitProjectileForPlayer = true;
				else
					noDelayedHitProjectileForPlayer = false;
				return true;
			}
			if (strcmp(key, "poisondamage") == 0)
			{
				if (str2int(value) == 1)
					poisonDamage = true;
				else
					poisonDamage = false;
				return true;
			}
			if (strcmp(key, "slowdown") == 0)
			{
				slowdown = str2int(value);
				return true;
			}
			if (strcmp(key, "radicaldistanceratio") == 0)
			{
				if (str2int(value) == 1)
					radicalDistanceRatio = true;
				else
					radicalDistanceRatio = false;
				return true;
			}
			if (strcmp(key, "terrainblendmax") == 0)
			{ 			 
				terrainBlendMax = str2int(value);
				return true;
			}
			if (strcmp(key, "terrainblendamount") == 0)
			{ 			 
				terrainBlendAmount = str2int(value);
				return true;
			}
			if (strcmp(key, "terrainblendradius") == 0)
			{ 			 
				terrainBlendRadius = (float)atof(value);
				return true;
			}
			if (strcmp(key, "terrain_object_radius_damage_amount") == 0)
			{ 			 
				terrainObjectRadiusDamageAmount = str2int(value);
				return true;
			}
			if (strcmp(key, "terrain_object_direct_damage_amount") == 0)
			{ 			 
				terrainObjectDirectDamageAmount = str2int(value);
				return true;
			}
			if (strcmp(key, "terrain_object_damage_probability") == 0)
			{ 			 
				terrainObjectDamageProbability = str2int(value);
				return true;
			}

			for (int gor = 0; gor < GORETYPE_AMOUNT; gor++)
			{
				char foobuf[128]; 
				assert(strlen(goreTypeName[gor]) < 100);
				strcpy(foobuf, "goreprobability_");
				strcat(foobuf, goreTypeName[gor]);
				if (strcmp(key, foobuf) == 0)
				{
					goreProbability[gor] = str2int(value);
					return true;
				}
			}

			if (strcmp(key, "terrainholeedges") == 0)
			{ 			 
				if (str2int(value) == 1)
					terrainHoleEdges = true;
				else
					terrainHoleEdges = false;
				return true;
			}
			if (strcmp(key, "terrainholedepth") == 0)
			{ 			 
				terrainHoleDepth = (float)atof(value);
				return true;
			}
			if (strcmp(key, "terrainholeradius") == 0)
			{ 			 
				terrainHoleRadius = (float)atof(value);
				return true;
			}
			if (strcmp(key, "terrainholetype") == 0)
			{ 			 
				if (strcmp(value, "circle") == 0)
				{
					terrainHoleType = TERRAIN_HOLE_TYPE_CIRCLE;
					return true;
				}
				if (strcmp(value, "sphere") == 0)
				{
					terrainHoleType = TERRAIN_HOLE_TYPE_SPHERE;
					return true;
				}
				return false;
			}
			if (strcmp(key, "physicsimpulseradius") == 0)
			{
				overridePhysicsImpulseRadius = true;
				physicsImpulseRadius = (float)atof(value);
				if (physicsImpulseRadius < 0.0f)
				{
					physicsImpulseRadius = 0.0f;
					return false;
				}
				return true;
			}
			if (strcmp(key, "physicsimpulsefactor") == 0)
			{
				physicsImpulseFactor = (float)atof(value);
				return true;
			}

			int isdam = DAMAGE_TYPE_INVALID;
			if (strcmp(key, "projectile") == 0)
			{
				isdam = DAMAGE_TYPE_PROJECTILE;
			}
			if (strcmp(key, "heat") == 0)
			{
				isdam = DAMAGE_TYPE_HEAT;
			}
			if (strcmp(key, "electric") == 0)
			{
				isdam = DAMAGE_TYPE_ELECTRIC;
			}
			if (strcmp(key, "stun") == 0)
			{
				isdam = DAMAGE_TYPE_STUN;
			}
			int damval = str2int(value);
			if (isdam != DAMAGE_TYPE_INVALID)
			{
				hitDamage[isdam] = damval;
				return true;
			}
			return false;
		}
		if (atSub == PARTTYPE_SUB_CHAIN)
		{
			if (strcmp(key, "hittype") == 0)
			{
				for (int i = 0; i < HITCHAIN_AMOUNT; i++)
				{
					if (strcmp(hitChainName[i], value) == 0)
					{
						atChain = i;
						chainDefined[atChain] = true;
						return true;
					}
				}
				return false;
			}
			if (strncmp(key, "hitsound", 8) == 0
				&& ((key[8] >= '1' && key[8] <= '9')
				|| key[8] == '\0'))
			{
				if (atChain == -1)
					return false;

				int snum = key[8] - '1';
				if (key[8] == '\0') snum = 0;
				if (snum < 0 || snum >= HITCHAIN_SOUNDS)
					return false;

				if (chainSound[atChain][snum] != NULL) delete [] chainSound[atChain][snum];
				chainSound[atChain][snum] = new char[strlen(value) + 1];
				strcpy(chainSound[atChain][snum], value);
				return true;
			}
			if (strcmp(key, "hitsoundprobability") == 0)
			{
				if (atChain == -1)
					return false;

				hitSoundProbability[atChain] = str2int(value);
				return true;
			}
			if (strcmp(key, "hitscript") == 0)
			{
				if (atChain == -1)
					return false;

				if (value == NULL || strcmp(value, "") == 0
					|| strcmp(value, "none") == 0 || strcmp(value, "null") == 0)
				{
					chainScript[atChain] = NULL;
					return true;
				} else {
					if (chainScript[atChain] != NULL) delete [] chainScript[atChain];
					chainScript[atChain] = new char[strlen(value) + 1];
					strcpy(chainScript[atChain], value);
					return true;
				}
				/*
				for (int i = 0; i < HITSCRIPT_AMOUNT; i++)
				{
					if (strcmp(hitScriptName[i], value) == 0)
					{
						chainScript[atChain] = i;
						return true;
					}
				}
				*/
			}
			if (strcmp(key, "hitbullet") == 0)
			{
				if (atChain == -1)
					return false;

				if (value[0] == '\0' || strcmp(value, "none") == 0)
				{
					chainBullet[atChain] = NULL;
					return true;
				}

				if (!PARTTYPE_ID_STRING_VALID(value)) return false;

				int val = PARTTYPE_ID_STRING_TO_INT(value);
				PartType *cbullet = getPartTypeById(val);
				if (cbullet == NULL) return false;
//				if (cbullet == this) return false; // doh?

				// make sure it's a bullet
				if (!cbullet->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull")))) 
					return false;

				chainBullet[atChain] = (Bullet *)cbullet;
				return true;
			}

			// psd
			if (strcmp(key, "hitsoundpriority") == 0)
			{
				if (atChain == -1 || !value)
					return false;

				if(strcmp(value, "high") == 0)
					chainSoundPriority[atChain] = DEFAULT_SOUND_PRIORITY_HIGH;
				if(strcmp(value, "normal") == 0)
					chainSoundPriority[atChain] = DEFAULT_SOUND_PRIORITY_NORMAL;
				if(strcmp(value, "low") == 0)
					chainSoundPriority[atChain] = DEFAULT_SOUND_PRIORITY_LOW;

				return true;
			}
			if (strcmp(key, "hitsoundrange") == 0)
			{
				if (atChain == -1 || !value)
					return false;

				try
				{
					chainSoundRange[atChain] = boost::lexical_cast<float> (std::string(value));
				}
				catch(...)
				{
					return false;
				}

				return true;
			}


			return false;
		}
		if (atSub == PARTTYPE_SUB_NONE)
		{
			if (strcmp(key, "lifetime") == 0)
			{
#ifndef PROJECT_SHADOWGROUNDS
				Logger::getInstance()->warning("Bullet::setData - Use of \"lifetime\" key deprecated (use lifetimemsec or lifetimeticks instead).");
#endif
				lifeTime = str2int(value);
				return true;
			}
			if (strcmp(key, "afterlifetime") == 0)
			{
#ifndef PROJECT_SHADOWGROUNDS
				Logger::getInstance()->warning("Bullet::setData - Use of \"afterlifetime\" key deprecated (use afterlifetimemsec or afterlifetimeticks instead).");
#endif
				afterLifeTime = str2int(value);
				return true;
			}
			if (strcmp(key, "lifetimeticks") == 0)
			{
				lifeTime = str2int(value);
				return true;
			}
			if (strcmp(key, "afterlifetimeticks") == 0)
			{
				afterLifeTime = str2int(value);
				return true;
			}
			if (strcmp(key, "lifetimemsec") == 0)
			{
				lifeTime = str2int(value) / GAME_TICK_MSEC;
				return true;
			}
			if (strcmp(key, "afterlifetimemsec") == 0)
			{
				afterLifeTime = str2int(value) / GAME_TICK_MSEC;
				return true;
			}
			if (strcmp(key, "parabolicpathheight") == 0)
			{
				parabolicPathHeight = str2int(value);
				return true;
			}
			if (strcmp(key, "norotation") == 0)
			{
				if (str2int(value) == 1)
					noRotation = true;
				else
					noRotation = false;
				return true;
			}
			if (strcmp(key, "no_difficulty_effect_on_damage_amount") == 0)
			{
				if (str2int(value) == 1)
					noDifficultyEffectOnDamageAmount = true;
				else
					noDifficultyEffectOnDamageAmount = false;
				return true;
			}
			if (strcmp(key, "accuracychange") == 0)
			{
				accuracyChange = str2int(value);
				return true;
			}
			if (strcmp(key, "parenttonextbullet") == 0)
			{
				if (str2int(value) == 1)
					parentToNextBullet = true;
				else
					parentToNextBullet = false;
				return true;
			}
			if (strcmp(key, "connecttoparent") == 0)
			{
				if (str2int(value) == 1)
					connectToParent = true;
				else
					connectToParent = false;
				return true;
			}
			if (strcmp(key, "splitraytrace") == 0)
			{
				splitRaytrace = str2int(value);
				return true;
			}
			if (strcmp(key, "pathtype") == 0)
			{
				if (strcmp(value, "ray") == 0)
				{
					flyPath = FLYPATH_RAY;
					return true;
				}
				if (strcmp(value, "direct") == 0)
				{
					flyPath = FLYPATH_DIRECT;
					return true;
				}
				if (strcmp(value, "gravity") == 0)
				{
					flyPath = FLYPATH_GRAVITY;
					return true;
				}
				if (strcmp(value, "parabolic") == 0)
				{
					flyPath = FLYPATH_PARABOLIC;
					return true;
				}
				if (strcmp(value, "static") == 0)
				{
					flyPath = FLYPATH_STATIC;
					return true;
				}
				if( strcmp( value, "gototarget" ) == 0 )
				{
					flyPath = FLYPATH_GOTOTARGET;
					return true;
				}
				return false;
			}
			if (strcmp(key, "pathgravityx") == 0)
			{
				pathGravity.x = (float)atof(value);
				return true;
			}
			if (strcmp(key, "pathgravityy") == 0)
			{
				pathGravity.y = (float)atof(value);
				return true;
			}
			if (strcmp(key, "pathgravityz") == 0)
			{
				pathGravity.z = (float)atof(value);
				return true;
			}
			if (strcmp(key, "endpadtohit") == 0)
			{ 			 
				if (str2int(value) == 1)
					endPadToHit = true;
				else
					endPadToHit = false;
				return true;
			}
			if (strcmp(key, "startskipamount") == 0)
			{ 			 
				startSkipAmount = (float)atof(value);
				return true;
			}
			if (strcmp(key, "proximityrange") == 0)
			{ 			 
				proximityRange = (float)atof(value);
				return true;
			}
			if (strcmp(key, "proximitycheckrate") == 0)
			{ 			 
				proximityCheckRate = str2int(value);
				// requires a 2^n value.
				if ((proximityCheckRate & (proximityCheckRate-1)) != 0)
				{
					return false;
				}
				return true;
			}
			if (strcmp(key, "proximitydroplifetime") == 0)
			{
#ifndef PROJECT_SHADOWGROUNDS
				Logger::getInstance()->warning("Bullet::setData - Use of \"proximitydroplifetime\" key deprecated (use proximitydroplifetimeticks instead).");
#endif
				proximityDropLifeTime = str2int(value);
				if (proximityDropLifeTime < 1)
				{
					proximityDropLifeTime = 1;
					return false;
				}
				return true;
			}
			if (strcmp(key, "proximitydroplifetimeticks") == 0)
			{
				proximityDropLifeTime = str2int(value);
				if (proximityDropLifeTime < 1)
				{
					proximityDropLifeTime = 1;
					return false;
				}
				return true;
			}
			if (strcmp(key, "velocity") == 0)
			{
				// convert from m/s to m/10ms (game ticks)
				velocity = (float)str2int(value) / GAME_TICKS_PER_SECOND;
				return true;
			}
			if (strcmp(key, "visualeffect") == 0)
			{
				if (value != NULL && strcmp(value, "none") == 0)
				{
					visualEffect = 0;
					return true;
				} else {
					visualEffect = ui::VisualEffectManager::getVisualEffectIdByName(value);
					if (visualEffect != -1)
						return true;
					/*
					for (int i = 0; i < PROJECTILE_VIS_AMOUNT; i++)
					{
						if (strcmp(projectileVisualName[i], value) == 0)
						{
							visualEffect = i;
							return true;
						}
					}
					*/
					Logger::getInstance()->warning("Bullet::setData - Visualeffect with given name not found.");
					visualEffect = 0;
					return false;
				}
			}
			if (strcmp(key, "hitbyprojectilebullet") == 0)
			{
				if (value[0] == '\0' || strcmp(value, "none") == 0)
				{
					hitByProjectileBullet = NULL;
					return true;
				}

				if (!PARTTYPE_ID_STRING_VALID(value)) return false;

				int val = PARTTYPE_ID_STRING_TO_INT(value);
				PartType *cbullet = getPartTypeById(val);
				if (cbullet == NULL) return false;
				if (cbullet == this) return false;

				// make sure it's a bullet
				if (!cbullet->isInherited(getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull")))) 
					return false;

				hitByProjectileBullet = (Bullet *)cbullet;
				return true;
			}
			if (strcmp(key, "fluidpushtime") == 0)
			{
				fluidPushTime = str2int(value);
				if (fluidPushTime < 0)
				{
					fluidPushTime = 0;
					return false;
				}
				return true;
			}
			if (strcmp(key, "fluidpushradius") == 0)
			{
				fluidPushRadius = (float)atof(value);
				if (fluidPushRadius < 0.0f)
				{
					fluidPushRadius = 0.0f;
					return false;
				}
				return true;
			}
			if (strcmp(key, "criticalhitpercent") == 0)
			{
				criticalHitPercent = (float)atof(value);
				return true;
			}
			if (strcmp(key, "criticalhitallowed") == 0)
			{
				criticalHitAllowed = str2int(value) == 0 ? false : true;
				return true;
			}
			if(strcmp(key, "forceweareffect") == 0 )
			{
				forcewearEffect = Forcewear::stringToType(value);
				return true;
			}
			if(strcmp(key, "remoteexplosive") == 0)
			{
				remoteExplosive = str2int(value) == 0 ? false : true;
				return true;
			}
			if(strcmp(key, "sticky") == 0)
			{
				sticky = str2int(value) == 0 ? false : true;
				return true;
			}
		}
		return setRootData(key, value);
	}

	Bullet::FLYPATH Bullet::getFlyPath()
	{
		return flyPath;
	}
 
	float Bullet::getVelocity()
	{
		return velocity;
	}

	int Bullet::createDamageTo(Part *part, float factor)
	{
		return createDamageToImpl(part, hitDamage, factor);
	}

	int Bullet::createDamageToImpl(Part *part, int *damages, float factor)
	{
		PartType *partType = part->getType();
		int doDamage[DAMAGE_TYPES_AMOUNT];
		int doPass[DAMAGE_TYPES_AMOUNT];
		int finalDamage = 0;
		int finalHeat = 0;
		int i;
		for (i = 0; i < DAMAGE_TYPES_AMOUNT; i++)
		{
			doDamage[i] = damages[i];
			doPass[i] = 0;
		}
		for (i = 0; i < DAMAGE_TYPES_AMOUNT; i++)
		{
			doDamage[i] -= partType->getResistance(i);
			if (doDamage[i] < 0) doDamage[i] = 0;
			doPass[i] += (doDamage[i] * partType->getDamagePass(i)) / 100;
			if (i == DAMAGE_TYPE_HEAT)
			{
				finalHeat += (doDamage[i] * partType->getDamageAbsorb(i)) / 100;
			} else {
				finalDamage += (doDamage[i] * partType->getDamageAbsorb(i)) / 100;
			}
		}
		part->addDamage((int)((float)finalDamage * factor));

		// pass damage on to sub-parts with doPass values...
		int slotAmount = part->getType()->getSlotAmount(); 
		for (i = 0; i < slotAmount; i++)
		{
			if (part->getSubPart(i) != NULL)
				finalHeat += createDamageToImpl(part->getSubPart(i), doPass, factor);
		}

		return finalHeat;
	}

	
	int Bullet::getVisualEffectNumber()
	{
		return visualEffect;
	}

	float Bullet::getImpactPush()
	{
		return impactPush;
	}

	int Bullet::getDamageRange()
	{
		return damageRange;
	}

	int Bullet::getPlayerDamageRange()
	{
		return playerDamageRange;
	}

	int Bullet::getHPDamage()
	{
		return hpDamage;
	}

	Bullet *Bullet::getChainBullet(int hitChainType)
	{
		assert(hitChainType >= 0 && hitChainType < HITCHAIN_AMOUNT);
		
		// material specific hitchains will fall back to terrainobject
		// hitchain, if they are not defined...
		if (!chainDefined[hitChainType]
			&& hitChainType >= FIRST_HITCHAIN_MATERIAL
			&& hitChainType <= LAST_HITCHAIN_MATERIAL)
		{
			hitChainType = HITCHAIN_TERRAINOBJECT;
		}

		return chainBullet[hitChainType];
	}

	/*
	int Bullet::getChainScript(int hitChainType)
	{
		assert(hitChainType >= 0 && hitChainType < HITCHAIN_AMOUNT);

		// material specific hitchains will fall back to terrainobject
		// hitchain, if they are not defined...
		if (!chainDefined[hitChainType]
			&& hitChainType >= FIRST_HITCHAIN_MATERIAL
			&& hitChainType <= LAST_HITCHAIN_MATERIAL)
		{
			hitChainType = HITCHAIN_TERRAINOBJECT;
		}

		return chainScript[hitChainType];
	}
	*/
	const char *Bullet::getChainScript(int hitChainType)
	{
		assert(hitChainType >= 0 && hitChainType < HITCHAIN_AMOUNT);
		
		// material specific hitchains will fall back to terrainobject
		// hitchain, if they are not defined...
		if (!chainDefined[hitChainType]
			&& hitChainType >= FIRST_HITCHAIN_MATERIAL
			&& hitChainType <= LAST_HITCHAIN_MATERIAL)
		{
			hitChainType = HITCHAIN_TERRAINOBJECT;
		}

		return chainScript[hitChainType];
	}

	const char *Bullet::getChainSound(int hitChainType, int soundNum)
	{
		assert(hitChainType >= 0 && hitChainType < HITCHAIN_AMOUNT);
		assert(soundNum >= 0 && soundNum < HITCHAIN_SOUNDS);

		// material specific hitchains will fall back to terrainobject
		// hitchain, if they are not defined...
		if (!chainDefined[hitChainType]
			&& hitChainType >= FIRST_HITCHAIN_MATERIAL
			&& hitChainType <= LAST_HITCHAIN_MATERIAL)
		{
			hitChainType = HITCHAIN_TERRAINOBJECT;
		}

		return chainSound[hitChainType][soundNum];
	}

	int Bullet::getChainSoundAmount(int hitChainType)
	{
		assert(hitChainType >= 0 && hitChainType < HITCHAIN_AMOUNT);

		// material specific hitchains will fall back to terrainobject
		// hitchain, if they are not defined...
		if (!chainDefined[hitChainType]
			&& hitChainType >= FIRST_HITCHAIN_MATERIAL
			&& hitChainType <= LAST_HITCHAIN_MATERIAL)
		{
			hitChainType = HITCHAIN_TERRAINOBJECT;
		}

		for (int i = 0; i < HITCHAIN_SOUNDS; i++)
		{
			if (chainSound[hitChainType][i] == NULL) 
				return i;
		}
		return HITCHAIN_SOUNDS;
	}

	int Bullet::getChainSoundProbability(int hitChainType)
	{
		assert(hitChainType >= 0 && hitChainType < HITCHAIN_AMOUNT);

		// material specific hitchains will fall back to terrainobject
		// hitchain, if they are not defined...
		if (!chainDefined[hitChainType]
			&& hitChainType >= FIRST_HITCHAIN_MATERIAL
			&& hitChainType <= LAST_HITCHAIN_MATERIAL)
		{
			hitChainType = HITCHAIN_TERRAINOBJECT;
		}

		return hitSoundProbability[hitChainType];
	}

	int Bullet::getTerrainBlendAmount()
	{
		return terrainBlendAmount;
	}

	int Bullet::getTerrainBlendMax()
	{
		return terrainBlendMax;
	}

	float Bullet::getTerrainBlendRadius()
	{
		return terrainBlendRadius;
	}

	bool Bullet::hasTerrainHoleEdges()
	{
		return terrainHoleEdges;
	}

	float Bullet::getTerrainHoleDepth()
	{
		return terrainHoleDepth;
	}

	float Bullet::getTerrainHoleRadius()
	{
		return terrainHoleRadius;
	}

	Bullet::TERRAIN_HOLE_TYPE Bullet::getTerrainHoleType()
	{
		return terrainHoleType;
	}

	int Bullet::getAfterLifeTime()
	{
		return afterLifeTime;
	}

	int Bullet::getLifeTime()
	{
		return lifeTime;
	}

	bool Bullet::isRadicalDistanceRatio()
	{
		return radicalDistanceRatio;
	}

	float Bullet::getStartSkipAmount()
	{
		return startSkipAmount;
	}

	bool Bullet::isEndPadToHit()
	{
		return endPadToHit;
	}

	int Bullet::getSplitRaytrace()
	{
		return splitRaytrace;
	}

	int Bullet::getSlowdown()
	{
		return slowdown;
	}

	int Bullet::getHitDamage(int damageType)
	{
		assert(damageType >= 0 && damageType < DAMAGE_TYPES_AMOUNT);

		return hitDamage[damageType];
	}

	int Bullet::getChainSoundPriority(int chain) const
	{
		assert(chain >= 0 && chain < HITCHAIN_AMOUNT);
		return chainSoundPriority[chain];
	}

	float Bullet::getChainSoundRange(int chain) const
	{
		assert(chain >= 0 && chain < HITCHAIN_AMOUNT);
		return chainSoundRange[chain];
	}

	int Bullet::getGoreProbability(int goreType) const
	{
		if (goreType < 0 || goreType >= GORETYPE_AMOUNT)
		{
			Logger::getInstance()->error("Bullet::getGoreProbability - gore type parameter out of range.");
			return 0;
		}

		return goreProbability[goreType];
	}

	bool Bullet::hasNoRotation() const
	{
		return noRotation;
	}

	int Bullet::getParabolicPathHeight() const
	{
		return parabolicPathHeight;
	}

	bool Bullet::doesConnectToParent() const
	{
		return connectToParent;
	}

	bool Bullet::doesParentToNextBullet() const
	{
		return parentToNextBullet;
	}

	Bullet *Bullet::getHitByProjectileBullet()
	{
		return hitByProjectileBullet;
	}

	bool Bullet::doesPoisonDamage() const
	{
		return poisonDamage;
	}

	Bullet *Bullet::getDelayedHitProjectileBullet() const
	{
		return delayedHitProjectileBullet;
	}

	int Bullet::getDelayedHitProjectileInterval() const
	{
		return delayedHitProjectileInterval;
	}

	int Bullet::getDelayedHitProjectileAmount() const
	{
		return delayedHitProjectileAmount;
	}

	bool Bullet::hasNoDelayedHitProjectileForPlayer() const
	{
		return noDelayedHitProjectileForPlayer;
	}

	bool Bullet::hasNoDifficultyEffectOnDamageAmount() const
	{
		return noDifficultyEffectOnDamageAmount;
	}

	bool Bullet::doesNoTerrainObjectBreaking() const
	{
		return noTerrainObjectBreaking;
	}

}

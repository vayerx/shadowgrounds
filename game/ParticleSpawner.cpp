
#include "precompiled.h"

#include "ParticleSpawner.h"

#include "Bullet.h"
#include "Weapon.h"
#include "ProjectileList.h"
#include "Projectile.h"
#include "ProjectileActor.h"
#include "Game.h"
#include "GameUI.h"
#include "GameRandom.h"
#include "SimpleOptions.h"
#include "options/options_sounds.h"
#include "../system/Logger.h"
#include "../sound/sounddefs.h"
#include "../sound/SoundMixer.h"


// 2 meters extra to spawners sound range inside which firesounds are made..
#define PARTICLESPAWNER_EXTRA_SOUND_RANGE 2.0f

// how much sound priority is adjusted by range...
#define PARTICLESPAWNER_SOUND_DISTANCE_PRIORITY_ADJUST 5

namespace game
{

	ParticleSpawner::ParticleSpawner(Game *game)
	{
    this->game = game;

    this->name = NULL;

    this->position = VC3(0,0,0);
    this->direction = VC3(0,0,0);

    this->weapon = NULL; 

    this->enabled = true;
		this->outsideScreen = false;
		this->distanceToListenerSq = 0.0f;

		this->waitTime = 0;

		this->projectileHandle = 0;
	}


	ParticleSpawner::~ParticleSpawner()
	{
		setName(NULL);
	}


	void ParticleSpawner::setName(const char *name)
	{
		if (this->name != NULL)
		{
			delete [] this->name;
			this->name = NULL;
		}
		if (name != NULL)
		{
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name);
		}
	}


	const char *ParticleSpawner::getName() const
	{
		return this->name;
	}


	void ParticleSpawner::setPosition(const VC3 &position)
	{
		this->position = position;	
	}


	void ParticleSpawner::setDirection(const VC3 &direction)
	{
		this->direction = direction;
	}


	void ParticleSpawner::setSpawnerWeapon(const Weapon *weapon)
	{
		this->weapon = weapon;

#ifndef PROJECT_SHADOWGROUNDS
		if (weapon->getFireWaitTime() > 0)
		{
			int firewait = (weapon->getFireWaitTime() * ((100 - weapon->getFireWaitVary()) + game->gameRandom->nextInt() % (weapon->getFireWaitVary() + 1)) / 100);
			this->waitTime = firewait;
		}
#endif
	}


	void ParticleSpawner::disable()
	{
		this->enabled = false;

		Projectile *proj = game->projectiles->getProjectileByHandle(this->projectileHandle);
		if (proj != NULL)
		{
			if (proj->getAfterLifeTime() > 1)
			{
				proj->setAfterLifeTime(1);
			}
			if (proj->getLifeTime() > 1)
			{
				proj->setLifeTime(1);
			}
		}
	}


	void ParticleSpawner::enable()
	{
		assert(weapon != NULL);
		this->enabled = true;
		// TODO: start the actual effect.. ? 
		// (immediately? waittime to zero?)
	}


	Projectile *ParticleSpawner::spawnProjectileWithWeapon(Game *game, const Weapon *weapon,
		const VC3 &position, const VC3 &direction, float distanceToListener)
	{
		Bullet *bullet = weapon->getBulletType();
		if (bullet != NULL)
		{
			Projectile *proj = new Projectile(NULL, bullet);
			game->projectiles->addProjectile(proj);

			VC3 projpos = position;

			VC3 dir = direction;

			float range = (float)weapon->getRange();
			if (range == 0.0f)
			{
				range = 0.01f;
			}
			VC3 endpos = projpos + dir * range;

			proj->setDirectPath(projpos, endpos, bullet->getVelocity());

			ProjectileActor pa = ProjectileActor(game);
			pa.createVisualForProjectile(proj);

			const char *fireSound = weapon->getFireSound();

			if (fireSound != NULL)
			{
				bool looped = false;
				int handleDummy = -1;
				int key = 0;

				float soundrange = bullet->getChainSoundRange(HITCHAIN_NOTHING);

				// HACK: only play sound if in sound range (+5m)...
				if (soundrange == 0 
					|| distanceToListener < soundrange + PARTICLESPAWNER_EXTRA_SOUND_RANGE)
				{
					int priority = bullet->getChainSoundPriority(HITCHAIN_NOTHING);

					// HACK: decrease/increase priority based on distance...
					if (distanceToListener < soundrange)
					{
						if (soundrange > 0)
						{
							priority -= (int)(PARTICLESPAWNER_SOUND_DISTANCE_PRIORITY_ADJUST * (distanceToListener / soundrange));
						}
					} else {
						priority -= PARTICLESPAWNER_SOUND_DISTANCE_PRIORITY_ADJUST;
					}
					if (priority < MIN_SOUND_PRIORITY) priority = MIN_SOUND_PRIORITY;
					if (priority > MAX_SOUND_PRIORITY) priority = MAX_SOUND_PRIORITY;

					int handle = game->gameUI->parseSoundFromDefinitionString(fireSound, projpos.x, projpos.y, projpos.z, &looped, &handleDummy, &key, false, soundrange, priority);

					if (handle == -1)
					{
						if (SimpleOptions::getBool(DH_OPT_B_SOUNDS_ENABLED)
							&& SimpleOptions::getBool(DH_OPT_B_FX_ENABLED))
						{
							if (gameui_sound_effect_errorcode == GAMEUI_SOUND_EFFECT_ERRORCODE_NO_SOUNDS
								|| gameui_sound_effect_errorcode == GAMEUI_SOUND_EFFECT_ERRORCODE_TOO_FAR)
							{
								// this is ok. if sounds disabled or too far away to be heard, don't spam errormessages.
							} else {
								if (soundmixer_play_errorcode == SOUNDMIXER_PLAY_ERRORCODE_COUNT_EXCEEDED)
								{
									Logger::getInstance()->debug("ParticleSpawner::run - Sound sample simultaneous playing count exceeded.");
									Logger::getInstance()->debug(fireSound);
								} else {
									Logger::getInstance()->warning("ParticleSpawner::run - Failed to create firesound.");
									Logger::getInstance()->debug(fireSound);
								}
							}
						}
					}

					if (looped)
					{
						Logger::getInstance()->warning("ParticleSpawner::run - Firesound definition was a looping sound, not properly supported.");
						// TODO: looped sounds not yet supported
						assert(!"looped particlespawner sounds not supported.");
					}
				}
			}

			return proj;
		} else {
			return NULL;
		}
	}


	void ParticleSpawner::run()
	{
		if (!enabled)
			return;

		if (weapon == NULL)
			return;

		if (waitTime > 0)
		{
			waitTime--;
		} else {

			if (outsideScreen)
			{
				// NEW: when outside screen, set waitTime based on weapon's fire wait time - to get randomized spawning times...
#ifndef PROJECT_SHADOWGROUNDS
				if (weapon->getFireWaitTime() > 0)
				{
					int firewait = (weapon->getFireWaitTime() * ((100 - weapon->getFireWaitVary()) + game->gameRandom->nextInt() % (weapon->getFireWaitVary() + 1)) / 100);
					waitTime = firewait;
				}
#endif
				return;
			}

			int firereload = (weapon->getFireReloadTime() * ((100 - weapon->getFireReloadVary()) + game->gameRandom->nextInt() % (weapon->getFireReloadVary() + 1)) / 100);
			waitTime = firereload;

			Projectile *proj = spawnProjectileWithWeapon(game, weapon, this->position, this->direction, sqrtf(this->distanceToListenerSq));

			if (proj != NULL)
				this->projectileHandle = proj->getHandle();
			else
				this->projectileHandle = 0;
		}
	}

}



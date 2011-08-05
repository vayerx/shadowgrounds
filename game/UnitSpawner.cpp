
#include "precompiled.h"

#include "UnitSpawner.h"

#include <assert.h>
#include "scaledefs.h"
#include "Game.h"
#include "GameScene.h"
#include "GameMap.h"
#include "GameRandom.h"
#include "Unit.h"
#include "Flashlight.h"
#include "Weapon.h"
#include "UnitList.h"
#include "unittypes.h"
#include "UnitLevelAI.h"
#include "UnitType.h"
#include "ScriptableAIDirectControl.h"
#include "Character.h"
#include "Part.h"
#include "GameUI.h"
#include "SimpleOptions.h"
#include "UnitPhysicsUpdater.h"
#include "../convert/str2int.h"
#include "../ui/AnimationSet.h"
#include "../ui/Spotlight.h"
#include "../system/Logger.h"
#include "options/options_physics.h"

#include "../util/Debug_MemoryManager.h"


namespace game
{

  bool UnitSpawner::spawner_dont_delete_unit = false;
	bool spawner_force_delete_unit = false;

 	void UnitSpawner::spawnUnit(Game *game, Unit *unit)
	{
		assert(game != NULL);
		assert(unit != NULL);


		// TODO: REspawning does not work properly!
		// (undealt issues with obstacle blocks, spawn coordinates, etc.)
		assert(!unit->isActive()); // try to catch possible respawns

if (unit->isActive())
	return;


		// FIXME: what about respawning computer units - the above
		// retire will delete them... hacked a fix... but might want
		// a more elegant solution for this. :(
		spawner_dont_delete_unit = true;
		
	  // first make sure the unit has been retired so we can respawn.
		if (unit->isActive())
			retireUnit(game, unit);

		// check for badly set unit sides...
		if (unit->getUnitType()->getAlwaysOnPlayerSide() != NO_UNIT_OWNER
			&& unit->getUnitType()->getAlwaysOnPlayerSide() != -1
			&& unit->getUnitType()->getAlwaysOnPlayerSide() != unit->getOwner())
		{
			if (!unit->isGhostOfFuture())
			{
				Logger::getInstance()->warning("UnitSpawner::spawnUnit - Unit owner is not correct, unit type requires specific owner for unit.");
				Logger::getInstance()->debug("Unit type follows:");
				Logger::getInstance()->debug(unit->getUnitType()->getName());
				Logger::getInstance()->debug("Given unit owner follows:");
				Logger::getInstance()->debug(int2str(unit->getOwner()));
				Logger::getInstance()->debug("Required owner follows:");
				Logger::getInstance()->debug(int2str(unit->getUnitType()->getAlwaysOnPlayerSide()));
			}
		}

		spawner_dont_delete_unit = false;

		// now, actually spawn the unit...

		Unit *u = unit;

#ifdef PROJECT_SURVIVOR
		int max_hp = u->getMaxHP();
#endif

    // hp based on character or unittype...
    if (u->getCharacter() != NULL)
    {
      u->setMaxHP(u->getUnitType()->getHP() 
				* u->getCharacter()->getSkillAmount(CHAR_SKILL_STAMINA) / 100);

      // choose character script if one.
      if (u->getCharacter()->getScript() != NULL)
        u->setScript(u->getCharacter()->getScript());
    } else {
      u->setMaxHP(u->getUnitType()->getHP());
    }
#ifdef PROJECT_SURVIVOR
		// hack: don't allow setting a lower HP
		// than the current one, so that we can
		// use HP upgrades which reset themselves
		// when necessary.

		if(max_hp > u->getMaxHP())
		{
			u->setMaxHP(max_hp);
		}
#endif

    u->setHP(u->getMaxHP());
    u->initWeapons();
    u->setActive(true);
    UnitLevelAI *ai = new UnitLevelAI(game, u);
    u->setAI(ai);
    u->setDestroyed(false);
    u->setSelected(false);
    u->calculateWeight();
    u->calculateMaxEnergy();
		u->calculateReconValue();
		u->calculateRunningValue();
		u->calculateStealthValue();
    u->setStartEnergy(u->getMaxEnergy());
    u->setEnergy(u->getMaxEnergy());
    u->calculateMaxHeat();
    u->setHeat(0);
    u->calculateRechargingAmount();
    u->calculateCoolingAmount();
    u->setLeader(NULL);
		u->setSightBonus(false);
    u->setHitByUnit(NULL, NULL);
		u->setHearNoiseByUnit(NULL);
		u->setHitMissByUnit(NULL);
		u->setPointedByUnit(NULL);
    u->targeting.clearLastTargetPosition();
    u->targeting.clearTarget();
    u->prepareAnimationSet();
		u->setOnGround(true);
		u->setGroundFriction(true);
		u->setIdleTime((game->gameRandom->nextInt() % 5000));
		u->setAnimation(0); // = ANIM_NONE
		u->setKeepFiringCount(0);
		u->setKeepReloading(false);
		u->setSelectedItem(-1);
		u->setAreaCircleId(-1);
		u->setLastLightUpdatePosition(VC3(-999999,-999999,-999999));
		u->setAliveMarker(true);
		u->setDiedByPoison(false);
		u->setFadeVisibilityImmediately(1.0f);
		u->setRushing(u->getUnitType()->isRusher());
		u->setDelayedHitProjectileBullet(NULL);
		u->setDelayedHitProjectileAmount(0);
		u->setBurnedCrispyAmount(0);
		u->setLastRotationDirection(0);
		u->setActCheckCounter(game->units->getIdForUnit(u) % GAME_UNIT_ACT_CHECK_COUNTER_INTERVAL);
		u->setActed(true);
		u->setDisappearCounter(0);
		u->setCustomTimeFactor(1.0f);
		u->setSlowdown(0);
		u->setPointerVisualEffect(NULL);
		u->setPointerHitVisualEffect(NULL);
		u->setShielded(false);
		if (u->getUnitType()->isDirectControl())
		{
			if (!u->isDirectControl())
			{
				u->setDirectControl(true);
				u->setDirectControlType(Unit::UNIT_DIRECT_CONTROL_TYPE_AI);
			}

			// TODO: this should have some proper factory.
			if (unit->getUnitType()->getDirectControlType() != NULL
				&& strcmp(unit->getUnitType()->getDirectControlType(), "scriptable") == 0)
			{
				ScriptableAIDirectControl *dc = new ScriptableAIDirectControl(game, u);
				u->setAIDirectControl(dc);
			}
			else if (unit->getUnitType()->getDirectControlType() != NULL
				&& strcmp(unit->getUnitType()->getDirectControlType(), "player") == 0)
			{
				// ok, but leave the dc to null.
			} else {
				Logger::getInstance()->warning("UnitSpawner::spawnUnit - Unit type is \"directcontrol\", but has no or invalid \"directcontroltype\".");
				Logger::getInstance()->debug(unit->getUnitType()->getName());
			}
		}

		if (u->getUnitType()->doesIgnoreSpawnBlocking())
		{
			u->setCollisionCheck(false);
		} else {
			u->setCollisionCheck(true);
		}

		u->setSideways(u->getUnitType()->isSideways());
		//if (u->getVisualObject() != NULL)
		//{
		//	u->getVisualObject()->setSideways(u->getUnitType()->isSideways());
		//}
		u->setSideGravityX(u->getUnitType()->getSideGravityX());
		u->setSideGravityZ(u->getUnitType()->getSideGravityZ());
		u->setSideVelocityMax(u->getUnitType()->getSideVelocityMax());

		if (u->getUnitType()->isSpottable())
		{
			u->setSpottable(true);
		} else {
			u->setSpottable(false);
		}


		// FIXME: a lot of unit class variables not properly set here???


		u->setImmortal(unit->getUnitType()->isImmortalByDefault());
		
		// HACK!!!
		if (u->getSelectedWeapon() == -1)
		{
			if (u->getWeaponType(0) != NULL
				&& (u->getWeaponType(0)->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_Pistol")
				|| u->getUnitType()->hasOnlyOneWeaponSelected()))
				u->setSelectedWeapon(0);
		}

		// FIXME: ???
		// WTF is this??? rocket??? should be grenade maybe??? (is this correct or not)
		/*
		if (u->getSelectedSecondaryWeapon() == -1)
		{
			if (u->getWeaponType(1) != NULL
				&& u->getWeaponType(1)->getPartTypeId() == PARTTYPE_ID_STRING_TO_INT("W_Rocket"))
				u->setSelectedSecondaryWeapon(1);
		}
		*/

		for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
		{
			u->reloadWeaponAmmoClip(i, true);
			if (i == unit->getSelectedWeapon() || !u->getUnitType()->hasOnlyOneWeaponSelected())
				unit->setWeaponActive(i, true);
			else
				unit->setWeaponActive(i, false);
		}

    // HAXOR...
    u->obstacleExists = false;

    Part *p = u->getRootPart();

    game->createVisualForParts(u, p);
    // need to create visuals before moving the unit.
    // as they may be moved relative to unit movement, so units
    // must be at 0,0 when visual is created.
    if (u->hasSpawnCoordinates())
    {
      // unit has it's own spawn coordinates, use them...
      float sx = 0;
      float sy = 0;
      VC3 spawn = u->getSpawnCoordinates();

      //sx = game->gameMap->configToScaledX((int)spawn.x);
      //sy = game->gameMap->configToScaledY((int)spawn.z);
			sx = spawn.x;
			sy = spawn.z;
			if (u->doesCollisionCheck())
			{
				int failcount = 0;
				for (failcount = 0; failcount < 1000; failcount++)
				{
					game->gameMap->keepWellInScaledBoundaries(&sx, &sy);
					if (!game->gameScene->isBlockedAtScaled(sx,sy,game->gameMap->getScaledHeightAt(sx, sy))) break;
					sx = spawn.x + ((game->gameRandom->nextInt() % (51 * (1 + failcount / 100))) - (25 * (1 + failcount / 100))) / 50.0f;
					sy = spawn.z + ((game->gameRandom->nextInt() % (51 * (1 + failcount / 100))) - (25 * (1 + failcount / 100))) / 50.0f;
				}
				if (failcount >= 1000)
				{
					Logger::getInstance()->warning("UnitSpawner::spawnUnit - Failed to find unblocked spawn position.");
				}
			}
      u->setPosition(VC3(sx, game->gameMap->getScaledHeightAt(sx, sy), sy));
			// update rotation in visualObject
			VC3 rotation = u->getRotation();
			if(u->getVisualObject())
				u->getVisualObject()->setRotation(rotation.x, rotation.y, rotation.z);


      UnitActor *ua = getUnitActorForUnit(u);
      ua->addUnitObstacle(u);
      /*
      if (!u->getUnitType()->isFlying())
      {
        ADD_UNIT_OBSTACLE(this, gameMap->scaledToObstacleX(sx), 
          gameMap->scaledToObstacleX(sy));
      }
      */

    } else {
      // no specific spawn coordinates, use player's spawn coords...
      if (u->getOwner() != NO_UNIT_OWNER)
      {
        float sx = 0;
        float sy = 0;
        int sxi = 0;
        int syi = 0;
        sxi = game->spawnX[u->getOwner()];
        syi = game->spawnY[u->getOwner()];
        sx = game->gameMap->configToScaledX(sxi);
        sy = game->gameMap->configToScaledY(syi);
        for (int failcount = 0; failcount < 1000; failcount++)
        {
          game->gameMap->keepWellInScaledBoundaries(&sx, &sy);
          if (!game->gameScene->isBlockedAtScaled(sx,sy,game->gameMap->getScaledHeightAt(sx, sy))) break;
          sx += (float)((game->gameRandom->nextInt() % 5) - 2) / 10.0f;
          sy += (float)((game->gameRandom->nextInt() % 5) - 2) / 10.0f;
        }
        u->setPosition(VC3(sx, game->gameMap->getScaledHeightAt(sx, sy), sy));
				// update rotation in visualObject
				VC3 rotation = u->getRotation();
				if(u->getVisualObject())
					u->getVisualObject()->setRotation(rotation.x, rotation.y, rotation.z);

				// NEW BEHAVIOUR: set spawn to initial position even for those units that
				// do not have specific spawn coordinates set!
				VC3 pos = u->getPosition();
				u->setSpawnCoordinates(pos);
        
        UnitActor *ua = getUnitActorForUnit(u);
				assert(ua != NULL);
        ua->addUnitObstacle(u);
        /*
        if (!u->getUnitType()->isFlying())
        {
          ADD_UNIT_OBSTACLE(this, sx, sy);
        }
        */
      } else {
        Logger::getInstance()->warning("Game::startCombat - Non-owned unit has no spawn coordinates.");
      }
    }
    u->setPath(NULL);
    u->setWaypoint(u->getPosition());
    u->setFinalDestination(u->getPosition());

		//if (u->getOwner() == game->singlePlayerNumber)
		if (u->getUnitType()->hasFlashlight())
		{
			//std::string spottype = std::string("flashlight");

			// TODO: need to get this to some commonly used class
			// now it's copy-pasted everywhere...
			Flashlight *fl = new Flashlight(game, u->getVisualObject());
			fl->setOffset(u->getUnitType()->getFlashlightOffset());
			u->setFlashlight(fl);
			VC3 rot = u->getRotation();
			float angle = UNIT_ANGLE_TO_RAD(rot.y + u->getLastBoneAimDirection() + u->getUnitType()->getAimBoneDirection());
			fl->setRotation(angle);
		}

		if (u->getUnitType()->hasHalo())
		{
			ui::Spotlight *sp;

			std::string spottype = std::string("playerhalo");
			char *haloType = u->getUnitType()->getHaloType();
			if (haloType != NULL)
			{
				spottype = std::string(haloType);
			}
			sp = new ui::Spotlight(
				*game->getGameScene()->getStorm3D(), *game->gameUI->getTerrain()->GetTerrain(),
				*game->getGameScene()->getStormScene(),
				NULL,
				spottype);
			u->setSecondarySpotlight(sp);
		}

		if (SimpleOptions::getBool(DH_OPT_B_PHYSICS_ENABLED))
		{
			UnitPhysicsUpdater::createPhysics(unit, game->getGamePhysics(), game);
		}

	}


	void UnitSpawner::retireUnit(Game *game, Unit *unit)
	{
		assert(game != NULL);
		assert(unit != NULL);

		UnitPhysicsUpdater::deletePhysics(unit, game->getGamePhysics());

		Unit *u = unit;

		Flashlight *fl = u->getFlashlight();
		if (fl != NULL)
		{
			delete fl;
			u->setFlashlight(NULL);
		}

		// remove visual attachments
		u->setEjectVisualEffect(NULL, 0);
		u->setMuzzleflashVisualEffect(NULL, 0);

		/*
		ui::Spotlight *sp = u->getSpotlight();
		if (sp != NULL)
		{
			delete sp;
			u->setSpotlight(NULL);
		}
		*/
		ui::Spotlight *sp;
		sp = u->getSecondarySpotlight();
		if (sp != NULL)
		{
			delete sp;
			u->setSecondarySpotlight(NULL);
		}

		// first delete visual object of the unit
    Part *p = u->getRootPart();
    if (p != NULL)
    {
      game->deleteVisualOfParts(u, p);
    }

		// no-longer active
		u->setActive(false);
		u->setShielded(false);

		// uninitialize weapons (clear optimization arrays and calculate
		// the ammos left, etc.)
    u->uninitWeapons();
		u->deleteCustomizedWeaponTypes();

		// delete AI
    UnitLevelAI *ai = (UnitLevelAI *)u->getAI();
    if (ai != NULL)
    {
      delete ai;
      u->setAI(NULL);
    }

		// disable forced animation
		unit->setForcedAnimation(0);

		// delete computer's units, but keep player's units
		// if called by the spawnUnit, don't actually delete this
		// unit (cos we will try to respawn it immediately after this)
    if ((game->isComputerOpponent(u->getOwner())
			&& !spawner_dont_delete_unit)
			|| spawner_force_delete_unit ||
			!u->getUnitType()->isPermanentPlayerUnit()) // player unit is not permanent
    {
			// when quitting, GameUI is deleted already (WinMain)
			if(game->gameUI != NULL)
			{
				for (int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
				{
					if (game->gameUI->getFirstPerson(c) == u)
					{
						game->gameUI->setFirstPerson(game->singlePlayerNumber, NULL, c);							
					}
				}
			}

      if (u->getRootPart() != NULL)
      {
        game->detachParts(u, u->getRootPart());
      }
			game->units->removeUnit(u);

			if (u->getCharacter() != NULL)
			{
				Character *c = u->getCharacter();
				u->setCharacter(NULL);
				delete c;
			}

			delete u;
    } else {
			u->retireFromCombat();
    }
	}


	void UnitSpawner::deleteUnit(Game *game, Unit *unit)
	{
		spawner_force_delete_unit = true;
		retireUnit(game, unit);
		spawner_force_delete_unit = false;
	}

	void UnitSpawner::markAliveHostileUnits(Game *game)
	{
		for (int p = 0; p < ABS_MAX_PLAYERS; p++)
		{
			if (game->isHostile(p, game->singlePlayerNumber))
			{
				LinkedList *ulist = game->units->getOwnedUnits(p);
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive())
					{
						if (!u->isDestroyed())
						{
							u->setAliveMarker(true);
						} else {
							u->setAliveMarker(false);
						}
					}
				}
			}
		}
	}

	void UnitSpawner::respawnMarkedHostileUnitsAlive(Game *game)
	{
		for (int p = 0; p < ABS_MAX_PLAYERS; p++)
		{
			if (game->isHostile(p, game->singlePlayerNumber))
			{
				LinkedList *ulist = game->units->getOwnedUnits(p);
				LinkedListIterator iter(ulist);
				while (iter.iterateAvailable())
				{
					Unit *u = (Unit *)iter.iterateNext();
					if (u->isActive())
					{
						bool returnToSpawn = false;

						if (u->isDestroyed())
						{
							if (u->hasAliveMarker() && u->getUnitType()->doesAllowRespawn())
							{
								// back to life! (hack)
								u->setDestroyed(false);

								u->setDisappearCounter(0);

								// renew the animation hack (copied from addSubPart script command :)
								if (u->getWalkDelay() < 2)
									u->setWalkDelay(2);
								if (u->getAnimationSet() != NULL)
								{
									u->setAnimation(0); // ANIM_NONE
									if (u->getAnimationSet()->isAnimationInSet(ANIM_STAND))
										u->getAnimationSet()->animate(u, ANIM_STAND);
								}
								returnToSpawn = true;
							}
						} else {
							returnToSpawn = true;
						}

						if (returnToSpawn)
						{
							u->setHP(u->getMaxHP());

							UnitActor *ua = getUnitActorForUnit(u);
							//if (ua->canWarpToPosition(u, unit->getSpawnCoordinates()))
							{
								ua->warpToPosition(u, u->getSpawnCoordinates());
							}

							u->targeting.clearTarget();
							ua->stopUnit(u);
						}
					}
				}
			}
		}
	}


	Unit *UnitSpawner::findReusableUnit(Game *game, UnitType *unitType, int playerSide)
	{

		LinkedList *ulist = game->units->getOwnedUnits(playerSide);
		LinkedListIterator uiter(ulist);

		while (uiter.iterateAvailable())
		{
			Unit *u = (Unit *)uiter.iterateNext();
			
			// NOTE: relying on the silly getCurrentVisibilityFadeValue to tell us when the unit has actually
			// disappeared...
			if(   u->isDestroyed() 
				&& u->getCurrentVisibilityFadeValue() == 0.0f
				&& u->getUnitType() == unitType)
			{
				return u;
			}
		}
		return NULL;
	}


	void UnitSpawner::reuseUnit(Game *game, Unit *unit)
	{
		assert(unit->isDestroyed());
		assert(unit->getUnitType()->doesAllowRespawn());

		/*
		// this one, copied from old hacky "respawn" probably isn't good enough...

		Unit *u = unit;
		// back to life! (hack)
		u->setDestroyed(false);

		u->setDisappearCounter(0);

		// renew the animation hack (copied from addSubPart script command :)
		if (u->getWalkDelay() < 2)
			u->setWalkDelay(2);
		if (u->getAnimationSet() != NULL)
		{
			u->setAnimation(0); // ANIM_NONE
			if (u->getAnimationSet()->isAnimationInSet(ANIM_STAND))
				u->getAnimationSet()->animate(u, ANIM_STAND);
		}

		u->setHP(u->getMaxHP());

		UnitActor *ua = getUnitActorForUnit(u);
		//if (ua->canWarpToPosition(u, unit->getSpawnCoordinates()))
		{
			ua->warpToPosition(u, u->getSpawnCoordinates());
		}

		u->targeting.clearTarget();
		ua->stopUnit(u);
		*/

		// this should be extensive enough...

		// clear script paths
		unit->scriptPaths.clear();
		unit->variables.clearVariables();

		assert(!spawner_dont_delete_unit);

		spawner_dont_delete_unit = true;
		retireUnit(game, unit);
		unit->setIdString(NULL);
		spawner_dont_delete_unit = false;

		// don't spawn yet! (that should be done separately by the script)
		//spawnUnit(game, unit);
	}

}




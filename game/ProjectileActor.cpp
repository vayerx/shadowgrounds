
#include "precompiled.h"

// NO NO NO!!!
#include <Storm3D_UI.h>

#include "ProjectileActor.h"

#include "hitchaindefs.h"
#include "../ui/animdefs.h"
#include "../ui/Animator.h"
#include "../ui/ParticleManager.h"
#include "../ui/VisualObject.h"
#include "../ui/VisualEffect.h"
#include "../ui/VisualEffectManager.h"
#include "../particle_editor2/particleeffect.h"
#include "../ui/AnimationSet.h"
#include "physics/AbstractPhysicsObject.h"
#include "projectilevisdefs.h"
#include "Projectile.h"
#include "ProjectileList.h"
#include "BuildingList.h"
#include "GameRandom.h"
#include "scripting/GameScripting.h"
#include "AniManager.h"
#include "MaterialManager.h"
#include "Game.h"
#include "Flashlight.h"
#include "DifficultyManager.h"
#include "UnitSelections.h"
#include "GameUI.h"
#include "GameScene.h"
#include "physics/GamePhysics.h"
#include "ParticleSpawner.h"
#include "SimpleOptions.h"
#include "options/options_cheats.h"
#include "options/options_game.h"
#include "options/options_physics.h"
#include "options/options_graphics.h"
#include "UnitType.h"
#include "UnitList.h"
#include "UnitLevelAI.h"
#include "../ui/UIEffects.h"
#include "../ui/ParticleManager.h"
#include "../ui/Terrain.h"
#include "../util/HelperPositionCalculator.h"
#include "../util/PositionsDirectionCalculator.h"
#include "../game/scaledefs.h"
#include "../sound/sounddefs.h"
#include "../convert/str2int.h"

#include "Forcewear.h"

// TEST
#include "../ui/ParticleCollision.h"

// don't necessarily like this dependency...
#include "../game/unittypes.h"

#include "../util/fb_assert.h"
#include "../util/AngleRotationCalculator.h"
#include "../util/Debug_MemoryManager.h"
#include "../system/Logger.h"

#include "../ui/CombatWindow.h"
#include "../ui/TargetDisplayWindowUpdator.h"

// for gore part
/*
#include "../game/scripting/scripting_macros_start.h"
#include "../game/scripting/unit_script_commands.h"
#include "../game/scripting/scripting_macros_end.h"
*/

using namespace ui;

// radius whithin which all hostile units are called the "hitmiss"
// script (from the projectile's hit position)
// or maybe from other projectile positions too...
#define PROJECTILE_MISS_RADIUS 5

// noise from hitting a unit or firing a weapon
#define PROJECTILE_HIT_NOISE_MIN_RADIUS 9
// noise from expolosions (damagerange factor to get noise range)
#define PROJECTILE_HIT_NOISE_RADIUS_FACTOR 3

// if hit makes damage greater than this, make medium damage
// flash effect
#define PLAYER_MEDIUM_HIT_LIMIT 40
// if hit makes damage greater than this, make big damage
// flash effect (also done if player gets destroyed)
#define PLAYER_BIG_HIT_LIMIT 100

// same for poison damage...
#define PLAYER_MEDIUM_POISON_LIMIT 8
#define PLAYER_BIG_POISON_LIMIT 20


// range within which we won't hit friendly units (100 meters)
#ifdef PROJECT_SURVIVOR
	#define UNITACTOR_DONT_HIT_FRIENDLY_RANGE 200
#else
	#define UNITACTOR_DONT_HIT_FRIENDLY_RANGE 100
#endif

#define PROJECTILEACTOR_PROJECTILEHIT_DISTANCE_SQ (2.5f*2.5f)
// HACK: different radius for FFire
#define PROJECTILEACTOR_PROJECTILEHIT_FFIRE_DISTANCE_SQ (1.5f*1.5f)

//#define PARABOLIC_PATH_HEIGHT 30

// ignore projectile raytrace collisions to destroyed units after they have
// been destroyed after this many ticks..
#define PROJECTILE_IGNORE_DESTROYED_AFTER (GAME_TICKS_PER_SECOND * 2)



namespace game
{

	ProjectileActor::ProjectileActor(Game *game)
	{
		this->game = game;
	}


	void ProjectileActor::createVisualForProjectile(Projectile *projectile, bool originToSpecialUnit, Unit *specialUnit)
	{
		int visualType = projectile->getVisualType();

		VC3 position = projectile->getOrigin();
		VC3 endPosition = projectile->getDestination();

		// HACK: fix zero range bullet rotation...
		// HACK: for specific bullets only (magnetic burst...)

		VC3 rotFixed = projectile->getRotation();
		VC3 velFixed = projectile->getVelocity();

		//if (projectile->getChain() == HITCHAIN_NOTHING)
		if (projectile->getBulletType() != NULL
			&& projectile->getBulletType()->hasNoRotation())
		{
			if (fabs(position.x - endPosition.x) < 0.2f
				&& fabs(position.y - endPosition.y) < 0.2f
				&& fabs(position.z - endPosition.z) < 0.2f)
			{
				rotFixed = VC3(0,0,0);
				velFixed = VC3(0,0.0001f,0);
			}
		}

		Unit *originUnit = projectile->getShooter();
		if (originToSpecialUnit)
		{
			originUnit = specialUnit;
			// HACK: this is used for ffire only, thus we'll hack this follow here...
			projectile->setFollowOrigin(true);
		}
		projectile->setOriginUnit(originUnit);


		// HACK: ...
		IStorm3D_Model *spawnModel = NULL;
		if (originUnit != NULL 
			&& originUnit->getVisualObject() != NULL) 
		{
			spawnModel = originUnit->getVisualObject()->getStormModel();
		}

		VisualEffect *ve =
			game->gameUI->getVisualEffectManager()->createNewVisualEffect(
			visualType, projectile, originUnit, 
			position, endPosition, rotFixed, velFixed,
			game, 1, spawnModel);
		projectile->setVisualEffect(ve);

		// TODO: only if options allow this - and only if fullscreen.
		// (no point in causing fps drop if it is not even gonna show)
		
		// TODO: a better check for explosions.
		// should move this whole thing inside visualeffectmanager.
		/*
		if (projectile->getVisualType() == game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("explosion2"))
		{
			// explosion
			//game->gameUI->getEffects()->startFlashEffect(ui::UIEffects::FLASH_EFFECT_TYPE_EXPLOSION);
		}
		*/
		// FIXME: these static ids may become invalid when visual effect manager gets reloaded, if the 
		// conf file (visualeffects.txt) has changed since the ids were solved.
		static int ve_id_explosion_cluster = -1;
		static int ve_id_explosion3 = -1;
		if (ve_id_explosion3 == -1)
		{
			ve_id_explosion3 = game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("explosion3");
		}
		if (ve_id_explosion_cluster == -1)
		{
			ve_id_explosion_cluster = game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("cluster_main_explosion");
		}

		if (projectile->getVisualType() == ve_id_explosion3)
		//	|| projectile->getVisualType() == game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("huge_explosion"))
		{
			// nuke explosion
			game->gameUI->getEffects()->startFlashEffect(ui::UIEffects::FLASH_EFFECT_TYPE_NUKE_EXPLOSION);
		}
		else if(projectile->getVisualType() == ve_id_explosion_cluster)
		{
			// cluster main explosion
			game->gameUI->getEffects()->startFlashEffect(ui::UIEffects::FLASH_EFFECT_TYPE_CLUSTER_EXPLOSION);
		}
//		if (projectile->getVisualType() == game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("electricmuzzleflash")
//			|| projectile->getVisualType() == game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("electricmuzzleflash2")
//			|| projectile->getVisualType() == game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("electricmuzzleflash3"))
		static int ve_id_electric_flow = -1;
		if (ve_id_electric_flow == -1)
		{
			ve_id_electric_flow = game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("electric_flow");
		}
		if (projectile->getVisualType() == ve_id_electric_flow)
		{
			game->gameUI->getEffects()->startFlashEffect(ui::UIEffects::FLASH_EFFECT_TYPE_LIGHTNING_GUN);
		}
		/*
		if (projectile->getVisualType() == game->gameUI->getVisualEffectManager()->getVisualEffectIdByName("envlightning"))
		{
			// environment lightning
			game->gameUI->getEffects()->startFlashEffect(ui::UIEffects::FLASH_EFFECT_TYPE_ENVIRONMENT_LIGHTNING);
		}
		*/

		// NOTE: this is slightly questionable - are fluids actually just merely "visualization"?
		// (currently, it is, may not be in the future)
		if (projectile->getBulletType() != NULL)
		{
			if (projectile->getBulletType()->getFluidPushTime() > 0
				&& projectile->getBulletType()->getFluidPushRadius() > 0.0f)
			{
				game->getGamePhysics()->createFluidPushPoint(position, projectile->getBulletType()->getFluidPushRadius(), projectile->getBulletType()->getFluidPushTime() / GAME_TICK_MSEC);
			}
		}

	}



	void ProjectileActor::act(Projectile *projectile)
	{
		float push_factor = SimpleOptions::getFloat(DH_OPT_F_PHYSICS_IMPACT_PUSH_FACTOR);
		bool connectToSomething = false;
		VC3 connectPosition(0,0,0);

		if (projectile->getParentUnit() != NULL)
		{
			connectToSomething = true;
			connectPosition = projectile->getParentUnit()->getPosition();
			VisualObject *unitVisual = projectile->getParentUnit()->getVisualObject();
			IStorm3D_Model *unitModel = 0;
			if(unitVisual)
			{				
				unitVisual->prepareForRender();

				unitModel = unitVisual->getStormModel();

				if(unitModel)
				{
					bool helperFound = util::getHelperPosition(unitModel, "HELPER_MODEL_WeaponBarrel", connectPosition);
					if (!helperFound)
					{
						helperFound = util::getHelperPosition(unitModel, "HELPER_BONE_WeaponBarrel", connectPosition);
						if (!helperFound)
						{
							Logger::getInstance()->warning("ProjectileActor::act - Parent unit weapon barrel helper not found.");
						}
					}
				}
			}

		}
		if (projectile->getParentProjectile() != NULL)
		{
			assert(!connectToSomething);
			connectToSomething = true;
			connectPosition = projectile->getParentProjectile()->getDestination();

			// TODO: getDestination valid only for ray pathtypes, other should use getPosition maybe?
			// HACK: this is just a hack to get flamer working...
			if (projectile->getBulletType() != NULL
				&& projectile->getBulletType()->doesParentToNextBullet())
			{
				connectPosition = projectile->getParentProjectile()->getPosition();

				// UBERHACK!!!
				// HACK: slowdown the projectile to match the particle drag force... :P
				// FIXME: no no no...
				VC3 vel = projectile->getVelocity();
				vel *= (1.0f - 0.04f);
				projectile->setVelocity(vel);
			}
		}

		if (projectile->doesFollowOrigin())
		{
			if (projectile->getOriginUnit() != NULL)
			{
				VC3 newpos = projectile->getOriginUnit()->getPosition() + projectile->getOriginUnit()->getPointerMiddleOffset();
				{
					QUAT q;
					q.MakeFromAxisRotation(VC3(0,-1,0), UNIT_ANGLE_TO_RAD(projectile->getOriginUnit()->getRotation().y));
					newpos += q.GetRotated(projectile->getOriginOffset());
				}
				projectile->setPosition(newpos);
				if (projectile->getVisualEffect() != NULL)
				{
					projectile->getVisualEffect()->setPosition(newpos);
				}
			}
		}

		if (connectToSomething)
		{
			// TODO: this will only connect "ray" pathtype bullets properly...

			projectile->setOrigin(connectPosition);
			VC3 origin = projectile->getOrigin();

			//VC3 origin = connectPosition;
			VC3 destination = projectile->getDestination();

			if (projectile->getBulletType() != NULL
				&& projectile->getBulletType()->doesParentToNextBullet())
			{
				destination = projectile->getPosition();
			}

			VC3 dist;
			float distLen;
			if (destination.x != origin.x 
				|| destination.y != origin.y
				|| destination.z != origin.z)
			{
				VC2 destFloat = VC2(
					(float)(destination.x - origin.x), (float)(destination.z - origin.z));
				float destAngleFloat = destFloat.CalculateAngle();
				float destAngle = -RAD_TO_UNIT_ANGLE(destAngleFloat) + (360+270);
				while (destAngle >= 360) destAngle -= 360;

				VC2 destFloat2 = VC2(
					(float)(destFloat.GetLength()), (float)(destination.y - origin.y));
				float destAngleFloat2 = destFloat2.CalculateAngle();
				float destAngle2 = RAD_TO_UNIT_ANGLE(destAngleFloat2) + 360;
				while (destAngle2 >= 360) destAngle2 -= 360;

				projectile->setRotation(destAngle2, destAngle, 0);
			}
			dist = destination - origin;
			distLen = dist.GetLength();

			VC3 halfWayPosition = origin + (dist / 2);

			if (projectile->getBulletType() != NULL
				&& !projectile->getBulletType()->doesParentToNextBullet())
			{
				projectile->setPosition(halfWayPosition);
			}

			if (projectile->getVisualEffect() != NULL)
			{
				VC3 rotation = projectile->getRotation();
				float alpha = 1.0f;
				float scale = 1.0f;

				// MORE HACK!!! visibility fadeout and scale up...
				if (projectile->getBulletType() != NULL
					&& projectile->getBulletType()->doesParentToNextBullet())
				{
					alpha = (float)(projectile->getLifeTime()+1) / (float)(projectile->getOriginalLifeTime()+1);
					scale = 1.5f - alpha;

					if (distLen > 1.0f)
					{
						alpha /= distLen * 2.0f;
						scale /= distLen;
					}
				}

				projectile->getVisualEffect()->moveBetween(origin, destination, rotation, alpha, scale);
			}
		}


		int ltime = projectile->getLifeTime();
		if (ltime > 0)
		{
			ltime--;
			projectile->setLifeTime(ltime);

			// is this projectile hittable by another projectile...
			if (projectile->getBulletType() != NULL
				&& projectile->getBulletType()->getHitByProjectileBullet() != NULL)
			{
				// if hit by another projectile, create proper bullet...
				bool wasHit = false;

				// TODO: this is _very_ inefficient, iterating thru all projectiles...
				// however, currently only gasoline pool uses this - thus, effectivess not important
				LinkedListIterator piter(game->projectiles->getAllProjectiles());
				while (piter.iterateAvailable())
				{
					Projectile *other = (Projectile *)piter.iterateNext();

					// HACK: !!!
					// UBERHACK: flamethrower (B_Flam... & B_FExt...) ignite it!!!
					// TODO: should definately be a conf for this, not hardcoded here!
					if (other != projectile 
//						&& other->getShooter() == projectile->getShooter()
						&& other->getBulletType() != NULL
						&& (strncmp(other->getBulletType()->getPartTypeIdString(), "B_Flam", 6) == 0
							|| strncmp(other->getBulletType()->getPartTypeIdString(), "B_FExt", 6) == 0
							|| strncmp(other->getBulletType()->getPartTypeIdString(), "B_FFir", 6) == 0
							|| strcmp(other->getBulletType()->getPartTypeIdString(), "B_GasoT3") == 0
							|| (other->getBulletType()->getDamageRange() > 0
								&& other->getBulletType()->getHPDamage() > 0
								&& !other->getBulletType()->doesPoisonDamage())))
					{
						float hitradiusSq = PROJECTILEACTOR_PROJECTILEHIT_DISTANCE_SQ;
						if (strncmp(other->getBulletType()->getPartTypeIdString(), "B_FFir", 6) == 0)
							hitradiusSq = PROJECTILEACTOR_PROJECTILEHIT_FFIRE_DISTANCE_SQ;

						VC3 otherpos = other->getPosition();
						VC3 posdiff = projectile->getPosition() - otherpos;
						if (posdiff.GetSquareLength() < hitradiusSq)
						{
							wasHit = true;
							break;
						}
					}
				}

				if (wasHit)
				{
					Bullet *hitbull = projectile->getBulletType()->getHitByProjectileBullet();

					Projectile *cproj = new Projectile(projectile->getShooter(), hitbull);
					game->projectiles->addProjectile(cproj);
						
					VC3 position = projectile->getPosition();

					cproj->setPosition(position);
					cproj->setDirectPath(position, position, 0.0f);

					ProjectileActor pa = ProjectileActor(game);
					pa.createVisualForProjectile(cproj);

					// make this one to the end of it's life...
					projectile->setLifeTime(0);
				}
			}

			if (projectile->getBulletType() != NULL
				&& projectile->getBulletType()->getSplitRaytrace() > 1)
			{
				int splitAmount = projectile->getBulletType()->getSplitRaytrace();

				int origltime = projectile->getOriginalLifeTime();

				// new: handling of gravity affect projectiles...
				bool gravPath = false;
				VC3 pathgrav = projectile->getBulletType()->getPathGravity();
				if (pathgrav.x != 0 || pathgrav.y != 0 || pathgrav.z != 0)
				{
					gravPath = true;	
				}

				int splitPos = projectile->getCurrentSplitPosition();
				if ((origltime - ltime) >= (splitPos * origltime) / splitAmount
					|| (gravPath && (ltime & 7) == 0 && projectile->getCurrentSplitPosition() < 999))
				{
					// reached next split position, do a new raytrace...
					projectile->setCurrentSplitPosition(splitPos + 1);

					VC3 orig;
					VC3 dir;
					float rayLen = 0.0f;

					if (gravPath)
					{
						// HACK: some magical hard-coded values here...
						float raydistfactor = 10.0f;
						orig = projectile->getPosition();
						VC3 vel = projectile->getVelocity();
						//if (vel.GetSquareLength() != 0.0f)
						//{
						//	vel.Normalize();
						//}
						VC3 dest = orig + (vel * raydistfactor);
						dir = dest - orig;
						rayLen = dir.GetLength();
						if (rayLen != 0)
						{
							dir /= rayLen;
						}
					} else {
						orig = projectile->getOrigin();
						VC3 dest = projectile->getDestination();
						dir = dest - orig;
						rayLen = dir.GetLength();
						if (rayLen != 0)
						{
							dir /= rayLen;
						}
						rayLen /= splitAmount;
						orig += (dir * (rayLen * (float)splitPos));
					}

					GameCollisionInfo cinfo;

					// TODO: disable collisions for building and stuff,
					// do for units only!!!
					LinkedList *blist = game->buildings->getAllBuildings();
					LinkedListIterator biter(blist);
					while (biter.iterateAvailable())
					{
						Building *b = (Building *)biter.iterateNext();
						if (b->getVisualObject() != NULL)
							b->getVisualObject()->setCollidable(false);
					}

					// FIXME: split raytrace bullets behave incorrectly for chained bullets...
					// (ricochet bullets from units can hit the same unit again, even though
					// that should not be allowed happen - should disable collision for that
					// unit instead of shooter)

					LinkedList disableCollUnits;
					getCollisionDisableList(projectile->getShooter(), disableCollUnits, projectile->getPosition());

					if (projectile->getShooter() != NULL
						&& projectile->getShooter()->getVisualObject() != NULL)
					{
						projectile->getShooter()->getVisualObject()->setCollidable(false);
					}

					game->getGameScene()->rayTrace(orig, dir, rayLen, cinfo, true, false);

					if (projectile->getShooter() != NULL
						&& projectile->getShooter()->getVisualObject() != NULL)
					{
						projectile->getShooter()->getVisualObject()->setCollidable(true);
					}

					// restore collision check for those units that got collision disabled
					while (!disableCollUnits.isEmpty())
					{
						Unit *ownu = (Unit *)disableCollUnits.popLast();
						ownu->getVisualObject()->setCollidable(true);
					}

					LinkedListIterator biter2(blist);
					while (biter2.iterateAvailable())
					{
						Building *b = (Building *)biter2.iterateNext();
						if (b->getVisualObject() != NULL)
							b->getVisualObject()->setCollidable(true);
					}
					if (gravPath)
					{
						// HACK: some mysterious crap values here..
						// (8 tick lifetime, as long as um... within map?)
						if (game->gameMap->isWellInScaledBoundaries(orig.x, orig.z))
							projectile->setLifeTime(8);
					}
					if (cinfo.hit)
					{
						if (cinfo.hitUnit || gravPath)
						{
							// TEMP HACK!!!
							// (should calc new lifetime (/direct path)
							//projectile->setDirectPath(projectile->getPosition(), cinfo.position, projectile->getBulletType()->getVelocity());

							if (ltime > 1)
							{
								VC3 hitDiff = cinfo.position - projectile->getPosition();
								float distToHit = hitDiff.GetLength();
								ltime = (int)(distToHit / projectile->getBulletType()->getVelocity());
								//assert(ltime <= projectile->getLifeTime());
								if (ltime > projectile->getLifeTime())
									ltime = projectile->getLifeTime();
#ifndef PROJECT_SHADOWGROUNDS
// I don't think 0 is an acceptable lifetime... thus this (though not for sg, to prevent breaking it)
if (ltime < 1) ltime = 1;
#endif
								projectile->setLifeTime(ltime);
								projectile->setDestination(cinfo.position);
							}

							if (cinfo.hitUnit)
							{
								projectile->setHitTarget(cinfo.unit, NULL);
								if (cinfo.unit->getUnitType()->isMetallic())
									projectile->setChain(HITCHAIN_METALLIC);
#ifndef PROJECT_SHADOWGROUNDS
// this was missing from sg - it's an error, but to make sure nothing breaks, keeping it that way with sg
								else if (cinfo.unit->getUnitType()->isType2())
									projectile->setChain(HITCHAIN_UNIT_TYPE2);
#endif
								else
									projectile->setChain(HITCHAIN_UNIT);
							} else {
								projectile->setHitTarget(NULL, NULL);
								// TODO: appropriate material... also, buildings and stuff???
								projectile->setChain(HITCHAIN_TERRAINOBJECT);
							}

							// just making sure the split path won't be split again =P
							projectile->setCurrentSplitPosition(999);
						}
					}
				}
			}


			VC3 projpos = projectile->getPosition();
			VC3 projvel = projectile->getVelocity();
			projpos += projvel;

			if (!game->gameMap->isInScaledBoundaries(projpos.x, projpos.z))
			{
				// out of map. just make it disappear.
				game->projectiles->removeProjectile(projectile);
				delete projectile;
				return;
			}

			// parabolic path?
			if (projectile->getParabolicPathHeight() != 0
				&& projectile->getOriginalLifeTime() > 2)
			{
				// y = y - H * t / T*T
				projpos.y -= projectile->getParabolicPathHeight() 
					* ((float)(projectile->getOriginalLifeTime() - ltime * 2) / 2.0f 
					/ (float)projectile->getOriginalLifeTime())
					/ (float)projectile->getOriginalLifeTime();
				projectile->setPosition(projpos);
			}

			// gravity?
			if (projectile->getBulletType() != NULL)
			{
				if (projectile->getBulletType()->getFlyPath() == Bullet::FLYPATH_GRAVITY)
				{
					bool hitGround = false;

					if (SimpleOptions::getBool(DH_OPT_B_GAME_SIDEWAYS))
					{
						projvel += projectile->getBulletType()->getPathGravity();
					} else {
#if defined(PROJECT_SHADOWGROUNDS) || defined(PROJECT_SURVIVOR)
						// stuck bullets don't fall
						if(!(projectile->getBulletType()->isSticky() && projectile->doesFollowOrigin()))
							projvel.y -= 0.0098f;
#else
						projvel += projectile->getBulletType()->getPathGravity();
#endif

						float mapY = game->gameMap->getScaledHeightAt(projpos.x, projpos.z);
						if (projpos.y < mapY)
						{
							hitGround = true;
							projpos.y = mapY;
							if(projectile->getBulletType()->isSticky())
							{
								// sticky bullets not so bouncy
								projvel = VC3(projvel.x * 0.6f, 0, projvel.z * 0.6f);
							}
							else
							{
								if (projvel.y < -0.09f)
								{
									projvel = VC3(projvel.x * 0.8f, -projvel.y * 0.4f, projvel.z * 0.8f);
								} else {
									projvel = VC3(projvel.x * 0.8f, 0.0f, projvel.z * 0.8f);
									//projvel = VC3(0,0,0);
								}
							}
						}
					}

					float velLen = projvel.GetLength();

					// stuck bullets don't collide
					if(!(projectile->getBulletType()->isSticky() && projectile->doesFollowOrigin())
						// remote explosive does not collide after triggering
						&& !(projectile->getBulletType()->isRemoteExplosive() && projectile->getLifeTime() <= 1))
					{
						VC3 dir = projvel;
						if (velLen > 0.0001f)
						{
							dir.Normalize();
							GameCollisionInfo cinfo;
							//VC3 raypos = projpos - (dir * 0.1f);
							VC3 raypos = projpos;

							if (projectile->getShooter() != NULL
								&& projectile->getShooter()->getVisualObject() != NULL)
							{
								projectile->getShooter()->getVisualObject()->setCollidable(false);
							}
							game->getGameScene()->rayTrace(raypos, dir, 1.9f, cinfo, true, false);
							if (projectile->getShooter() != NULL
								&& projectile->getShooter()->getVisualObject() != NULL)
							{
								projectile->getShooter()->getVisualObject()->setCollidable(true);
							}

							if (cinfo.hit && cinfo.range < 0.5f)
							{
								if (cinfo.hitUnit)
								{
									if (cinfo.unit != NULL 
										&& projectile->getShooter() != NULL
										&& cinfo.unit->getOwner() != projectile->getShooter()->getOwner()
										&& !cinfo.unit->getUnitType()->isMetallic()
										&& !cinfo.unit->isDestroyed())
									{
										if (ltime > 1) 
										{
#ifndef PROJECT_SHADOWGROUNDS
// this was missing from sg - it's an error, but to make sure nothing breaks, keeping it that way with sg
											projectile->setHitTarget(cinfo.unit, NULL);
											if (cinfo.unit->getUnitType()->isMetallic())
												projectile->setChain(HITCHAIN_METALLIC);
											else if (cinfo.unit->getUnitType()->isType2())
												projectile->setChain(HITCHAIN_UNIT_TYPE2);
											else
												projectile->setChain(HITCHAIN_UNIT);

											projectile->setHitTarget(cinfo.unit, NULL);

#endif
											projectile->setLifeTime(1);
										}
									}
								}

								if (!cinfo.hitGround && !cinfo.hitTerrainObject)
								{
									projpos = cinfo.position;
									projectile->setPosition(projpos);
								}

								VC3 hitPlaneNormal = cinfo.hitPlaneNormal;

								VC3 tmp = dir;
								float projected = hitPlaneNormal.GetDotWith(tmp);
								projvel = tmp - hitPlaneNormal * (2 * projected);

								if (projvel.GetSquareLength() > 0.00001f)
								{
									projvel.Normalize();
									if(projectile->getBulletType()->isSticky())
									{
										// sticky bullets not so bouncy
										projvel *= (velLen * 0.35f);
									}
									else
									{
										projvel *= (velLen * 0.65f);
									}

									// if hit ground, don't allow y-component going downward
									if (hitGround && projvel.y < 0.0f)
									{
										projvel.y = -projvel.y;
									}
								}
							}
						}
					}
					
					projectile->setVelocity(projvel);

					projectile->setDestination(projpos);

					/*
					// TEST
					{
						VC3 oldPos = projectile->getPosition();
						VC3 vel = projectile->getVelocity();
						VC3 pos = oldPos + vel;
						VC3 vel_mps = vel * (float)GAME_TICKS_PER_SECOND;
						float slowFactor = 0.80f;
						ParticleCollision pcoll(game->gameMap);
						bool collided = pcoll.getCollision(oldPos, pos, vel_mps, slowFactor);
						if (collided)
						{
							vel = vel_mps / (float)GAME_TICKS_PER_SECOND;
							projectile->setVelocity(vel);
							projectile->setPosition(pos);
						}
					}
					*/

					//float rot = projectile->getLifeTime() / projectile->getOriginalLifeTime();
					if (velLen > 0.1f)
					{
						float rot = (float)((int)(game->gameTimer * 5 * (velLen * 10.0f)) % 360);
						projectile->setRotation(rot, 0, 0);
						if (projectile->getVisualEffect() != NULL)
						{
							projectile->getVisualEffect()->setRotation(VC3(rot,0,0));
						}
					}
				} else {
					// new: non gravity pathtypes may have gravity too...
					VC3 pathgrav = projectile->getBulletType()->getPathGravity();
					if (pathgrav.x != 0 || pathgrav.y != 0 || pathgrav.z != 0)
					{
						projvel += projectile->getBulletType()->getPathGravity();

						projectile->setVelocity(projvel);
						//projectile->setDestination(projpos);

						if (projvel.GetSquareLength() > 0.0f)
						{
							float rot = util::PositionDirectionCalculator::calculateDirection(VC3(0,0,0), projvel);
							projectile->setRotation(0, rot, 0);
							if (projectile->getVisualEffect() != NULL)
							{
								projectile->getVisualEffect()->setRotation(VC3(0,rot,0));
							}
						}
					}
				}
			}

			// this old crap moved properly to proximity confs... (see below)
			/*
			// special case: mine
			// TODO: proper configuration key/value for this
			if (projectile->getBulletType() != NULL
				&& (projectile->getBulletType()->getPartTypeId() == 
					PARTTYPE_ID_STRING_TO_INT("B_MineA")))
//				|| projectile->getBulletType()->getPartTypeId() == 
//					PARTTYPE_ID_STRING_TO_INT("B_Grenl")
//				|| projectile->getBulletType()->getPartTypeId() == 
//					PARTTYPE_ID_STRING_TO_INT("B_Clust")))
			{
				if ((game->gameTimer & 15) == 0)
				{
					LinkedList *ulist = game->units->getAllUnits();
					LinkedListIterator uiter = LinkedListIterator(ulist);
					while (uiter.iterateAvailable())
					{
						Unit *u = (Unit *)uiter.iterateNext();
						if (projectile->getShooter() != NULL
							&& u->getOwner() != projectile->getShooter()->getOwner()
							&& !u->isDestroyed())
						{
							VC3 posDiff = u->getPosition() - projpos;
							if (posDiff.GetSquareLength() < 4 * 4)
							{
								if (ltime > 5) 
								{
									projectile->setLifeTime(5);
								}
							}
						}
					}
				}
			}
			*/

			if (projectile->getBulletType()->getProximityRange() > 0.0f)
			{
				if ((game->gameTimer & (projectile->getBulletType()->getProximityCheckRate()-1)) == 0)
				{
					float checkDist = projectile->getBulletType()->getProximityRange();
					float checkDistSq = checkDist * checkDist;

					LinkedList *ulist = game->units->getAllUnits();
					LinkedListIterator uiter = LinkedListIterator(ulist);
					while (uiter.iterateAvailable())
					{
						Unit *u = (Unit *)uiter.iterateNext();
						if (projectile->getShooter() != NULL
//							&& u->getOwner() != projectile->getShooter()->getOwner()
							&& game->isHostile(projectile->getShooter()->getOwner(), u->getOwner())
							&& !u->isDestroyed())
						{
							VC3 upos = u->getPosition();

							// HACK: instead of unit position, use a slightly higher one...
							if (u->getSpeed() == Unit::UNIT_SPEED_CRAWL)
								upos += u->getUnitType()->getAimHeightCrawling() * 0.5f;
							else
								upos += u->getUnitType()->getAimHeightStanding() * 0.5f;

							VC3 posDiff = upos - projpos;

							// HACK: using squeezed sphere to simulate a nearly cylinderic collision
							// (we don't want a fully cylinderic though (posDiff.y = 0))
							// notice, that multiplying the diff vector by 0.5 is correct to actually stretch the hit area up/downward.
							posDiff.y *= 0.5f;

							float scaleX = game->gameMap->getScaleX() / GAMEMAP_HEIGHTMAP_MULTIPLIER;
							float checkRad = (float)(u->getUnitType()->getCollisionCheckRadius() - 1) * scaleX;

							float sqrLength = posDiff.GetSquareLength() - checkRad * checkRad;
							if(sqrLength < 0)
								sqrLength = 0;

							if (sqrLength < checkDistSq)
							{
								// find the closest one
								checkDistSq = sqrLength;

								// which unit was hit
								{
									if (u->getUnitType()->isMetallic())
										projectile->setChain(HITCHAIN_METALLIC);
									else if (u->getUnitType()->isType2())
										projectile->setChain(HITCHAIN_UNIT_TYPE2);
									else
										projectile->setChain(HITCHAIN_UNIT);

									projectile->setHitTarget(u, NULL);
								}
								// where was hit
								{
									posDiff.Normalize();
									float diff = checkDistSq - sqrLength;
									projectile->setPosition(projpos + posDiff * sqrtf(diff));
								}
								// change lifetime
								if (ltime > projectile->getBulletType()->getProximityDropLifeTime()) 
								{
									projectile->setLifeTime(projectile->getBulletType()->getProximityDropLifeTime());
									projectile->setDestination(projectile->getPosition());
								}
							}
						}
					}
				}
			}


			projectile->setPosition(projpos);


			// Burn, baby, burn
			/*
			if((ltime % 80) == 40)
			{
			//if (projectile->getVisualType() == PROJECTILE_VIS_FLAME1)
			//{
				if (projectile->getBulletType() != NULL
					&& projectile->getBulletType()->getPartTypeId() == 
						PARTTYPE_ID_STRING_TO_INT("B_Flame1"))
				{
					Terrain *terrain = game->gameUI->getTerrain();
					assert(terrain);

					const Vector &position = projectile->getPosition();
					const Vector &direction = projectile->getVelocity();

					Vector2D paintPosition(position.x, position.z);
					Vector2D paintDirection(direction.x, direction.z);

					terrain->BurnLand(paintPosition, paintDirection, 100.f, projectile->getBulletType()->getTerrainBlendAmount(), projectile->getBulletType()->getTerrainBlendMax());
				}
			}
			*/

/*
			if (ltime >= 12 
				&& projectile->getVisualType() == PROJECTILE_VIS_FLAME1)
			{
				if ((ltime & 1) == 0)
				{
					VC3 vel = projectile->getVelocity();
					vel.x += 0.05f*(float)cos((float)(game->gameTimer % 360)*(3.1415f/180.0f));
					vel.z += 0.05f*(float)sin((float)(game->gameTimer % 360)*(3.1415f/180.0f));

					game->gameUI->getParticleManager()->spawnParticleSystem(
						ui::ParticleSystem::FLAME,
						projectile->getPosition() - projectile->getVelocity() * (float)(20 - projectile->getLifeTime()), 
						vel, 0);
				}
			}
*/

			// another quick haxor...
			if (projectile->getBulletType() != NULL
				&& projectile->getBulletType()->getPartTypeId() == 
					PARTTYPE_ID_STRING_TO_INT("B_HvRoc"))
			{
				if ((ltime & 1) == 0)
				{
					//VC3 invvel = -projectile->getVelocity();
					//invvel.Normalize();
					/*
					game->gameUI->getParticleManager()->spawnParticleSystem(
					ui::ParticleSystem::ROCKETTAIL,
					projectile->getPosition(), 
					VC3(0,0,0), 0);
					*/
				}
			}

		} else {
			if (projectile->getLifeTime() == 0)
			{
				// if we have damage range, damage all within it...

				int drange = projectile->getBulletType()->getDamageRange();
				if (drange > 0)
				{
					// TODO: optimize this structure.
					// iterate such units that are within ceratain area or something...
					// not all of them...
					//LinkedList *ulist = game->units->getAllUnits();
					//LinkedListIterator uiter = LinkedListIterator(ulist);
					
					// WARNING: some magic number here
					// (max. 35 meter radius for area damage checks)
					IUnitListIterator *uiter = game->units->getNearbyAllUnits(projectile->getPosition(), 35.0f);

					float drangeSq = (float)(drange * drange);

					int drangePlayer = projectile->getBulletType()->getPlayerDamageRange();
					float drangePlayerSq = drangeSq;
					if (drangePlayer != -1)
					{
						drangePlayerSq = (float)(drangePlayer * drangePlayer);
					}

					// HACK: (OLD STUFF) lowered damage range for units that are prone
					// 3 meters off from the range for crawlers...
					// (not below 3 meters though)
					float loweredDrangeSq;
					if (drange > 3)
					{
						if (drange > 6)
						{
							loweredDrangeSq = (float)((drange-3) * (drange-3));
						} else {
							loweredDrangeSq = (3.0f * 3.0f);
						}
					} else {
						loweredDrangeSq = drangeSq;
					}

					VC3 ppos = projectile->getPosition();
					while (uiter->iterateAvailable())
					{
						Unit *u = (Unit *)uiter->iterateNext();
						if (u->isActive()
							&& (!projectile->getBulletType()->doesNoSelfDamage()
								|| u != projectile->getShooter())
							&& (!projectile->getBulletType()->doesPoisonDamage()
								|| !u->getUnitType()->isMetallic())
							)
// FIXME: currently works as does no hostile damage!
// HACK: enemies only...
// DONE?
//							|| u->getOwner() != 1))
						{
							VC3 posdiff = (u->getPosition() + VC3(0,1,0)) - ppos;
							float psqlen = posdiff.GetSquareLength();

							int finaldrange = drange;
							float finaldrangeSq = drangeSq;
							if (u->getSpeed() == Unit::UNIT_SPEED_CRAWL)
							{
								finaldrangeSq = loweredDrangeSq;
							}
							// HACK: player may get different damage range...
							if (u->getOwner() == game->singlePlayerNumber)
							{
								if (drangePlayerSq != finaldrangeSq)
								{
									finaldrangeSq = drangePlayerSq;
									if (drangePlayer > 0)
									{
										finaldrange = drangePlayer;
									} else {
										finaldrange = 1;
									}
								}
							}

							if (psqlen < finaldrangeSq)
							{
								if (psqlen == 0)
								{
									posdiff.x += (float)((game->gameRandom->nextInt() % 3) - 1) / 10.0f;
									posdiff.z += (float)((game->gameRandom->nextInt() % 3) - 1) / 10.0f;
									if (posdiff.x == 0 && posdiff.z == 0)
									{
										posdiff.x = 1.0f;
									}
								}
								float dist = sqrtf(psqlen);
								posdiff.Normalize();
								posdiff *= (((float)finaldrange - dist) / (float)finaldrange);
								//posdiff.x = ((float)drange - posdiff.x) / drange;
								//posdiff.y = ((float)drange - posdiff.y) / drange;
								//posdiff.z = ((float)drange - posdiff.z) / drange;
								float damFactor = 1.0f - (dist / (float)finaldrange);
								// -10% damage for units that are prone...
								if (u->getSpeed() != Unit::UNIT_SPEED_CRAWL)
								{
									damFactor -= 0.1f;
									if (damFactor < 0.01f) damFactor = 0.01f;
								}
								doUnitHit(projectile, u, NULL, posdiff, damFactor, false);
							}
						}
					}

					delete uiter;
				}


				//bool doTerrainHole = false;
				//bool doTerrainBlend = false;
				float doBlendRadius = 0.0f;
				float doHoleDepth = 0.0f;
				float doHoleRadius = 0.0f;

				bool overrideHoleRadiusForImpulse = false;
				if (projectile->getBulletType() != NULL)
				{
					overrideHoleRadiusForImpulse = projectile->getBulletType()->doesOverridePhysicsImpulseRadius();
				}

				projectile->setLifeTime(-1);

				float blastHeight = 0;

				// can't be const because param 2 of doUnitHit is not const
				Unit *hitUnit = projectile->getHitUnit();

				// do hitmisses... (and noises too)
				doHitMisses(projectile, hitUnit);

				if (hitUnit != NULL)
				{
					// hit unit

					VC3 pushVect = projectile->getVelocity();
					if (pushVect.GetSquareLength() != 0)
					{
						pushVect.Normalize();
					}
					pushVect.y += 0.2f;

					// TODO: PUT THIS BACK WHEN BONE COLLISIONS WORK PROPERLY
					//Part *hitPart = projectile->getHitPart();
					Part *hitPart = NULL;

					// damage factor 50% - 100% based on shooter's distance.
					// push factor 50% - 100% based on shooter's distance.
					// if "radical distance ratio" 25% - 100%

					// FIXME!
					// TODO!
					// HACK: 15m distance considered a static limit.
					// should be based on weapons max range, but we don't have
					// that available. 

					float distFactor = 1.0f;
					VC3 distVec = hitUnit->getPosition() - projectile->getOrigin();
					float distLen = distVec.GetLength();
					if (projectile->getBulletType() != NULL
						&& projectile->getBulletType()->isRadicalDistanceRatio())
					{
						if (distLen < 15.0f)
						{
							distFactor -= 0.75f * (distLen / 15.0f);
						} else {
							distFactor = 0.25f; 
						}
					} else {
						if (distLen < 15.0f)
						{
							distFactor -= 0.5f * (distLen / 15.0f);
						} else {
							distFactor = 0.5f; 
						}
					}

					pushVect *= distFactor;

					doUnitHit(projectile, hitUnit, hitPart, 
						pushVect, distFactor, true);

					// we hit unit so do damage to terrain if close enough...
					if (projectile->getBulletType() != NULL)
					{
						VC3 hitPos = projectile->getPosition();
						blastHeight = hitPos.y - game->gameMap->getScaledHeightAt(hitPos.x, hitPos.z);
						if (blastHeight < 0) blastHeight = 0;

						doHoleDepth = projectile->getBulletType()->getTerrainHoleDepth() - blastHeight;
						if (doHoleDepth < 0) doHoleDepth = 0;
						if (blastHeight < 5)
							doHoleRadius = projectile->getBulletType()->getTerrainHoleRadius();
						else
							doHoleRadius = projectile->getBulletType()->getTerrainHoleRadius() - blastHeight;
						if (doHoleRadius < 0) doHoleRadius = 0;
						doBlendRadius = projectile->getBulletType()->getTerrainBlendRadius() - blastHeight;
						if (doBlendRadius < 0) doBlendRadius = 0;
					}

				} else {
					// hit something else...

					if (projectile->getChain() == HITCHAIN_GROUND)
					{
						// hit ground
						//createChainedProjectile(projectile, 
						//	projectile->getPosition(), projectile->getChain());
						VC3 gpos = projectile->getDestination();
						game->gameMap->keepWellInScaledBoundaries(&gpos.x, &gpos.z);
						gpos.y = game->gameMap->getScaledHeightAt(gpos.x, gpos.z);

						createChainedProjectile(projectile, 
							gpos, projectile->getChain(), projectile->getDirection());

						// we hit ground so do damage to terrain...
						if (projectile->getBulletType() != NULL)
						{
							doHoleDepth = projectile->getBulletType()->getTerrainHoleDepth();
							doHoleRadius = projectile->getBulletType()->getTerrainHoleRadius();
							doBlendRadius = projectile->getBulletType()->getTerrainBlendRadius();
						}
					} else {
						if (projectile->getChain() == HITCHAIN_BUILDING)
						{ 						
							// hit building
							createChainedProjectile(projectile, 
								projectile->getDestination(), projectile->getChain(), projectile->getDirection());
						}
						if (projectile->getChain() == HITCHAIN_TERRAINOBJECT
							|| (projectile->getChain() >= FIRST_HITCHAIN_MATERIAL
							&& projectile->getChain() <= LAST_HITCHAIN_MATERIAL))
						{
							// hit terrain object
							createChainedProjectile(projectile, 
								projectile->getDestination(), projectile->getChain(), projectile->getDirection());
						}
						if (projectile->getChain() == HITCHAIN_NOTHING)
						{
							// hit nothing
							createChainedProjectile(projectile, 
								projectile->getDestination(), projectile->getChain(), projectile->getDirection());
						}

						if (projectile->getBulletType() != NULL)
						{
							// do damage to terrain if close enought to ground...
							if (projectile->getBulletType()->getTerrainHoleDepth() > 0)
							{
								VC3 hitPos = projectile->getPosition();
								blastHeight = hitPos.y - game->gameMap->getScaledHeightAt(hitPos.x, hitPos.z);
								if (blastHeight < 0) blastHeight = 0;

								doHoleDepth = projectile->getBulletType()->getTerrainHoleDepth() - blastHeight;
								if (doHoleDepth < 0) doHoleDepth = 0;
								if (blastHeight < 5)
									doHoleRadius = projectile->getBulletType()->getTerrainHoleRadius();
								else
									doHoleRadius = projectile->getBulletType()->getTerrainHoleRadius() - blastHeight;
								if (doHoleRadius < 0) doHoleRadius = 0;
								doBlendRadius = projectile->getBulletType()->getTerrainBlendRadius() - blastHeight;
								if (doBlendRadius < 0) doBlendRadius = 0;
							}
						}
					}
				}

				std::vector<TerrainObstacle> removedObjects;
				std::vector<ExplosionEvent> objectEvents;

				// TERRAIN HOLES (HMAP DEFORMATIONS) NO LONGER SUPPORTED!
				/*
				if (doHoleDepth > 0)
				{
					// Do a hole on terrain
					Terrain *terrain = game->gameUI->getTerrain();
					assert(terrain);

					Vector position = projectile->getPosition();
					float mapHeight = game->gameMap->getScaledHeightAt(position.x, position.z);
					if (position.y < mapHeight) 
						position.y = mapHeight;
					const Vector &direction = projectile->getVelocity();
					
					Vector2D blastPosition(position.x, position.z);
					Vector2D blastDirection(direction.x, direction.z);

					// first raise the land a bit with bigger radius,
					// then lower it with exact radius -> we get crater "edges".
					if (projectile->getBulletType()->hasTerrainHoleEdges())
					{
						if (projectile->getBulletType()->getTerrainHoleType() == Bullet::TERRAIN_HOLE_TYPE_CIRCLE)
						{
							terrain->BlastHoleCircle(blastPosition, blastDirection, doHoleRadius * 1.2f, -doHoleDepth * 0.75f, removedObjects);
							terrain->BlastHoleCircle(blastPosition, blastDirection, doHoleRadius, doHoleDepth * 1.5f, removedObjects);
						}
						if (projectile->getBulletType()->getTerrainHoleType() == Bullet::TERRAIN_HOLE_TYPE_SPHERE)
						{
							//terrain->BlastHoleSphere(position, direction, doHoleRadius * 1.2f, -doHoleDepth * 0.75f);
							terrain->BlastHoleCircle(blastPosition, blastDirection, doHoleRadius * 1.2f, -doHoleDepth * 0.75f, removedObjects);
							terrain->BlastHoleSphere(position, direction, doHoleRadius, doHoleDepth * 1.2f, removedObjects);
						}
					} else {
						if (projectile->getBulletType()->getTerrainHoleType() == Bullet::TERRAIN_HOLE_TYPE_CIRCLE)
						{
							terrain->BlastHoleCircle(blastPosition, blastDirection, doHoleRadius, doHoleDepth, removedObjects);
						}
						if (projectile->getBulletType()->getTerrainHoleType() == Bullet::TERRAIN_HOLE_TYPE_SPHERE)
						{
							terrain->BlastHoleSphere(position, direction, doHoleRadius, doHoleDepth, removedObjects);
						}
					}
				} else {
					// no hole to ground (too high maybe)...
					// still, blast away terrain objects only without making hole.
					if (projectile->getBulletType() != NULL
						&& projectile->getBulletType()->getTerrainHoleRadius() > 0)
					{
						Terrain *terrain = game->gameUI->getTerrain();
						const Vector &position = projectile->getPosition();
						const Vector2D blastPosition(position.x, position.z);
						terrain->BlastTerrainObjects(blastPosition, 
							projectile->getBulletType()->getTerrainHoleRadius(), 
							removedObjects, blastHeight);
					}
				}
				*/

				bool hitBreakableObject = false;
				VC3 projPos = projectile->getPosition();
				if (game->gameMap->isWellInScaledBoundaries(projPos.x, projPos.z))
				{
					int ox = game->gameMap->scaledToObstacleX(projPos.x);
					int oy = game->gameMap->scaledToObstacleY(projPos.z);
					if (game->gameMap->getAreaMap()->isAreaValue(ox, oy, AREAMASK_BREAKABLE, AREAVALUE_BREAKABLE_YES))
						hitBreakableObject = true;
				}

#ifndef PHYSICS_NONE
				//PSDHAX
				hitBreakableObject = true;
#endif

				// NOW, JUST DOING SOME OBJECT BREAKING...
				if (doHoleRadius > 0 && !projectile->getBulletType()->doesPoisonDamage()
					&& projectile->getBulletType() != NULL
					&& !projectile->getBulletType()->doesNoTerrainObjectBreaking())
				{
					bool only_breaktexture = true;
					if (projectile->getBulletType()->getTerrainObjectDamageProbability() >= 100
						|| (game->gameRandom->nextInt() % 100) < projectile->getBulletType()->getTerrainObjectDamageProbability())
					{
						only_breaktexture = false;
					}
					Terrain *terrain = game->gameUI->getTerrain();
					const Vector &position = projectile->getPosition();
					//const Vector2D blastPosition(position.x, position.z);
					terrain->BreakTerrainObjects(position, projectile->getVelocity(), 
						projectile->getBulletType()->getTerrainHoleRadius(), 
						removedObjects, objectEvents, projectile->getBulletType()->getTerrainObjectRadiusDamageAmount(), false, only_breaktexture);
					terrain->BlastTerrainObjects(position, 
						projectile->getBulletType()->getTerrainHoleRadius(), 
						removedObjects, blastHeight);

					if (!overrideHoleRadiusForImpulse)
					{
						terrain->physicsImpulse(position, projectile->getVelocity(), 
							projectile->getBulletType()->getTerrainHoleRadius(), projectile->getBulletType()->getPhysicsImpulseFactor() * push_factor, false);
					}
			
					if(game && game->getGameUI() && game->getGameUI()->getVisualEffectManager() && game->getGameUI()->getVisualEffectManager()->getParticleEffectManager())
					{
						game->getGameUI()->getVisualEffectManager()->getParticleEffectManager()->addPhysicsExplosion(position, 1.f);
					}
				}

				if (overrideHoleRadiusForImpulse)
				{
					if (projectile->getBulletType()->getPhysicsImpulseRadius() > 0)
					{
						Terrain *terrain = game->gameUI->getTerrain();
						const Vector &position = projectile->getPosition();
						terrain->physicsImpulse(position, projectile->getVelocity(), 
							projectile->getBulletType()->getPhysicsImpulseRadius(), projectile->getBulletType()->getPhysicsImpulseFactor() * push_factor, false);	

						if(game && game->getGameUI() && game->getGameUI()->getVisualEffectManager() && game->getGameUI()->getVisualEffectManager()->getParticleEffectManager())
						{
							game->getGameUI()->getVisualEffectManager()->getParticleEffectManager()->addPhysicsExplosion(position, projectile->getBulletType()->getPhysicsImpulseFactor() * push_factor, projectile->getBulletType()->getPhysicsImpulseRadius());
						}
					}
				}

				else if ((projectile->getChain() == HITCHAIN_TERRAINOBJECT
					|| (projectile->getChain() >= FIRST_HITCHAIN_MATERIAL
						&& projectile->getChain() <= LAST_HITCHAIN_MATERIAL))
					&& hitBreakableObject
					&& projectile->getBulletType() != NULL
					&& !projectile->getBulletType()->doesNoTerrainObjectBreaking())
				//else if (hitBreakableObject)
				{
					if (projectile->getBulletType()->getHPDamage() > 0
						&& !projectile->getBulletType()->doesPoisonDamage())
					{
						bool only_breaktexture = true;
						if (projectile->getBulletType()->getTerrainObjectDamageProbability() >= 100
							|| (game->gameRandom->nextInt() % 100) < projectile->getBulletType()->getTerrainObjectDamageProbability())
						{
							only_breaktexture = false;
						}
						Terrain *terrain = game->gameUI->getTerrain();
						Vector position = projectile->getPosition();
						//const Vector2D blastPosition(position.x, position.z);
						if(projectile->getBulletType()->getFlyPath() == Bullet::FLYPATH_RAY)
						{
							position = projectile->getDestination();
						}
						terrain->BreakTerrainObjects(position, projectile->getVelocity(), 
							0.4f, 
							removedObjects, objectEvents, projectile->getBulletType()->getTerrainObjectDirectDamageAmount(), true, only_breaktexture);
					}
				}

#ifndef PHYSICS_NONE
				if (projectile->getChain() == HITCHAIN_TERRAINOBJECT
					|| (projectile->getChain() >= FIRST_HITCHAIN_MATERIAL
					&& projectile->getChain() <= LAST_HITCHAIN_MATERIAL)
					|| hitBreakableObject)
				{
					const Vector &position = projectile->getPosition();
					Terrain *terrain = game->gameUI->getTerrain();
					terrain->physicsImpulse(position, projectile->getVelocity(),  0.4f, projectile->getBulletType()->getPhysicsImpulseFactor() * push_factor, true);
				}
#endif

				ProjectileActor::handleTerrainBreaking(game, removedObjects, objectEvents);

				/*
				bool dontBlend = false;
				if (projectile->getBulletType() != NULL
					&& projectile->getBulletType()->getPartTypeId() == 
						PARTTYPE_ID_STRING_TO_INT("B_Flame1"))
					dontBlend = true;

				if (doBlendRadius > 0 && !dontBlend)
				{
					// Explosion damage to terrain, color blending
					Terrain *terrain = game->gameUI->getTerrain();
					assert(terrain);

					const Vector &position = projectile->getPosition();
					Vector2D blastPosition(position.x, position.z);
			
					terrain->BlendDamage(blastPosition, doBlendRadius, projectile->getBulletType()->getTerrainBlendAmount(), projectile->getBulletType()->getTerrainBlendMax());
				}
				*/
			}
			if (projectile->getAfterLifeTime() > 0)
			{
				// lifetime exhausted, but still has some afterlife left :)
				// (even though the projectile is now dead when considering the
				// gameplay, we still want it to be visible a while, thus 
				// we have an afterlife.)
				projectile->setAfterLifeTime(projectile->getAfterLifeTime() - 1);

				// special case: laser and xenon beam fadeout
				// TODO: proper configuration key/value for this
				if (projectile->getBulletType() != NULL
					&& (projectile->getBulletType()->getPartTypeId() == 
						PARTTYPE_ID_STRING_TO_INT("B_Xenon1")
						|| projectile->getBulletType()->getPartTypeId() == 
							PARTTYPE_ID_STRING_TO_INT("B_Laser2")))
				{
					int origAfterLife = projectile->getBulletType()->getAfterLifeTime();
					if (origAfterLife >= 1) 
					{
						float visRatio = (float)(projectile->getAfterLifeTime()) / (float)(origAfterLife);
						if (projectile->getVisualEffect() != NULL
							&& projectile->getVisualEffect() != NULL)
						{
							projectile->getVisualEffect()->setObjectVisibility(visRatio);
						}
					}				
				}


			} else {
				// we're done. lifetime and afterlife used up...
				// delete the projectile.
				game->projectiles->removeProjectile(projectile);
				delete projectile;
			}
		}
	}



	// makes some gore effect based on bullet's/unit's random gore probabilities...
	bool ProjectileActor::doGore(Projectile *projectile, Unit *hitUnit, bool onlyPartial, float probabilityFactor, int additionalProbability)
	{
		bool did_gore = false;

		if (projectile->getBulletType() != NULL)
		{
			for (int gor = 0; gor < GORETYPE_AMOUNT; gor++)
			{
				int prob = (int)((projectile->getBulletType()->getGoreProbability(gor) + additionalProbability) * probabilityFactor);
				if (prob > 0 && hitUnit->getUnitType()->getGoreAmount(gor) > 0)
				{
					// in some cases we want to support only the partial gore effects
					// (the ones that do not completely remove (explode away) the unit)
					if (goreTypeRemovesOrigin[gor] && onlyPartial)
					{
						prob = 0;
					}

					if ((game->gameRandom->nextInt() % 100) < prob)
					{
						// gore effect.
						// throw the unit somewhere away...
						VC3 upos = hitUnit->getPosition();
						VC3 urot = hitUnit->getRotation();

						did_gore = true;

						UnitActor *ua = getUnitActorForUnit(hitUnit);
						ua->removeUnitObstacle(hitUnit);

						const char *effname = NULL;
						const char *goresnd = NULL;
						const char *gorepart = NULL;
						if (hitUnit->getUnitType()->getGoreAmount(gor) > 0)
						{
							int num = (game->gameRandom->nextInt() % hitUnit->getUnitType()->getGoreAmount(gor));
							effname = hitUnit->getUnitType()->getGore(gor, num);
							goresnd = hitUnit->getUnitType()->getGoreSound(gor, num);
							gorepart = hitUnit->getUnitType()->getGorePart(gor, num);
						}

						if (goresnd != NULL && goresnd[0] != '\0')
						{
							// note: normal death sound will be played too, but it will
							// be somewhere far away from map area due to gore warp (i think).
							game->gameUI->playSoundEffect(goresnd, upos.x, upos.y, upos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, DEFAULT_SOUND_RANGE, DEFAULT_SOUND_PRIORITY_NORMAL);
							
						}

						if (effname != NULL && effname[0] != '\0')
						{
							int visualType = game->gameUI->getVisualEffectManager()->getVisualEffectIdByName(effname);

							if (visualType >= 0) {
								VisualEffect *ve =
									game->gameUI->getVisualEffectManager()->createNewManagedVisualEffect(
									visualType, GAME_TICKS_PER_SECOND * 2, NULL, NULL, upos, upos, urot, VC3(0,0,0), game);

								if (ve != NULL)
								{
									if(projectile->getBulletType() && projectile->getBulletType()->getDamageRange() > 0)
										ve->setParticleExplosion(projectile->getPosition(), true);
									else
										ve->setParticleExplosion(projectile->getPosition(), false);
								}
							}
						}

						if (goreTypeRemovesOrigin[gor])
						{
							hitUnit->setPosition(VC3(-999123,-99999,-99999));
							// or maybe? game->deleteVisualOfParts(hitUnit, hitUnit->getRootPart());

							if (gorepart != NULL && gorepart[0] != '\0')
							{
								LOG_WARNING_W_DEBUG("ProjectileActor::doGore - Gore has gore part defined but removes origin model.", gorepart);
							}
						} else {
							if (gorepart != NULL && gorepart[0] != '\0')
							{
								/*int dummy1 = 0;
								int dummy2 = 0;
								GameScriptData gsd;
								gsd.unit = hitUnit;
								game->gameScripting->runSingleCommand(GS_CMD_ADDUNITROOTPART, 0, gorepart, &dummy1, &dummy2, &gsd);*/
								PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(gorepart));
								if (pt != NULL)
								{
									game->deleteVisualOfParts(hitUnit, hitUnit->getRootPart());
									if(hitUnit->getRootPart() != NULL)
									{
										game->detachParts(hitUnit, hitUnit->getRootPart());
									}
									Part *part = pt->getNewPartInstance();
									part->setOwner(hitUnit->getOwner());
									hitUnit->setRootPart(part);
									game->createVisualForParts(hitUnit, hitUnit->getRootPart());
								}
							}
						}
					}
				}
			}
		}

		return did_gore;
	}


	void ProjectileActor::doHitMisses(Projectile *projectile, 
		const Unit *hitUnit)
	{
		Unit *shooter = projectile->getShooter();

		if (shooter == NULL) return;

		int shooterOwner = shooter->getOwner();
		VC3 shooterPos = shooter->getPosition();

		VC3 projPos = projectile->getPosition() - shooterPos;
		float projPosLenSq = projPos.GetSquareLength();
		VC3 projPosNorm = projPos.GetNormalized();

		// hitunit is ignored, but other nearby units get "missed".
		//LinkedList *ulist = game->units->getAllUnits();
		//inkedListIterator iter = LinkedListIterator(ulist);

		VC3 radiusPos = (projectile->getPosition() + shooterPos) / 2.0f;
		// WARNING: some magic number here
		// (max. 35 meter radius for hit miss checks)
		IUnitListIterator *iter = game->units->getNearbyAllUnits(radiusPos, 35.0f);

		while (iter->iterateAvailable())
		{
			Unit *u = (Unit *)iter->iterateNext();
			if (u->isActive() && !u->isDestroyed()
				&& game->isHostile(u->getOwner(), shooterOwner)
				&& u != hitUnit && u != shooter)
			{
				// a "pipelike" area check from shooter to hit position...

				// TODO: 
				// should use dot product / cross product maybe instead
				// to avoid the nasty sqrtf at length call.

				VC3 upos = u->getPosition() - shooterPos;
				float uposLen = upos.GetLength();

				VC3 hitAndUnitDiff = upos - projPos;
				float hitAndUnitDiffLenSq = hitAndUnitDiff.GetSquareLength();
				
				int damRange = (PROJECTILE_HIT_NOISE_MIN_RADIUS / PROJECTILE_HIT_NOISE_RADIUS_FACTOR); 
				if (projectile->getBulletType() != NULL)
					damRange = projectile->getBulletType()->getDamageRange();
				damRange *= PROJECTILE_HIT_NOISE_RADIUS_FACTOR;
				float noiseRadiusSq = (float)(damRange * damRange);

				// hear it if either near shooter or near hit position
				if (uposLen < PROJECTILE_HIT_NOISE_MIN_RADIUS
					|| hitAndUnitDiffLenSq < noiseRadiusSq)
				{
					u->setHearNoiseByUnit(shooter);
				}

				if (uposLen * uposLen < projPosLenSq)
				{
					VC3 pipedPos = projPosNorm * uposLen;
					VC3 posdiff = upos - pipedPos;
					// 5 meter radius
					if (posdiff.GetSquareLength() < PROJECTILE_MISS_RADIUS * PROJECTILE_MISS_RADIUS)
					{
						u->setHitMissByUnit(shooter);
					}
				}
			}
		}

		delete iter;
	}


	// hitUnit is not const
	void ProjectileActor::doUnitHit(Projectile *projectile, 
		Unit *hitUnit, Part *hitPart, VC3 &pushVector, float damageFactor,
		bool directHit)
	{
		bool runHitScript = true;

#ifdef PROJECT_CLAW_PROTO
		// if unit is a cop + projectile's origin unit is a cop, don't do a hit
		if( projectile && projectile->getShooter())
		{
			if( hitUnit->getUnitType()->getUnitTypeId() == 455611187 &&
				projectile->getShooter()->getUnitType()->getUnitTypeId() == 455611187 )
			{
				return;
			}
		}
#endif

		// special case, this weapon won't wake up the enemies
		//if (projectile->getBulletType()->getVisualEffectNumber()
		//	== PROJECTILE_VIS_GLOWFLARE)
		if (projectile->getBulletType() != NULL
			&& projectile->getBulletType()->getPartTypeId() == 
				PARTTYPE_ID_STRING_TO_INT("B_GFlare"))
		{
			runHitScript = false;
		}

		if (hitUnit->isDestroyed())
		{
			runHitScript = false;
		}

		if (directHit)
		{
			//if (projectile->getBulletType()->getFlyPath() == Bullet::FLYPATH_RAY)
			//{
				// ray creates the chained bullet to target units position
			//	createChainedProjectile(projectile, 
			//		hitUnit->getPosition(), projectile->getChain(), VC3(0,0,0));
			//} else {
				// others to current projectiles position
				Projectile *proj = createChainedProjectile(projectile, 
					projectile->getDestination(), projectile->getChain(), projectile->getDirection());

				// stick new projectile to hitUnit
				if(proj != NULL && proj->getBulletType()->isSticky())
				{
					proj->setFollowOrigin(true);
					proj->setOriginUnit(hitUnit);
					VC3 dir = projectile->getPosition() - hitUnit->getPosition();
					dir.Normalize();
					QUAT q;
					q.MakeFromAxisRotation(VC3(0,-1,0), UNIT_ANGLE_TO_RAD(hitUnit->getRotation().y));
					q.Inverse();
					q.RotateVector(dir);
					proj->setOriginOffset(dir * 0.35f * ((float)hitUnit->getUnitType()->getCollisionCheckRadius() - 1));
				}
			//}
		} else {
			// indirect (ranged) hit.
			VC3 dir = hitUnit->getPosition() - projectile->getPosition();
			if(dir.GetSquareLength() > 0.00001f)
			{
				dir.Normalize();
			} else {
				dir = VC3(0,1,0);
			}

			int chaintype = HITCHAIN_INDIRECT;

			if (hitUnit->getUnitType()->isMetallic())
			{
				chaintype = HITCHAIN_INDIRECT_METALLIC;
			}
			else if (hitUnit->getUnitType()->isType2())
			{
				chaintype = HITCHAIN_INDIRECT_TYPE2;
			}

			createChainedProjectile(projectile, 
				hitUnit->getPosition(), chaintype, dir, hitUnit);
		}

		if (projectile->getBulletType() != NULL
			&& projectile->getBulletType()->getDelayedHitProjectileBullet() != NULL
			&& projectile->getBulletType()->getDelayedHitProjectileAmount() > 0)
		{
			// HACK: special cases where no delayed damage to player
			if (hitUnit->getOwner() != game->singlePlayerNumber
				|| !projectile->getBulletType()->hasNoDelayedHitProjectileForPlayer())
			{
				hitUnit->setDelayedHitProjectileBullet(projectile->getBulletType()->getDelayedHitProjectileBullet());
				hitUnit->setDelayedHitProjectileAmount(projectile->getBulletType()->getDelayedHitProjectileAmount());
				hitUnit->setDelayedHitProjectileInterval(projectile->getBulletType()->getDelayedHitProjectileInterval());
			}
		}

		bool poison_damage = projectile->getBulletType() != NULL
												 && projectile->getBulletType()->doesPoisonDamage();

		if (hitUnit->isImmortal() ||
			   // ignore poison damage altogether?
			  (hitUnit->getPoisonResistance() >= 1.0f && poison_damage))
		{
			if(hitUnit->isImmortal() && hitUnit->isImmortalWithHitScript() && runHitScript)
			{
				hitUnit->setHitByUnit(projectile->getShooter(), projectile->getBulletType());
			}
			// hack: immortal units update timer too
			hitUnit->setLastTimeDamaged(game->gameTimer);
			return;
		}

		// unit is using shield
		if(hitUnit->isShielded())
		{
			// get angle
			VC2 vel(projectile->getVelocity().x, projectile->getVelocity().z);
			VC2 dir(-sinf(UNIT_ANGLE_TO_RAD(hitUnit->getRotation().y)),
				      -cosf(UNIT_ANGLE_TO_RAD(hitUnit->getRotation().y)));
			vel.Normalize();
			float cos_angle = dir.GetDotWith(vel);

			// angle is greater than 90 deg
			if(cos_angle < 0.0f)
			{
				// no damage
				return;
			}
		}

		// HACK: player cannot damage friendlys/npcs
		if (projectile->getShooter() != NULL
			&& projectile->getShooter()->getOwner() == 0
			&& (hitUnit->getOwner() == 3 || hitUnit->getOwner() == 2)
			&& !hitUnit->getUnitType()->isMetallic()
			&& !hitUnit->getUnitType()->isType2())
		{
			return;
		}

		VC3 originalPushVector = pushVector;

		bool fallByPushVector = false;
		bool highImpact = false;
		bool mediumImpact = false;

		// push the unit if the bullet says so...
		// and not under player's direct control
		if (!hitUnit->getUnitType()->isStationary()
			&& hitUnit->getDirectControlType() != Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER
// TEMP DEMO HACK: push only hostiles
			&& hitUnit->getOwner() == 1)
		{
			float impactPush = 0;

			if (projectile->doesInflictDamage())
				impactPush = projectile->getBulletType()->getImpactPush();
			if (impactPush > 0)
			{
				if (!directHit)
					fallByPushVector = true;
				pushVector *= impactPush;
				if (hitUnit->isDestroyed() 
					|| hitUnit->getSpeed() == Unit::UNIT_SPEED_CRAWL
					|| hitUnit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
				{
					pushVector *= 0.5f;
				}
				// the actual push velocity depends on mass...
				// 100kg means factor 1, any bigger will get less velocity,
				// any smaller will get more velocity (200kg = 0.5x, 1kg = 100x)
				if (pushVector.GetSquareLength() > 0.1f * 0.1f)
				{
					highImpact = true;
				} else {
					mediumImpact = true;
				}

				assert(hitUnit->getWeight() > 0);
				float divisor = (float(hitUnit->getWeight()) / 100.0f);
				assert(divisor > 0.0001f && divisor < 10000.0f);

				pushVector /= divisor;
				// need to "detach" sticky units from ground...
				if (hitUnit->getUnitType()->isSticky())
				{
					VC3 upos = hitUnit->getPosition();
					upos.y += 0.05f;
					hitUnit->setPosition(upos);
					hitUnit->setOnGround(false);
					hitUnit->setGroundFriction(false);
				}

				pushVector += hitUnit->getVelocity();
				// HACK: limit max upward velocity
				if (pushVector.y > 0.20f)
					pushVector.y = 0.20f;


#ifdef PROJECT_SURVIVOR
				float max_speed = hitUnit->getUnitType()->getMaxSpeed() / GAME_TICKS_PER_SECOND;
				if(fabsf(pushVector.x) > max_speed)
				{
					pushVector.x = pushVector.x < 0.0f ? -max_speed : max_speed;
				}
				if(fabsf(pushVector.y) > max_speed)
				{
					pushVector.y = pushVector.y < 0.0f ? -max_speed : max_speed;
				}
				if(fabsf(pushVector.z) > max_speed)
				{
					pushVector.z = pushVector.z < 0.0f ? -max_speed : max_speed;
				}
#endif
				hitUnit->setVelocity(pushVector);
			}
		}

		int subPartNum = -1;
		int destroyedCriticalParts = 0;

		if (hitPart == NULL) 
		{
			// no hit part defined, choose root part or one of its sub parts
			// it should always be defined though (?)
			hitPart = hitUnit->getRootPart();
			if (hitPart != NULL)
			{
				// torso has 2x much chance of getting hit than other parts
				int randNum = game->gameRandom->nextInt() 
					% (hitPart->getType()->getSlotAmount() + 2);
				if (randNum < hitPart->getType()->getSlotAmount())
				{
					subPartNum = randNum;
					if (hitPart->getSubPart(randNum) != NULL)
						hitPart = hitPart->getSubPart(randNum);
				}
			}
		}

		Part *rootp = hitUnit->getRootPart();
		int slotAmount = rootp->getType()->getSlotAmount();
		for (int i = 0; i < slotAmount; i++)
		{
			int slotPos = rootp->getType()->getSlotPosition(i);
			if (slotPos == SLOT_POSITION_HEAD
				|| slotPos == SLOT_POSITION_LEFT_ARM
				|| slotPos == SLOT_POSITION_RIGHT_ARM
				|| slotPos == SLOT_POSITION_LEFT_LEG
				|| slotPos == SLOT_POSITION_RIGHT_ARM)
			{
				if (rootp->getSubPart(i) != NULL
					&& rootp->getSubPart(i)->getDamage() 
					>= rootp->getSubPart(i)->getType()->getMaxDamage())
				{
					destroyedCriticalParts++;
				}
			}
			if (subPartNum == -1)
			{
				if (rootp->getSubPart(i) == hitPart)
				{
					subPartNum = i;
				}
			}
		}

		if (hitPart != NULL)
		{
			// solve impact direction...
			VC2 impactDir;
			if (fallByPushVector)
			{
				impactDir = VC2(pushVector.x, pushVector.z);
			} else {
				VC3 vel = projectile->getVelocity();
				impactDir = VC2(vel.x, vel.z);
			}
			float impactDirFloat = impactDir.CalculateAngle();
			float dir = -RAD_TO_UNIT_ANGLE(impactDirFloat) + (360+270);
			while (dir >= 360) dir -= 360;

			float rotSpeed = 0;
			VC3 urotation = hitUnit->getRotation();

			rotSpeed = util::AngleRotationCalculator::getRotationForAngles(urotation.y, dir, 90);

			bool hitBack;
			if (rotSpeed == 0)
				hitBack = true;
			else
				hitBack = false;

			// if we got surprised from behind (or actually just shot into back)
			// does not apply to friendly (own) fire.
			float surpriseFactor = 1.0f;
			if (hitBack && directHit 
				&& (projectile->getShooter() != NULL
				&& hitUnit->getOwner() != projectile->getShooter()->getOwner()))
			{
				// HACK: player 0 (human) gets only 1.5x damage, other 2x
#if defined(PROJECT_SHADOWGROUNDS)
				if (hitUnit->getOwner() == 0)
					surpriseFactor = 1.5f;
				else
					surpriseFactor = 2.0f;
#else
				surpriseFactor = hitUnit->getUnitType()->getBackDamageFactor();
#endif
			}

			
			// DISABLED!!!
			// NO MORE PART DAMAGE!!!

			/*
			//float heatFactor = 1.0f + (float)hitUnit->getHeat() / (float)hitUnit->getMaxHeat();
			float heatFactor = 1.0f;
			int heatadd = 0;

			if (projectile->doesInflictDamage())
			{
				// damage the hit part...
				heatadd = projectile->getBulletType()->createDamageTo(hitPart, 
					heatFactor * damageFactor * surpriseFactor);
				heatadd = 0;
				int dmg = projectile->getBulletType()->getHPDamage() - hitUnit->calculateArmorRating();
				dmg = (int)((float)dmg * damageFactor);
				if (dmg < 1) dmg = 1;

				hitPart->addDamage(dmg);

				// other parts get a bit damage too...
				Part *rootp = hitUnit->getRootPart();
				int slotAmount = rootp->getType()->getSlotAmount();

				assert(hitPart->getType()->getMaxDamage() != 0);
				int hitPartDamageRatio = 100 * hitPart->getDamage() / hitPart->getType()->getMaxDamage();
		
				for (int i = 0; i < slotAmount; i++)
				{
					// if other part in better shape than this part, damage it too
					// if it's already in worse shape, don't damage
					// (this will equalize the parts' damages a bit)
					Part *otherPart = rootp->getSubPart(i);
					if (otherPart != NULL
						&& otherPart != hitPart
						&& 100 * otherPart->getDamage() / otherPart->getType()->getMaxDamage()
							>= hitPartDamageRatio)
					{
						// one fifth (25%) of hit parts damage amount to other parts too
						int otherDmg = dmg / 4;
						if (otherDmg < 1) otherDmg = 1;
						otherPart->addDamage(otherDmg);
					}
				}

				// heat up, damage if max heat.
				if (hitUnit->getHeat() + heatadd <= hitUnit->getMaxHeat())
				{
					hitUnit->setHeat(hitUnit->getHeat() + heatadd);
				} else {
					hitUnit->setHeat(hitUnit->getMaxHeat());
					// TODO: blow it up!
					// now, we'll just hack it like this...
					hitPart->addDamage(10);
				}
			}
			*/

			bool isCriticalPart = false;	// arms, legs, etc.
			bool isVeryCriticalPart = false; // head or torso
			if (hitPart == hitUnit->getRootPart()) 
			{
				isCriticalPart = true;
				isVeryCriticalPart = true;
			}
			if (hitPart->getParent() == hitUnit->getRootPart()
				&& subPartNum != -1)
			{
				int slotPos = hitPart->getParent()->getType()->getSlotPosition(subPartNum);
				if (slotPos == SLOT_POSITION_HEAD)
				{
					isCriticalPart = true;
					isVeryCriticalPart = true;
				}
				if (slotPos == SLOT_POSITION_LEFT_ARM
					|| slotPos == SLOT_POSITION_RIGHT_ARM
					|| slotPos == SLOT_POSITION_LEFT_LEG
					|| slotPos == SLOT_POSITION_RIGHT_ARM)
				{
					isCriticalPart = true;
				}
			}

			game->gameUI->setUnitDamagedFlag(hitUnit->getOwner());
			hitUnit->setLastTimeDamaged(game->gameTimer);

			bool fallDown = false;	// if unconscious or dying
			int hpDamage = 0;

			if (!hitUnit->isDestroyed())
			{
				float jumpFactor = 1.0f;
				// HACK: jump attacks do 50% extra damage.
				if (projectile->getShooter() != NULL
					&& projectile->getShooter()->getSpeed() == Unit::UNIT_SPEED_JUMP)
				{
					jumpFactor = 1.5f;
				}

				hpDamage = projectile->getBulletType()->getHPDamage()
					- hitUnit->calculateArmorRating();
				hpDamage = (int)((float)hpDamage * damageFactor * surpriseFactor * jumpFactor);

#ifdef PROJECT_SURVIVOR
				// HP is below gaining limit
				if(hitUnit->getHPGainLimit() > 0 && hitUnit->getHP() < hitUnit->getMaxHP() * hitUnit->getHPGainLimit())
				{
					// scale damage
					hpDamage = (int)((float)hpDamage * hitUnit->getHPGainDamageFactor());
				}
#endif

				if (hitUnit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER
					|| hitUnit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_REMOTE_PLAYER)
				{
					float playerDamageRatio = game->getDifficultyManager()->getPlayerDamageRatio();
					if (projectile->getBulletType() != NULL
						&& projectile->getBulletType()->hasNoDifficultyEffectOnDamageAmount())
					{
						// "normal difficulty level damage"
						playerDamageRatio = 0.75f;
					}
					hpDamage = (int)(hpDamage * playerDamageRatio * projectile->getBulletType()->getPlayerDamageFactor());
				}

				// electric/stun paralyze time
				int paralyzeTime = 0;

				// heat damage makes some little burning effects...
				int heatDam = projectile->getBulletType()->getHitDamage(DAMAGE_TYPE_HEAT);
				if (heatDam > 0 && !hitUnit->getUnitType()->isVehicle())
				{
					if (!hitUnit->getUnitType()->doesDisableEffectLayer())
					{
						// crispy look... (but not for player)
						if (hitUnit->getDirectControlType() != Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER
							&& hitUnit->getDirectControlType() != Unit::UNIT_DIRECT_CONTROL_TYPE_REMOTE_PLAYER)
						{
							if (hitUnit->getBurnedCrispyAmount() < 100)
							{
								int crispPerc = 100;
								if (hitUnit->getMaxHP() > 1)
									crispPerc = 100 * 100 / hitUnit->getMaxHP();
								int newCrisp = hitUnit->getBurnedCrispyAmount() + (7 * crispPerc / 100);
								if (newCrisp > 100)
									newCrisp = 100;
								hitUnit->setBurnedCrispyAmount(newCrisp);
							}
						}


						if (SimpleOptions::getInt(DH_OPT_I_LAYER_EFFECTS_LEVEL) >= 50)
						{
							if (hitUnit->getUnitEffectLayerType() != Unit::UNIT_EFFECT_LAYER_BURNING
								|| hitUnit->getUnitEffectLayerDuration() < (heatDam * 2) / 3 / GAME_TICK_MSEC)
							{
								hitUnit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_BURNING, heatDam / GAME_TICK_MSEC);

								assert(hitUnit->isActive());

								game->deleteVisualOfParts(hitUnit, hitUnit->getRootPart(), true);
								game->createVisualForParts(hitUnit, hitUnit->getRootPart(), true);
							}
						}
					}
				}

				// electric damage "paralyzes" for a while...
				int electricDam = projectile->getBulletType()->getHitDamage(DAMAGE_TYPE_ELECTRIC);
				if (electricDam > 0 && !hitUnit->getUnitType()->isVehicle())
				{
					if (hitUnit->getUnitType()->doesReactToStunningWeapons())
					{
						if (paralyzeTime < electricDam / GAME_TICK_MSEC)
						{
							paralyzeTime = electricDam / GAME_TICK_MSEC;
							hitUnit->setMoveState(Unit::UNIT_MOVE_STATE_ELECTRIFIED);
						}
					}

					if (SimpleOptions::getInt(DH_OPT_I_LAYER_EFFECTS_LEVEL) >= 25)
					{
						bool alreadyVisualized = false;
						if (hitUnit->getUnitEffectLayerType() == Unit::UNIT_EFFECT_LAYER_ELECTRIC
							&& hitUnit->getUnitEffectLayerDuration() > 0)
						{
							alreadyVisualized = true;
						}
						hitUnit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_ELECTRIC, electricDam / GAME_TICK_MSEC);

						assert(hitUnit->isActive());

						if (!alreadyVisualized)
						{
							game->deleteVisualOfParts(hitUnit, hitUnit->getRootPart(), true);
							game->createVisualForParts(hitUnit, hitUnit->getRootPart(), true);
						}
					}
				}
				// stun damage "paralyzes" as well as electric, but no effect for this one.
				// NOTE: will not work properly together with electrified damage
				// (use only one of these for a single weapon)
				int stunDam = projectile->getBulletType()->getHitDamage(DAMAGE_TYPE_STUN);
				if (stunDam > 0)
				{
					if (paralyzeTime < stunDam / GAME_TICK_MSEC)
					{
						paralyzeTime = stunDam / GAME_TICK_MSEC;
						if(hitUnit->getUnitType()->doesReactToStunningWeapons())
						{
							hitUnit->setMoveState(Unit::UNIT_MOVE_STATE_STUNNED);
						}
					}

					assert(hitUnit->isActive());
				}

				if (paralyzeTime > 0)
				{
					if (hitUnit->getUnitType()->doesReactToStunningWeapons())
					{
						for (int w = 0; w < UNIT_MAX_WEAPONS; w++)
						{
							if (hitUnit->getWeaponType(w) != NULL
								&& hitUnit->getWeaponFireTime(w) > 1)
							{
								hitUnit->setWeaponFireTime(w, 1);
							}
						}

						hitUnit->setMoveStateCounter(paralyzeTime);
						hitUnit->targeting.clearTarget();
						//hitUnit->setWalkDelay(paralyzeTime);
						hitUnit->setWalkDelay(0);
						for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
						{
							if (hitUnit->getWeaponType(i) != NULL)
							{
								if (hitUnit->getFireReloadDelay(i) < paralyzeTime)
								{
									hitUnit->setFireReloadDelay(i, paralyzeTime);
								}
							}
						}
					}
				}

				/*
				// handled elsewhere.. for area damage
				if (projectile->getBulletType()->doesPoisonDamage()
					&& hitUnit->getUnitType()->isMetallic())
				{
					hpDamage = 1;
				}
				*/

				if (hpDamage < 1) hpDamage = 1;

				if (!projectile->doesInflictDamage())
					hpDamage = 0;

				// hacked for survivor, the poison resistance unit type parameter
				if( projectile->getBulletType()->doesPoisonDamage() )
				{
					hpDamage = (int)( ( (float)hpDamage * ( 1.0f - hitUnit->getPoisonResistance() ) ) + 0.5f );
				}

				bool force_gore = false;

#ifdef PROJECT_SURVIVOR
				// hacked for survivor, the critical hit thingie
				if( projectile )
				{
					// no critical hits from poison or heat damage
					bool allow_crithit = !projectile->getBulletType()->doesPoisonDamage()
						&& projectile->getBulletType()->getHitDamage(DAMAGE_TYPE_HEAT) == 0;

					// no critical hits from human players friendly fire (except when shooting self)
					if(projectile->getShooter() && hitUnit
						&& projectile->getShooter()->getOwner() == 0
						&& hitUnit->getOwner() == 0
						&& projectile->getShooter() != hitUnit)
					{
						allow_crithit = false;
					}

					if(!projectile->getBulletType()->allowCriticalHits())
					{
						allow_crithit = false;
					}

					// read percentage from bullet
					float percent = projectile->getBulletType()->getCriticalHitPercent();

					// if set, the units percentage overrides the one set in bullet
					if( projectile->getShooter() && projectile->getShooter()->getCriticalHitPercent() >= 0.0f)
					{
						percent = projectile->getShooter()->getCriticalHitPercent();
					}

					percent *= projectile->criticalHitProbabilityMultiplier;

					// if target is locked, always give critical hit
					if( hitUnit->getTargetLockCounter() > 0 && hitUnit->getTargetLockCounter() == hitUnit->getTargetLockCounterMax() )
					{
						percent = 1.0f;
					}

					// same if projectile is stuck
					if(projectile->getForceGoreExplosionUnit() == hitUnit)
					{
						percent = 1.0f;
					}

					if( allow_crithit && ((float)( game->gameRandom->nextInt() % 100 ) / 100.0f ) < percent  )
					{
						float damage_factor = projectile->criticalHitDamageMultiplier;
						int max_damage = projectile->criticalHitDamageMax;
						bool hit = false;

						// when hitting a player, don't increase damage over 40hp
						if (hitUnit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER)
						{
							max_damage = 40 * 2;
						}

						// (if damage is already over max, then ignore critical hit)
						if(hpDamage < max_damage)
						{
							hpDamage = (int)(hpDamage * damage_factor);
							if(hpDamage > max_damage)
								hpDamage = max_damage;
							hit = true;
						}

						if(hit)
							force_gore = true;

						// hacking the critical hit message thing
						std::string message = "gui_ingame_critical_hit";
						int player = 0;
						if( hit && message.empty() == false && game && game->gameUI && game->gameUI->getCombatWindow( player ) &&
							game->gameUI->getCombatWindow( player )->getSubWindow( "TargetDisplayWindow" ) )
						{
							((TargetDisplayWindowUpdator*)game->gameUI->getCombatWindow( player )->getSubWindow( "TargetDisplayWindow" ))->risingMessage( hitUnit, message );
							// game->gameUI->getCombatWindow( player )->
						}
						
						// Logger::getInstance()->error( "Critical Hit" );
					}
				}
#endif

				int newHP = hitUnit->getHP() - hpDamage;
				if (newHP < hitUnit->getUnitType()->getMinHP()) 
					newHP = hitUnit->getUnitType()->getMinHP();

				if (hitUnit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER)
				{
					if (SimpleOptions::getBool(DH_OPT_B_PLAYER_DEMO_INVULNERABILITY))
					{
						if (newHP < 25)
							newHP = 25;
					}
				}

				if (projectile->getBulletType()->getSlowdown() != 0)
				{
					// HACK: 1 poison does more slowdown damage if unit has no target...
					// (effectively allowing "poison-surprises" (otherwise the enemies would just
					// immediately rush to kill player).
					// (HACK TODO: 2 poison slows down only some of the enemies, it makes others 
					// go crazy...)
					if (!projectile->getBulletType()->doesNoPlayerSlowdown()
						|| (hitUnit->getDirectControlType() != Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER
						&& hitUnit->getDirectControlType() != Unit::UNIT_DIRECT_CONTROL_TYPE_REMOTE_PLAYER))
					{
						if (projectile->getBulletType()->doesPoisonDamage())
						{
							if (!hitUnit->targeting.hasTarget())
							{
								hitUnit->addSlowdown((float)projectile->getBulletType()->getSlowdown() * 2 / 100.0f);
							}
						}
						hitUnit->addSlowdown((float)projectile->getBulletType()->getSlowdown() / 100.0f);
					}


					if (projectile->getBulletType()->doesPoisonDamage())
					{
						// TODO: poison effect (something greenish?)
					} else {
						if (hitUnit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER
							|| hitUnit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_REMOTE_PLAYER)
						{
							if (SimpleOptions::getInt(DH_OPT_I_LAYER_EFFECTS_LEVEL) >= 25)
							{
								// spitworm spit effect (slowdown)
								// NOTE: some interesting constant value here :)
								hitUnit->setUnitEffectLayerType(Unit::UNIT_EFFECT_LAYER_SLIME, 10000 / GAME_TICK_MSEC);

								assert(hitUnit->isActive());

								game->deleteVisualOfParts(hitUnit, hitUnit->getRootPart(), true);
								game->createVisualForParts(hitUnit, hitUnit->getRootPart(), true);
							}
						} else {
							// TODO: wrencher, etc. tranquilizer effect
						}

					}
				}

				// If damage occured to the 1st player, send an effect to Forcewear.
				if( hitUnit->getOwner() == 0 )
				{
					unsigned int ef = projectile->getBulletType()->getForcewearEffect();
					unsigned int side;

					if(hitBack)
					{
						side = FW_BACK;
					}
					else
					{
						side = FW_FRONT;
					}

					Forcewear::SendEffect ( ef, side );
				}
				
				hitUnit->setHP(newHP);

				if (newHP <= 0) 
				{
					fallDown = true;
				}

				if (newHP <= hitUnit->getUnitType()->getMinHP()
					|| ((isVeryCriticalPart || (isCriticalPart && destroyedCriticalParts >= 1))
					&& hitPart->getDamage() >= hitPart->getType()->getMaxDamage()))
				{
					fallDown = true;
					hitUnit->setHP(hitUnit->getUnitType()->getMinHP());
					hitUnit->setDestroyed(true);

					bool onlyPartial = false;
					if (!directHit && damageFactor < 0.5f)
					{
						onlyPartial = true;
					}

					int prob = 0;
					if(force_gore)
					{
						prob = 100;
					}

					bool did_gore = doGore(projectile, hitUnit, onlyPartial, 1.0f, prob);

					// stop any running animation for this unit.
					if (did_gore && hitUnit->isAnimated())
					{
						AniManager::getInstance()->stopAniForUnit(hitUnit);
					}
				}
			} else {
				// SPECIAL CASE FOR EXPLODE GORE:
				// was already destroyed, but dead ones can be blown up too...
				if (projectile->getBulletType() != NULL)
				{
					if (projectile->getBulletType()->getGoreProbability(GORETYPE_EXPLODE) > 0 
						&& hitUnit->getUnitType()->getGoreAmount(GORETYPE_EXPLODE) > 0
						&& (directHit || damageFactor > 0.5f))
					{
						doGore(projectile, hitUnit, false, 2.0f, 0);
					}
				}
			}

			if (hitUnit->getHitAnimationCounter() > hitUnit->getUnitType()->getHitAnimationTime() / GAME_TICK_MSEC * 2 / 4)
			{
				// already starting / well on the way with a hit animation, don't restart...
				//hitUnit->setHitAnimationCounter(hitUnit->getUnitType()->getHitAnimationTime() / GAME_TICK_MSEC * 3 / 4);
			} else {
				// special hack for shotty and other multi-projectile hits...
				if (hitUnit->getHitAnimationCounter() > hitUnit->getUnitType()->getHitAnimationTime() / GAME_TICK_MSEC * 7 / 8)
				{
					if (damageFactor < 0.5f)
						hitUnit->setHitAnimationFactor(damageFactor * 2);
					else
						hitUnit->setHitAnimationFactor(1.0f);
				} else {
					hitUnit->setHitAnimationFactor(damageFactor);
				}

				hitUnit->setHitAnimationCounter(hitUnit->getUnitType()->getHitAnimationTime() / GAME_TICK_MSEC);
				//hitUnit->setHitAnimationVector(originalPushVector);

				VC3 animDir = hitUnit->getPosition() - projectile->getPosition();
				if(animDir.GetLength() > 0.1f)
				{
					animDir.Normalize();
					hitUnit->setHitAnimationVector(animDir);
				} else {
					hitUnit->setHitAnimationVector(originalPushVector);
				}

			}

			if (fallDown)
			{

#ifdef PROJECT_CLAW_PROTO
//				hitUnit->setPhysicsObjectLock( true );
//				hitUnit->setPhysicsObjectFeedbackEnabled( true );
				// If falls down from physics impact, let the physics object
				// drag the unit for a while, at least.
				if (projectile->getBulletType() != NULL
					&& projectile->getBulletType()->getPartTypeId() == 
						PARTTYPE_ID_STRING_TO_INT("B_Phys2"))
				{
					// disabled for now: friction doesn't apply for physics object for some reason.
//					hitUnit->setPhysicsObjectLock( true );
				}
#endif

				if (hitUnit->getUnitType()->getAimBone() != NULL
					&& !hitUnit->getUnitType()->isVehicle())
				{
					if (hitUnit->getVisualObject() != NULL)
					{
						hitUnit->getVisualObject()->releaseRotatedBone(hitUnit->getUnitType()->getAimBone());
					}
				}

				if (hitUnit->getMoveState() != Unit::UNIT_MOVE_STATE_UNCONSCIOUS)
				{
					bool fallFront = hitBack;

					int anim;
					if (hitUnit->getSpeed() == Unit::UNIT_SPEED_CRAWL)
					{
						anim = ANIM_DIE_PRONE;
					} else {
						if (highImpact)
						{
							bool fallFrontAnim = fallFront;
#ifndef PROJECT_CLAW_PROTO
							// Falling animation should be determinstic in Claw.
							if ((game->gameRandom->nextInt() & 7) == 0)
							{
								fallFrontAnim = !fallFront;
							}
#endif
							if (fallFrontAnim)
							{
								anim = ANIM_DIE_IMPACT_FRONT;
							} else {
								anim = ANIM_DIE_IMPACT_BACK;
							}
							if (fallFront)
							{
								urotation.y = dir;
								hitUnit->setMoveState(Unit::UNIT_MOVE_STATE_IMPACT_FORWARD);
								hitUnit->setMoveStateCounter(100);
							} else {
								urotation.y = dir + 180;
								if (urotation.y >= 360) urotation.y -= 360;
								hitUnit->setMoveState(Unit::UNIT_MOVE_STATE_IMPACT_BACKWARD);
								hitUnit->setMoveStateCounter(100);
							}
							hitUnit->setRotation(urotation.x, urotation.y, urotation.z);
						} else {
							// old stuff for randomizing back&front deaths...
							//if ((game->gameRandom->nextInt() & 3) == 0)
							//{
							//	fallFront = !fallFront;
							//}
							if (fallFront)
							{
								anim = ANIM_DIE_FRONT;
							} else {
								anim = ANIM_DIE_BACK;
							}
						}
					}

					// poison death anim?
					if (projectile->getBulletType() != NULL
						&& projectile->getBulletType()->doesPoisonDamage())
					{
						if (hitUnit->getAnimationSet() != NULL)
						{
							if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_DIE_POISON)
								&& anim != ANIM_DIE_PRONE)
							{
								anim = ANIM_DIE_POISON;
							}
						}
					}

#ifdef PROJECT_CLAW_PROTO
				VC3 unitPos = hitUnit->getPosition();
				float mapY = game->gameMap->getScaledHeightAt(unitPos.x, unitPos.z);
				if(fabs( unitPos.y - mapY ) >= 0.05f )
				{
					anim = ANIM_SPECIAL12 ;
				}
#endif

					if (projectile->getBulletType() != NULL
						&& projectile->getBulletType()->getHitDamage(DAMAGE_TYPE_HEAT) > 0)
					{
						if (hitUnit->getAnimationSet() != NULL)
						{
							if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_DIE_HEAT)
								&& anim != ANIM_DIE_PRONE)
							{
								anim = ANIM_DIE_HEAT;
								if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_DIE_HEAT2))
								{
									if ((game->gameRandom->nextInt() & 1) == 0)
									{
										anim = ANIM_DIE_HEAT2;
									}
								}
							}
						}
					}

					if (hitUnit->getAnimationSet() != NULL)
					{
						if (hitUnit->getAnimationSet()->isAnimationInSet(anim))
						{
							// new randomization to front/back (w/ impact) death anims...
							if (anim == ANIM_DIE_BACK)
							{
								if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_DIE_BACK2))
								{
									if ((game->gameRandom->nextInt() & 1) == 0)
									{
										anim = ANIM_DIE_BACK2;
									}
								}
							}
							else if (anim == ANIM_DIE_FRONT)
							{
								if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_DIE_FRONT2))
								{
									if ((game->gameRandom->nextInt() & 1) == 0)
									{
										anim = ANIM_DIE_FRONT2;
									}
								}
							}
							else if (anim == ANIM_DIE_IMPACT_BACK)
							{
								if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_DIE_IMPACT_BACK2))
								{
									if ((game->gameRandom->nextInt() & 1) == 0)
									{
										anim = ANIM_DIE_IMPACT_BACK2;
									}
								}
							}
							else if (anim == ANIM_DIE_IMPACT_FRONT)
							{
								if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_DIE_IMPACT_FRONT2))
								{
									if ((game->gameRandom->nextInt() & 1) == 0)
									{
										anim = ANIM_DIE_IMPACT_FRONT2;
									}
								}
							}

							hitUnit->getAnimationSet()->animate(hitUnit, anim);

							if (anim == ANIM_DIE_BACK || anim == ANIM_DIE_IMPACT_BACK
								|| anim == ANIM_DIE_BACK2 || anim == ANIM_DIE_IMPACT_BACK2)
							{
								hitUnit->setFallenOnBack(true);
							} else {
								hitUnit->setFallenOnBack(false);
							}
						} else {
							hitUnit->setFallenOnBack(false);
							if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_DIE))
							{
								hitUnit->getAnimationSet()->animate(hitUnit, ANIM_DIE);
							} else {
								hitUnit->getAnimationSet()->animate(hitUnit, ANIM_NONE);
							}
						}
					}

					for (int i = 0; i < UNIT_MAX_BLEND_ANIMATIONS; i++)
					{
						if (hitUnit->getBlendAnimation(i) != 0)
						{
							Animator::endBlendAnimation(hitUnit, i, true);
						}
					}

					// unnecessary check
					//if (newHP < 0)
					//{
					hitUnit->setMoveState(Unit::UNIT_MOVE_STATE_UNCONSCIOUS);
					hitUnit->setMoveStateCounter(0);
					//}

					if (hitUnit->isSelected())
					{
						game->unitSelections[hitUnit->getOwner()]->selectUnit(
							hitUnit, false);
					}

					if (hitUnit->isDestroyed())
					{
						game->gameUI->setUnitDestroyedFlag(hitUnit->getOwner());

						if (projectile->getBulletType() != NULL
							&& projectile->getBulletType()->doesPoisonDamage())
						{
							// TODO: poison death stuff?
							hitUnit->setDiedByPoison(true);
						} else {
							int heatDam = 0;
							int electricDam = 0;
							if (projectile->getBulletType() != NULL)
							{
								heatDam = projectile->getBulletType()->getHitDamage(DAMAGE_TYPE_HEAT);
								electricDam = projectile->getBulletType()->getHitDamage(DAMAGE_TYPE_ELECTRIC);
							}
							if (heatDam == 0 && electricDam == 0)
							{
								hitUnit->setDeathBleedDelay(1);
							}
						}

						// spawn another projectile based on unit's destroy bullet
						//UnitType *ut = getUnitTypeById(hitUnit->getUnitTypeId());
						UnitType *ut = hitUnit->getUnitType();
						assert(ut != NULL);
						Bullet *unitExplosionBullet = ut->getExplosionBullet();
						if (unitExplosionBullet != NULL)
						{
							Projectile *expproj = new Projectile(NULL, unitExplosionBullet);
							game->projectiles->addProjectile(expproj);

							expproj->setDirectPath(hitUnit->getPosition(), hitUnit->getPosition(), 
								unitExplosionBullet->getVelocity());

							ProjectileActor pa = ProjectileActor(game);
							pa.createVisualForProjectile(expproj);
						}

						// make death sound
						if (projectile->getBulletType() != NULL
							&& projectile->getBulletType()->doesPoisonDamage())
						{
							// TODO: poison death sound?
						} else {
							char *deathSound = ut->getExplosionSound();
							if (deathSound != NULL)
							{
								VC3 pos = hitUnit->getPosition();
								game->gameUI->playSoundEffect(deathSound, pos.x, pos.y, pos.z, false, DEFAULT_SOUND_EFFECT_VOLUME, DEFAULT_SOUND_RANGE, DEFAULT_SOUND_PRIORITY_NORMAL);
							}
						}
					}

				}
			} else {

#ifdef PROJECT_CLAW_PROTO
				//hitUnit->setPhysicsObjectLock( true );
#endif

				if (mediumImpact && !hitUnit->isDestroyed()
					&& hitUnit->getHP() > 0
					&& hitUnit->getMoveState() == Unit::UNIT_MOVE_STATE_NORMAL)
				{
					if ((hitUnit->getAnimationSet() != NULL
						&& (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_STAGGER_BACKWARD)
						|| hitUnit->getAnimationSet()->isAnimationInSet(ANIM_STAGGER_FORWARD)))
						&& (hitUnit->getDirectControlType() != Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER
						&& hitUnit->getDirectControlType() != Unit::UNIT_DIRECT_CONTROL_TYPE_REMOTE_PLAYER))
					{
						if ((game->gameRandom->nextInt() & 7) == 0)
						{
							bool fallFront = hitBack;

							if (fallFront)
							{
								if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_STAGGER_FORWARD))
								{
									if (hitUnit->getAnimationSet() != NULL)
									{
										hitUnit->getAnimationSet()->animate(hitUnit, ANIM_STAGGER_FORWARD);
									}
									hitUnit->setMoveState(Unit::UNIT_MOVE_STATE_STAGGER_FORWARD);
									hitUnit->setMoveStateCounter(100);
								}
							} else {
								if (hitUnit->getAnimationSet()->isAnimationInSet(ANIM_STAGGER_BACKWARD))
								{
									if (hitUnit->getAnimationSet() != NULL)
									{
										hitUnit->getAnimationSet()->animate(hitUnit, ANIM_STAGGER_BACKWARD);
									}
									hitUnit->setMoveState(Unit::UNIT_MOVE_STATE_STAGGER_BACKWARD);
									hitUnit->setMoveStateCounter(100);
								}
							}
						}
					}
				}
			} // end of if(fallDown) stuff

			// flashlight impact effect...
			if (hitUnit->getFlashlight() != NULL)
			{
				// TODO: value from options.
				hitUnit->getFlashlight()->setImpact(SimpleOptions::getFloat(DH_OPT_F_FLASHLIGHT_IMPACT_FACTOR));
			}

			// player hit effects
			if (hitUnit->getDirectControlType() == Unit::UNIT_DIRECT_CONTROL_TYPE_LOCAL_PLAYER)
			{
				if (projectile->getBulletType() != NULL
					&& projectile->getBulletType()->doesPoisonDamage())
				{
					if (hpDamage > 0)
					{
						if (hpDamage > PLAYER_BIG_POISON_LIMIT)
						{
							game->gameUI->getEffects()->startFlashEffect(UIEffects::FLASH_EFFECT_TYPE_PLAYER_POISON_BIG);
						} else {
							if (hpDamage > PLAYER_MEDIUM_POISON_LIMIT)
							{
								game->gameUI->getEffects()->startFlashEffect(UIEffects::FLASH_EFFECT_TYPE_PLAYER_POISON_MEDIUM);
							} else {
								game->gameUI->getEffects()->startFlashEffect(UIEffects::FLASH_EFFECT_TYPE_PLAYER_POISON_SMALL);
							}
						}
					}
				} else {
					if (hitUnit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS
						|| hitUnit->isDestroyed())
					{
						game->gameUI->getEffects()->startFlashEffect(UIEffects::FLASH_EFFECT_TYPE_PLAYER_HIT_BIG);
					} else {
						if (hpDamage > 0)
						{
							if (hpDamage > PLAYER_BIG_HIT_LIMIT)
							{
								game->gameUI->getEffects()->startFlashEffect(UIEffects::FLASH_EFFECT_TYPE_PLAYER_HIT_BIG);
							} else {
								if (hpDamage > PLAYER_MEDIUM_HIT_LIMIT)
								{
									game->gameUI->getEffects()->startFlashEffect(UIEffects::FLASH_EFFECT_TYPE_PLAYER_HIT_MEDIUM);
								} else {
									game->gameUI->getEffects()->startFlashEffect(UIEffects::FLASH_EFFECT_TYPE_PLAYER_HIT_SMALL);
								}
							}
						}
					}
				}
			}

		} 

		if (runHitScript)
		{
			// don't run the hitscript on oneself.
			// except when damaged by poison
			if (hitUnit != projectile->getShooter() || projectile->getBulletType()->doesPoisonDamage())
			{
				// this means the script will be run on next ai run
				hitUnit->setHitByUnit(projectile->getShooter(), projectile->getBulletType());
			}
		}
	}



	Projectile *ProjectileActor::createChainedProjectile(Projectile *projectile, 
		const VC3 &position, int hitchain, const VC3 &direction, Unit *indirectHitUnit)
	{
		Bullet *chainBullet = 
			projectile->getBulletType()->getChainBullet(hitchain);

		Projectile *cproj = NULL;

		// create new projectile
		if (projectile->getBulletType()->getChainScript(hitchain) != NULL) 
		{
			Unit *hitUnit = projectile->getHitUnit();
			if (indirectHitUnit != NULL)
			{
				fb_assert(hitchain == HITCHAIN_INDIRECT
					|| hitchain == HITCHAIN_INDIRECT_TYPE2
					|| hitchain == HITCHAIN_INDIRECT_METALLIC);
				fb_assert(hitUnit == NULL);
				hitUnit = indirectHitUnit;
			}

			game->gameScripting->runHitChainScript(projectile->getBulletType()->getChainScript(hitchain),
				projectile, hitUnit, projectile->getShooter(), 
				chainBullet, position, hitchain, direction, projectile->getHitNormal());
				projectile->getBulletType()->getChainBullet(hitchain);

		} else {
			if (chainBullet != NULL)
			{
				// TODO: base these on the chainscript
				cproj = new Projectile(projectile->getShooter(), chainBullet);
				//Projectile *cproj = new Projectile(NULL, chainBullet);
				game->projectiles->addProjectile(cproj);

				// stuck bullet
				if(projectile->getBulletType()->isSticky() && projectile->getOriginUnit())
				{
					// force explosion on stuck unit
					cproj->setForceGoreExplosionUnit(projectile->getOriginUnit());
				}

				cproj->setPosition(position);
				// to get correct lifetime...
				float vel = chainBullet->getVelocity();
				if (vel < 0.01f) vel = 0.01f;
				cproj->setDirectPath(position, position + (direction * vel), 0.0f);
				//chainBullet->getVelocity()

				ProjectileActor pa = ProjectileActor(game);
				pa.createVisualForProjectile(cproj);
			}
		}

		// SHOULD THIS BE HANDLED INDIVIDUALLY FOR EACH CHAIN SPAWN... ???

		// make sound
		if (projectile->doesHitSound())
		{
			int chainSoundAmount = 
				projectile->getBulletType()->getChainSoundAmount(hitchain);
			
			if (chainSoundAmount > 0)
			{
				int snum = (game->gameRandom->nextInt() % chainSoundAmount);
				int priority = projectile->getBulletType()->getChainSoundPriority(hitchain);
				projectile->getBulletType()->getChainSoundRange(hitchain);

				const char *chainSound =			 
					projectile->getBulletType()->getChainSound(hitchain, snum);

				if (chainSound != NULL && chainSound[0] != '\0')
				{
					if ((game->gameRandom->nextInt() % 100)
						< projectile->getBulletType()->getChainSoundProbability(hitchain))
						game->gameUI->playSoundEffect(chainSound, position.x, position.y, position.z, false, DEFAULT_SOUND_EFFECT_VOLUME, DEFAULT_SOUND_RANGE, priority);
				}
			}
		}

		return cproj;
	}


	void ProjectileActor::getCollisionDisableList(Unit *shooter, LinkedList &noCollUnits,
		const VC3 &weaponPosition)
	{
		Unit *unit = shooter;

		bool friendly_fire = SimpleOptions::getBool(DH_OPT_B_GAME_FRIENDLY_FIRE);

		// OLD: disable collision check for nearby own units...
		// NEW: disable collision for nearby own and allied units...
		// except if the own unit happens to be the target.
		if (unit != NULL)
		{
			//LinkedList *ownulist = game->units->getOwnedUnits(unit->getOwner());

#ifdef PROJECT_CLAW_PROTO
			{
				// don't let cops to kill each other	
				LinkedList *unitList = game->units->getAllUnits();
				LinkedListIterator unitListIter = LinkedListIterator(unitList);
				while (unitListIter.iterateAvailable())
				{
					Unit *unit = (Unit *)unitListIter.iterateNext();
					if( unit->getUnitTypeId() == shooter->getUnitTypeId() )
					{
						noCollUnits.append(unit);
					}
				}
			}
#endif

			for (int ally = 0; ally < ABS_MAX_PLAYERS; ally++)
			{
				// HACK: assuming player is 0 and ally is 3. (thus, possible ally combos are 3,0 or 0,3)
				// MEGA HACK: player gets no damage from arwyn
				if (ally == unit->getOwner()
					|| (ally == 0 && unit->getOwner() == 3)
					|| (ally == 3 && unit->getOwner() == 0)
					|| (ally == 0 && unit->getIdString() != NULL && strcmp(unit->getIdString(), "arwyn") == 0))
				{
#if defined(PROJECT_SHADOWGROUNDS)
					// WARNING: some magic number here (range)
					// collect units within max. 30 meters range.
					IUnitListIterator *ownUnitsIter = game->units->getNearbyOwnedUnits(ally, weaponPosition, 30.0f);
#else
					IUnitListIterator *ownUnitsIter = game->units->getNearbyOwnedUnits(ally, weaponPosition, UNITACTOR_DONT_HIT_FRIENDLY_RANGE);
#endif

					//LinkedList *ownulist = game->units->getOwnedUnits(ally);
					//LinkedListIterator ownUnitsIter = LinkedListIterator(ownulist);

					// hack: when friendly fire is allowed, no player owned units are
					// included in here, except for the unit itself
					bool ignore_all_but_self = friendly_fire && unit->getOwner() == 0 && ally == 0;

					while (ownUnitsIter->iterateAvailable())
					{
						Unit *ownu = (Unit *)ownUnitsIter->iterateNext();
						
						if(ignore_all_but_self && ownu != unit) continue;

						if (ownu->isActive())
						{
							VC3 posdiff = ownu->getPosition() - weaponPosition;
							// x meters safe distance inside which we won't hit own units
							if (posdiff.GetSquareLength() < UNITACTOR_DONT_HIT_FRIENDLY_RANGE * UNITACTOR_DONT_HIT_FRIENDLY_RANGE)
							{
								//if (unit->targeting.getTargetUnit() != ownu)
								//{
									noCollUnits.append(ownu);
									ownu->getVisualObject()->setCollidable(false);
								//}
							}
						}
					}

					delete ownUnitsIter;
				}
			}
		}
		// TEMP:
		// HACK: disable collision for destroyed units!
		LinkedList *deadlist = game->units->getAllUnits();
		LinkedListIterator deadUnitsIter = LinkedListIterator(deadlist);
		while (deadUnitsIter.iterateAvailable())
		{
			Unit *deadu = (Unit *)deadUnitsIter.iterateNext();
			if (deadu->isActive() && deadu->isDestroyed())
			{
				// NEW: only units that have been destroyed for a few seconds or so?
				if (deadu->getDestroyedTime() > PROJECTILE_IGNORE_DESTROYED_AFTER)
				{
					noCollUnits.append(deadu);
					deadu->getVisualObject()->setCollidable(false);
				}
			}
		}
	}


	void ProjectileActor::doProjectileRaytrace(Unit *shooter, Unit *noCollisionUnit, Projectile *projectile,
		Bullet *bulletType, const VC3 &_weaponPosition, const VC3 &weaponRayPosition, const VC3 &targetPosition,
		const VC3 &direction, float maxRange, Unit *targetUnit, float velocityFactor)
	{
		Unit *unit = shooter; // lazy me... don't want to replace. :)

		VC3 weaponPosition = _weaponPosition;

		VC3 target = targetPosition;
		VC3 dir = direction;

		Unit *targUnit = targetUnit;

		Projectile *proj = projectile;

		// FLYPATH_GOTOTARGET hack
		if( bulletType != NULL && bulletType->getFlyPath() == Bullet::FLYPATH_GOTOTARGET )
		{
			proj->setDirectPath( targetPosition, targetPosition, 0 );
			proj->setChain( HITCHAIN_NOTHING );

			return;
		}


		// now, we're gonna do the actual raytrace, a bit later on...
		GameCollisionInfo cinfo;

		// actually, "collision disabled units"
		LinkedList ownUnitsNear;

		if (bulletType != NULL 
			&& bulletType->getSplitRaytrace() > 1)
		{
			// disable collision to _all_ units!
			// (bullet handles the hits later on)
			LinkedList *alllist = game->units->getAllUnits();
			LinkedListIterator allUnitsIter = LinkedListIterator(alllist);
			while (allUnitsIter.iterateAvailable())
			{
				Unit *allu = (Unit *)allUnitsIter.iterateNext();
				if (allu->isActive())
				{
					// NEW: but ignore doors that are in ~3 meter radius...
					// (prevents shooting thru doors)
					if (allu->getUnitType()->hasDoorExecute())
					{
						VC3 posdiff = allu->getPosition() - weaponPosition;
						if (posdiff.GetSquareLength() > 3.0f*3.0f)
						{
							ownUnitsNear.append(allu);
							allu->getVisualObject()->setCollidable(false);
						}
					} else {
						ownUnitsNear.append(allu);
						allu->getVisualObject()->setCollidable(false);
					}
				}				
			}

		} else {

			getCollisionDisableList(unit, ownUnitsNear, weaponPosition);

			// disable collision check for shooting unit
			//unit->getVisualObject()->setCollidable(false);
			// now, actually for the no-collision unit... (may not be the same for ricochet bullets)
			if (noCollisionUnit != NULL)
				noCollisionUnit->getVisualObject()->setCollidable(false);

		}

		bool gravity = false;
		bool parabolic = false;
		bool flypathParabolic = false;
		if (bulletType != NULL)
		{
			if (bulletType->getFlyPath() == Bullet::FLYPATH_PARABOLIC)
			{
				parabolic = true;
			}
			if (bulletType->getFlyPath() == Bullet::FLYPATH_GRAVITY)
			{
				gravity = true;
			}


		}


		// do the raytrace for parabolic or direct flypath...
		if (parabolic)
		{
			// this gives us a vector up and slighty towards target...
			VC3 parabolicStartDir = VC3(0, (float)bulletType->getParabolicPathHeight() / 2, 0) + ((target - weaponRayPosition) * 0.5f);
			float raylen = parabolicStartDir.GetLength();
			parabolicStartDir.Normalize();

			game->getGameScene()->rayTrace(weaponPosition, parabolicStartDir, raylen, cinfo, true, false);

			//if (cinfo.hit && cinfo.hitBuilding)
			//if (cinfo.hit && !cinfo.hitTerrainObject)
			if (cinfo.hit)
			{
				// hit building (roof) when going up...
				target = weaponPosition + parabolicStartDir * cinfo.range;

				// flypath is not parabolic...
				// ????
				// would be way too fast... 
				// (parabolic/non-parabolic velocity don't match)
				// FIXME...?
				flypathParabolic = true;

			} else {
				// did not hit building (roof) when going up...
				// see if we hit something when going down...
				VC3 parabolicEndDir = VC3(0, -(float)bulletType->getParabolicPathHeight() / 2, 0) + ((target - weaponPosition) * 0.5f);
				float raylen = parabolicEndDir.GetLength() * 2;
				VC3 endRayPos = target - parabolicEndDir;
				parabolicEndDir.Normalize();
				game->getGameScene()->rayTrace(endRayPos, parabolicEndDir, raylen, cinfo, true, false);
				if (cinfo.hit)
				{
					target = endRayPos + parabolicEndDir * cinfo.range;
					if (cinfo.hitGround)
					{
						if (game->gameMap->isWellInScaledBoundaries(target.x, target.z))
						{
							float groundY = game->gameMap->getScaledHeightAt(target.x, target.y);
							if (target.y < groundY)
								target.y = groundY;
						}
					}
				}
				flypathParabolic = true;
			}
		} else {
			// old check, raytrace from weapons position
			//game->getGameScene()->rayTrace(weaponPosition, dir, weapRange, cinfo, true, false);
			// new check, raytrace from near the center of the unit
			game->getGameScene()->rayTrace(weaponRayPosition, dir, maxRange, cinfo, true, false);
		}

		// restore collision check for unit
		//unit->getVisualObject()->setCollidable(true);
		if (noCollisionUnit != NULL)
			noCollisionUnit->getVisualObject()->setCollidable(true);

		// restore collision check for nearby own units
		while (!ownUnitsNear.isEmpty())
		{
			Unit *ownu = (Unit *)ownUnitsNear.popLast();
			ownu->getVisualObject()->setCollidable(true);
		}

		// HACK: if we hit near a unit lying on the ground, 
		// and we are almost next to it, just make it probably hit.
		if (!parabolic && cinfo.hitGround)
		{
			if (targUnit != NULL)
//						&& (targUnit->getSpeed() == Unit::UNIT_SPEED_CRAWL
//						|| targUnit->getMoveState() == Unit::UNIT_MOVE_STATE_UNCONSCIOUS))
			{
				VC3 posdiff = weaponRayPosition - target;
				// target lying right next to shooter?
				if (posdiff.GetSquareLength() < 4*4)
				{
					VC3 posdiff2 = cinfo.position - target;
					// hit right next to it on ground...
					if (posdiff2.GetSquareLength() < 4*4)
					{
						// 75% chance
						if ((game->gameRandom->nextInt() & 3) != 0)
						{
							cinfo.hitGround = false;
							cinfo.hitUnit = true;
							cinfo.part = NULL;
							cinfo.unit = targUnit;
							cinfo.range = posdiff.GetLength();
							cinfo.position = target;
						}
					}
				}
			}
		}

		// SPECIAL HACK FOR GRAVITY FLYPATH...
		if (gravity
#ifdef PROJECT_SURVIVOR
			&& (cinfo.hitUnit || cinfo.hitGround)
#endif
			)
		{
			cinfo.hit = false;
		}

		// see what we hit and set the projectile's hit chain accordingly
		// can't be const
		Unit *hitUnit = NULL;
		Part *hitPart = NULL;
		if (cinfo.hit)
		{
			if (cinfo.hitUnit)
			{
				if (cinfo.unit->getUnitType()->isMetallic())
				{
					proj->setChain(HITCHAIN_METALLIC);
				} else {
					if (cinfo.unit->getUnitType()->isType2())
					{
						proj->setChain(HITCHAIN_UNIT_TYPE2);
					} else {
						proj->setChain(HITCHAIN_UNIT);
					}
				}
				hitUnit = cinfo.unit;
				hitPart = cinfo.part;
//				if (!parabolic)
//					target = cinfo.position;
			} else {
				if (cinfo.hitBuilding)
				{
					proj->setChain(HITCHAIN_BUILDING);
					//if (!parabolic)
					//	target = cinfo.position;
				} else {
					if (cinfo.hitTerrainObject)
					{
						int material = -1;
						if (cinfo.terrainInstanceId != -1
							&& cinfo.terrainModelId != -1)
						{
							material = game->gameUI->getTerrain()->getMaterialForObject(cinfo.terrainModelId, cinfo.terrainInstanceId);
						}
						if (material == -1)
						{
							if (game->gameMap->isWellInScaledBoundaries(cinfo.position.x, cinfo.position.z))
							{
								int areaX = game->gameMap->scaledToObstacleX(cinfo.position.x);
								int areaY = game->gameMap->scaledToObstacleY(cinfo.position.z);
								if (game->gameMap->getAreaMap()->isAreaAnyValue(areaX, areaY, AREAMASK_OBSTACLE_BUILDINGWALL))
								{
									proj->setChain(HITCHAIN_BUILDING);
								} else {
									material = MaterialManager::getMaterialUnderPosition(game->gameMap, cinfo.position);
								}
							} else {
								proj->setChain(HITCHAIN_TERRAINOBJECT);
							}
						}
						if (material != -1)
						{
#ifdef PROJECT_SHADOWGROUNDS
							switch(material)
							{
							case MATERIAL_SAND:
								proj->setChain(HITCHAIN_MATERIAL_SAND);
								break;
							case MATERIAL_ROCK:
								proj->setChain(HITCHAIN_MATERIAL_ROCK);
								break;
							case MATERIAL_CONCRETE:
								proj->setChain(HITCHAIN_MATERIAL_CONCRETE);
								break;
							case MATERIAL_METAL_HARD:
								proj->setChain(HITCHAIN_MATERIAL_METAL_HARD);
								break;
							case MATERIAL_METAL_TIN:
								proj->setChain(HITCHAIN_MATERIAL_METAL_TIN);
								break;
							case MATERIAL_GLASS:
								proj->setChain(HITCHAIN_MATERIAL_GLASS);
								break;
							case MATERIAL_WOOD:
								proj->setChain(HITCHAIN_MATERIAL_WOOD);
								break;
							case MATERIAL_METAL_GRATE:
								proj->setChain(HITCHAIN_MATERIAL_METAL_GRATE);
								break;
							default:
								proj->setChain(HITCHAIN_TERRAINOBJECT);
								assert(!"Unsupported hit material?");
								break;
							}
#else
							switch(material)
							{
							case MATERIAL_SAND:
								proj->setChain(HITCHAIN_MATERIAL_SAND);
								break;
							case MATERIAL_ROCK:
								proj->setChain(HITCHAIN_MATERIAL_ROCK);
								break;
							case MATERIAL_CONCRETE:
								proj->setChain(HITCHAIN_MATERIAL_CONCRETE);
								break;
							case MATERIAL_METAL_HARD:
								proj->setChain(HITCHAIN_MATERIAL_METAL_HARD);
								break;
							case MATERIAL_METAL_TIN:
								proj->setChain(HITCHAIN_MATERIAL_METAL_TIN);
								break;
							case MATERIAL_WOOD:
								proj->setChain(HITCHAIN_MATERIAL_WOOD);
								break;
							case MATERIAL_GLASS:
								proj->setChain(HITCHAIN_MATERIAL_GLASS);
								break;
							case MATERIAL_PLASTIC:
								proj->setChain(HITCHAIN_MATERIAL_PLASTIC);
								break;
							case MATERIAL_LEAVES:
								proj->setChain(HITCHAIN_MATERIAL_LEAVES);
								break;
							case MATERIAL_LIQUID:
								proj->setChain(HITCHAIN_MATERIAL_LIQUID);
								break;
							case MATERIAL_METAL_GRATE:
								proj->setChain(HITCHAIN_MATERIAL_METAL_GRATE);
								break;
							case MATERIAL_SOIL:
								proj->setChain(HITCHAIN_MATERIAL_SOIL);
								break;
							case MATERIAL_GRASS:
								proj->setChain(HITCHAIN_MATERIAL_GRASS);
								break;
							case MATERIAL_SNOW_SHALLOW:
								proj->setChain(HITCHAIN_MATERIAL_SNOW_SHALLOW);
								break;
							case MATERIAL_SNOW_DEEP:
								proj->setChain(HITCHAIN_MATERIAL_SNOW_DEEP);
								break;
							case MATERIAL_RESERVED_15:
								proj->setChain(HITCHAIN_MATERIAL_RESERVED_15);
								break;
							default:
								proj->setChain(HITCHAIN_TERRAINOBJECT);
								assert(!"Unsupported hit material?");
								break;
							}
#endif
						}
					} else {
						proj->setChain(HITCHAIN_GROUND);
					}
					//if (!parabolic)
					//	target = weaponRayPosition + (dir * cinfo.range);
				}
			}
			if (!parabolic)
				target = weaponRayPosition + (dir * cinfo.range);

			// FIXME: this is not always true - why?
			// new target should roughly be the same as the raytrace hit position
			//fb_assert(fabs(target.x - cinfo.position.x) < 3.0f
			//	&& fabs(target.y - cinfo.position.y) < 3.0f
			//	&& fabs(target.z - cinfo.position.z) < 3.0f);

		} else {
			proj->setChain(HITCHAIN_NOTHING);
			if (!parabolic)
				target = weaponRayPosition + (dir * maxRange);
//						target = weaponPosition + (dir * (float)weapRange);
		}

		proj->setHitTarget(hitUnit, hitPart);

		if (flypathParabolic)
		{
			VC3 targVector = (target - weaponPosition);
			// flypath top height is about 10 meters... (or something...?)
			proj->setParabolicPathHeight((float)bulletType->getParabolicPathHeight());
			// velocity gets scaled based on distance 
			// 100 meters = 1x ratio (200m = 2x, and so on.)
			//proj->setDirectPath(weaponPosition, target, 
			//	bulletType->getVelocity() * (targDist / 100.0f));
			// ...no more scaled velocity...
			proj->setDirectPath(weaponPosition, target, 
				bulletType->getVelocity() * velocityFactor);
		} else {
			// Try to avoid problems with barrel being intruded inside 
			// the target unit...
			if (cinfo.hit)
			{
				if (cinfo.hitUnit)
				{
					VC3 targVector = (target - weaponPosition);
					float targDistSq = targVector.GetSquareLength();
					// NOTE: do not do this for flamethrower!
					if (targDistSq < 2*2 && !bulletType->doesConnectToParent())
					{
						weaponPosition = weaponRayPosition;
					}
				}
			}
			proj->setDirectPath(weaponPosition, target, 
				bulletType->getVelocity() * velocityFactor);

			proj->setHitNormal(cinfo.hitPlaneNormal);
		}
	}

	void ProjectileActor::handleTerrainBreaking(Game *game, std::vector<TerrainObstacle> &removedObjects, std::vector<ExplosionEvent> &events)
	{
		// removed objects...
		int removedAmount = removedObjects.size();
		if (removedAmount > 0)
		{
			game->getGameScene()->removeTerrainObstacles(removedObjects);
		}
		// TODO: delete contents of removedObjects (objects within) 
		// not relevant anymore. should be handled automagically...?

		// events...
		int eventAmount = events.size();
		if (eventAmount > 0)
		{
			for (int i = 0; i < eventAmount; i++)
			{
				if (!events[i].projectile.empty())
				{
					const char *projname = events[i].projectile.c_str();
					if (PARTTYPE_ID_STRING_VALID(projname))
					{
						PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(projname));
						if (pt != NULL)
						{
							if (pt->isInherited(
								getPartTypeById(PARTTYPE_ID_STRING_TO_INT("Bull"))))
							{ 
								// WARNING: unsafe cast! (check above)
								Bullet *evebull = (Bullet *)pt;
								Projectile *eveproj = new Projectile(NULL, evebull);
								game->projectiles->addProjectile(eveproj);

								VC3 evepos = events[i].position;

								eveproj->setDirectPath(evepos, evepos, 
									evebull->getVelocity());

								ProjectileActor pa = ProjectileActor(game);
								pa.createVisualForProjectile(eveproj);
							} else {
								Logger::getInstance()->error("ProjectileActor::act - Explosion event projectile, no part type found with given name.");
							}
						} else {
							Logger::getInstance()->error("ProjectileActor::act - Explosion event projectile is not of bullet type.");
						}
					} else {
						Logger::getInstance()->error("ProjectileActor::act - Explosion event projectile name is not a valid part type id.");
					}
				}
				if (!events[i].effect.empty())
				{
					ui::VisualEffectManager *vman = game->gameUI->getVisualEffectManager();
					VC3 pos = events[i].position;
					int effId = vman->getVisualEffectIdByName(events[i].effect.c_str());
					if (effId == -1)
					{
						Logger::getInstance()->error("ProjectileActor::act - No visual effect with given name found.");
						Logger::getInstance()->debug(events[i].effect.c_str());
					} else {
						// TODO: proper lifetime
						int lifetime = GAME_TICKS_PER_SECOND / 2;
						VisualEffect *effect = vman->createNewManagedVisualEffect(effId, lifetime, NULL, NULL, pos, pos, VC3(events[i].rotation.x,events[i].rotation.y,events[i].rotation.z), events[i].velocity, game);
						if (effect != NULL)
						{
							effect->setParticleExplosion(events[i].explosionPosition, events[i].useExplosion);
						}
					}
				}
				if (!events[i].script.empty())
				{
					// NOTE: not really pickup, but spawn... but who cares :)
					//game->gameScripting->runItemPickupScript(projectile->getShooter(), NULL, "spawn");
					const char *sname = events[i].script.c_str();
					UnifiedHandle uh = events[i].unifiedHandle;

					game->gameScripting->runOtherScriptForUnifiedHandle(sname, "spawn", uh, events[i].position);
				}

				if(!events[i].sound.empty())
				{
					const ExplosionEvent &e = events[i];
					game->getGameUI()->playSoundEffect(e.sound.c_str(), e.position.x, e.position.y, e.position.z, false, DEFAULT_SOUND_SCRIPT_VOLUME, DEFAULT_SOUND_RANGE, DEFAULT_SOUND_PRIORITY_NORMAL);
				}
			}
		}
	}
}

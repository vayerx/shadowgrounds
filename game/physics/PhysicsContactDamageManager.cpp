
#include "precompiled.h"

#include "PhysicsContactDamageManager.h"
#include "PhysicsContactUtils.h"
#include "AbstractPhysicsObject.h"
#include "../Game.h"
#include "../Unit.h"
#include "../UnitType.h"
#include "../Projectile.h"
#include "../ProjectileList.h"
#include "../ProjectileActor.h"
//#include "../ClawController.h"
#include "CarPhysicsObject.h"

#include "../../ui/animdefs.h"

// needed for the horrible terrain object destruction hack...
#include "../../ui/Terrain.h"
#include "../GameUI.h"
#include "../GameScene.h"
#include "../physics/GamePhysics.h"
#include "../SimpleOptions.h"
#include "../options/options_game.h"
#include "../../ui/VisualEffect.h"
#include "../../ui/VisualEffectManager.h"
#include "../../util/ObjectDurabilityParser.h"
#include "../scripting/GameScripting.h"
#include "../../sound/sounddefs.h"


// HACK: !!!
#include "../tracking/ObjectTracker.h"
#include "../tracking/tracker_signals.h"


// TEMP!
#include "../../system/Logger.h"

//#define CONTACT_REQUIRED_DAMAGE_FORCE 100.0f

// either accel or ang_accel must be above this limit... (not both of them)
//#define CONTACT_REQUIRED_DAMAGE_ACCEL 1.0f
//#define CONTACT_REQUIRED_DAMAGE_ANG_ACCEL 1.0f

#define CONTACT_TERRAIN_OBJECT_DAMAGE 15

#define UNIT_COLLISION_VELOCITY_THRESHOLD 10.0f

// HACK: !!!
bool signal_this_terrain_object_break_hack = false;


namespace game
{

	class PhysicsContactDamageManagerImpl
	{
	public:
		PhysicsContactDamageManagerImpl()
		{
			durp = new util::ObjectDurabilityParser();
		}

		~PhysicsContactDamageManagerImpl()
		{
			delete durp;
		}

		util::ObjectDurabilityParser *durp;
		Game *game;
	};



	PhysicsContactDamageManager::PhysicsContactDamageManager(Game *game)
	{
		impl = new PhysicsContactDamageManagerImpl();
		impl->game = game;
	}

	PhysicsContactDamageManager::~PhysicsContactDamageManager()
	{
		delete impl;
	}

	void PhysicsContactDamageManager::reloadConfiguration()
	{
		delete impl->durp;
		impl->durp = new util::ObjectDurabilityParser();
	}

	void PhysicsContactDamageManager::physicsContact(const PhysicsContact &contact)
	{
		// WARNING: unsafe IGamePhysicsObject -> AbstractPhysicsObject casts!
		AbstractPhysicsObject *o1 = (AbstractPhysicsObject *)contact.obj1;
		AbstractPhysicsObject *o2 = (AbstractPhysicsObject *)contact.obj2;

		/*if( contact.contactForceLen > 4000.0f )
		{
			char buf[256];
			sprintf(buf, "%f", contact.contactForceLen);
			Logger::getInstance()->error(buf);
		}*/

		// damn, need this for the claw too, thus, cannot check if both o1 and o2 are non-null
		//if (o1 == NULL || o2 == NULL)
		//	return;
		// this ok? one must not be null?
		if (o1 == NULL && o2 == NULL)
			return;

		int o1handle = 0;
		if (o1) o1handle = o1->getHandle();
		int o2handle = 0;
		if (o2) o2handle = o2->getHandle();

		int durtype1 = OBJECTDURABILITYPARSER_NO_DURABILITY_INDEX;
		int durtype2 = OBJECTDURABILITYPARSER_NO_DURABILITY_INDEX;
		float durforce1 = 999999.0f;
		float durforce2 = 999999.0f;
		if (o1 != NULL)
		{
			durtype1 = o1->getDurabilityType();
			if (durtype1 >= 0)
				durforce1 = impl->durp->getObjectDurabilities()[durtype1].requiredForce;
		}
		if (o2 != NULL)
		{
			durtype2 = o2->getDurabilityType();
			if (durtype2 >= 0)
				durforce2 = impl->durp->getObjectDurabilities()[durtype2].requiredForce;
		}

		if(durtype1 == OBJECTDURABILITYPARSER_NO_DURABILITY_INDEX 
			&& durtype2 == OBJECTDURABILITYPARSER_NO_DURABILITY_INDEX)
		{
			return;
		}

		/*
if(contact.contactForceLen > 0)
{
char buffoo2[256];
sprintf(buffoo2, "type1 = %d, type2 = %d", durtype1, durtype2);
Logger::getInstance()->error(buffoo2);
}
*/

		//bool makeDamage = false;
		bool makeDamage[2] = { false, false };

		//const util::ObjectDurabilityParser::ObjectDurabilityList &durlist = impl->durp->getObjectDurabilities();

		// TODO: optimize, these values should be cached to simple static variables or such,
		// as this is called so often that accessing options is not quite effective enough here...
		//float reqForce = SimpleOptions::getFloat(DH_OPT_F_GAME_CONTACT_DAMAGE_REQUIRED_FORCE_TERRAIN_OBJECT);
		//float reqAccel = SimpleOptions::getFloat(DH_OPT_F_GAME_CONTACT_DAMAGE_REQUIRED_LINEAR_ACCELERATION);
		//float reqAngAccel = SimpleOptions::getFloat(DH_OPT_F_GAME_CONTACT_DAMAGE_REQUIRED_ANGULAR_ACCELERATION);
		float minReqForce = impl->durp->getMinimumDurabilityRequiredForce();

		// HACK: use claw force limit instead???
		/*
		if (o1 == NULL || o2 == NULL)
		{
			reqForce = SimpleOptions::getFloat(DH_OPT_F_GAME_CONTACT_DAMAGE_REQUIRED_FORCE_OTHER);
		}
		*/

		// NEW UNOPTIMAL CRAP: ...
		// TODO: optimize somehow???

		int unitCollisionNumber = -1;

		Unit *some_unit = NULL;
		{
			for (int i = 0; i < 2; i++)
			{
				AbstractPhysicsObject *o = o1;
				if (i == 1) 
				{
					o = o2;
				}

				if (o != NULL)
				{
					// WARNING: unsafe void * to int cast
					Unit *unit = NULL;
					int terrObjModelId = -1;
					int terrObjInstanceId = -1;

					PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(impl->game, o, &unit, &terrObjModelId, &terrObjInstanceId);

					if (unit != NULL)
					{
						//reqForce = SimpleOptions::getFloat(DH_OPT_F_GAME_CONTACT_DAMAGE_REQUIRED_FORCE_UNIT);
						//reqAccel = 0.0f;
						//reqAngAccel = 0.0f;
						unitCollisionNumber = i;
						some_unit = unit;
					}
				}
			}
		}

		/*
if(contact.contactForceLen > 0)
{
char buffoo1[256];
sprintf(buffoo1, "force %f - min %f, o1 req %f, o2 req %f", contact.contactForceLen, minReqForce, durforce1, durforce2);
Logger::getInstance()->error(buffoo1);
}
*/


// HACK: ÜBERHACK: don't kill policemen when hit with a burning object, instead ignite them.
#ifdef PROJECT_CLAW_PROTO
#define TRACKER_SIGNAL_BURN_THE_POLICE 1234
	if (unitCollisionNumber != -1)
	{
		AbstractPhysicsObject *o = o1;
		float force = durforce1;
		if (unitCollisionNumber == 0) 
		{
			force = durforce2;
			o = o2;
		}

		if (o != NULL)
		{
			//if (force < 1234)
			if (true)
			{
				Unit *unit = NULL;
				int terrObjModelId = -1;
				int terrObjInstanceId = -1;

				PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(impl->game, o, &unit, &terrObjModelId, &terrObjInstanceId);

				if (terrObjInstanceId != -1 && terrObjModelId != -1)
				{
					UnifiedHandle trackableUH = impl->game->gameUI->getTerrain()->getUnifiedHandle(terrObjModelId, terrObjInstanceId);
					impl->game->objectTracker->signalToTrackerFromTrackable(trackableUH, TRACKER_SIGNAL_BURN_THE_POLICE);

					/*
					// Seems like this makes polices sometimes immortal. (The game thinks the police is burning while it actually isn't?)
					// So commented out for now. It breaks the hack, I think but it's much better this way. 
					if(impl->game->gameScripting->getGlobalIntVariableValue("signal_return_value") != 0) 
					{
						// (it was burning.)
						return;
					}
					*/
				}
			}
		}
	}
#endif

		if(contact.contactForceLen >= minReqForce || unitCollisionNumber != -1)
		{
			bool velocityThresholdOk = true;

			for (int i = 0; i < 2; i++) 
			{
				AbstractPhysicsObject *o = o1;
				if (i == 1) 
				{
					o = o2;
				}

				if (o != NULL)
				{
					/*
					VC3 accel = o->getAcceleration();
					VC3 angaccel = o->getAngularAcceleration();

					if (accel.GetSquareLength() >= reqAccel * reqAccel
						|| angaccel.GetSquareLength() >= reqAngAccel * reqAngAccel)
					{
					*/
					if (unitCollisionNumber != -1)
					{
						if (i != unitCollisionNumber)
						{
							// HACK: if static, don't damage unless unit is grabbed in the claw 
							if (!o->isDynamic())
							{
								if (some_unit != NULL && !some_unit->isPhysicsObjectLock())
									velocityThresholdOk = false;
							} else {
								float threshold = SimpleOptions::getFloat(DH_OPT_F_GAME_CONTACT_DAMAGE_REQUIRED_UNIT_DAMAGE_VELOCITY);
								if (o->getVelocity().GetSquareLength() < threshold*threshold)
								{
									velocityThresholdOk = false;
								}
							}
						}
					}
					if (contact.contactForceLen >= durforce1)
						makeDamage[0] = true;
					if (contact.contactForceLen >= durforce2)
						makeDamage[1] = true;
					/*
					}
					*/
				} else {
					if (contact.contactForceLen >= durforce1)
						makeDamage[0] = true;
					if (contact.contactForceLen >= durforce2)
						makeDamage[1] = true;
				}
			}

			if (!velocityThresholdOk) 
			{
				makeDamage[0] = false;
				makeDamage[1] = false;
			}

			/*
char buf[256];
int foo1 = (int)makeDamage[0];
int foo2 = (int)makeDamage[1];
sprintf(buf, "damage1 = %d, damage2 = %d", foo1, foo2);
Logger::getInstance()->error(buf);
*/

			for (int i = 0; i < 2; i++)
			{
				// make damage!
				int destroyedObject = -1;

				if(makeDamage[i])
				{
					float damageFactor = 1.0f;
					bool impactDamage = false;

					float reqForce = durforce1;
					if (i == 1)
						reqForce = durforce2;

					if (reqForce > 0)
					{
						// 25% - 100% damage factor (100% required force - 200% required force)
						damageFactor = (contact.contactForceLen / reqForce) - 1.0f;
						damageFactor *= 0.75f;
						damageFactor += 0.25f;
						if (damageFactor > 1.5f)
						{
							impactDamage = true;
						}
						if (damageFactor > 1.0f)
						{
							damageFactor = 1.0f;
						}

						assert(damageFactor >= 0.25f);
						assert(damageFactor <= 1.0f);
					}

				/*
				int destroyedObject = -1;

				for (int i = 0; i < 2; i++)
				{
				*/
					AbstractPhysicsObject *o = o1;
					if (i == 1) 
					{
						// if something was destroyed, it just might be that the other object destroyed as well...
						// for some peculiar reason... say, and explosion?
						if (destroyedObject != -1 && o2 != NULL)
						{
							// WARNING: unsafe cast!
							o = (AbstractPhysicsObject *)impl->game->getGamePhysics()->getInterfaceObjectForHandle(o2handle);
						} else {
							o = o2;
						}
					}

					if (o != NULL)
					{
						// WARNING: unsafe void * to int cast
						Unit *unit = NULL;
						int terrObjModelId = -1;
						int terrObjInstanceId = -1;

						PhysicsContactUtils::mapPhysicsObjectToUnitOrTerrainObject(impl->game, o, &unit, &terrObjModelId, &terrObjInstanceId);

						if (unit != NULL)
						{
							//Logger::getInstance()->error("PHYSICS DAMAGE UNIT!");
							if (unit->getUnitType()->getPhysicsContactDamageBullet() != NULL)
							{
								Bullet *damBullet = unit->getUnitType()->getPhysicsContactDamageBullet();
								

								if (impactDamage 
									|| (unit != NULL && !unit->isPhysicsObjectLock()))
								{
									if (unit->getUnitType()->getPhysicsContactImpactDamageBullet() != NULL)
									{
										damBullet = unit->getUnitType()->getPhysicsContactImpactDamageBullet();
									}
//Logger::getInstance()->error("IMPACT!!");
//								} else {
//Logger::getInstance()->error("SLOW!!");
								}

								Unit *shooter = NULL;
								// HACK: player1 will do the damage... (to get the hitscript run)
								shooter = impl->game->gameUI->getFirstPerson(0);

								Projectile *damproj = new Projectile(shooter, damBullet);
								impl->game->projectiles->addProjectile(damproj);

								VC3 othervel = VC3(0,0,0);
								if (i == 0 && o2 != NULL && destroyedObject != 1)
								{
									// check this just in case the other object has been destroyed...
									if (impl->game->getGamePhysics()->getInterfaceObjectForHandle(o2handle) != NULL)
										othervel = o2->getVelocity();
								}
								else if (i == 1 && o1 != NULL && destroyedObject != 0)
								{
									// check this just in case the other object has been destroyed...
									if (impl->game->getGamePhysics()->getInterfaceObjectForHandle(o1handle) != NULL)
										othervel = o1->getVelocity();
								}

								// If othervel is still in initial state, there isn't other object
								// which made the contact but the unit itself hit something.
								if(othervel.x == 0.0f && othervel.y == 0.0f && othervel.z == 0.0f)
								{
									if(!unit->isPhysicsObjectLock())
									{ 
										othervel = -unit->getVelocity();
									}
									else
									{
										othervel = -unit->getGamePhysicsObject()->getVelocity();
									}
#ifdef PROJECT_CLAW_PROTO
									// Some claw hacks below. 
									VC3 unitPos = unit->getGamePhysicsObject()->getPosition();
									float mapY = impl->game->gameMap->getScaledHeightAt(unitPos.x, unitPos.z);

									// Check if hit floor instead of wall.
									// If hit floor, invert the hit direction.
									//if( o2 && fabs( o2->getFeedbackNormal().GetNormalized().y ) > 0.9f ) //  o2 doesn't contain always a correct normal?
									if( o2 && fabsf( o2->getPosition().y - mapY) <= 0.05f )
									{
										if(!unit->isPhysicsObjectLock())
										{ 
											othervel = unit->getVelocity();
										}
										else
										{
											othervel = unit->getGamePhysicsObject()->getVelocity();
										}
									}
									else
									{
										// This occurs when the cop (or possibly some other unit) is thrown
										// into a wall. Tries to rotate the cop to a right direction
										// and slows the cop's velocity down.

										unit->getGamePhysicsObject()->setVelocity( unit->getGamePhysicsObject()->getVelocity() * 0.25f );
										VC3 vel = unit->getGamePhysicsObject()->getVelocity();
										MAT m;
										m.CreateCameraMatrix( VC3(), VC3(-vel.x, 0, vel.z) , VC3(0, 1, 0));
										QUAT rotat = m.GetRotation();
										
										//unit->setPhysicsObjectLock( false );
										unit->getVisualObject()->setRotationQuaternion( rotat );
										unit->getGamePhysicsObject()->setRotation( rotat );
										VC3 unitRot = unit->getRotation();
										unit->setRotation( unitRot.x, unit->getVisualObject()->getRenderYAngle(), unitRot.z);

									}
#endif
								}

								VC3 dampos = contact.contactPosition;
								VC3 startpos = dampos - (othervel / 10.0f);

								VC3 diff = dampos - startpos;
								// HACK: change bullet push value to match the velocity...
								int pushValue = (int)(othervel.GetLength());
								char valbuf[16];
								strcpy(valbuf, int2str(pushValue));
								damBullet->setSub("hit");
								damBullet->setData("push", valbuf);
								damBullet->setSub(NULL);

//char foo[256];
//sprintf(foo, "DIR %f,%f,%f", diff.x, diff.y, diff.z);
//Logger::getInstance()->error(foo);

								damproj->setDirectPath(startpos, dampos, damBullet->getVelocity());
								damproj->setHitTarget(unit, NULL);

								ProjectileActor pa = ProjectileActor(impl->game);
								pa.createVisualForProjectile(damproj);
							}
						} else {
							if (terrObjInstanceId != -1 && terrObjModelId != -1)
							{
								//Logger::getInstance()->error("PHYSICS DAMAGE TERRAIN OBJECT!");
								VC3 dampos = contact.contactPosition;
								VC2 dampos2d = VC2(dampos.x, dampos.z);
								VC3 damvel = VC3(0,0,0);

								VC3 physPrevVel = VC3(0,0,0);
								if (i == 0 && o1 != NULL)
								{
									// check this just in case the other object has been destroyed...
									if (impl->game->getGamePhysics()->getInterfaceObjectForHandle(o1handle) != NULL)
										physPrevVel = o1->getVelocity();
								}
								if (i == 1 && o2 != NULL)
								{
									// check this just in case the other object has been destroyed...
									if (impl->game->getGamePhysics()->getInterfaceObjectForHandle(o2handle) != NULL)
										physPrevVel = o2->getVelocity();
								}

								std::vector<TerrainObstacle> removedObjects;
								std::vector<ExplosionEvent> objectEvents;


/*
#ifdef PROJECT_CLAW_PROTO
UnifiedHandle trackableUH = impl->game->gameUI->getTerrain()->getUnifiedHandle(terrObjModelId, terrObjInstanceId);
// always true (inside terr.object if clause)
//if (trackableUH != UNIFIED_HANDLE_NONE
//	&& IS_UNIFIED_HANDLE_TERRAIN_OBJECT(trackableUH))
//{
	impl->game->objectTracker->signalToTrackerFromTrackable(trackableUH, TRACKER_SIGNAL_TERRAIN_OBJECT_HIT_UNIT);
//}
#endif
*/



// HACK: !!!
signal_this_terrain_object_break_hack = false;
								bool broken = true;
#ifdef PROJECT_CLAW_PROTO
								/*
								// quick hack to get rid of annoying terrobj-drops-drom-claw-when-broken bug: terrain objects doesn't break when in claw. 
								if( impl->game->getClawController()->getTerrainObjectInstanceId() == terrObjInstanceId 
								 && impl->game->getClawController()->getTerrainObjectModelId() == terrObjModelId)
									broken = false;
								*/
#endif
								if(broken)
									broken = impl->game->gameUI->getTerrain()->breakObjects(terrObjModelId, terrObjInstanceId, (int)(CONTACT_TERRAIN_OBJECT_DAMAGE * damageFactor), removedObjects, objectEvents, dampos2d, damvel, dampos, false, false);
								//bool broken = false;


								if (signal_this_terrain_object_break_hack)
								{
									UnifiedHandle trackableUH = impl->game->gameUI->getTerrain()->getUnifiedHandle(terrObjModelId, terrObjInstanceId);
									impl->game->objectTracker->signalToTrackerFromTrackable(trackableUH, TRACKER_SIGNAL_TERRAIN_OBJECT_BREAK_SELF_LOOP);
								}

								// HACK: horrible copy&paste and stuff ahead!!!

								int removedAmount = 0;
								if (broken) removedAmount++;
								//int removedAmount = removedObjects.size();
								if (removedAmount > 0)
								{
									impl->game->getGameScene()->removeTerrainObstacles(removedObjects);
									if (destroyedObject == -1)
									{
										// at least one destroyed (may be both if this is the first object)
										destroyedObject = i;
									} else {
										// both destroyed
										destroyedObject = -2;
									}
								}

								int eventAmount = objectEvents.size();
								if (eventAmount > 0)
								{
									for (int i = 0; i < eventAmount; i++)
									{
										if (!objectEvents[i].projectile.empty())
										{
											const char *projname = objectEvents[i].projectile.c_str();
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
														impl->game->projectiles->addProjectile(eveproj);

														VC3 evepos = objectEvents[i].position;

														eveproj->setDirectPath(evepos, evepos, 
															evebull->getVelocity());

														ProjectileActor pa = ProjectileActor(impl->game);
														pa.createVisualForProjectile(eveproj);
													} else {
														Logger::getInstance()->error("PhysicsContactDamageManager::physicsContact - Explosion event projectile, no part type found with given name.");
													}
												} else {
													Logger::getInstance()->error("PhysicsContactDamageManager::physicsContact - Explosion event projectile is not of bullet type.");
												}
											} else {
												Logger::getInstance()->error("PhysicsContactDamageManager::physicsContact - Explosion event projectile name is not a valid part type id.");
											}
										}
										if (!objectEvents[i].effect.empty())
										{
											bool effectRateOk = true;
											if (!broken && terrObjInstanceId != -1 && terrObjModelId != -1)
											{
												// TODO: get this from some object type configuration
												int maxEffectRate = 200;
												int lastTime = impl->game->gameUI->getTerrain()->getLastObjectEffectTime(terrObjModelId, terrObjInstanceId);
												if (impl->game->gameTimer < lastTime + (maxEffectRate / GAME_TICK_MSEC))
												{
													effectRateOk = false;
												} else {
													impl->game->gameUI->getTerrain()->setLastObjectEffectTime(terrObjModelId, terrObjInstanceId, impl->game->gameTimer);
												}
											}
											if (effectRateOk)
											{
												ui::VisualEffectManager *vman = impl->game->gameUI->getVisualEffectManager();
												VC3 pos = objectEvents[i].position;
												int effId = vman->getVisualEffectIdByName(objectEvents[i].effect.c_str());
												if (effId == -1)
												{
													Logger::getInstance()->error("PhysicsContactDamageManager::physicsContact - No visual effect with given name found.");
													Logger::getInstance()->debug(objectEvents[i].effect.c_str());
												} else {
													// TODO: proper lifetime
													int lifetime = GAME_TICKS_PER_SECOND / 2;
if (strncmp(objectEvents[i].effect.c_str(), "LONG_", 5) == 0)
	lifetime = GAME_TICKS_PER_SECOND * 10;
//													VisualEffect *effect = vman->createNewManagedVisualEffect(effId, lifetime, NULL, NULL, pos, pos, VC3(objectEvents[i].rotation.x,objectEvents[i].rotation.y,objectEvents[i].rotation.z), objectEvents[i].velocity, impl->game);
													VC3 physPrevVelInTicks = physPrevVel / GAME_TICKS_PER_SECOND;
													VisualEffect *effect = vman->createNewManagedVisualEffect(effId, lifetime, NULL, NULL, pos, pos, VC3(objectEvents[i].rotation.x,objectEvents[i].rotation.y,objectEvents[i].rotation.z), physPrevVelInTicks, impl->game);
													if (effect != NULL)
													{
														effect->setParticleExplosion(objectEvents[i].explosionPosition, objectEvents[i].useExplosion);
													}
												}
											}
										}
										if (!objectEvents[i].script.empty())
										{
											// NOTE: not really pickup, but spawn... but who cares :)
											//game->gameScripting->runItemPickupScript(projectile->getShooter(), NULL, "spawn");
											const char *sname = objectEvents[i].script.c_str();
											UnifiedHandle uh = objectEvents[i].unifiedHandle;
											impl->game->gameScripting->runOtherScriptForUnifiedHandle(sname, "spawn", uh, objectEvents[i].position);
										}

										if(!objectEvents[i].sound.empty())
										{
											const ExplosionEvent &e = objectEvents[i];
											impl->game->getGameUI()->playSoundEffect(e.sound.c_str(), e.position.x, e.position.y, e.position.z, false, DEFAULT_SOUND_SCRIPT_VOLUME, DEFAULT_SOUND_RANGE, DEFAULT_SOUND_PRIORITY_NORMAL);
										}
									}
								}

#ifdef PROJECT_CLAW_PROTO
								/*
								if(broken)
								{
									impl->game->getClawController()->dropActor();
									impl->game->getClawController()->setGrabFlag( true );
									impl->game->getClawController()->pickActor();
									impl->game->getClawController()->update();
								}
								*/
#endif

							}
						}
					} // end if (o != NULL)
				}

				// if one of the objects was destroyed, restore the previous velocity for the other object...
				// (kind of a hack to get dynamic objects break through static breakable walls, etc.)
				if (destroyedObject >= 0)
				{
					int survivingObject = 0;
					if (destroyedObject == 0)
						survivingObject = 1;

					/*
					// can't do it like this, as the surviving object may have also been destroyed..
					// (by an explosion or something?)
					AbstractPhysicsObject *o = o1;
					if (survivingObject == 1) 
					{
						o = o2;
					}
					*/
					AbstractPhysicsObject *o = o1;
					int ohandle = o1handle;
					if (survivingObject == 1) 
					{
						ohandle = o2handle;
						o = o2;
					}
					if (o != NULL)
					{
						// WARNING: unsafe cast!
						o = (AbstractPhysicsObject *)impl->game->getGamePhysics()->getInterfaceObjectForHandle(ohandle);
					}

					if (o != NULL)
					{
						o->restorePreviousVelocities(0.6f, 0.2f);
					}					
				}

			} // end if makedamage

		} // end if(contact.contactForceLen >= CONTACT_REQUIRED_DAMAGE_FORCE)
	}

} // end namespace game



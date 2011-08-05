#include <boost/scoped_ptr.hpp>
#include <assert.h>
#include <vector>
#include <map>

#include "precompiled.h"

#include "GamePhysics.h"
#include "IPhysicsContactListener.h"
#ifdef PHYSICS_PHYSX
#include "../../physics/physics_lib.h"
#include "../../physics/actor_base.h"
#include "../../physics/visualizer.h"
#include "../../physics/IPhysicsLibScriptRunner.h"
#endif
#include "../gamedefs.h"
#include "../SimpleOptions.h"
#include "../options/options_physics.h"
#include "../options/options_game.h"

// oh ffs...
#include <Storm3D_UI.h>
#include "../editor/parser.h"
#include "../particle_editor2/particleeffect.h"

#include "StaticPhysicsObject.h"
#ifdef PHYSICS_PHYSX
#include "ConvexPhysicsObject.h"
#include "../physics/joint_base.h"
#if !defined PROJECT_SURVIVOR && !defined PROJECT_SHADOWGROUNDS
#include "../physics/d6_joint.h"
#endif
#endif

#ifdef PHYSICS_FEEDBACK
#include "PhysicsContactFeedback.h"
#endif

#ifdef PROJECT_CLAW_PROTO
#ifndef USE_CLAW_CONTROLLER
#define USE_CLAW_CONTROLLER
#endif
#endif

#ifdef USE_CLAW_CONTROLLER
#include "../ClawController.h"
game::ClawController *gamephysics_clawController = NULL;
#endif


#ifdef PHYSICS_PHYSX
// HACK!!!
extern void clear_static_physics_temp_models();
#endif

// max. fluid height offset from ground focus in meters
#define FLUID_CONTAINMENT_HEIGHT_OFFSET 1.8f


// TODO: this should be a physics debug option...
#define GAMEPHYSICS_VISUALIZE_RANGE 10


namespace game
{
#ifdef PHYSICS_FEEDBACK
	extern PhysicsContactFeedback *gameui_physicsFeedback;
#endif

namespace {

#ifdef PHYSICS_PHYSX

#if !defined PROJECT_SURVIVOR && !defined PROJECT_SHADOWGROUNDS
	struct JointCreationInfo
	{
		boost::shared_ptr<AbstractPhysicsObject> objectA;
		boost::shared_ptr<AbstractPhysicsObject> objectB;
		std::string id;

		frozenbyte::physics::PhysicsJoint joint;
	};

	typedef std::vector<JointCreationInfo> JointCreationList;
	typedef std::vector<boost::shared_ptr<frozenbyte::physics::JointBase> > JointList;
#endif

#endif
} // unnamed

	class GamePhysicsObjectImpl
	{
	public:
		explicit GamePhysicsObjectImpl(PHYSICS_ACTOR &implObject, int handle, IGamePhysicsObject *interfaceObject)
		{
			this->implObject = implObject;
			this->handle = handle;
			this->deleteRequestFlag = false;
			this->deleted = false;
			this->interfaceObject = interfaceObject;
			this->implObjectWasActive = true;
		}

		~GamePhysicsObjectImpl()
		{
			implObject.reset();
		}

		void freeMe()
		{
			assert(!this->deleted);
#ifdef PHYSICS_PHYSX
			assert(implObject.unique());
#endif

			this->handle = 0;
			this->deleted = true;
			this->deleteRequestFlag = false;
			this->implObject.reset();
			this->interfaceObject = NULL;
			this->implObjectWasActive = true;
		}

		void reAssignMe(PHYSICS_ACTOR &implObject, int handle, IGamePhysicsObject *interfaceObject)
		{
			assert(this->deleted);

			this->handle = handle;
			this->deleted = false;
			this->deleteRequestFlag = false;
			this->implObject = implObject;
			this->implObjectWasActive = true;
			this->interfaceObject = interfaceObject;
		}

		bool deleted;
		int handle;
		bool deleteRequestFlag;
		bool implObjectWasActive;
		PHYSICS_ACTOR implObject;
		IGamePhysicsObject *interfaceObject;
	};

	typedef std::map<int, int> PhysicsHandleHash;

	class GamePhysicsImpl
	{	
	private:
#ifdef PHYSICS_PHYSX
		frozenbyte::physics::PhysicsLib *physicsLib;
		float fluidContainmentHeight;
		float lastFluidContainmentHeight;

		VC3 fluidPushPosition;
		float fluidPushRange;
		int fluidPushEndTime;
		int fluidPushCurrentTime;

#if !defined PROJECT_SURVIVOR && !defined PROJECT_SHADOWGROUNDS
		JointCreationList jointCreationList;
		JointList jointList;
#endif
#endif
		bool running;

		std::vector<GamePhysicsObjectImpl> physicsObjects;
		PhysicsHandleHash physicsObjectsHash;

		std::vector<IGamePhysicsObject *> interfaceObjectsToAdd;

		int nextFreeHandle;
		bool visualize;
		bool physicsBeingDeleted;

		std::vector<IPhysicsContactListener *> physicsContactListeners;

		bool ignoreContacts;


		GamePhysicsImpl()
		{
			running = false;
			nextFreeHandle = 1;
			visualize = false;
			physicsBeingDeleted = false;

#ifdef PHYSICS_PHYSX
			physicsLib = NULL;
			fluidContainmentHeight = 9999.0f;
			lastFluidContainmentHeight = fluidContainmentHeight;

			fluidPushPosition = VC3(0,0,0);
			fluidPushRange = 0.0f;
			fluidPushEndTime = 0;
			fluidPushCurrentTime = 0;
#endif

			ignoreContacts = false;
		}



		~GamePhysicsImpl()
		{
			deletePhysics();
		}


		IGamePhysicsObject *getInterfaceObjectForHandle(int handle)
		{
			assert(handle != 0);

			PhysicsHandleHash::iterator iter = physicsObjectsHash.find(handle);
			if (iter != physicsObjectsHash.end())
			{
				int arraypos = (*iter).second;

				if(physicsObjects[arraypos].deleteRequestFlag)
					return NULL;

				assert((int)physicsObjects.size() > arraypos);
				assert(!physicsObjects[arraypos].deleted);
				assert(physicsObjects[arraypos].handle == handle);
				assert(physicsObjects[arraypos].interfaceObject != NULL);

				if(physicsObjects[arraypos].deleted)
				{
					Logger::getInstance()->error("GamePhysicsImpl::getInterfaceObjectForHandle - Requested an object by handle which maps to a deleted object.");
					return NULL;
				}

				return physicsObjects[arraypos].interfaceObject;
			} else {
				return NULL;
			}
		}


		void createPhysics(IGamePhysicsScriptRunner *scriptRunner)
		{
#ifdef PHYSICS_PHYSX
			assert(physicsLib == NULL);

			frozenbyte::physics::PhysicsParams physicsParams;

			if (SimpleOptions::getBool(DH_OPT_B_GAME_SIDEWAYS))
			{
				physicsParams.gravity = VC3(0, 0, SimpleOptions::getFloat(DH_OPT_F_PHYSICS_GRAVITY));
			} else {
				physicsParams.gravity = VC3(0, SimpleOptions::getFloat(DH_OPT_F_PHYSICS_GRAVITY), 0);
			}
			physicsParams.defaultRestitution = SimpleOptions::getFloat(DH_OPT_F_PHYSICS_DEFAULT_RESTITUTION);
			physicsParams.defaultStaticFriction = SimpleOptions::getFloat(DH_OPT_F_PHYSICS_DEFAULT_DYNAMIC_FRICTION);
			physicsParams.defaultDynamicFriction = SimpleOptions::getFloat(DH_OPT_F_PHYSICS_DEFAULT_STATIC_FRICTION);
			physicsParams.ccd = SimpleOptions::getBool(DH_OPT_B_PHYSICS_USE_CCD);
			physicsParams.ccdMaxThickness = SimpleOptions::getFloat(DH_OPT_B_PHYSICS_CCD_MAX_THICKNESS);
			if (scriptRunner != NULL)
				physicsParams.scriptRunner = (frozenbyte::physics::IPhysicsLibScriptRunner *)(scriptRunner->getGamePhysicsScriptRunnerImplementation());

			// hardware physics game option.
			physicsLib = new frozenbyte::physics::PhysicsLib(
				SimpleOptions::getBool(DH_OPT_B_PHYSICS_USE_HARDWARE),
				SimpleOptions::getBool(DH_OPT_B_PHYSICS_USE_HARDWARE_FULLY),
				SimpleOptions::getBool(DH_OPT_B_PHYSICS_USE_MULTITHREADING),
				&physicsParams);
			if (!physicsLib->getSDK()) {
				fprintf(stderr, "Unable to load physics, please make sure PhysX is installed correctly.\n");
				exit(0);
			}

			int timeStep = SimpleOptions::getInt(DH_OPT_I_PHYSICS_TIME_STEP);
			if (timeStep < 1)
				timeStep = 1;
			if (timeStep > 1000)
				timeStep = 1000;
			physicsLib->setTimeStep(timeStep / 1000.0f);
			physicsLib->addGroundPlane(SimpleOptions::getFloat(DH_OPT_F_PHYSICS_GROUND_PLANE_HEIGHT));
			if (SimpleOptions::getFloat(DH_OPT_F_PHYSICS_SKY_PLANE_HEIGHT) > SimpleOptions::getFloat(DH_OPT_F_PHYSICS_GROUND_PLANE_HEIGHT))
			{
				physicsLib->addSkyPlane(SimpleOptions::getFloat(DH_OPT_F_PHYSICS_SKY_PLANE_HEIGHT));
			}
			
			physicsLib->addFluidContainmentPlane(fluidContainmentHeight);

#endif
#ifdef PHYSICS_ODE
			PhysicsActorOde::init();
#endif

#ifdef USE_CLAW_CONTROLLER
			if(gamephysics_clawController)
				gamephysics_clawController->createPhysics(physicsLib);
#endif
		}


		void prepareForDelete()
		{
			if (running)
			{
				physicsBeingDeleted = true;
				endTick();
				physicsBeingDeleted = false;
			}
		}

		void deletePhysics()
		{
//#ifdef PHYSICS_PHYSX
			//assert(physicsLib != NULL);
//#endif

			if (running)
			{
				physicsBeingDeleted = true;
				endTick();
				physicsBeingDeleted = false;
			}

			interfaceObjectsToAdd.clear();

			physicsObjectsHash.clear();
			physicsObjects.clear();

#ifdef PHYSICS_PHYSX
			ConvexPhysicsObject::clearImplementationResources();
#if !defined PROJECT_SURVIVOR && !defined PROJECT_SHADOWGROUNDS
			jointList.clear();
#endif
#endif
			StaticPhysicsObject::clearImplementationResources();
			// TODO: other mesh, etc. resources too

#ifdef USE_CLAW_CONTROLLER
			gamephysics_clawController->createPhysics(0);
#endif

#ifdef PHYSICS_PHYSX

			if (physicsLib != NULL)
			{
				delete physicsLib;
				physicsLib = NULL;
			}
#endif
#ifdef PHYSICS_ODE
			PhysicsActorOde::uninit();
#endif
		}



		void renderedScene()
		{
			visualize = true;
		}



		void setGroundFocusHeight(float height)
		{
#ifdef PHYSICS_PHYSX
			fluidContainmentHeight = height + FLUID_CONTAINMENT_HEIGHT_OFFSET;
#endif
		}

		void createFluidPushPoint(const VC3 &position, float range, int timeTicks)
		{
#ifdef PHYSICS_PHYSX
			fluidPushPosition = position;
			fluidPushRange = range;
			fluidPushEndTime = timeTicks;
			fluidPushCurrentTime = 0;
#endif
		}


		void runPhysics(IStorm3D_Scene *stormScene, frozenbyte::particle::ParticleEffectManager *particleEffectManager)
		{
#ifdef PHYSICS_PHYSX
			if (physicsLib == NULL)
			{
				return;
			}
#endif

			//assert(physicsLib != NULL);

			if (running)
			{
				endTick();

#ifdef USE_CLAW_CONTROLLER
				if(gamephysics_clawController)
					gamephysics_clawController->update();
#endif
			}

			if (visualize)
			{
#ifdef PHYSICS_PHYSX
				frozenbyte::physics::visualize(*physicsLib, *stormScene, GAMEPHYSICS_VISUALIZE_RANGE);
				VC3 vispos = stormScene->GetCamera()->GetPosition();
				physicsLib->updateVisualization(vispos, GAMEPHYSICS_VISUALIZE_RANGE, false);
#endif
				visualize = false;
			}

			// add any interface objects that do not yet have a real physics implementation
			for (int i = 0; i < (int)interfaceObjectsToAdd.size(); i++)
			{
				IGamePhysicsObject *o = interfaceObjectsToAdd[i];
				//char foo[256];
				//sprintf(foo, "implementing %p", o);
				//Logger::getInstance()->error(foo);
				PHYSICS_ACTOR actor = o->createImplementationObject();
				addNewObjectToList(o, actor);
			}
			interfaceObjectsToAdd.clear();

#ifdef PHYSICS_PHYSX
			std::vector<frozenbyte::physics::ActorBase *> activeList;
			physicsLib->getActiveActors(activeList, true);
			for (int i = 0; i < (int)activeList.size(); i++)
			{
				void *udata = activeList[i]->getUserData();
				assert(udata != NULL);
				// WARNING: int to void * cast!
				int handle = (int)udata;

				PhysicsHandleHash::iterator iter = physicsObjectsHash.find(handle);
				if (iter != physicsObjectsHash.end())
				{
					int arraypos = (*iter).second;

					assert((int)physicsObjects.size() > arraypos);
					assert(!physicsObjects[arraypos].deleted);
					assert(physicsObjects[arraypos].handle == handle);

					physicsObjects[arraypos].implObjectWasActive = true;
				} else {
					assert(!"GamePhysicsImpl::runPhysics - active actors list referred to a handle that was not found.");
				}
			}

			float water_height = SimpleOptions::getFloat(DH_OPT_F_PHYSICS_WATER_HEIGHT);
			float water_damping = 0.0001f * SimpleOptions::getFloat(DH_OPT_F_PHYSICS_WATER_DAMPING);
#endif

			// sync or delete existing objects...
			std::vector<int> deleteList;
			for (int i = 0; i < (int)physicsObjects.size(); i++)
			{
				GamePhysicsObjectImpl &o = physicsObjects[i];
				if (o.deleteRequestFlag)
				{
					deleteList.push_back(i);
				} else if(!o.deleted) {
					// sync real physics objects <-> interface physics objects...

#ifdef PHYSICS_PHYSX
					if (physicsObjects[i].implObject)
					{
						if (physicsObjects[i].implObjectWasActive)
						{

							VC3 volume_min, volume_max;
							if(water_height > 0 && water_damping > 0
								&& physicsObjects[i].implObject->isDynamic()
								&& physicsObjects[i].implObject->getVolumeUnderWater(water_height, volume_min, volume_max))
							{
								// compute total volume
								VC3 size = (volume_max - volume_min);
								float volume = size.x * size.y * size.z;
								// compute center
								VC3 center = (volume_max + volume_min) * 0.5f;
								// measure velocity in center
								VC3 vel;
								physicsObjects[i].implObject->getVelocityAt(vel, center);
								if(vel.GetSquareLength() > 1.0f)
								{
									// create impulse to counter it
									// make sure it doesn't reverse movement
									float scaling = water_damping * volume;
									if(scaling > 0.01f) scaling = 0.01f; // it really doesn't take much to reverse.. ??
									VC3 impulse = -vel * scaling;
									physicsObjects[i].implObject->addImpulse(center, impulse, true);
								}
							}

							physicsObjects[i].interfaceObject->syncImplementationObject(physicsObjects[i].implObject);
							physicsObjects[i].implObjectWasActive = false;
						} else {
							// this seems to cause a significant hit (because of the high number of objs)
							// because these are virtual calls, the syncing is assumed to be costly...
							// therefore, accessing some major flags directly here... --jpk
							AbstractPhysicsObject *apo = (AbstractPhysicsObject *)physicsObjects[i].interfaceObject;
							if (apo->dynamicActor && apo->dirty)
							{
								physicsObjects[i].interfaceObject->syncInactiveImplementationObject(physicsObjects[i].implObject);
							}
						}
					}
#else
					physicsObjects[i].interfaceObject->syncImplementationObject(physicsObjects[i].implObject);
#endif
				}
			}

			for (int i = 0; i < (int)deleteList.size(); i++)
			{
				int handle = physicsObjects[deleteList[i]].handle;

#ifdef USE_CLAW_CONTROLLER
				if(physicsObjects[deleteList[i]].implObject)
				{
					NxActor *actor = physicsObjects[deleteList[i]].implObject->getActor();
					if(gamephysics_clawController)
						gamephysics_clawController->removeActor(actor);
				}
#endif

				// don't actually delete, just mark deleted...
				//physicsObjects.erase(physicsObjects.begin() + deleteList[i]);
				physicsObjects[deleteList[i]].freeMe();
				assert(!running);

				PhysicsHandleHash::iterator iter = physicsObjectsHash.find(handle);
				if (iter != physicsObjectsHash.end())
				{
					physicsObjectsHash.erase(iter);
				} else {
					assert(!"GamePhysicsImpl::runPhysics - Unable to find handle to be deleted in hashmap, data integrity error (bug).");
				}
			}
			deleteList.clear();

#ifdef PHYSICS_PHYSX
			// HACK!!!
			clear_static_physics_temp_models();
#endif

			if (particleEffectManager != NULL)
			{
				particleEffectManager->updatePhysics();
			}

#ifdef PHYSICS_PHYSX
			// if the fluid containment has changed, move toward the desired value...
			if (lastFluidContainmentHeight != fluidContainmentHeight)
			{
				if (fabs(lastFluidContainmentHeight - fluidContainmentHeight) > 2.0f)
				{
					lastFluidContainmentHeight = fluidContainmentHeight;
				} else {
					if (fluidContainmentHeight > lastFluidContainmentHeight)
					{
						lastFluidContainmentHeight += 0.05f;
						if (fluidContainmentHeight < lastFluidContainmentHeight)
							lastFluidContainmentHeight = fluidContainmentHeight;
					}
					else
					{
						lastFluidContainmentHeight -= 0.05f;
						if (fluidContainmentHeight > lastFluidContainmentHeight)
							lastFluidContainmentHeight = fluidContainmentHeight;
					}
				}
				physicsLib->moveFluidContainmentPlane(lastFluidContainmentHeight);
			}

			// fluid pusher...
			if (fluidPushCurrentTime < fluidPushEndTime)
			{
				if (fluidPushEndTime > 0)
				{
					fluidPushCurrentTime++;

					/*
					float ratio = (float)fluidPushCurrentTime / (float)fluidPushEndTime;

					// NEW: non-linear scaling...
					ratio = sinf(3.1415f / 2 * ratio);

					float sphereSize = (fluidPushRange + 1.0f) * ratio;

					physicsLib->createOrMoveFluidContainmentSphere(fluidPushPosition, 1.0f + sphereSize);
					*/

					int rotTimes = 3;
					int timeDiv = 8;
					int moveTime = fluidPushCurrentTime % timeDiv;
					int angTime = fluidPushCurrentTime / timeDiv;
					int ang = (int)(((float)angTime / (float)((int)(fluidPushEndTime / timeDiv))) * 360.0f * (float)rotTimes);
					//int rotNum = (ang / 360);
					int sect = (ang % 360);
					//sect = sect ^ (1 + 4 + 32 + 128);
					//ang = rotNum * 360 + sect;
					ang = sect;
					float angRad = ang * 3.1415f / 180.0f;

					float sphereSize = fluidPushRange;
					VC3 dir = VC3(sinf(angRad),0,cosf(angRad));
					dir *= 2;
					float ratio = (float)moveTime / (float)timeDiv;

					physicsLib->createOrMoveFluidContainmentSphere(fluidPushPosition + dir * ratio, sphereSize);
				}
			} else {
				if (fluidPushEndTime > 0)
				{
					fluidPushEndTime = 0;
					fluidPushCurrentTime = 0;
					physicsLib->removeFluidContainmentSphere();
				}
			}
#endif

#ifdef PHYSICS_PHYSX
#if !defined PROJECT_SURVIVOR && !defined PROJECT_SHADOWGROUNDS
			// Create joints
			for(JointCreationList::iterator it = jointCreationList.begin(); it != jointCreationList.end(); ++it)
			{
				JointCreationInfo &info = *it;

				boost::shared_ptr<frozenbyte::physics::ActorBase> a;
				if(info.objectA)
					a = getImplementingBaseObject(info.objectA.get());
				boost::shared_ptr<frozenbyte::physics::ActorBase> b;
				if(info.objectB)
					b = getImplementingBaseObject(info.objectB.get());

				// I guess this could be a map with id as a key or something
				jointList.push_back(boost::static_pointer_cast<frozenbyte::physics::JointBase> (physicsLib->createGeneralJoint(a, b, info.joint)));
			}

			jointCreationList.clear();
#endif
#endif

			if (SimpleOptions::getBool(DH_OPT_B_PHYSICS_UPDATE))
			{
				startTick();
			}
		}



		void addNewObjectToList(
			IGamePhysicsObject *interfaceObject,
			PHYSICS_ACTOR &implObject)
		{
			if(!implObject) return;

			size_t vecpos = physicsObjects.size();

			// TODO: should optimize this... O(n) insert algo is not very good for this case
			for (int i = 0; i < (int)physicsObjects.size(); i++)
			{
				if (physicsObjects[i].deleted)
				{
					vecpos = i;
					break;
				}
			}

			if (vecpos == physicsObjects.size())
			{
				physicsObjects.push_back(GamePhysicsObjectImpl(implObject, nextFreeHandle, interfaceObject));
			} else {
				physicsObjects[vecpos].reAssignMe(implObject, nextFreeHandle, interfaceObject);
			}
			physicsObjectsHash.insert(std::pair<int,int>(nextFreeHandle,vecpos));

			interfaceObject->setHandle(nextFreeHandle);

#ifdef PHYSICS_PHYSX
			// WARNING: int to void * cast!
			if(implObject)
				implObject->setUserData((void *)nextFreeHandle);
#endif

			nextFreeHandle++;
			if (nextFreeHandle > 100000000)
				nextFreeHandle = 1;
		}



		void startTick()
		{
#ifdef PHYSICS_PHYSX
			assert(physicsLib != NULL);
			physicsLib->startSimulation(SimpleOptions::getInt(DH_OPT_I_PHYSICS_TICK_TIME) / 1000.0f);
#endif
#ifdef PHYSICS_ODE
			PhysicsActorOde::setMultithreading(SimpleOptions::getBool(DH_OPT_B_PHYSICS_USE_MULTITHREADING));
			PhysicsActorOde::startSimulation(SimpleOptions::getInt(DH_OPT_I_PHYSICS_TICK_TIME));
#endif
			running = true;
		}



		void endTick()
		{
#ifdef PHYSICS_PHYSX
			assert(physicsLib != NULL);

			physicsLib->finishSimulation();

#ifdef PHYSICS_FEEDBACK
			gameui_physicsFeedback->tick();
#endif

			if (!physicsBeingDeleted)
			{
				const std::vector<frozenbyte::physics::ContactListEntry> &contacts = physicsLib->getContactList();

				if (contacts.size() > 0
					&& !ignoreContacts)
				{
					for (int i = 0; i < (int)contacts.size(); i++)
					{
//char buf[256];
//sprintf(buf, "%f", contacts[i].contactForce);
//Logger::getInstance()->error(buf);

						if (contacts[i].actor1 != NULL && contacts[i].actor1 != NULL)
						{
							PhysicsContact pc;

							// WARNING: void * to int casts ahead!
							int handle1 = 0;
							if (contacts[i].actor1 != NULL)
							{
								handle1 = (int)contacts[i].actor1->getUserData();
								if (handle1 != 0)
								{
									pc.obj1 = this->getInterfaceObjectForHandle(handle1);
								}
							}
							int handle2 = 0;
							if (contacts[i].actor2 != NULL)
							{
								handle2 = (int)contacts[i].actor2->getUserData();
								if (handle2 != 0)
								{
									pc.obj2 = this->getInterfaceObjectForHandle(handle2);
								}
							}

							pc.physicsObject1 = contacts[i].actor1;
							pc.physicsObject2 = contacts[i].actor2;
							pc.contactForceLen = contacts[i].contactForce;
							pc.contactNormal = contacts[i].contactNormal;
							pc.contactPosition = contacts[i].contactPosition;

							// ok, physx seems to give our contant normals somehow based on whichever
							// object is first in the list... so we may need to flip the contant normal, depending
							// on which object is which...

							// WARNING: can we really rely on this physx behaviour??? 
							// (so that normal direction is always from po2 toward po1?)

							if (pc.obj1 != NULL && pc.obj2 != NULL)
							{
								// for now, the flipping solution is:
								//   - normal should always point towards a possible unit
								//   - else, normal should always point towards a dynamic object (away from static surface)

								bool po2isunit = false;
								// WARNING: unsafe cast!
								AbstractPhysicsObject *apo2 = (AbstractPhysicsObject *)pc.obj2;
								int id = (int)(apo2->getCustomData());
								if (id != 0)
								{
									// bit 32 tells us if this is unit or terr.object
									if ((id & 0x80000000) == 0)
									{
										po2isunit = true;
									}
								}

								if (po2isunit)
								{
									pc.contactNormal = -pc.contactNormal;
								} else {
									if (pc.physicsObject2->isDynamic() && !pc.physicsObject1->isDynamic())
									{
										pc.contactNormal = -pc.contactNormal;
									}
								}

							for (int j = 0; j < (int)physicsContactListeners.size(); j++)
							{
								physicsContactListeners[j]->physicsContact(pc);
							}
						}
					}
				}
			}
			}
#endif
#ifdef PHYSICS_ODE
			PhysicsActorOde::finishSimulation();
#endif
			running = false;
		}



		void addNewObject(IGamePhysicsObject *obj)
		{
			if (!SimpleOptions::getBool(DH_OPT_B_PHYSICS_ENABLED))
			{
				return;
			}

			interfaceObjectsToAdd.push_back(obj);
		}



		void removeObject(IGamePhysicsObject *obj)
		{
#ifdef PHYSICS_PHYSX
			// cannot do this, as some objects (coop player units) may be deleted before physics lib is created!
			/*
			if (physicsLib == NULL)
			{
				return;
			}
			*/
#endif

			// maybe we have not even created an implementation yet, if so, just drop from the add list...
			for (int i = 0; i < (int)interfaceObjectsToAdd.size(); i++)
			{
				if (interfaceObjectsToAdd[i] == obj)
				{
					interfaceObjectsToAdd.erase(interfaceObjectsToAdd.begin() + i);
					return;
				}
			}

			// ok, the implementation existed, set the delete flag...
			int handle = obj->getHandle();

			PhysicsHandleHash::iterator iter = physicsObjectsHash.find(handle);
			if (iter != physicsObjectsHash.end())
			{
				int vecpos = (*iter).second;
				if (!physicsObjects[vecpos].deleted)
				{
					physicsObjects[vecpos].deleteRequestFlag = true;
				} else {
					assert(!"GamePhysicsImpl::deleteObject - Attempt to delete an object containing a handle for an already deleted object, data integrity error (bug).");
				}
				
			} else {
				//assert(!"GamePhysicsImpl::deleteObject - Attempt to delete an object that was not found in object list.");
			}
		}

		PHYSICS_ACTOR getImplementingBaseObject(IGamePhysicsObject *interfaceObject)
		{
			if(!interfaceObject)
				return PHYSICS_ACTOR ();

			int handle = interfaceObject->getHandle();
			if (handle == 0)
				return PHYSICS_ACTOR ();

			PhysicsHandleHash::iterator iter = physicsObjectsHash.find(handle);
			if (iter != physicsObjectsHash.end())
			{
				int arraypos = (*iter).second;
				if(physicsObjects[arraypos].deleteRequestFlag)
					return PHYSICS_ACTOR ();

				assert((int)physicsObjects.size() > arraypos);
				assert(!physicsObjects[arraypos].deleted);
				assert(physicsObjects[arraypos].handle == handle);
				assert(physicsObjects[arraypos].interfaceObject != NULL);

				if(physicsObjects[arraypos].deleted)
				{
					Logger::getInstance()->error("GamePhysics::getImplementingObject - Requested an object by handle which maps to a deleted object.");
					return PHYSICS_ACTOR ();
				}

				return PHYSICS_ACTOR(physicsObjects[arraypos].implObject);
			} else {
				return PHYSICS_ACTOR ();
			}

		}


		friend class GamePhysics;
		friend class IGamePhysicsObject;
	};



	GamePhysics::GamePhysics()
	{
		impl = new GamePhysicsImpl();
	}

	GamePhysics::~GamePhysics()
	{
		delete impl;
	}

	void GamePhysics::createPhysics(IGamePhysicsScriptRunner *scriptRunner)
	{
		impl->createPhysics(scriptRunner);
	}

	void GamePhysics::deletePhysics()
	{
		impl->deletePhysics();
	}

	void GamePhysics::prepareForDelete()
	{
		impl->prepareForDelete();
	}

	void GamePhysics::runPhysics(IStorm3D_Scene *stormScene, frozenbyte::particle::ParticleEffectManager *particleEffectManager)
	{
		impl->runPhysics(stormScene, particleEffectManager);
	}

	void GamePhysics::addNewObject(IGamePhysicsObject *obj)
	{
		//char foo[256];
		//sprintf(foo, "adding %p", obj);
		//Logger::getInstance()->error(foo);

		impl->addNewObject(obj);
	}

	// NOTICE: this does NOT delete the object itself!!!
	// do that manually after calling this!
	void GamePhysics::removeObject(IGamePhysicsObject *obj)
	{
		//char foo[256];
		//sprintf(foo, "removing %p", obj);
		//Logger::getInstance()->error(foo);

		impl->removeObject(obj);
	}

#ifdef PHYSICS_PHYSX
	frozenbyte::physics::PhysicsLib *GamePhysics::getPhysicsLib()
	{
		return impl->physicsLib;
	}
#endif

	void GamePhysics::renderedScene()
	{
		impl->renderedScene();
	}

	void GamePhysics::addPhysicsContactListener(IPhysicsContactListener *contactListener)
	{
		impl->physicsContactListeners.push_back(contactListener);
	}

	void GamePhysics::removePhysicsContactListener(IPhysicsContactListener *contactListener)
	{
		for (int i = 0; i < (int)impl->physicsContactListeners.size(); i++)
		{
			if (impl->physicsContactListeners[i] == contactListener)
			{
				impl->physicsContactListeners.erase(impl->physicsContactListeners.begin() + i);
				return;
			}
		}
		assert(!"GamePhysics::removePhysicsContactListener - given listener was not found in list.");
	}

	void GamePhysics::setGroundFocusHeight(float height)
	{
		impl->setGroundFocusHeight(height);
	}

	void GamePhysics::createFluidPushPoint(const VC3 &position, float range, int timeTicks)
	{
		if (timeTicks < 0)
			timeTicks = 1;
		impl->createFluidPushPoint(position, range, timeTicks);
	}

	IGamePhysicsObject *GamePhysics::getInterfaceObjectForHandle(int handle)
	{
		return impl->getInterfaceObjectForHandle(handle);
	}

	void GamePhysics::setIgnoreContacts(bool ignoreContacts)
	{
		impl->ignoreContacts = ignoreContacts;
	}

#ifdef PHYSICS_PHYSX
#if !defined PROJECT_SURVIVOR && !defined PROJECT_SHADOWGROUNDS
	void GamePhysics::addJoint(boost::shared_ptr<AbstractPhysicsObject> &objectA, boost::shared_ptr<AbstractPhysicsObject> &objectB, const frozenbyte::physics::PhysicsJoint &joint, const std::string &id)
	{
		JointCreationInfo info;
		info.objectA = objectA;
		info.objectB = objectB;
		info.id = id;
		info.joint = joint;

		impl->jointCreationList.push_back(info);
	}

	void GamePhysics::deleteJoints()
	{
		impl->jointCreationList.clear();
		impl->jointList.clear();
	}
#endif
#endif

#ifdef PROJECT_CLAW_PROTO
	NxActor *GamePhysics::getImplementingObject(IGamePhysicsObject *interfaceObject)
	{
		boost::shared_ptr<frozenbyte::physics::ActorBase> base = impl->getImplementingBaseObject(interfaceObject);
		if(base)
			return base->getActor();

		return 0;

		/*
		if(!interfaceObject)
			return 0;

		int handle = interfaceObject->getHandle();

		//assert(handle != 0);
		if (handle == 0)
			return NULL;

		PhysicsHandleHash::iterator iter = impl->physicsObjectsHash.find(handle);
		if (iter != impl->physicsObjectsHash.end())
		{
			int arraypos = (*iter).second;

			if(impl->physicsObjects[arraypos].deleteRequestFlag)
				return NULL;

			assert((int)impl->physicsObjects.size() > arraypos);
			assert(!impl->physicsObjects[arraypos].deleted);
			assert(impl->physicsObjects[arraypos].handle == handle);
			assert(impl->physicsObjects[arraypos].interfaceObject != NULL);

			if(impl->physicsObjects[arraypos].deleted)
			{
				Logger::getInstance()->error("GamePhysics::getImplementingObject - Requested an object by handle which maps to a deleted object.");
				return NULL;
			}

			return impl->physicsObjects[arraypos].implObject->getActor();
		} else {
			return NULL;
		}
		*/
	}

	boost::shared_ptr<frozenbyte::physics::ActorBase> GamePhysics::getImplementingBaseObject(IGamePhysicsObject *interfaceObject)
	{
		return impl->getImplementingBaseObject(interfaceObject);
	}

#endif

}


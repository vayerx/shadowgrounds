
#include "precompiled.h"

#include <boost/lexical_cast.hpp>
#include <map>

#include "physics_lib.h"
#include "box_actor.h"
#include "convex_actor.h"
#include "../convert/str2int.h"
#include "capsule_actor.h"
#include "static_mesh_actor.h"
#include "heightmap_actor.h"
#include "rack_actor.h"
#include "spherical_joint.h"
#include "fluid.h"
#include "NxPhysics.h"
#include "../system/Logger.h"
#include "../game/physics/physics_collisiongroups.h"
#include "igios.h"

#ifdef PROJECT_CLAW_PROTO
#include "car_actor.h"
#include "physics_lib.h"
NxMaterial * frozenbyte::physics::PhysicsLib::unitMaterial = NULL;
#endif

#if defined PHYSICS_PHYSX && defined _MSC_VER
//#pragma comment(lib, "PhysXLoaderDEBUG.lib")
#pragma comment(lib, "PhysXLoader.lib")
#endif

int frozenbyte::physics::physicslib_group_cont[PHYSICS_MAX_COLLISIONGROUPS][PHYSICS_MAX_COLLISIONGROUPS] = { { PHYSICSLIB_GROUP_CONTACT_NONE } };
bool frozenbyte::physics::physicslib_group_coll[PHYSICS_MAX_COLLISIONGROUPS][PHYSICS_MAX_COLLISIONGROUPS] = { { false } };


namespace frozenbyte {
namespace physics {

// HACK: ...
NxPhysicsSDK *physxSDK = NULL;



	// TEMP!!!
	struct Logger: public NxUserOutputStream
	{
		void reportError(NxErrorCode code, const char *message, const char *file, int line)
		{
			print(message);
		}
	 
		NxAssertResponse reportAssertViolation(const char *message, const char *file, int line)
		{
			print(message);
			return NX_AR_CONTINUE;
		}
	 
		void print(const char *message)
		{
			//MessageBox(0, message, "Shit happens", MB_OK);

			igiosWarning("physics error: %s\n", message);
			std::string msgstr = std::string("physics_lib - ") + message;
			::Logger::getInstance()->error(msgstr.c_str());

			assert(!"Physics error");
		}
	};

	struct ContactReport: public NxUserContactReport
	{
		int contactEventAmount;
		std::vector<ContactListEntry> contactList;

		ContactReport()
		:	contactEventAmount(0)
		{
		}

		void onContactNotify(NxContactPair &pair, NxU32 events)
		{
			contactEventAmount++;

			/*
			if((events & NX_NOTIFY_ON_START_TOUCH) != 0
				|| (events & NX_NOTIFY_ON_TOUCH) != 0)
			{
				//++contacts;
				// continue processing the contact...
			} else {
				return;
			}
			*/

			//NxReal maxForceLen = 0.0f;
			NxReal maxForceLen = -99999.9f;
			NxVec3 maxContactNormal(0, 0, 0);
			NxVec3 maxContactPosition(0, 0, 0);

			NxContactStreamIterator i(pair.stream);
			while(i.goNextPair())
			{
				while(i.goNextPatch())
				{
					const NxVec3 &contactNormal = i.getPatchNormal();
					while(i.goNextPoint())
					{
						const NxVec3& contactPoint = i.getPoint();

						assert(i.getShapeFlags() & NX_SF_POINT_CONTACT_FORCE);

						//NxVec3 contactForce = contactNormal * i.getPointNormalForce(); 
						NxReal forceLen = i.getPointNormalForce();

						if (forceLen > maxForceLen)
						{
							maxContactNormal = contactNormal;
							maxContactPosition = contactPoint;
							maxForceLen = forceLen;
						}
					}
				}
			}

//			if (maxForceLen > 0.0f)
			if (maxForceLen >= 0.0f)
			{
				float contactForceLen = maxForceLen;
				VC3 contactNormal = VC3(maxContactNormal.x, maxContactNormal.y, maxContactNormal.z);

				VC3 contactPosition = VC3(maxContactPosition.x, maxContactPosition.y, maxContactPosition.z);

				// WARNING: unsafe void * to ActorBase * cast!
				ActorBase *contactActor1 = (ActorBase *)(pair.actors[0]->userData); 
				ActorBase *contactActor2 = (ActorBase *)(pair.actors[1]->userData);
				
				ContactListEntry entry(contactActor1, contactActor2, contactNormal, contactPosition, contactForceLen);
				contactList.push_back(entry);
			}
		}
	};

	class UserNotify : public NxUserNotify
	{
	public:
		virtual bool onJointBreak(NxReal breakingForce, NxJoint & brokenJoint)
		{
			return false;
		}

		virtual void onWake(NxActor **actors, NxU32 count)
		{
		}

		virtual void onSleep(NxActor **actors, NxU32 count)
		{
		}
	};

	static boost::scoped_ptr<Logger> logger;

NxUserOutputStream *getLogger()
{
	if(!logger)
		logger.reset(new Logger());

	return logger.get();
}


PhysicsParams physics_defaultParams;

struct PhysicsLib::Data
{
	NxPhysicsSDK *sdk;
	NxScene *scene;
	bool crashed;
	int statsNumActors;
	int statsNumDynamicActors;
	int statsNumDynamicActorsInAwakeGroups;
	int statsMaxDynamicActorsInAwakeGroups;
	int statsSimulationTime;
	int statsSimulationWaitTime;
	int statsSimulationStartWaitTime;
	int startSimulationTime;
	int statsContacts;
	bool ccd;
	float ccdMaxThickness;

	ContactReport contactReport;
	UserNotify userNotify;

	typedef std::map<PhysicsLib::Feature, bool> FeatureMap;
	FeatureMap featureMap;

	bool runningInHardware;

	// fluid containment plane
	NxActor *physicslib_fluid_containment_actor;
	NxPlaneShape *physicslib_fluid_containment_shape;
	// fluid containment sphere
	NxActor *physicslib_fluid_containment_sphere_actor;
	NxCapsuleShape *physicslib_fluid_containment_sphere_shape;

	Data(bool useHardware, bool useHardwareOnly, bool useMultithreading, PhysicsParams *params)
	:	sdk(0),
		scene(0),
		crashed(false),

		statsNumActors(0),
		statsNumDynamicActors(0),
		statsNumDynamicActorsInAwakeGroups(0),
		statsMaxDynamicActorsInAwakeGroups(0),
		statsSimulationTime(0),
		statsSimulationWaitTime(0),
		statsSimulationStartWaitTime(0),
		startSimulationTime(0),

		statsContacts(0),
		ccd(false),
		ccdMaxThickness(0.5f),

		runningInHardware(false),
		physicslib_fluid_containment_actor(0),
		physicslib_fluid_containment_shape(0),
		physicslib_fluid_containment_sphere_actor(0),
		physicslib_fluid_containment_sphere_shape(0)
	{
		if (params == NULL)
		{
			params = &physics_defaultParams;
			::Logger::getInstance()->debug("PhysicsLib - No physics params given, using defaults.");
		}

		if (params->scriptRunner == NULL)
		{
			::Logger::getInstance()->warning("PhysicsLib - No script runner given, collision groups, etc. will be initialized to default values.");
		}

		NxPhysicsSDKDesc sdkDesc;
		if(!useHardware)
			sdkDesc.flags |= NX_SDKF_NO_HARDWARE;

		sdk = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, getLogger(), sdkDesc);
		//sdk = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);

		// HACK: ...
		physxSDK = sdk;

		if(sdk)
		{
			if(sdk->getHWVersion() == NX_HW_VERSION_NONE)
				useHardware = false;

			if (params->ccd)
			{
				sdk->setParameter(NX_CONTINUOUS_CD, true);
				sdk->setParameter(NX_CCD_EPSILON, 0.001f);
			}
			this->ccd = params->ccd;
			this->ccdMaxThickness = params->ccdMaxThickness;

			NxSceneDesc sceneDesc;
			sceneDesc.gravity = NxVec3(params->gravity.x, params->gravity.y, params->gravity.z);	
			sceneDesc.userContactReport = &contactReport;
			//sceneDesc.userNotify = &userNotify;

			if(useHardware)
			{
				sceneDesc.simType = NX_SIMULATION_HW;
				if (useHardwareOnly)
				{
					sceneDesc.flags |= NX_SF_RESTRICTED_SCENE;
				}
			}
			else
			{
				sceneDesc.simType = NX_SIMULATION_SW;
			}

			if (!useMultithreading)
			{
				// Disable threading
				sceneDesc.flags = 0;
			}

			runningInHardware = useHardware;

			sceneDesc.flags |= NX_SF_ENABLE_ACTIVETRANSFORMS;
			scene = sdk->createScene(sceneDesc);
			if(scene)
			{
				NxMaterial *defaultMaterial = scene->getMaterialFromIndex(0);
				defaultMaterial->setStaticFriction(params->defaultStaticFriction);
				defaultMaterial->setDynamicFriction(params->defaultDynamicFriction);
				defaultMaterial->setRestitution(params->defaultRestitution);

#ifdef PROJECT_CLAW_PROTO
				// Create material for cops (larger friction, no restitution)
				NxMaterialDesc materialDesc;
				materialDesc.restitution = 0.0f;   
				materialDesc.restitutionCombineMode = NX_CM_MIN;
				materialDesc.staticFriction = 10.0f;
				materialDesc.dynamicFriction = 10.0f;
				unitMaterial = scene->createMaterial(materialDesc);
#endif

				sdk->setParameter(NX_VISUALIZATION_SCALE, 1.f);

#ifndef PROJECT_SHADOWGROUNDS
				sdk->setParameter(NX_SKIN_WIDTH, 0.01f);
#endif

				// NEW: the new scriptable collision/contact group initialization
				int contactFlags1 = NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_FORCES;
#ifdef PHYSICS_FEEDBACK
				int contactFlags2 = NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_FORCES;
#else
				int contactFlags2 = NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_FORCES;
#endif
				if (params->scriptRunner != NULL)
				{
					bool ok = params->scriptRunner->runPhysicsLibScript("physics_init", "set_contacts");
					if (!ok)
						::Logger::getInstance()->error("PhysicsLib - Failed to run physics_init:set_contacts script (script/sub may not be loaded or did not return expected value).");
				}

				for (int i = 0; i < PHYSICS_MAX_COLLISIONGROUPS; i++)
				{
					for (int j = 0; j < PHYSICS_MAX_COLLISIONGROUPS; j++)
					{
						if (physicslib_group_cont[i][j] != physicslib_group_cont[j][i])
						{
							::Logger::getInstance()->error("PhysicsLib - Improperly mirrored physics contacts data (group1 -> group2 does not have same flag as group2 -> group1).");
							::Logger::getInstance()->debug("contacts data group numbers follow:");
							::Logger::getInstance()->debug(int2str(i));
							::Logger::getInstance()->debug(int2str(j));
						}
						if (physicslib_group_cont[i][j] == PHYSICSLIB_GROUP_CONTACT_FLAGS1)
							scene->setActorGroupPairFlags(i, j, contactFlags1);
						else if (physicslib_group_cont[i][j] == PHYSICSLIB_GROUP_CONTACT_FLAGS2)
							scene->setActorGroupPairFlags(i, j, contactFlags2);
					}
				}

				if (params->scriptRunner != NULL)
				{
					bool ok = params->scriptRunner->runPhysicsLibScript("physics_init", "set_collisions");
					if (!ok)
						::Logger::getInstance()->error("PhysicsLib - Failed to run physics_init:set_collision script (script/sub may not be loaded or did not return expected value).");
				}

				for (int i = 0; i < PHYSICS_MAX_COLLISIONGROUPS; i++)
				{
					for (int j = 0; j < PHYSICS_MAX_COLLISIONGROUPS; j++)
					{
						if (physicslib_group_coll[i][j] != physicslib_group_coll[j][i])
						{
							::Logger::getInstance()->error("PhysicsLib - Improperly mirrored physics collision data (group1 -> group2 does not have same flag as group2 -> group1).");
							::Logger::getInstance()->debug("collision data group numbers follow:");
							::Logger::getInstance()->debug(int2str(i));
							::Logger::getInstance()->debug(int2str(j));
						}
						scene->setGroupCollisionFlag(i, j, physicslib_group_coll[i][j]);
					}
				}

// OLD SCHEISSE...
/*
				// Contact groups
				{
					int contactFlags1 = NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_FORCES;
#ifdef PHYSICS_FEEDBACK
					int contactFlags2 = NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_FORCES;
#else
					int contactFlags2 = NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_FORCES;
#endif
					//int contactFlags3 = NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_FORCES;

					scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_STATIC, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, contactFlags2);
					scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_STATIC, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, contactFlags2);
					scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_STATIC, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, contactFlags1);
					scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, contactFlags2);
					scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, contactFlags2);
					//scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, contactFlags2);
scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, contactFlags1);
					scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, PHYSICS_COLLISIONGROUP_UNITS, contactFlags2);
scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_STATIC, PHYSICS_COLLISIONGROUP_UNITS, contactFlags2);
scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_CLAW, PHYSICS_COLLISIONGROUP_STATIC, contactFlags1);
					//scene->setActorGroupPairFlags(PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, PHYSICS_COLLISIONGROUP_UNITS, contactFlags3);
				}

				// Collision groups
				{
					// claw
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_CLAW, PHYSICS_COLLISIONGROUP_CLAW, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_CLAW, PHYSICS_COLLISIONGROUP_NOCOLLISION, false);
scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_CLAW, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_CLAW, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_CLAW, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_CLAW, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_CLAW, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_CLAW, PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, false);

					// Model particles
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, false);

					// Doors
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_DOORS, PHYSICS_COLLISIONGROUP_DOORS, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_DOORS, PHYSICS_COLLISIONGROUP_STATIC, false);

					// Units and model particles
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_UNITS, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_UNITS, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, false);

					// Fluids
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUIDS, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUIDS, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUIDS, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUIDS, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, false);

					// Fluids w/ detailed collision
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUIDS_DETAILED, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUIDS_DETAILED, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUIDS_DETAILED, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUIDS_DETAILED, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, false);

					// Fluid containment
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_STATIC, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_UNITS, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_DOORS, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, PHYSICS_COLLISIONGROUP_FLUIDS_DETAILED, false);

					// No collision
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_STATIC, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_WO_UNIT_COLL_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_MODEL_PARTICLES_NO_SOUND, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_UNITS, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_DOORS, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES, false);
					scene->setGroupCollisionFlag(PHYSICS_COLLISIONGROUP_NOCOLLISION, PHYSICS_COLLISIONGROUP_NOCOLLISION, false);
				}
*/
			}
		}
	}

	~Data()
	{
		removeFluidContainmentSphere();

#ifdef PROJECT_CLAW_PROTO
		unitMaterial = NULL;
#endif

		if(physicslib_fluid_containment_actor != NULL)
		{
			if(scene && physicslib_fluid_containment_actor)
			{
				assert(scene->isWritable());
				scene->releaseActor(*physicslib_fluid_containment_actor);
			}

			physicslib_fluid_containment_actor = NULL;
			physicslib_fluid_containment_shape = NULL;
		}

		if(sdk && scene)
		{
#ifndef NDEBUG
			int actors = scene->getNbActors();
			assert(actors == 1 || actors == 2);

#ifndef NX_DISABLE_FLUIDS
			int fluids = scene->getNbFluids();
			assert(fluids == 0);
#endif
#endif
			scene->shutdownWorkerThreads();
			sdk->releaseScene(*scene);
		}

		if(sdk)
			sdk->release();
	}

	void applyOptions()
	{
		if(!sdk || !scene)
			return;

		//bool visualizeCollisionShapes = featureMap[PhysicsLib::VISUALIZE_COLLISION_SHAPES];
		//bool visualizeDynamic = featureMap[PhysicsLib::VISUALIZE_DYNAMIC];
		//bool visualizeStatic = featureMap[PhysicsLib::VISUALIZE_STATIC];
		//bool visualizeCollisionContacts = featureMap[PhysicsLib::VISUALIZE_COLLISION_CONTACTS];
		bool visualizeFluids = featureMap[PhysicsLib::VISUALIZE_FLUIDS];
		bool visualizeJoints = featureMap[PhysicsLib::VISUALIZE_JOINTS];
		//bool visualizeCCD = featureMap[PhysicsLib::VISUALIZE_CCD];

		sdk->setParameter(NX_VISUALIZE_FLUID_POSITION, visualizeFluids);
		sdk->setParameter(NX_VISUALIZE_FLUID_BOUNDS, visualizeFluids);
		sdk->setParameter(NX_VISUALIZE_FLUID_PACKETS, visualizeFluids);
		sdk->setParameter(NX_VISUALIZE_FLUID_KERNEL_RADIUS, visualizeFluids);

		sdk->setParameter(NX_VISUALIZE_JOINT_LIMITS, visualizeJoints);
		//sdk->setParameter(NX_VISUALIZE_JOINT_WORLD_AXES, visualizeJoints);
	}



	void removeFluidContainmentSphere()
	{
		assert(scene->isWritable());

		if(physicslib_fluid_containment_sphere_actor != NULL)
		{
			if(scene && physicslib_fluid_containment_sphere_actor)
			{
				assert(scene->isWritable());
				scene->releaseActor(*physicslib_fluid_containment_sphere_actor);
			}

			physicslib_fluid_containment_sphere_actor = NULL;
			physicslib_fluid_containment_sphere_shape = NULL;
		}
	}

};

PhysicsLib::PhysicsLib(bool useHardware, bool useHardwareOnly, bool useMultithreading, PhysicsParams *params)
:	data(new Data(useHardware, useHardwareOnly, useMultithreading, params))
{
}

PhysicsLib::~PhysicsLib()
{
}

NxPhysicsSDK *PhysicsLib::getSDK()
{
	return data->sdk;
}

NxScene *PhysicsLib::getScene()
{
	return data->scene;
}

boost::shared_ptr<ConvexMesh> PhysicsLib::createConvexMesh(const char *filename)
{
std::string msg = std::string("Loading convex cook file: ") + std::string(filename);
::Logger::getInstance()->debug(msg.c_str());

	boost::shared_ptr<ConvexMesh> mesh(new ConvexMesh(*data->sdk, filename));
::Logger::getInstance()->debug("Done");

	return mesh;
}

boost::shared_ptr<StaticMesh> PhysicsLib::createStaticMesh(const char *filename)
{
std::string msg = std::string("Loading static cook file: ") + std::string(filename);
::Logger::getInstance()->debug(msg.c_str());

	boost::shared_ptr<StaticMesh> mesh(new StaticMesh(*data->sdk, filename));
::Logger::getInstance()->debug("Done");

	return mesh;
}

/*
boost::shared_ptr<SphereActor> PhysicsLib::createSphereActor(float radius)
{
	SphereActor *actor = 0;

	if(data->scene)
	{
	}

	return boost::shared_ptr<SphereActor> (actor);
}
*/
boost::shared_ptr<BoxActor> PhysicsLib::createBoxActor(const VC3 &sizes, const VC3 &position, const VC3 &localPosition)
{
	BoxActor *actor = 0;

	if(data->scene)
	{
		actor = new BoxActor(*data->scene, sizes, position, localPosition, data->ccd, data->ccdMaxThickness);
		if(!actor->isValid())
		{
			delete actor;
			actor = 0;
		}
	}

	return boost::shared_ptr<BoxActor> (actor);
}

boost::shared_ptr<CapsuleActor> PhysicsLib::createCapsuleActor(float height, float radius, const VC3 &position, float offset, int axisNumber)
{
	CapsuleActor *actor = 0;

	if(data->scene)
	{
		actor = new CapsuleActor(*data->scene, height, radius, position, offset, axisNumber);
		if(!actor->isValid())
		{
			delete actor;
			actor = 0;
		}
	}

	return boost::shared_ptr<CapsuleActor> (actor);
}

boost::shared_ptr<ConvexActor> PhysicsLib::createConvexActor(const boost::shared_ptr<ConvexMesh> &mesh, const VC3 &position)
{
	ConvexActor *actor = 0;

	if(data->scene && mesh)
	{
		actor = new ConvexActor(*data->scene, mesh, position);
		if(!actor->isValid())
		{
			delete actor;
			actor = 0;
		}
	}

	return boost::shared_ptr<ConvexActor> (actor);
}

boost::shared_ptr<StaticMeshActor> PhysicsLib::createStaticMeshActor(const boost::shared_ptr<StaticMesh> &mesh, const VC3 &position, const QUAT &rotation)
{
	StaticMeshActor *actor = 0;

	if(data->scene && mesh)
	{
		actor = new StaticMeshActor(*data->scene, mesh, position, rotation);
		if(!actor->isValid())
		{
			delete actor;
			actor = 0;
		}
	}

	return boost::shared_ptr<StaticMeshActor> (actor);
}

boost::shared_ptr<HeightmapActor> PhysicsLib::createHeightmapActor(const unsigned short *buffer, int samplesX, int samplesY, const VC3 &size)
{
	HeightmapActor *actor = 0;

	if(data->scene)
	{
		actor = new HeightmapActor(*data->sdk, *data->scene, buffer, samplesX, samplesY, size);
		if(!actor->isValid())
		{
			delete actor;
			actor = 0;
		}
	}

	return boost::shared_ptr<HeightmapActor> (actor);
}

boost::shared_ptr<RackActor> PhysicsLib::createRackActor(const VC3 &position)
{
	RackActor *actor = 0;

	if(data->scene)
	{
		actor = new RackActor(*data->scene, position);
		if(!actor->isValid())
		{
			delete actor;
			actor = 0;
		}
	}

	return boost::shared_ptr<RackActor> (actor);
}

boost::shared_ptr<SphericalJoint> PhysicsLib::createSphericalJoint(boost::shared_ptr<ActorBase> &a, boost::shared_ptr<ActorBase> &b, const VC3 &globalAnchor)
{
	SphericalJoint *joint = 0;

	if(a && b && data->scene)
	{
		NxSphericalJointDesc jointDesc;
		jointDesc.setToDefault();

		jointDesc.actor[0] = a->getActor();
		jointDesc.actor[1] = b->getActor();
		jointDesc.setGlobalAnchor(NxVec3(globalAnchor.x, globalAnchor.y, globalAnchor.z));

		// Limits etc stuff?
		jointDesc.swingLimit.value = PI/20.f;

/*
		jointDesc.swingLimit.value = PI/20.f;

		static int foofoo = 0;
		int value = foofoo++;
		if(value == 0)
			jointDesc.swingLimit.value /= 2.f;
		if(value == 1)
			jointDesc.swingLimit.value /= 2.f;
		if(value == 2)
			jointDesc.swingLimit.value /= 2.f;
		if(value == 3)
			jointDesc.swingLimit.value /= 2.f;
*/
		jointDesc.swingLimit.restitution = 1.f;
		jointDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED;

		jointDesc.twistLimit.low.value = -PI/8.f;
		jointDesc.twistLimit.high.value = PI/8.f;
		jointDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED;

		joint = new SphericalJoint(*data->scene, jointDesc, a, b);
		if(!joint->isValid())
		{
			delete joint;
			joint = 0;
		}
	}

	return boost::shared_ptr<SphericalJoint> (joint);
}

#ifndef NX_DISABLE_FLUIDS
boost::shared_ptr<Fluid> PhysicsLib::createFluid(FluidType fluidType, int maxParticles, float fluidStaticRestitution, float fluidStaticAdhesion, float fluidDynamicRestitution, float fluidDynamicAdhesion, float fluidDamping, float fluidStiffness, float fluidViscosity, float fluidKernelRadiusMultiplier, float fluidRestParticlesPerMeter, float fluidRestDensity, float fluidMotionLimit, int fluidPacketSizeMultiplier, int collGroup)
{
	Fluid *fluid = 0;

	if(data->scene)
	{		
		NxFluidDesc fluidDesc;
		fluidDesc.setToDefault();

		fluidDesc.flags |= NX_FF_DISABLE_GRAVITY;
		fluidDesc.maxParticles = maxParticles;
		fluidDesc.kernelRadiusMultiplier = fluidKernelRadiusMultiplier;
		fluidDesc.restParticlesPerMeter = fluidRestParticlesPerMeter;
		fluidDesc.stiffness = fluidStiffness;
		fluidDesc.viscosity = fluidViscosity;
		fluidDesc.restDensity = fluidRestDensity;
		fluidDesc.damping = fluidDamping;
		fluidDesc.restitutionForStaticShapes = fluidStaticRestitution;
		fluidDesc.dynamicFrictionForStaticShapes = fluidStaticAdhesion;
		fluidDesc.restitutionForDynamicShapes = fluidDynamicRestitution;
		fluidDesc.dynamicFrictionForDynamicShapes = fluidDynamicAdhesion;
		fluidDesc.collisionGroup = collGroup;
		fluidDesc.packetSizeMultiplier = /*8*/ fluidPacketSizeMultiplier;

		// Optimize this
		//fluidDesc.motionLimitMultiplier = 3.f * fluidDesc.kernelRadiusMultiplier;
		fluidDesc.motionLimitMultiplier = fluidMotionLimit * fluidDesc.restParticlesPerMeter;

		if(fluidType == FLUID_TYPE_SIMPLE)
			fluidDesc.simulationMethod = NX_F_NO_PARTICLE_INTERACTION;
		else
		{
			//fluidDesc.simulationMethod = NX_F_SPH;
			fluidDesc.simulationMethod = NX_F_MIXED_MODE;
		}

		fluid = new Fluid(*data->scene, fluidDesc, data->runningInHardware);
		if(!fluid->isValid())
		{
			delete fluid;
			fluid = 0;
		}
	}

	return boost::shared_ptr<Fluid> (fluid);
}
#endif

void PhysicsLib::addGroundPlane(float height)
{
	if(!data->scene)
		return;

	assert(data->scene->isWritable());

	NxPlaneShapeDesc planeDesc;
	planeDesc.d = height;
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&planeDesc);

	data->scene->createActor(actorDesc);
}

void PhysicsLib::addSkyPlane(float height)
{
	if(!data->scene)
		return;

	NxPlaneShapeDesc planeDesc;
	planeDesc.d = -height;
	planeDesc.normal.set(NxReal(0.0),NxReal(-1.0),NxReal(0.0));
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&planeDesc);

	data->scene->createActor(actorDesc);
}

// HACK: fluid containment plane quick hack, should rewrite this...

void PhysicsLib::addFluidContainmentPlane(float height)
{
	if(!data->scene)
		return;

	assert(data->scene->isWritable());
	assert(data->physicslib_fluid_containment_actor == NULL);

	NxPlaneShapeDesc planeDesc;
	planeDesc.d = -height;
	planeDesc.normal.set(NxReal(0.0),NxReal(-1.0),NxReal(0.0));

	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&planeDesc);

	data->physicslib_fluid_containment_actor = data->scene->createActor(actorDesc);
	if(data->physicslib_fluid_containment_actor)
	{
		data->physicslib_fluid_containment_actor->setGroup(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES);

		NxShape *const*shapes = data->physicslib_fluid_containment_actor->getShapes();
		if(shapes && shapes[0])
		{
			shapes[0]->setGroup(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES);
			data->physicslib_fluid_containment_shape = shapes[0]->isPlane();
		}
	}
}

void PhysicsLib::moveFluidContainmentPlane(float height)
{
	assert(data->scene->isWritable());

	if(data->physicslib_fluid_containment_shape)
		data->physicslib_fluid_containment_shape->setPlane(NxVec3(0, -1.f, 0), -height);
}

void PhysicsLib::createOrMoveFluidContainmentSphere(const VC3 &position, float radius)
{
	assert(data->scene->isWritable());

	if(!data->scene)
		return;

	static float last_fluid_containment_sphere_radius = 0.0f;

	if (fabs(last_fluid_containment_sphere_radius - radius) > 0.2f)
	{
		data->removeFluidContainmentSphere();
		last_fluid_containment_sphere_radius = radius;
	}

	if (data->physicslib_fluid_containment_sphere_actor == NULL)
	{
		NxCapsuleShapeDesc capsDesc;
		capsDesc.radius = radius;
		capsDesc.height = 0.01f;

		NxBodyDesc bodyDesc;
		bodyDesc.flags |= NX_BF_DISABLE_GRAVITY;

		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&capsDesc);
		actorDesc.body = &bodyDesc;
		actorDesc.density = 10.0f;
		actorDesc.globalPose.t.set(NxVec3(position.x, position.y, position.z));

		data->physicslib_fluid_containment_sphere_actor = data->scene->createActor(actorDesc);
		if(data->physicslib_fluid_containment_sphere_actor)
		{
			data->physicslib_fluid_containment_sphere_actor->setGroup(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES);

			NxShape *const*shapes = data->physicslib_fluid_containment_sphere_actor->getShapes();
			if(shapes && shapes[0])
			{
				shapes[0]->setGroup(PHYSICS_COLLISIONGROUP_FLUID_CONTAINMENT_PLANES);
				data->physicslib_fluid_containment_sphere_shape = shapes[0]->isCapsule();
			}
		}
	} else {
		// this does not work, thus, the above occasional delete and re-create hack..
		if(data->physicslib_fluid_containment_sphere_actor)
		{
			//data->physicslib_fluid_containment_sphere_shape->setDimensions(radius, 0.01f);
			data->physicslib_fluid_containment_sphere_actor->setGlobalPosition(NxVec3(position.x, position.y, position.z));
			data->physicslib_fluid_containment_sphere_actor->wakeUp();
		}
	}
}

void PhysicsLib::removeFluidContainmentSphere()
{
	data->removeFluidContainmentSphere();
}

void PhysicsLib::enableCollision(int group1, int group2, bool enable)
{
	if(!data->scene)
		return;

	data->scene->setGroupCollisionFlag(group1, group2, enable);
}

void PhysicsLib::setTimeStep(float step)
{
	if(!data->scene)
		return;

	//data->scene->setTiming(step, 32, NX_TIMESTEP_FIXED);
	//data->scene->setTiming(step, 8, NX_TIMESTEP_FIXED);
	data->scene->setTiming(step, 2, NX_TIMESTEP_FIXED);
}

void PhysicsLib::startSimulation(float timeDelta)
{
	if(!data->scene)
		return;

	data->contactReport.contactEventAmount = 0;
	data->contactReport.contactList.clear();
	data->startSimulationTime = Timer::getCurrentTime();
	data->scene->simulate(timeDelta);
	data->scene->flushStream();

	data->statsSimulationStartWaitTime = Timer::getCurrentTime() - data->startSimulationTime;
}

void PhysicsLib::finishSimulation()
{
	if(!data->scene)
		return;

	int startWaitTime = Timer::getCurrentTime();

	NxU32 errorState = 0;
	data->scene->fetchResults(NX_RIGID_BODY_FINISHED, true, &errorState);

	int endSimulationTime = Timer::getCurrentTime();
	data->statsSimulationTime = endSimulationTime - data->startSimulationTime;
	data->statsSimulationWaitTime = endSimulationTime - startWaitTime;

	NxSceneStats stats;
	data->scene->getStats(stats);

	data->statsNumActors = stats.numActors;
	data->statsNumDynamicActors = stats.numDynamicActors;
	data->statsNumDynamicActorsInAwakeGroups = stats.numDynamicActorsInAwakeGroups;
	data->statsMaxDynamicActorsInAwakeGroups = stats.maxDynamicActorsInAwakeGroups;
	data->statsContacts = data->contactReport.contactEventAmount;
	/*
	igiosWarning("Simulation stats\n" \
					"numActors = %i\n" \
					"numDynamicActors = %i\n" \
					"numDynamicActorsInAwakeGroups = %i\n" \
					"maxDynamicActorsInAwakeGroups = %i\n" \
					"contactEventAmount = %i\n" \
					"loggable stats \n%s\n",
					stats.numActors,
					stats.numDynamicActors,
					stats.numDynamicActorsInAwakeGroups,
					stats.maxDynamicActorsInAwakeGroups,
					data->contactReport.contactEventAmount,
					getLoggableStatistics().c_str());
	*/
	if(errorState)
		data->crashed = true;
}

const std::vector<ContactListEntry> &PhysicsLib::getContactList() const
{
	return data->contactReport.contactList;
}

void PhysicsLib::getActiveActors(std::vector<ActorBase *> &list, bool includeOnlyIfHasUserData) const
{
	if(data->scene)
	{
		NxU32 activeAmount = 0;
		NxActiveTransform *actors = data->scene->getActiveTransforms(activeAmount);

		for(NxU32 i = 0; i < activeAmount; ++i)
		{
			NxActiveTransform &transform = actors[i];
			if(!transform.userData)
				continue;

			ActorBase *base = reinterpret_cast<ActorBase *> (transform.userData);
			if(includeOnlyIfHasUserData && !base->getUserData())
				continue;

			list.push_back(base);
		}
	}
}

bool PhysicsLib::isRunningInHardware() const
{
	return data->runningInHardware;
}

#ifndef NX_DISABLE_FLUIDS
int PhysicsLib::getActiveFluidParticleAmount() const
{
	return physics::getFluidParticleCount();
}
#endif

std::string PhysicsLib::getLoggableStatistics() const
{
	std::string result;
	NxSceneStats stats;

	if(data->scene)
	{
		result += std::string("") + boost::lexical_cast<std::string> (data->statsNumDynamicActors) + std::string(";");
		result += std::string("") + boost::lexical_cast<std::string> (data->statsNumDynamicActorsInAwakeGroups) + std::string(";");
		result += std::string("") + boost::lexical_cast<std::string> (data->statsContacts) + std::string(";");
#ifndef NX_DISABLE_FLUIDS
		result += std::string("") + boost::lexical_cast<std::string> (getFluidBaseCount()) + std::string(";");
		result += std::string("") + boost::lexical_cast<std::string> (physics::getFluidParticleCount()) + std::string(";");
#endif
		result += std::string("") + boost::lexical_cast<std::string> (data->statsSimulationStartWaitTime) + std::string(";");
		result += std::string("") + boost::lexical_cast<std::string> (data->statsSimulationWaitTime) + std::string(";");
	}

	return result;
}

std::string PhysicsLib::getStatistics() const
{
	std::string result;
	NxSceneStats stats;

	if(data->scene)
	{
		result += std::string("Actors: ") + boost::lexical_cast<std::string> (data->statsNumActors) + std::string("\n");
		result += std::string("Dynamic actors: ") + boost::lexical_cast<std::string> (data->statsNumDynamicActors) + std::string("\n");
		result += std::string("Active actors: ") + boost::lexical_cast<std::string> (data->statsNumDynamicActorsInAwakeGroups) + std::string("\n");
		result += std::string("Max active actors: ") + boost::lexical_cast<std::string> (data->statsMaxDynamicActorsInAwakeGroups) + std::string("\n");
		result += std::string("Contacts reported: ") + boost::lexical_cast<std::string> (data->statsContacts) + std::string("\n");
#ifndef NX_DISABLE_FLUIDS
		result += std::string("Fluids: ") + boost::lexical_cast<std::string> (getFluidBaseCount()) + std::string("\n");
		result += std::string("Fluid particles: ") + boost::lexical_cast<std::string> (physics::getFluidParticleCount()) + std::string("\n");
#endif
		result += std::string("Physics time: ") + boost::lexical_cast<std::string> (data->statsSimulationTime) + std::string("\n");
		result += std::string("Physics wait for start: ") + boost::lexical_cast<std::string> (data->statsSimulationStartWaitTime) + std::string("\n");
		result += std::string("Physics wait for end: ") + boost::lexical_cast<std::string> (data->statsSimulationWaitTime);

		if(data->crashed)
			result += "\nPhysics lib is currently in ** CRASHED STATE **\n";
	}

	return result;
}

bool PhysicsLib::checkOverlapOBB(const VC3 &center, const VC3 &radius, const QUAT &rotation, CollisionType collisionType) const
{
	if(!data->scene)
		return false;

	NxBox box;
	box.center.set(center.x, center.y, center.z);
	box.extents.set(radius.x, radius.y, radius.z);
	box.rot.id();

	NxShapesType type = NX_DYNAMIC_SHAPES;
	if(collisionType == CollisionStatic)
		type = NX_STATIC_SHAPES;
	else if (collisionType == CollisionAll)
		type = NX_ALL_SHAPES;

	return data->scene->checkOverlapOBB(box, type);
}

void PhysicsLib::enableFeature(PhysicsLib::Feature feature, bool enable)
{
	if(!data->sdk || !data->scene)
		return;

	if(!data->scene->isWritable())
	{
		::Logger::getInstance()->warning("PhysicsLib::enableFeature - Attempt to set physics features while physics running (pause game/physics update first).");
		return;
	}

	data->featureMap[feature] = enable;
	data->applyOptions();

	// not necessarily a good idea? will cause a huge hit for this frame until proper settings get updated..
	updateVisualization(VC3(0,0,0), 999999.0f, true);
}

#ifdef PROJECT_CLAW_PROTO
boost::shared_ptr<CarActor> PhysicsLib::createCarActor(const VC3 &position)
{
	CarActor *actor = 0;

	if(data->scene)
	{
		actor = new CarActor(*data->scene, position);
		if(!actor->isValid())
		{
			delete actor;
			actor = 0;
		}
	}

	return boost::shared_ptr<CarActor> (actor);
}
#endif

void PhysicsLib::updateVisualization(const VC3 &cameraPosition, float range, bool forceUpdate)
{
	bool visualizeCollisionShapes = data->featureMap[PhysicsLib::VISUALIZE_COLLISION_SHAPES];
	bool visualizeDynamic = data->featureMap[PhysicsLib::VISUALIZE_DYNAMIC];
	bool visualizeStatic = data->featureMap[PhysicsLib::VISUALIZE_STATIC];
	bool visualizeCollisionContacts = data->featureMap[PhysicsLib::VISUALIZE_COLLISION_CONTACTS];
	bool visualizeFluids = data->featureMap[PhysicsLib::VISUALIZE_FLUIDS];
	bool visualizeJoints = data->featureMap[PhysicsLib::VISUALIZE_JOINTS];
	bool visualizeCCD = data->featureMap[PhysicsLib::VISUALIZE_CCD];

	if (forceUpdate
		|| visualizeCollisionShapes
		|| visualizeDynamic
		|| visualizeStatic
		|| visualizeCollisionContacts
		|| visualizeFluids
		|| visualizeJoints
		|| visualizeCCD)
	{
		// (do the update)
	} else {
		// do not unnecessarily do this stuff!
		return;
	}

	float rangeSq = range * range;

	int actorAmount = data->scene->getNbActors();
	NxActor **actorArray = data->scene->getActors();

	if(visualizeCollisionShapes || visualizeStatic || visualizeCollisionContacts)
		data->sdk->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	else
		data->sdk->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 0);

	if(visualizeDynamic)
		data->sdk->setParameter(NX_VISUALIZE_BODY_MASS_AXES, 1);
	else
		data->sdk->setParameter(NX_VISUALIZE_BODY_MASS_AXES, 0);

	data->sdk->setParameter(NX_VISUALIZE_CONTACT_NORMAL, visualizeCollisionContacts);
	data->sdk->setParameter(NX_VISUALIZE_CONTACT_FORCE, visualizeCollisionContacts);
	data->sdk->setParameter(NX_VISUALIZE_CONTACT_POINT, visualizeCollisionContacts);

	data->sdk->setParameter(NX_VISUALIZE_COLLISION_SKELETONS, visualizeCCD);	

	for(int i = 0; i < actorAmount; ++i)
	{
		NxActor *actor = actorArray[i];
		if(!actor)
			continue;

		NxVec3 nxpos = actor->getGlobalPosition();
		VC3 pos(nxpos.x, nxpos.y, nxpos.z);
		VC3 diff = (pos - cameraPosition);
		diff.y = 0; // ignore height

		bool inRange = false;		
		if (diff.GetSquareLength() < rangeSq)
			inRange = true;

		if(actor->isDynamic())
		{
			//if(visualizeDynamic && inRange)
			if(visualizeDynamic)
				actor->raiseBodyFlag(NX_BF_VISUALIZATION);
			else
				actor->clearBodyFlag(NX_BF_VISUALIZATION);
		}

		int shapeAmount = actor->getNbShapes();
		NxShape *const*shapes = actor->getShapes();

		while(shapeAmount--)
		{
			NxShape *shape = shapes[shapeAmount];

			if(actor->isDynamic())
			{
				//if(visualizeCollisionShapes && inRange)
				if(visualizeCollisionShapes)
					shape->setFlag(NX_SF_VISUALIZATION, true);
				else
					shape->setFlag(NX_SF_VISUALIZATION, false);
			}
			else
			{
				if(visualizeStatic && !shape->isHeightField() && inRange)
					shape->setFlag(NX_SF_VISUALIZATION, true);
				else
					shape->setFlag(NX_SF_VISUALIZATION, false);
			}
		}
	}
}

void PhysicsLib::connectToRemoteDebugger(const char *host, unsigned short port)
{
	data->sdk->getFoundationSDK().getRemoteDebugger()->connect(host, port);
}



} // physics
} // frozenbyte

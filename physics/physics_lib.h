#ifndef INCLUDED_FROZENBYTE_PHYSICS_LIB_H
#define INCLUDED_FROZENBYTE_PHYSICS_LIB_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <datatypedef.h>
#include <string>
#include <vector>
#include "IPhysicsLibScriptRunner.h"
#include "../game/physics/physics_collisiongroups.h"


#define PHYSICSLIB_GROUP_CONTACT_NONE 0
#define PHYSICSLIB_GROUP_CONTACT_FLAGS1 1
#define PHYSICSLIB_GROUP_CONTACT_FLAGS2 2


class NxPhysicsSDK;
class NxScene;
class NxMaterial;

namespace frozenbyte {
namespace physics {

// HACK: too lazy to make any proper methods for these..
extern int physicslib_group_cont[PHYSICS_MAX_COLLISIONGROUPS][PHYSICS_MAX_COLLISIONGROUPS];
extern bool physicslib_group_coll[PHYSICS_MAX_COLLISIONGROUPS][PHYSICS_MAX_COLLISIONGROUPS];


class ConvexMesh;
class StaticMesh;
class SphereActor;
class BoxActor;
class CapsuleActor;
class ConvexActor;
class StaticMeshActor;
class HeightmapActor;
class RackActor;
class Fluid;
class ActorBase;
class SphericalJoint;
class D6Joint;

#ifdef PROJECT_CLAW_PROTO
class CarActor;
#endif

struct ContactListEntry
{
	ContactListEntry(
		const ActorBase *actor1, 
		const ActorBase *actor2,
		const VC3 &contactNormal,
		const VC3 &contactPosition,
		float contactForce)
	{
		this->actor1 = actor1;
		this->actor2 = actor2;
		this->contactForce = contactForce;
		this->contactNormal = contactNormal;
		this->contactPosition = contactPosition;
	}

	const ActorBase *actor1;
	const ActorBase *actor2;
	float contactForce;
	VC3 contactNormal;
	VC3 contactPosition;
};


// trying to get some sense into physics lib init with this... --jpk
class PhysicsParams
{
public:
	VC3 gravity;
	float defaultDynamicFriction;
	float defaultStaticFriction;
	float defaultRestitution;
	IPhysicsLibScriptRunner *scriptRunner;
	bool ccd;
	float ccdMaxThickness;

	PhysicsParams()
		: gravity(0, -9.81f, 0),
		defaultDynamicFriction(0.8f),
		defaultStaticFriction(0.8f),
		defaultRestitution(0.3f),
		scriptRunner(NULL),
		ccd(true),
		ccdMaxThickness(0.5f)
	{
	}
};

struct PhysicsJoint
{
	struct Limit
	{
		VC3 angle;
		VC3 restitution;
		VC3 spring;
		VC3 damping;

		bool hasX() const
		{
			return angle.x != 0.f || spring.x != 0.f;
		}

		bool hasY() const
		{
			return angle.y != 0.f || spring.y != 0.f;
		}

		bool hasZ() const
		{
			return angle.z != 0.f || spring.z != 0.f;
		}
	};

	VC3 globalAnchor;
	VC3 globalAxis;
	VC3 globalNormal;

	// Only x-angle supports low and high limits. Others use only high settings.
	Limit low;
	Limit high;

	float breakForce;

	PhysicsJoint()
	:	breakForce(0.f)
	{
	}
};

class JointDeformingListener
{
public:
	virtual void onJointBreak(D6Joint *jointBase, const VC3 &position) = 0;
};

class JointDeformingInfo
{
public:
	JointDeformingInfo(float bendAngle, float breakAngle, float durability, int resetCollisionGroupOnBreak = -1, float resetAngularDampingOnBreak = -1.0f) :
			bendAngle(bendAngle),
			breakAngle(breakAngle),
			durability(durability),
			resetCollisionGroupOnBreak(resetCollisionGroupOnBreak),
			resetAngularDampingOnBreak(resetAngularDampingOnBreak)
	{}

	JointDeformingInfo() : resetCollisionGroupOnBreak(-1), resetAngularDampingOnBreak(-1.0f) {}
public:
	float bendAngle;
	float breakAngle;
	float durability;
	float resetAngularDampingOnBreak;
	int resetCollisionGroupOnBreak;
};

class PhysicsLib
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	PhysicsLib(bool useHardware, bool useHardwareOnly, bool useMultithreading, PhysicsParams *params = NULL);
	~PhysicsLib();

	NxPhysicsSDK *getSDK();
	NxScene *getScene();

	// Geometry data (cooked file)
	boost::shared_ptr<ConvexMesh> createConvexMesh(const char *filename);
	boost::shared_ptr<StaticMesh> createStaticMesh(const char *filename);

	// Actors
	boost::shared_ptr<SphereActor> createSphereActor(float radius, const VC3 &position);
	boost::shared_ptr<BoxActor> createBoxActor(const VC3 &sizes, const VC3 &position, const VC3 &localPosition = VC3());
	boost::shared_ptr<BoxActor> createBoxActor(const std::string &shapes, const VC3 &position);
	boost::shared_ptr<CapsuleActor> createCapsuleActor(float height, float radius, const VC3 &position, float offset = 0.0f, int axisNumber = 1);
	boost::shared_ptr<ConvexActor> createConvexActor(const boost::shared_ptr<ConvexMesh> &mesh, const VC3 &position);
	boost::shared_ptr<StaticMeshActor> createStaticMeshActor(const boost::shared_ptr<StaticMesh> &mesh, const VC3 &position, const QUAT &rotation);
	boost::shared_ptr<HeightmapActor> createHeightmapActor(const unsigned short *buffer, int samplesX, int samplesY, const VC3 &size);
	boost::shared_ptr<RackActor> createRackActor(const VC3 &position);
#ifdef PROJECT_CLAW_PROTO
	boost::shared_ptr<CarActor> createCarActor(const VC3 &position);
	static NxMaterial * unitMaterial;
#endif

	// Joints
	boost::shared_ptr<SphericalJoint> createSphericalJoint(boost::shared_ptr<ActorBase> &a, boost::shared_ptr<ActorBase> &b, const VC3 &globalAnchor);
	boost::shared_ptr<D6Joint> createGeneralJoint(boost::shared_ptr<ActorBase> &a, boost::shared_ptr<ActorBase> &b, const PhysicsJoint &joint);

	// Fluid
	enum FluidType
	{
		FLUID_TYPE_SIMPLE,
		FLUID_TYPE_INTERACTION
	};

	boost::shared_ptr<Fluid> createFluid(FluidType fluidType, int maxParticles, float fluidStaticRestitution, float fluidStaticAdhesion, float fluidDynamicRestitution, float fluidDynamicAdhesion, float fluidDamping, float fluidStiffness, float fluidViscosity, float fluidKernelRadiusMultiplier, float fluidRestParticlesPerMeter, float fluidRestDensity, float fluidMotionLimit, int fluidPacketSizeMultiplier, int collGroup);

	// Misc
	void addGroundPlane(float height);
	void addSkyPlane(float height);
	void enableCollision(int group1, int group2, bool enable);

	// Fluid containment (to max height)
	void addFluidContainmentPlane(float height);
	void moveFluidContainmentPlane(float height);

	// Fluid containment sphere (used by explosions, etc.)
	void createOrMoveFluidContainmentSphere(const VC3 &position, float radius);
	void removeFluidContainmentSphere();

	// Simulation
	void setTimeStep(float step);
	void startSimulation(float timeDelta);
	void finishSimulation();

	void updateVisualization(const VC3 &cameraPosition, float range, bool forceUpdate);

	void connectToRemoteDebugger(const char *host, unsigned short port);

	// Get the list of contact events
	// Should be called right after finishSimulation, before creating or deleting any actors 
	const std::vector<ContactListEntry> &getContactList() const;
	void getActiveActors(std::vector<ActorBase *> &list, bool includeOnlyIfHasUserData) const;

	enum CollisionType
	{
		CollisionStatic,
		CollisionDynamic,
		CollisionAll,
	};

	bool checkOverlapOBB(const VC3 &center, const VC3 &radius, const QUAT &rotation, CollisionType collisionType) const;

	enum Feature
	{
		VISUALIZE_COLLISION_SHAPES,
		VISUALIZE_DYNAMIC,
		VISUALIZE_STATIC,
		VISUALIZE_COLLISION_CONTACTS,
		VISUALIZE_FLUIDS,
		VISUALIZE_JOINTS,
		VISUALIZE_CCD
	};

	void enableFeature(PhysicsLib::Feature feature, bool enable);

	bool isRunningInHardware() const;
	int getActiveFluidParticleAmount() const;
	std::string getStatistics() const;
	std::string getLoggableStatistics() const;
};

} // physics
} // frozenbyte

#endif

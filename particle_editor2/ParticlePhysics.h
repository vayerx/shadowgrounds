#ifndef FROZENBYTE_PARTICLE_PHYSICS_H
#define FROZENBYTE_PARTICLE_PHYSICS_H

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <datatypedef.h>
#include <vector>

class IStorm3D_Model_Object;

namespace frozenbyte {
namespace physics {
	class ConvexMesh;
	class ActorBase;
	class PhysicsLib;
	class Fluid;
} // physics

namespace particle {

class PhysicsMesh;
class PhysicsActor;
class PhysicsFluid;

class ParticlePhysics
{
	struct Data;
	boost::shared_ptr<Data> data;
	friend class PhysicsActor;
	friend class PhysicsFluid;

public:
	ParticlePhysics();
	~ParticlePhysics();

	void setMaxParticleAmount(int amount);
	void setMaxParticleSpawnAmount(int amount);
	void physicsExplosion(const VC3 &position, float forceFactor, float radius);

	boost::shared_ptr<PhysicsMesh> createConvexMesh(const char *filename, IStorm3D_Model_Object *object);
	boost::shared_ptr<PhysicsActor> createActor(boost::shared_ptr<PhysicsMesh> &mesh, const VC3 &position, const QUAT &rotation, const VC3 &velocity, const VC3 &angularVelocity, float mass, int collisionGroup, int soundGroup);
	boost::shared_ptr<PhysicsFluid> createFluid(int type, int maxParticles, float fluidStaticRestitution, float fluidStaticAdhesion, float fluidDynamicRestitution, float fluidDynamicAdhesion, float fluidDamping, float fluidStiffness, float fluidViscosity, float fluidKernelRadiusMultiplier, float fluidRestParticlesPerMeter, float fluidRestDensity, float fluidMotionLimit, int fluidPacketSizeMultiplier, int collGroup);

	void setPhysics(boost::shared_ptr<physics::PhysicsLib> &physics);
	void resetFluidRendering();
	void update();
};

class PhysicsMesh
{
	//boost::shared_ptr<physics::ConvexMesh> mesh;
	VC3 localPosition;
	VC3 size;
	float volume;
	friend class ParticlePhysics;

public:
	PhysicsMesh();
	~PhysicsMesh();

	float getVolume() const;
};

class PhysicsActor
{
	VC3 position;
	QUAT rotation;
	VC3 force;

	/*
	VC3 lastPosition1;
	VC3 lastAngular1;
	VC3 lastPosition2;
	VC3 lastAngular2;
	int sleepCounter;
	*/

	bool forceUpdate;

	boost::shared_ptr<physics::ActorBase> actor;
	boost::weak_ptr<ParticlePhysics::Data> lib;
	friend class ParticlePhysics;

public:
	explicit PhysicsActor(const VC3 &position, const QUAT &rotation);
	~PhysicsActor();

	bool isEnabled() const;
	void getPosition(VC3 &position) const;
	void getRotation(QUAT &rotation) const;

	void applyForce(const VC3 &force);
};

class PhysicsFluid
{
	boost::shared_ptr<physics::Fluid> fluid;

	boost::weak_ptr<ParticlePhysics::Data> lib;
	friend class ParticlePhysics;

	VC3 acceleration;
	std::vector<char> buffer;
	int addAmount;

public:

	bool renderFlag;

	struct Particle
	{
		VC3 position;
		VC3 velocity;
		float life;
		float density;
		unsigned int id;

		Particle()
		:	life(0),
			density(0),
			id(0)
		{
		}
	};

	PhysicsFluid();
	~PhysicsFluid();

	bool canSpawn() const;
	const Particle *getParticles(int &amount) const;

	void addParticles(const void *buffer, int amount);
	void setAcceleration(const VC3 &force);
};

} // particle
} // frozenbyte

#endif

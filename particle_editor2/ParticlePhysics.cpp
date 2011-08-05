#include "precompiled.h"
#define NOMINMAX

#include "ParticlePhysics.h"
#include "../physics/physics_lib.h"
#include "../physics/actor_base.h"
#include "../physics/convex_actor.h"
#include "../physics/box_actor.h"
#include "../physics/cooker.h"
#include "../physics/fluid.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../game/physics/physics_collisiongroups.h"
#include <boost/weak_ptr.hpp>
//#include <map>
#include <vector>
#include <list>
#include <IStorm3D_Model.h>
#include <IStorm3D_Mesh.h>

#define PHYSX_GRAVITY
//#define PHYSX_SPAWN_TEST

#ifdef PHYSX_SPAWN_TEST
#include "NxPhysics.h"

struct BoxStruct
{
	NxMat33 rotation;
	NxVec3 extents;
	NxVec3 center;
};

#endif

namespace frozenbyte {
namespace particle {

	void rotateToward(const VC3 &a, const VC3 &b, QUAT &result)
	{
		VC3 axis = a.GetCrossWith(b);
		float dot = a.GetDotWith(b);

		if(dot < -0.99f)
		{
			result = QUAT();
			return;
		}

		result.x = axis.x;
		result.y = axis.y;
		result.z = axis.z;
		result.w = (dot + 1.0f);
		result.Normalize();
	}


	//const float MIN_BOX_SIZE = 0.15f;
	static const float MIN_BOX_SIZE = 0.20f;

	//const int MAX_ACTOR_AMOUNT = 500;
	//const int MAX_ACTOR_CREATE_PER_TICK = 20;
	//const int MAX_ACTOR_CREATE_PER_TICK = 1;

	static const float MAX_VELOCITY = 5.f;
	static const int MAX_WAIT_FOR_CREATE = 30;

	struct ActorData
	{
		boost::weak_ptr<PhysicsActor> actor;
		boost::weak_ptr<PhysicsMesh> mesh;

		VC3 velocity;
		VC3 angularVelocity;
		float mass;
		int collisionGroup;
		int soundGroup;

		int createDelayCounter;

		ActorData()
		:	mass(5.f),
			collisionGroup(1),
			soundGroup(0),
			createDelayCounter(0)
		{
		}
	};

	struct MeshData
	{
		boost::weak_ptr<PhysicsMesh> mesh;
		IStorm3D_Model_Object *object;
		std::string filename;

		MeshData()
		:	object(0)
		{
		}
	};

#ifndef NX_DISABLE_FLUIDS
	struct FluidData
	{
		boost::weak_ptr<PhysicsFluid> fluid;
		int type;
		int maxParticles;

		float fluidStaticRestitution;
		float fluidStaticAdhesion;
		float fluidDynamicRestitution;
		float fluidDynamicAdhesion;
		float fluidDamping;
		float fluidMotionLimit;
		int fluidPacketSizeMultiplier;

		float fluidStiffness;
		float fluidViscosity;
		float fluidKernelRadiusMultiplier;
		float fluidRestParticlesPerMeter;
		float fluidRestDensity;
		int collisionGroup;

		FluidData()
		:	type(0),
			maxParticles(0),
			fluidStaticRestitution(0),
			fluidStaticAdhesion(0),
			fluidDynamicRestitution(0),
			fluidDynamicAdhesion(0),
			fluidDamping(0),
			fluidMotionLimit(0),
			fluidPacketSizeMultiplier(0),

			fluidStiffness(0),
			fluidViscosity(0),
			fluidKernelRadiusMultiplier(0),
			fluidRestParticlesPerMeter(0),
			fluidRestDensity(0),
			collisionGroup(PHYSICS_COLLISIONGROUP_FLUIDS)
		{
		}
	};
#endif

	struct ActorListSorter
	{
		bool operator () (const ActorData &a, const ActorData &b) const
		{
			boost::shared_ptr<PhysicsMesh> am = a.mesh.lock();
			boost::shared_ptr<PhysicsMesh> bm = b.mesh.lock();

			float av = 0;
			if(am)
				av = am->getVolume();

			float bv = 0;
			if(bm)
				bv = bm->getVolume();

			return av > bv;
		}
	};

	typedef std::vector<ActorData> ActorList;
	typedef std::vector<MeshData> MeshList;
#ifndef NX_DISABLE_FLUIDS
	typedef std::vector<FluidData> FluidList;
#endif
	typedef std::vector<boost::shared_ptr<physics::ActorBase> > ActorBaseList;
#ifndef NX_DISABLE_FLUIDS
	typedef std::vector<boost::shared_ptr<physics::Fluid> > FluidBaseList;
#endif

	typedef std::list<PhysicsActor *> PhysicsActorList;
#ifndef NX_DISABLE_FLUIDS
	typedef std::list<PhysicsFluid *> FluidActorList;
#endif

struct ParticlePhysics::Data
{
	boost::shared_ptr<physics::PhysicsLib> physics;

	int maxParticleAmount;
	int maxParticleSpawnAmount;

	// Create lists
	ActorList actorList;
	MeshList meshList;
#ifndef NX_DISABLE_FLUIDS
	FluidList fluidList;
#endif

	// Delete lists
	ActorBaseList actorBaseList;
#ifndef NX_DISABLE_FLUIDS
	FluidBaseList fluidBaseList;
#endif
	
	// Update lists
	PhysicsActorList physicsActorList;
#ifndef NX_DISABLE_FLUIDS
	FluidActorList fluidActorList;
#endif

	int createdActors;

	Data()
	:	maxParticleAmount(50),
		maxParticleSpawnAmount(20),
		createdActors(0)
	{
	}

	~Data()
	{
	}
/*
	boost::shared_ptr<PhysicsMesh> createConvexMesh(const char *filename, IStorm3D_Model_Object *object)
	{
		boost::shared_ptr<PhysicsMesh> mesh(new PhysicsMesh());

		MeshData data;
		data.mesh = mesh;
		data.object = object;
		data.filename = filename;

		meshList.push_back(data);
		return mesh;
	}
*/
	boost::shared_ptr<PhysicsActor> createActor(boost::shared_ptr<PhysicsMesh> &mesh, const VC3 &position, const QUAT &rotation, const VC3 &velocity, const VC3 &angularVelocity, float mass, int collisionGroup, int soundGroup)
	{
		boost::shared_ptr<PhysicsActor> actor(new PhysicsActor(position, rotation));
		physicsActorList.push_back(actor.get());

		ActorData data;
		data.actor = actor;
		data.mesh = mesh;
		data.velocity = velocity;
		data.angularVelocity = angularVelocity;
		data.mass = mass;
		data.collisionGroup = collisionGroup;
		data.soundGroup = soundGroup;

		actorList.push_back(data);
		return actor;
	}

#ifndef NX_DISABLE_FLUIDS
	boost::shared_ptr<PhysicsFluid> createFluid(int type, int maxParticles, float fluidStaticRestitution, float fluidStaticAdhesion, float fluidDynamicRestitution, float fluidDynamicAdhesion, float fluidDamping, float fluidStiffness, float fluidViscosity, float fluidKernelRadiusMultiplier, float fluidRestParticlesPerMeter, float fluidRestDensity, float fluidMotionLimit, int fluidPacketSizeMultiplier, int collGroup)
	{
		boost::shared_ptr<PhysicsFluid> fluid(new PhysicsFluid());
		fluidActorList.push_back(fluid.get());

		FluidData data;
		data.fluid = fluid;
		data.type = type;
		data.maxParticles = maxParticles;
		data.fluidStaticRestitution = fluidStaticRestitution;
		data.fluidStaticAdhesion = fluidStaticAdhesion;
		data.fluidDynamicRestitution = fluidDynamicRestitution;
		data.fluidDynamicAdhesion = fluidDynamicAdhesion;
		data.fluidDamping = fluidDamping;
		data.fluidStiffness = fluidStiffness;
		data.fluidViscosity = fluidViscosity;
		data.fluidKernelRadiusMultiplier = fluidKernelRadiusMultiplier;
		data.fluidRestParticlesPerMeter = fluidRestParticlesPerMeter;
		data.fluidRestDensity = fluidRestDensity;
		data.fluidMotionLimit = fluidMotionLimit;
		data.fluidPacketSizeMultiplier = fluidPacketSizeMultiplier;
		data.collisionGroup = collGroup;

		fluidList.push_back(data);
		return fluid;
	}
#endif

};

ParticlePhysics::ParticlePhysics()
:	data(new Data())
{
}

ParticlePhysics::~ParticlePhysics()
{
}

void ParticlePhysics::setMaxParticleAmount(int amount)
{
	data->maxParticleAmount = amount;
}

void ParticlePhysics::setMaxParticleSpawnAmount(int amount)
{
	data->maxParticleSpawnAmount = amount;
}

void ParticlePhysics::physicsExplosion(const VC3 &explosionPosition, float forceFactor, float radius)
{
	// added support for radius parameter --jpk
	// TODO: this implementation does not take radius into account when calculating forces... (probably should)
	float radiusSq = radius * radius;

	for(PhysicsActorList::iterator it = data->physicsActorList.begin(); it != data->physicsActorList.end(); ++it)
	{
		PhysicsActor *actor = *it;
		if(!actor)
			continue;

		VC3 velocity;
		if(actor->actor)
			actor->actor->getMassCenterPosition(velocity);
		else
			velocity = actor->position;

		velocity -= explosionPosition;
		float len = velocity.GetSquareLength();
		if(len > radiusSq)
			continue;
		len = sqrtf(len);

		velocity.y *= 1.3f;
		velocity.y += 1.0f;

		if(len > 0.01f)
		{
			velocity /= len;

			if(len < 1.5f)
				len = 1.5f;

			velocity /= len;
		}
		else
			velocity.y = 1.f;

		velocity *= 7.5f * forceFactor;
		//actor->sleepCounter = 1;
		actor->force += velocity;
		actor->forceUpdate = true;
	}
}

boost::shared_ptr<PhysicsMesh> ParticlePhysics::createConvexMesh(const char *filename, IStorm3D_Model_Object *object)
{
	boost::shared_ptr<PhysicsMesh> mesh(new PhysicsMesh());
	IStorm3D_Mesh *stormMesh = object->GetMesh();

	int vertexAmount = stormMesh->GetVertexCount();
	const Storm3D_Vertex *buffer = stormMesh->GetVertexBufferReadOnly();

	VC3 minValues(10000.f, 10000.f, 10000.f);
	VC3 maxValues(-10000.f, -10000.f, -10000.f);
	for(int i = 0; i < vertexAmount; ++i)
	{
		VC3 pos = buffer[i].position;

		minValues.x = std::min(minValues.x, pos.x);
		minValues.y = std::min(minValues.y, pos.y);
		minValues.z = std::min(minValues.z, pos.z);

		maxValues.x = std::max(maxValues.x, pos.x);
		maxValues.y = std::max(maxValues.y, pos.y);
		maxValues.z = std::max(maxValues.z, pos.z);
	}

	VC3 size = maxValues - minValues;
	if(size.x < MIN_BOX_SIZE)
	{
		float diff = (MIN_BOX_SIZE - size.x) * 0.5f;
		maxValues.x += diff;
		minValues.x -= diff;
	}
	if(size.y < MIN_BOX_SIZE)
	{
		float diff = (MIN_BOX_SIZE - size.y) * 0.5f;
		maxValues.y += diff;
		minValues.y -= diff;
	}
	if(size.z < MIN_BOX_SIZE)
	{
		float diff = (MIN_BOX_SIZE - size.z) * 0.5f;
		maxValues.z += diff;
		minValues.z -= diff;
	}

	mesh->size = (maxValues - minValues) * 0.5f;
	mesh->localPosition = minValues + mesh->size;
	mesh->localPosition.y -= mesh->size.y;
	mesh->volume = mesh->size.x * mesh->size.y * mesh->size.z * 2.f;

	return mesh;
}

boost::shared_ptr<PhysicsActor> ParticlePhysics::createActor(boost::shared_ptr<PhysicsMesh> &mesh, const VC3 &position, const QUAT &rotation, const VC3 &velocity, const VC3 &angularVelocity, float mass, int collisionGroup, int soundGroup)
{
	boost::shared_ptr<PhysicsActor> actor = data->createActor(mesh, position, rotation, velocity, angularVelocity, mass, collisionGroup, soundGroup);
	if(actor)
		actor->lib = data;

	return actor;
}

#ifndef NX_DISABLE_FLUIDS
boost::shared_ptr<PhysicsFluid> ParticlePhysics::createFluid(int type, int maxParticles, float fluidStaticRestitution, float fluidStaticAdhesion, float fluidDynamicRestitution, float fluidDynamicAdhesion, float fluidDamping, float fluidStiffness, float fluidViscosity, float fluidKernelRadiusMultiplier, float fluidRestParticlesPerMeter, float fluidRestDensity, float fluidMotionLimit, int fluidPacketSizeMultiplier, int collGroup)
{
	boost::shared_ptr<PhysicsFluid> fluid = data->createFluid(type, maxParticles, fluidStaticRestitution, fluidStaticAdhesion, fluidDynamicRestitution, fluidDynamicAdhesion, fluidDamping, fluidStiffness, fluidViscosity, fluidKernelRadiusMultiplier, fluidRestParticlesPerMeter, fluidRestDensity, fluidMotionLimit, fluidPacketSizeMultiplier, collGroup);
	if(fluid)
		fluid->lib = data;

	return fluid;
}
#endif

void ParticlePhysics::setPhysics(boost::shared_ptr<physics::PhysicsLib> &physics)
{
	data->physics = physics;

	if(!physics)
	{
		data->actorBaseList.clear();
#ifndef NX_DISABLE_FLUIDS
		data->fluidBaseList.clear();
#endif
	}
}

#ifndef NX_DISABLE_FLUIDS
void ParticlePhysics::resetFluidRendering()
{
	//physics::resetFluidParticleCount();
	for(FluidActorList::iterator it = data->fluidActorList.begin(); it != data->fluidActorList.end(); ++it)
	{
		PhysicsFluid *fluid = *it;
		if(!fluid->fluid)
			continue;

		fluid->renderFlag = false;
	}
}
#endif

void ParticlePhysics::update()
{
	if(!data->physics)
		return;

	data->actorBaseList.clear();
#ifndef NX_DISABLE_FLUIDS
	data->fluidBaseList.clear();
#endif

	// Update physics actors
	{
		for(PhysicsActorList::iterator it = data->physicsActorList.begin(); it != data->physicsActorList.end(); ++it)
		{
			PhysicsActor *actor = *it;
			if(!actor->actor)
				continue;

			/*
			if(actor->actor)
			{
				VC3 velocity;
				actor->actor->getVelocity(velocity);

				float len = velocity.GetLength();
				if(len > 1)
				{
					velocity /= len;
					actor->actor->setVelocity(velocity);
				}
			}
			*/

#ifndef PHYSX_GRAVITY
			if(actor->force.GetSquareLength() > 0.01f)
			{
				if(!actor->forceUpdate && ++actor->sleepCounter % 10 == 0)
				{
					VC3 currentPos;
					actor->getPosition(currentPos);
					VC3 currentAngular;
					actor->actor->getAngularVelocity(currentAngular);

					if(
						currentPos.GetSquareRangeTo(actor->lastPosition1) < 0.00001f
						||
						currentAngular.GetSquareRangeTo(actor->lastAngular1) < 0.00001f
						||
						currentPos.GetSquareRangeTo(actor->lastPosition2) < 0.00001f
						||
						currentAngular.GetSquareRangeTo(actor->lastAngular2) < 0.00001f)
					{
						if(!actor->actor->isSleeping())
							actor->actor->putToSleep();
					}

					actor->lastPosition2 = actor->lastPosition1;
					actor->lastAngular2 = actor->lastAngular1;

					actor->lastPosition1 = currentPos;
					actor->lastAngular1 = currentAngular;
				}
	
				if(actor->forceUpdate || !actor->actor->isSleeping())
					actor->actor->addVelocityChange(actor->force);

				actor->force = VC3();
			}
#else
			if(actor->forceUpdate)
			{
				VC3 base;
				actor->actor->getVelocity(base);

				VC3 newForce = base;
				newForce += actor->force;

				float len = newForce.GetSquareLength();
				if(len > MAX_VELOCITY * MAX_VELOCITY)
				{
					len = sqrtf(len);
					newForce /= len;

					newForce *= MAX_VELOCITY;
				}

				newForce -= base;
				actor->actor->addVelocityChange(newForce);

				{
					QUAT rot;
					rotateToward(VC3(0, 1.f, 0), actor->force.GetNormalized(), rot);

					VC3 test(rot.x, rot.y, rot.z);
					test.x += ((float)(rand() - RAND_MAX/2) / (float)RAND_MAX/2);
					test.z += ((float)(rand() - RAND_MAX/2) / (float)RAND_MAX/2);

					if(test.GetSquareLength() > 0.001f)
						test.Normalize();

					test *= 2.f;
					actor->actor->setAngularVelocity(test * 2.f);
				}

				actor->force = VC3();
			}
#endif

			actor->forceUpdate = false;
		}

#ifndef NX_DISABLE_FLUIDS
		physics::resetFluidParticleCount();
		for(FluidActorList::iterator it = data->fluidActorList.begin(); it != data->fluidActorList.end(); ++it)
		{
			PhysicsFluid *fluid = *it;
			if(!fluid->fluid)
				continue;

			if(fluid->addAmount)
			{
				fluid->fluid->addParticles(&fluid->buffer[0], fluid->addAmount);
				fluid->buffer.clear();
				fluid->addAmount = 0;
			}

			fluid->fluid->setAcceleration(fluid->acceleration);
			fluid->fluid->update();
			//fluid->renderFlag = false;
		}
#endif
	}

	/*
	// Meshes
	{
		for(MeshList::iterator it = data->meshList.begin(); it != data->meshList.end(); ++it)
		{
			MeshData &convex = *it;
			boost::shared_ptr<PhysicsMesh> mesh = convex.mesh.lock();
			if(!mesh)
				continue;

			filesystem::FB_FILE *fp = filesystem::fb_fopen(convex.filename.c_str(), "rb");
			if(!fp)
			{
				physics::Cooker cooker;
				cooker.cookApproxConvex(convex.filename.c_str(), convex.object);
			}
			else
				filesystem::fb_fclose(fp);

			mesh->mesh = data->physics->createConvexMesh(convex.filename.c_str());
		}

		data->meshList.clear();
	}
	*/

	// Actors
	{
		/*
		if(actorList.size() + physicsActorList.size() > MAX_ACTOR_AMOUNT)
		{
			boost::shared_ptr<PhysicsActor> nullActor;
			return nullActor;
		}
		*/

		bool sort = false;

		int createAmount = data->actorList.size();
		if(createAmount > data->maxParticleSpawnAmount)
		{
			createAmount = data->maxParticleSpawnAmount;
			sort = true;
		}
		if(createAmount + data->createdActors > data->maxParticleAmount)
		{
			sort = true;
			createAmount = data->maxParticleAmount - data->createdActors;
		}

		if(sort)
			std::sort(data->actorList.begin(), data->actorList.end(), ActorListSorter());

		// For collision test
#ifdef PHYSX_SPAWN_TEST
		std::vector<BoxStruct> previousBoxes;
#endif

		int index = 0;
		for(ActorList::iterator it = data->actorList.begin(); it != data->actorList.end(); ++it)
		{
			ActorData &convex = *it;
			if(convex.createDelayCounter++ >= MAX_WAIT_FOR_CREATE)
				continue;

			boost::shared_ptr<PhysicsActor> actor = convex.actor.lock();
			if(!actor)
				continue;

			boost::shared_ptr<PhysicsMesh> mesh = convex.mesh.lock();
			if(!mesh)
				continue;

			/*
			//VC3 boxCenter = mesh->localPosition + actor->position;
			VC3 boxCenter = mesh->localPosition;
			actor->rotation.RotateVector(boxCenter);
			boxCenter += actor->position;

			// Test to previous tick physics
			if(data->physics->checkOverlapOBB(boxCenter, mesh->size, actor->rotation, physics::PhysicsLib::CollisionStatic))
			{
				convex.actor.reset();
				convex.mesh.reset();
				continue;
			}
			*/

#ifdef PHYSX_SPAWN_TEST
			BoxStruct currentBox;
			currentBox.center.set(boxCenter.x, boxCenter.y, boxCenter.z);
			currentBox.extents.set(mesh->size.x, mesh->size.y, mesh->size.z);
			QUAT r = actor->rotation.GetInverse();
			NxQuat quat;
			quat.setXYZW(r.x, r.y, r.z, r.w);
			currentBox.rotation.fromQuat(quat);

			// Test to previous tick physics
			if(data->physics->checkOverlapOBB(boxCenter, mesh->size, actor->rotation))
				continue;

			bool hit = false;
			for(unsigned int i = 0; i < previousBoxes.size(); ++i)
			{
				const BoxStruct &b = previousBoxes[i];
				if(NxBoxBoxIntersect(currentBox.extents, currentBox.center, currentBox.rotation, b.extents, b.center, b.rotation, true))
				{
					hit = true;
					break;
				}
			}

			if(hit)
				continue;
#endif

			// Don't create too many particles
			if(index++ >= createAmount)
				break;

			/*
			boost::shared_ptr<physics::ConvexActor> convexActor = data->physics->createConvexActor(mesh->mesh, actor->position);
			if(convexActor)
			{
				convexActor->setRotation(actor->rotation);
				convexActor->setVelocity(convex.velocity);
				convexActor->setAngularVelocity(convex.angularVelocity);
				convexActor->setMass(convex.mass);
				convexActor->setCollisionGroup(2);
				//convexActor->enableFeature(physics::ActorBase::DISABLE_GRAVITY, true);
				actor->actor = convexActor;
			}
			*/

			boost::shared_ptr<physics::BoxActor> boxActor = data->physics->createBoxActor(mesh->size, actor->position, mesh->localPosition);
			if(boxActor)
			{
				++data->createdActors;

				boxActor->setRotation(actor->rotation);
				boxActor->setVelocity(convex.velocity);
				boxActor->setAngularVelocity(convex.angularVelocity);
				boxActor->setMass(convex.mass);
				boxActor->setCollisionGroup(convex.collisionGroup);

#ifndef PHYSX_GRAVITY
				boxActor->enableFeature(physics::ActorBase::DISABLE_GRAVITY, true);
#endif

				boxActor->setIntData(convex.soundGroup);
				actor->actor = boxActor;

#ifdef PHYSX_SPAWN_TEST
				previousBoxes.push_back(currentBox);
#endif
			}
		}

		int size = data->actorList.size();
		for(int i = index; i < size; ++i)
		{
			data->actorList[i].createDelayCounter++;
		}

		if(index < size)
		{
			for(int i = 0; i < size - index; ++i)
				data->actorList[i] = data->actorList[i + index];

			data->actorList.resize(size - index);
		}
		else 
			data->actorList.clear();
	}

	// Fluids
	{
#ifndef NX_DISABLE_FLUIDS
		for(FluidList::iterator it = data->fluidList.begin(); it != data->fluidList.end(); ++it)
		{
			FluidData &fluidData = *it;
			boost::shared_ptr<PhysicsFluid> fluid = fluidData.fluid.lock();
			if(!fluid)
				continue;

			boost::shared_ptr<physics::Fluid> fluidActor = data->physics->createFluid(physics::PhysicsLib::FluidType(fluidData.type), fluidData.maxParticles, fluidData.fluidStaticRestitution, fluidData.fluidStaticAdhesion, fluidData.fluidDynamicRestitution, fluidData.fluidDynamicAdhesion, fluidData.fluidDamping, fluidData.fluidStiffness, fluidData.fluidViscosity, fluidData.fluidKernelRadiusMultiplier, fluidData.fluidRestParticlesPerMeter, fluidData.fluidRestDensity, fluidData.fluidMotionLimit, fluidData.fluidPacketSizeMultiplier, fluidData.collisionGroup);
			fluid->fluid = fluidActor;
			fluid->lib = data;
		}

		data->fluidList.clear();
#endif
	}
}

// --

PhysicsMesh::PhysicsMesh()
:	volume(0)
{
}

PhysicsMesh::~PhysicsMesh()
{
}

float PhysicsMesh::getVolume() const
{
	return volume;
}

// --

PhysicsActor::PhysicsActor(const VC3 &position_, const QUAT &rotation_)
:	position(position_),
	rotation(rotation_),
	//sleepCounter(0),
	forceUpdate(false)
{
}

PhysicsActor::~PhysicsActor()
{
	boost::shared_ptr<ParticlePhysics::Data> physics = lib.lock();
	if(physics)
	{
		if(actor)
		{
			physics->actorBaseList.push_back(actor);
			--physics->createdActors;
		}

		physics->physicsActorList.remove(this);
	}
}

bool PhysicsActor::isEnabled() const
{
	if(actor)
		return true;

	return false;
}

void PhysicsActor::getPosition(VC3 &position_) const
{
	if(actor)
		actor->getPosition(position_);
	else
		position_ = position;
}

void PhysicsActor::getRotation(QUAT &rotation_) const
{
	if(actor)
		actor->getRotation(rotation_);
	else
		rotation_ = rotation;
}

void PhysicsActor::applyForce(const VC3 &force_)
{
#ifndef PHYSX_GRAVITY
	force += force_;
#endif
}

// --

#ifndef NX_DISABLE_FLUIDS
PhysicsFluid::PhysicsFluid()
:	addAmount(0),
	renderFlag(false)
{
}

PhysicsFluid::~PhysicsFluid()
{
	boost::shared_ptr<ParticlePhysics::Data> physics = lib.lock();
	if(physics)
	{
		if(fluid)
			physics->fluidBaseList.push_back(fluid);

		physics->fluidActorList.remove(this);
	}
}

bool PhysicsFluid::canSpawn() const
{
	if(fluid)
		return true;

	return false;
}

const PhysicsFluid::Particle *PhysicsFluid::getParticles(int &amount) const
{
	if(fluid)
		return reinterpret_cast<const Particle *> (fluid->getParticles(amount));

	return 0;
}

void PhysicsFluid::addParticles(const void *buffer_, int amount)
{
	if(fluid)
	{
		int index = buffer.size();
		int size = amount * sizeof(physics::FluidParticle);
		buffer.resize(index + size);

		memcpy(&buffer[index], buffer_, size);
		addAmount += amount;
	}
}

void PhysicsFluid::setAcceleration(const VC3 &force)
{
	if(fluid)
		acceleration = force;
}

#endif

} // particle
} // frozenbyte

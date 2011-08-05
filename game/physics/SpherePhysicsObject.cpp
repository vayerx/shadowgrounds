
#include "precompiled.h"

#include "SpherePhysicsObject.h"

#include "GamePhysics.h"
#ifdef PHYSICS_PHYSX
#include "../../physics/physics_lib.h"
#include "../../physics/sphere_actor.h"
#include "../../physics/actor_base.h"
#endif

namespace game
{
	SpherePhysicsObject::SpherePhysicsObject(GamePhysics *gamePhysics, float radius, float mass) 
		: AbstractPhysicsObject(gamePhysics)
	{ 
		this->radius = radius;
		this->mass = mass;
	}

	SpherePhysicsObject::~SpherePhysicsObject() 
	{ 
		// nop, ~AbstractPhysicsObject handles everything of any interest...
	}

	PHYSICS_ACTOR SpherePhysicsObject::createImplementationObject()
	{
#ifdef PHYSICS_PHYSX
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createSphereActor(radius);

		actor->setIntData(soundMaterial);
		actor->setMass(mass);
		return actor;
#endif
#ifdef PHYSICS_ODE
		PhysicsActorOde actor = PhysicsActorOde::createNewSpherePhysicsObject(radius, mass);
		return actor;
#endif
#ifdef PHYSICS_NONE
		PhysicsActorNone actor;
		return actor;
#endif
	}

	void SpherePhysicsObject::syncImplementationObject(PHYSICS_ACTOR &obj)
	{
		AbstractPhysicsObject::syncImplementationObject(obj);

		// TODO: sync mass if it has changed... (in future, if mass changeable)
	}
}



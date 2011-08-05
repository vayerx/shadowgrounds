
#include "precompiled.h"

#include "CapsulePhysicsObject.h"

#include "GamePhysics.h"
#ifdef PHYSICS_PHYSX
#include "../../physics/physics_lib.h"
#include "../../physics/capsule_actor.h"
#include "../../physics/actor_base.h"
#endif

namespace game
{
	CapsulePhysicsObject::CapsulePhysicsObject(GamePhysics *gamePhysics, float height, float radius, float mass, int collisionGroup, const VC3 &position) 
		: AbstractPhysicsObject(gamePhysics)
	{ 
		this->radius = radius;
		this->height = height;
		this->mass = mass;
		this->position = position;
		this->collisionGroup = collisionGroup;
	}

	CapsulePhysicsObject::~CapsulePhysicsObject() 
	{ 
		// nop, ~AbstractPhysicsObject handles everything of any interest...
	}

	PHYSICS_ACTOR CapsulePhysicsObject::createImplementationObject()
	{
#ifdef PHYSICS_PHYSX
#ifdef GAME_SIDEWAYS
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createCapsuleActor(height, radius, position, -(radius + (0.5f * height)), 2);
#else
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createCapsuleActor(height, radius, position);
#endif
		if(actor)
		{
			actor->setMass(mass);
			actor->setCollisionGroup(collisionGroup);
			actor->setIntData(soundMaterial);
		}
		return actor;
#endif
#ifdef PHYSICS_ODE
		PhysicsActorOde actor = PhysicsActorOde::createNewCapsulePhysicsObject(radius, height, mass);
		VC3 tmp = this->position;
		setPosition(tmp);
		return actor;
#endif
#ifdef PHYSICS_NONE
		PhysicsActorNone actor;
		return actor;
#endif
	}

	void CapsulePhysicsObject::syncImplementationObject(PHYSICS_ACTOR &obj)
	{
		AbstractPhysicsObject::syncImplementationObject(obj);

		// TODO: sync mass if it has changed... (in future, if mass changeable)
	}
}



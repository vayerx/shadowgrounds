
#include "precompiled.h"

#include "RackPhysicsObject.h"
#include "GamePhysics.h"
#ifdef PHYSICS_PHYSX
#include "../../physics/physics_lib.h"
#include "../../physics/rack_actor.h"
#include "../../physics/actor_base.h"
#endif

namespace game
{
	RackPhysicsObject::RackPhysicsObject(GamePhysics *gamePhysics, float mass, int collisionGroup, const VC3 &position) 
	:	AbstractPhysicsObject(gamePhysics)
	{ 
		this->position = position;
		this->mass = mass;
		this->collisionGroup = collisionGroup;
	}

	RackPhysicsObject::~RackPhysicsObject() 
	{
		// nop, ~AbstractPhysicsObject handles everything of any interest...
	}

	PHYSICS_ACTOR RackPhysicsObject::createImplementationObject()
	{
#ifdef PHYSICS_PHYSX
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createRackActor(position);
		if(actor)
		{
			actor->setMass(mass);
			actor->setCollisionGroup(collisionGroup);
			actor->setIntData(soundMaterial);
		}
		return actor;
#endif
#ifdef PHYSICS_NONE
		PhysicsActorNone actor;
		return actor;
#endif
	}

	void RackPhysicsObject::syncImplementationObject(PHYSICS_ACTOR &obj)
	{
		AbstractPhysicsObject::syncImplementationObject(obj);

		// TODO: sync mass if it has changed... (in future, if mass changeable)
	}

}



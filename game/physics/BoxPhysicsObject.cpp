
#include "precompiled.h"

#include "BoxPhysicsObject.h"

#include "GamePhysics.h"
#ifdef PHYSICS_PHYSX
#include "../../physics/physics_lib.h"
#include "../../physics/box_actor.h"
#include "../../physics/actor_base.h"
#endif

namespace game
{
	BoxPhysicsObject::BoxPhysicsObject(GamePhysics *gamePhysics, const VC3 &sizes, float mass, int collisionGroup, const VC3 &position) 
		: AbstractPhysicsObject(gamePhysics)
	{ 
		this->position = position;
		this->sizes = sizes;
		this->mass = mass;
		this->collisionGroup = collisionGroup;
	}

	BoxPhysicsObject::~BoxPhysicsObject() 
	{
		// nop, ~AbstractPhysicsObject handles everything of any interest...
	}

	PHYSICS_ACTOR BoxPhysicsObject::createImplementationObject()
	{
#ifdef PHYSICS_PHYSX
#ifdef GAME_SIDEWAYS
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createBoxActor(sizes, position, VC3(0,-sizes.y,0));
#else
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createBoxActor(sizes, position);
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
		PhysicsActorOde actor = PhysicsActorOde::createNewBoxPhysicsObject(sizes, mass);
		VC3 tmp = this->position;
		setPosition(tmp);
		return actor;
#endif
#ifdef PHYSICS_NONE
		PhysicsActorNone actor;
		return actor;
#endif
	}

	void BoxPhysicsObject::syncImplementationObject(PHYSICS_ACTOR &obj)
	{
		AbstractPhysicsObject::syncImplementationObject(obj);

		// TODO: sync mass if it has changed... (in future, if mass changeable)
	}

}




#include "precompiled.h"

#include "CylinderPhysicsObject.h"

#include "GamePhysics.h"
#ifdef PHYSICS_PHYSX
#include "../../physics/physics_lib.h"
#include "../../physics/cylinder_actor.h"
#include "../../physics/actor_base.h"
#endif

namespace game
{
	CylinderPhysicsObject::CylinderPhysicsObject(GamePhysics *gamePhysics, float height, float radius, float mass, int collisionGroup, const VC3 &position) 
		: AbstractPhysicsObject(gamePhysics)
	{ 
		this->position = position;
		this->radius = radius;
		this->height = height;
		this->mass = mass;
		this->collisionGroup = collisionGroup;
	}

	CylinderPhysicsObject::~CylinderPhysicsObject() 
	{ 
		// nop, ~AbstractPhysicsObject handles everything of any interest...
	}

	PHYSICS_ACTOR CylinderPhysicsObject::createImplementationObject()
	{
#ifdef PHYSICS_PHYSX
		boost::shared_ptr<frozenbyte::physics::ActorBase> actor = gamePhysics->getPhysicsLib()->createCylinderActor(height, radius, position);

		actor->setMass(mass);
		return actor;
#endif
#ifdef PHYSICS_ODE
		PhysicsActorOde actor = PhysicsActorOde::createNewCylinderPhysicsObject(radius, height, mass);
		VC3 tmp = this->position;
		setPosition(tmp);
		return actor;
#endif
#ifdef PHYSICS_NONE
		PhysicsActorNone actor;
		return actor;
#endif
	}

	void CylinderPhysicsObject::syncImplementationObject(PHYSICS_ACTOR &obj)
	{
		AbstractPhysicsObject::syncImplementationObject(obj);

		// TODO: sync mass if it has changed... (in future, if mass changeable)
	}
}




#ifndef RACKPHYSICSOBJECT_H
#define RACKPHYSICSOBJECT_H

#include "AbstractPhysicsObject.h"
#include "DatatypeDef.h"

namespace game
{
	class GamePhysics;

	class RackPhysicsObject : public AbstractPhysicsObject
	{
	public:
		RackPhysicsObject(GamePhysics *gamePhysics, float mass, int collisionGroup, const VC3 &position);
		~RackPhysicsObject();

	protected:
		virtual PHYSICS_ACTOR createImplementationObject();
		virtual void syncImplementationObject(PHYSICS_ACTOR &obj);

		float mass;
		int collisionGroup;
	};
}

#endif


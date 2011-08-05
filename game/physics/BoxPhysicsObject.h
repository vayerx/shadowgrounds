
#ifndef BOXPHYSICSOBJECT_H
#define BOXPHYSICSOBJECT_H

#include "AbstractPhysicsObject.h"
#include "DatatypeDef.h"

namespace game
{
	class GamePhysics;

	class BoxPhysicsObject : public AbstractPhysicsObject
	{
	public:
		BoxPhysicsObject(GamePhysics *gamePhysics, const VC3 &sizes, float mass, int collisionGroup, const VC3 &position);

		virtual ~BoxPhysicsObject();

	protected:
		virtual PHYSICS_ACTOR createImplementationObject();

		virtual void syncImplementationObject(PHYSICS_ACTOR &obj);

		VC3 sizes;
		float mass;
		int collisionGroup;
	};
}

#endif



#ifndef CAPSULEPHYSICSOBJECT_H
#define CAPSULEPHYSICSOBJECT_H

#include "AbstractPhysicsObject.h"
#include "DatatypeDef.h"

namespace game
{
	class GamePhysics;

	class CapsulePhysicsObject : public AbstractPhysicsObject
	{
	public:
		CapsulePhysicsObject(GamePhysics *gamePhysics, float height, float radius, float mass, int collisionGroup, const VC3 &position);

		~CapsulePhysicsObject();

	private:
		PHYSICS_ACTOR createImplementationObject();

		void syncImplementationObject(PHYSICS_ACTOR &obj);

		float radius;
		float height;
		float mass;
		int collisionGroup;
	};
}

#endif


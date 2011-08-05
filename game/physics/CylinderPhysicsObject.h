
#ifndef CYLINDERPHYSICSOBJECT_H
#define CYLINDERPHYSICSOBJECT_H

#include "AbstractPhysicsObject.h"
#include "DatatypeDef.h"

namespace game
{
	class GamePhysics;

	class CylinderPhysicsObject : public AbstractPhysicsObject
	{
	public:
		CylinderPhysicsObject(GamePhysics *gamePhysics, float height, float radius, float mass, int collisionGroup, const VC3 &position);

		~CylinderPhysicsObject();

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



#ifndef SPHEREPHYSICSOBJECT_H
#define SPHEREPHYSICSOBJECT_H

#include "AbstractPhysicsObject.h"
#include "DatatypeDef.h"

namespace game
{
	class GamePhysics;

	class SpherePhysicsObject : public AbstractPhysicsObject
	{
	public:
		SpherePhysicsObject(GamePhysics *gamePhysics, float radius, float mass);

		~SpherePhysicsObject();

	private:
		PHYSICS_ACTOR createImplementationObject();

		void syncImplementationObject(PHYSICS_ACTOR &obj);

		float radius;
		float mass;
	};
}

#endif


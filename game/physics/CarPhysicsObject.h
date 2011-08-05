
#ifndef CARHYSICSOBJECT_H
#define CARPHYSICSOBJECT_H

#include "AbstractPhysicsObject.h"
#include "DatatypeDef.h"

namespace game
{
	class GamePhysics;

	class CarPhysicsObject : public AbstractPhysicsObject
	{
	public:
		CarPhysicsObject(GamePhysics *gamePhysics, float mass, int collisionGroup, const VC3 &position);
		~CarPhysicsObject();


	protected:
		virtual PHYSICS_ACTOR createImplementationObject();
		virtual void syncImplementationObject(PHYSICS_ACTOR &obj);


		float mass;
		int collisionGroup;
	};
}

#endif


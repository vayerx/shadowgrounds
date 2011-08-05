
#ifndef CONVEXPHYSICSOBJECT_H
#define CONVEXPHYSICSOBJECT_H

#include "AbstractPhysicsObject.h"
#include "DatatypeDef.h"

#ifdef PHYSICS_PHYSX

namespace game
{
	class GamePhysics;
	class GamePhysicsImpl;
	class ConvexPhysicsObjectImpl;

	class ConvexPhysicsObject : public AbstractPhysicsObject
	{
	public:
		ConvexPhysicsObject(GamePhysics *gamePhysics, const char *filename, float mass, int collisionGroup, const VC3 &position);

		virtual ~ConvexPhysicsObject();

	protected:
		virtual boost::shared_ptr<frozenbyte::physics::ActorBase> createImplementationObject();

		virtual void syncImplementationObject(boost::shared_ptr<frozenbyte::physics::ActorBase> &obj);

	private:
		ConvexPhysicsObjectImpl *impl;

		static void clearImplementationResources();

		friend class GamePhysicsImpl;

		float mass;
		int collisionGroup;
	};
}

#endif
#endif


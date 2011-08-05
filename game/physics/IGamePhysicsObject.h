
#ifndef IGAMEPHYSICSOBJECT_H
#define IGAMEPHYSICSOBJECT_H

#ifdef PHYSICS_PHYSX
#include <boost/shared_ptr.hpp>
#include "../../physics/actor_base.h"

#define PHYSICS_ACTOR boost::shared_ptr<frozenbyte::physics::ActorBase>
#endif

#ifdef PHYSICS_ODE
#define PHYSICS_ACTOR PhysicsActorOde
#include "../../physics_ode/physics_ode.h"
#endif

#ifdef PHYSICS_NONE
#define PHYSICS_ACTOR PhysicsActorNone
#include "physics_none.h"
#endif

#ifndef PHYSICS_ACTOR
#error "No physics library selection define, choose one or define PHYSICS_NONE."
#endif


namespace game
{
	class GamePhysicsImpl;

	class IGamePhysicsObject
	{
	public:
		virtual ~IGamePhysicsObject() { }

		// HACK: moved from protected to public to be able to handle objects properly in some cases where
		// they may be destroyed without any knowledge...
		virtual int getHandle() const = 0;

	protected:

		virtual void setHandle(int objectHandle) = 0;

		// these methods are called by GamePhysicsImpl at the proper moment
		virtual PHYSICS_ACTOR createImplementationObject() = 0;
		virtual void syncImplementationObject(PHYSICS_ACTOR &obj) = 0;

		// optimized method to be called in place to syncImplementationObject, when the implementation object is known 
		// to be inactive (has not moved or anything)
		virtual void syncInactiveImplementationObject(PHYSICS_ACTOR &obj) = 0;

		friend class GamePhysicsImpl;
	};
}

#endif

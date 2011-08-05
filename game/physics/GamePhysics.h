
#ifndef GAMEPHYSICS_H
#define GAMEPHYSICS_H

#include "IGamePhysicsObject.h"
#include "IGamePhysicsScriptRunner.h"
#include "IPhysicsContactListener.h"
#include <boost/shared_ptr.hpp>

#ifdef PROJECT_CLAW_PROTO
class NxActor;
#endif

namespace frozenbyte
{
namespace physics
{
	class PhysicsLib;
}
namespace particle
{
	class ParticleEffectManager;
}
}

class IStorm3D_Scene;

namespace game
{
	class GamePhysicsImpl;
	class IGamePhysicsObject;

	class GamePhysics
	{
	public:
		GamePhysics();
		~GamePhysics();

		void createPhysics(IGamePhysicsScriptRunner *scriptRunner);
		void deletePhysics();
		void prepareForDelete();

		void runPhysics(IStorm3D_Scene *stormScene, frozenbyte::particle::ParticleEffectManager *particleEffectManager);

#ifdef PHYSICS_PHYSX
		frozenbyte::physics::PhysicsLib *getPhysicsLib();
#endif

		void renderedScene();

		void addPhysicsContactListener(IPhysicsContactListener *contactListener);

		void removePhysicsContactListener(IPhysicsContactListener *contactListener);

		// set the physics simulation "focus" to specified ground height,
		// at the moment, this affects the fluid simulation containment - the fluids are contained
		// to hover near the given ground height and not to raise significantly above it.
		void setGroundFocusHeight(float height);

		// creates an expanding fluid pushing sphere at given position
		void createFluidPushPoint(const VC3 &position, float range, int timeTicks);

		// required for physics stabilization... (so that objects won't break or anything)
		void setIgnoreContacts(bool ignoreContacts);

		// "private:"
		// called by AbstractPhysicsObject constructor/destructor. don't call these directly.
		void addNewObject(IGamePhysicsObject *obj);
		void removeObject(IGamePhysicsObject *obj);

		IGamePhysicsObject *getInterfaceObjectForHandle(int handle);

#ifdef PROJECT_CLAW_PROTO
		// HACK: get the implementing physx object...
		NxActor *getImplementingObject(IGamePhysicsObject *interfaceObject);
#endif

	private:
		GamePhysicsImpl *impl;
	};
}

#endif


#ifndef GAMEPHYSICS_H
#define GAMEPHYSICS_H

#include "IGamePhysicsObject.h"
#include "IGamePhysicsScriptRunner.h"
#include "IPhysicsContactListener.h"
#include <boost/shared_ptr.hpp>
#include <string>

#ifdef PROJECT_CLAW_PROTO
class NxActor;
#endif

namespace frozenbyte
{
namespace physics
{
	class JointDeformingInfo;
	class PhysicsLib;
	class ActorBase;
	struct PhysicsJoint;
}
namespace particle
{
	class ParticleEffectManager;
}
}

class IStorm3D_Scene;


namespace game
{
	class AbstractPhysicsObject;
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

#ifdef PHYSICS_PHYSX
		void deleteJoints();
		void addJoint(boost::shared_ptr<AbstractPhysicsObject> &objectA, boost::shared_ptr<AbstractPhysicsObject> &objectB, const frozenbyte::physics::PhysicsJoint &joint, const std::string &id);
		void addDeformingToPreviousJoint(const frozenbyte::physics::JointDeformingInfo *info);
		void reconnectJoints(boost::shared_ptr<AbstractPhysicsObject> &oldObject, boost::shared_ptr<AbstractPhysicsObject> &newObject);
		bool hasJointAttachedToWorld(frozenbyte::physics::ActorBase *actor);
#endif

#ifdef PROJECT_CLAW_PROTO
		// HACK: get the implementing physx object...
		NxActor *getImplementingObject(IGamePhysicsObject *interfaceObject);
		boost::shared_ptr<frozenbyte::physics::ActorBase> getImplementingBaseObject(IGamePhysicsObject *interfaceObject);
#endif

	private:
		GamePhysicsImpl *impl;
	};
}

#endif

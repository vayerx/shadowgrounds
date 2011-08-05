
#ifndef IPHYSICSCONTACTLISTENER_H
#define IPHYSICSCONTACTLISTENER_H

#include <DatatypeDef.h>

namespace frozenbyte {
namespace physics {

	class ActorBase;

} // physics
} // frozenbyte

namespace game
{
	class IGamePhysicsObject;

	class PhysicsContact
	{
	public:
		IGamePhysicsObject *obj1;
		IGamePhysicsObject *obj2;
		const frozenbyte::physics::ActorBase *physicsObject1;
		const frozenbyte::physics::ActorBase *physicsObject2;

		float contactForceLen;
		VC3 contactNormal;
		VC3 contactPosition;

		PhysicsContact()
		:	obj1(NULL),
			obj2(NULL),
			physicsObject1(NULL),
			physicsObject2(NULL),
			contactForceLen(0.0f),
			contactNormal(0,0,0),
			contactPosition(0,0,0)
		{
		}
	};

	class IPhysicsContactListener
	{
	public:
		virtual void physicsContact(const PhysicsContact &contact) = 0;
		virtual ~IPhysicsContactListener() {};
	};
}

#endif

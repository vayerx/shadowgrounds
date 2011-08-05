
#ifndef PHYSICS_NONE_H
#define PHYSICS_NONE_H

#include "DatatypeDef.h"

class PhysicsActorNone
{
public:
	// dummies...
	void getPosition(VC3 &position) const { };
	void getRotation(QUAT &rotation) const { };
	void getMassCenterPosition(VC3 &position) const { };
	void getVelocity(VC3 &velocity) const { };
	void getAngularVelocity(VC3 &angularVelocity) const { };

	void setPosition(const VC3 &position) { };
	void setRotation(const QUAT &rotation) { };
	void setMass(float mass) { };
	void movePosition(const VC3 &position) { };
	void moveRotation(const QUAT &rotation) { };

	void setVelocity(const VC3 &velocity) { };
	void setAngularVelocity(const VC3 &velocity) { };
	void addImpulse(const VC3 &position, const VC3 &impulse) { };

	// Collision group
	void setCollisionGroup(int group) { };

	void reset() { };

	void putToSleep() { }

	PhysicsActorNone *operator-> ()
	{
    return this;
	}

	bool operator!() { return true; }
};

#endif


#ifndef INCLUDED_FROZENBYTE_ACTOR_BASE_H
#define INCLUDED_FROZENBYTE_ACTOR_BASE_H

#include <DatatypeDef.h>

class NxActor;
class NxScene;

namespace frozenbyte {
namespace physics {

int getActorBaseCreateCount();
int getActorBaseCount();

class ActorBase
{
protected:
	NxActor *actor;
	NxScene *scene;
	void *userData;
	int intData;

	ActorBase();
	virtual ~ActorBase();

	void init();

public:
	// Interface
	virtual bool isValid() const = 0;
	NxActor *getActor() const;

	void getPosition(VC3 &position) const;
	void getRotation(QUAT &rotation) const;
	void getMassCenterPosition(VC3 &position) const;
	void getVelocity(VC3 &velocity) const;
	void getVelocityAt(VC3 &velocity, const VC3 &pos) const;
	void getAngularVelocity(VC3 &angularVelocity) const;
	bool isSleeping() const;
	float getKineticEnergy() const;
	bool checkOverlapOBB(const VC3 &center, const VC3 &radius, const QUAT &rotation) const;
	bool getVolumeUnderWater(float water_height, VC3 &min, VC3 &max) const;

	void setPosition(const VC3 &position);
	void setRotation(const QUAT &rotation);
	void setMass(float mass);
	void movePosition(const VC3 &position);
	void moveRotation(const QUAT &rotation);

	void setVelocity(const VC3 &velocity);
	void setAngularVelocity(const VC3 &velocity);
	void addImpulse(const VC3 &position, const VC3 &impulse, bool smooth = false);
	void addImpulse(const VC3 &impulse, bool smooth = false);
	void addAcceleration(const VC3 &position, const VC3 &force);
	void addAcceleration(const VC3 &force);
	void addVelocityChange(const VC3 &position, const VC3 &force, bool smooth = false);
	void addVelocityChange(const VC3 &force, bool smooth = false);

	void addTorqueVelocityChange(const VC3 &torque, bool smooth = false);
	void addTorqueImpulse(const VC3 &torque, bool smooth = false);

	void setDamping(float linearDamping, float angularDamping);
	void putToSleep();

	// Collision group
	void setCollisionGroup(int group);

	// user data (and other contact stuff?)
	void setUserData(void *data);
	void *getUserData() const;
	void setIntData(int data);
	int getIntData() const;
	bool isDynamic() const;

	enum Feature
	{
		FREEZE_POSITION_X,
		FREEZE_POSITION_Y,
		FREEZE_POSITION_Z,
		FREEZE_ROTATION_X,
		FREEZE_ROTATION_Y,
		FREEZE_ROTATION_Z,
		DISABLE_GRAVITY,
		KINEMATIC_MODE
	};

	void enableFeature(Feature feature, bool enable);
};

} // physics
} // frozenbyte

#endif

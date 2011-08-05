
#include "precompiled.h"

#include "actor_base.h"
#include "NxPhysics.h"
#include <cassert>

namespace frozenbyte {
namespace physics {
namespace {

	int actorCreateCount = 0;
	int actorCount = 0;

} // unnamed

int getActorBaseCreateCount()
{
	return actorCreateCount;
}

int getActorBaseCount()
{
	return actorCount;
}

ActorBase::ActorBase()
:	actor(0),
	scene(0),
	userData(NULL),
	intData(0)
{
	++actorCreateCount;
	++actorCount;
}

ActorBase::~ActorBase()
{
	assert(scene->isWritable());
	if(scene && actor)
		scene->releaseActor(*actor);

	--actorCount;
	assert(actorCount >= 0);
}

void ActorBase::init()
{
	assert(scene->isWritable());
	if(!actor)
		return;

	// HAXHAX
	int shapeAmount = actor->getNbShapes();
	NxShape *const*shapes = actor->getShapes();

	while(shapeAmount--)
	{
		NxShape *shape = shapes[shapeAmount];
		shape->setFlag(NX_SF_POINT_CONTACT_FORCE, true);
	}

	if(actor->isDynamic())
	{
		setDamping(0.2f, 0.0f);
	}

	actor->userData = this;
}

NxActor *ActorBase::getActor() const
{
	return actor;
}

void ActorBase::getPosition(VC3 &position) const
{
	NxVec3 pos = actor->getGlobalPosition();
	position.x = pos.x;
	position.y = pos.y;
	position.z = pos.z;
}

void ActorBase::getRotation(QUAT &rotation) const
{
	NxQuat quat = actor->getGlobalOrientationQuat();
	rotation.x = quat.x;
	rotation.y = quat.y;
	rotation.z = quat.z;
	rotation.w = quat.w;

	rotation.Inverse();
}

void ActorBase::getMassCenterPosition(VC3 &position) const
{
	NxVec3 pos = actor->getCMassGlobalPosition();
	position.x = pos.x;
	position.y = pos.y;
	position.z = pos.z;
}

void ActorBase::getVelocity(VC3 &velocity) const
{
	NxVec3 vel = actor->getLinearVelocity();
	velocity.x = vel.x;
	velocity.y = vel.y;
	velocity.z = vel.z;
}

void ActorBase::getAngularVelocity(VC3 &angularVelocity) const
{
	NxVec3 angvel = actor->getAngularVelocity();
	angularVelocity.x = angvel.x;
	angularVelocity.y = angvel.y;
	angularVelocity.z = angvel.z;
}

void ActorBase::getVelocityAt(VC3 &velocity, const VC3 &pos) const
{
	NxVec3 vel = actor->getPointVelocity(NxVec3(pos.x, pos.y, pos.z));
	velocity.x = vel.x;
	velocity.y = vel.y;
	velocity.z = vel.z;
}

bool ActorBase::isSleeping() const
{
	return actor->isSleeping();
}

float ActorBase::getKineticEnergy() const
{
	return actor->computeKineticEnergy();
}

bool ActorBase::checkOverlapOBB(const VC3 &center, const VC3 &radius, const QUAT &rotation) const
{
	if(!actor)
		return false;

	int shapeAmount = actor->getNbShapes();
	NxShape *const*shapes = actor->getShapes();

	NxBox box;
	box.center.set(center.x, center.y, center.z);
	box.extents.set(radius.x, radius.y, radius.z);
	box.rot.id();

	QUAT r = rotation.GetInverse();
	NxQuat quat;
	quat.setXYZW(r.x, r.y, r.z, r.w);
	box.rot.fromQuat(quat);

	while(shapeAmount--)
	{
		NxShape *shape = shapes[shapeAmount];
	
		bool intersection = shape->checkOverlapOBB(box);
		if(intersection)
			return true;
	}

	return false;
}

bool ActorBase::getVolumeUnderWater(float water_height, VC3 &min, VC3 &max) const
{
	if(!actor)
		return false;

	int shapeAmount = actor->getNbShapes();
	NxShape *const*shapes = actor->getShapes();

	// get total size of shapes
	NxBounds3 totalBounds;
	totalBounds.setEmpty();
	while(shapeAmount--)
	{
		NxShape *shape = shapes[shapeAmount];
		NxBounds3 bounds;
		shape->getWorldBounds(bounds);

		if(totalBounds.isEmpty())
			totalBounds = bounds;
		else
			totalBounds.combine(bounds);
	}

	// not under water
	if(water_height < totalBounds.min.y)
		return false;

	float shape_height = totalBounds.max.y - totalBounds.min.y;
	float height_under_water = (water_height - totalBounds.min.y);
	// completely under water
	if(height_under_water > shape_height)
	{
		min = VC3(totalBounds.min.x, totalBounds.min.y, totalBounds.min.z);
		max = VC3(totalBounds.max.x, totalBounds.max.y, totalBounds.max.z);
		return true;
	}

	// partially under water
	min = VC3(totalBounds.min.x, height_under_water, totalBounds.min.z);
	max = VC3(totalBounds.max.x, totalBounds.max.y, totalBounds.max.z);
	return true;
}

void ActorBase::setPosition(const VC3 &position)
{
	actor->setGlobalPosition(NxVec3(position.x, position.y, position.z));
}

void ActorBase::setRotation(const QUAT &rotation)
{
	QUAT r = rotation.GetInverse();
	NxQuat quat;
	quat.setXYZW(r.x, r.y, r.z, r.w);
	actor->setGlobalOrientationQuat(quat);
}

void ActorBase::setMass(float mass)
{
	actor->updateMassFromShapes(0, mass);
}

void ActorBase::movePosition(const VC3 &position)
{
	actor->moveGlobalPosition(NxVec3(position.x, position.y, position.z));
}

void ActorBase::moveRotation(const QUAT &rotation)
{
	QUAT r = rotation.GetInverse();
	NxQuat quat;
	quat.setXYZW(r.x, r.y, r.z, r.w);
	actor->moveGlobalOrientationQuat(quat);
}

void ActorBase::setVelocity(const VC3 &velocity)
{
	actor->setLinearVelocity(NxVec3(velocity.x, velocity.y, velocity.z));
}

void ActorBase::setAngularVelocity(const VC3 &velocity)
{
	actor->setAngularVelocity(NxVec3(velocity.x, velocity.y, velocity.z));
}

void ActorBase::addImpulse(const VC3 &position, const VC3 &impulse, bool smooth)
{
	actor->addForceAtPos(NxVec3(impulse.x, impulse.y, impulse.z), NxVec3(position.x, position.y, position.z), smooth ? NX_SMOOTH_IMPULSE : NX_IMPULSE);
}

void ActorBase::addImpulse(const VC3 &impulse, bool smooth)
{
	actor->addForce(NxVec3(impulse.x, impulse.y, impulse.z), smooth ? NX_SMOOTH_IMPULSE : NX_IMPULSE);
}

void ActorBase::addAcceleration(const VC3 &position, const VC3 &force)
{
	actor->addForceAtPos(NxVec3(force.x, force.y, force.z), NxVec3(position.x, position.y, position.z), NX_ACCELERATION);
}

void ActorBase::addAcceleration(const VC3 &force)
{
	actor->addForce(NxVec3(force.x, force.y, force.z), NX_ACCELERATION);
}

void ActorBase::addVelocityChange(const VC3 &position, const VC3 &force, bool smooth)
{
	actor->addForceAtPos(NxVec3(force.x, force.y, force.z), NxVec3(position.x, position.y, position.z), smooth ? NX_SMOOTH_VELOCITY_CHANGE  : NX_VELOCITY_CHANGE);
}

void ActorBase::addVelocityChange(const VC3 &force, bool smooth)
{
	actor->addForce(NxVec3(force.x, force.y, force.z), smooth ? NX_SMOOTH_VELOCITY_CHANGE  : NX_VELOCITY_CHANGE);
}

void ActorBase::addTorqueVelocityChange(const VC3 &torque, bool smooth)
{
	actor->addTorque(NxVec3(torque.x, torque.y, torque.z), smooth ? NX_SMOOTH_VELOCITY_CHANGE  : NX_VELOCITY_CHANGE);
}

void ActorBase::addTorqueImpulse(const VC3 &torque, bool smooth)
{
	actor->addTorque(NxVec3(torque.x, torque.y, torque.z), smooth ? NX_SMOOTH_IMPULSE : NX_IMPULSE);
}

void ActorBase::setDamping(float linearDamping, float angularDamping)
{
	actor->setLinearDamping(linearDamping);
	actor->setAngularDamping(angularDamping);
}

void ActorBase::putToSleep()
{
	if (actor)
		actor->putToSleep();
}

void ActorBase::setCollisionGroup(int group)
{
	assert(group >= 0 && group < 31);

	//HAXHAX
	actor->setGroup(group);

	int shapeAmount = actor->getNbShapes();
	NxShape *const*shapes = actor->getShapes();

	while(shapeAmount--)
	{
		shapes[shapeAmount]->setGroup(group);
	}
}

void ActorBase::enableFeature(Feature feature, bool enable)
{
	NxBodyFlag flag;
	if(feature == FREEZE_POSITION_X)
		flag = NX_BF_FROZEN_POS_X;
	else if(feature == FREEZE_POSITION_Y)
		flag = NX_BF_FROZEN_POS_Y;
	else if(feature == FREEZE_POSITION_Z)
		flag = NX_BF_FROZEN_POS_Z;
	else if(feature == FREEZE_ROTATION_X)
		flag = NX_BF_FROZEN_ROT_X;
	else if(feature == FREEZE_ROTATION_Y)
		flag = NX_BF_FROZEN_ROT_Y;
	else if(feature == FREEZE_ROTATION_Z)
		flag = NX_BF_FROZEN_ROT_Z;
	else if(feature == DISABLE_GRAVITY)
		flag = NX_BF_DISABLE_GRAVITY;
	else if(feature == KINEMATIC_MODE)
		flag = NX_BF_KINEMATIC;
	else {
		assert(!"Invalid parameter");
		return;
	}

	if(enable)
		actor->raiseBodyFlag(flag);
	else
		actor->clearBodyFlag(flag);
}

void ActorBase::setUserData(void *data)
{
  this->userData = data;
}

void *ActorBase::getUserData() const
{
	return userData;
}

void ActorBase::setIntData(int data)
{
	intData = data;
}

int ActorBase::getIntData() const
{
	return intData;
}

bool ActorBase::isDynamic() const
{
	if(!actor) return false;
	return actor->isDynamic();
}

} // physics
} // frozenbyte

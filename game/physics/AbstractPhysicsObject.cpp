
#include "precompiled.h"

#include "AbstractPhysicsObject.h"

#include "GamePhysics.h"
#ifdef PHYSICS_PHYSX
#include "../../physics/physics_lib.h"
#include "../../physics/box_actor.h"
#include "../../physics/actor_base.h"
#endif
#include "../../util/SoundMaterialParser.h"
#include "../../util/ObjectDurabilityParser.h"

namespace game
{
	AbstractPhysicsObject::AbstractPhysicsObject(GamePhysics *gamePhysics) 
	{ 
		this->handle = 0;
		this->position = VC3(0,0,0);
		this->rotation = QUAT();
		this->velocity = VC3(0,0,0);
		this->massCenterPosition = VC3(0,0,0);
		this->angularVelocity = VC3(0,0,0);
		this->acceleration = VC3(0,0,0);
		this->angularAcceleration = VC3(0,0,0);

		this->previousVelocity = VC3(0,0,0);
		this->previousAngularVelocity = VC3(0,0,0);

		this->impulsePosition = VC3(0,0,0);
		this->impulse = VC3(0,0,0);
		this->doImpulse = false;

		// are we forcing the object to be moved to some position/rotation/velocity?
		this->moveToVelocity = false;
		this->moveToAngularVelocity = false;
		this->moveToPosition = false;
		this->moveToRotation = false;
		// is the this movement "softly forced", that is only attempt to get the object to new parameters, but 
		// don't force it there if blocked or something like that.
		this->attemptedPosition = false;
		this->attemptedRotation = false;

		this->dirty = false;

		this->gamePhysics = gamePhysics;
		this->disableAngularVelocitySet = false;
		this->disableYMovementSet = false;
		this->enableYMovementSet = false;
		this->dynamicActor = true;

		this->toSleep = false;

#ifdef PHYSICS_FEEDBACK
		this->feedbackNormal = VC3(0,0,0);
		this->feedbackNormalLeft = VC3(0,0,0);
		this->feedbackNormalRight = VC3(0,0,0);
		this->feedbackEnabled = false;
#endif

		this->soundMaterial = SOUNDMATERIALPARSER_NO_SOUND_INDEX;
		this->durabilityType = OBJECTDURABILITYPARSER_NO_DURABILITY_INDEX;
		this->lastEffectTick = 0;
		this->customData = 0;

		this->lastEffectSoundTick = 0;

//		this->m_actor = NULL;

		// FIXME: ugly hack
		// workaround gamePhysics sometimes being NULL
		// WHY?
		if (gamePhysics != NULL)
		gamePhysics->addNewObject(this); 
	}

	AbstractPhysicsObject::~AbstractPhysicsObject() 
	{ 
		// FIXME: ugly hack
		// workaround gamePhysics sometimes being NULL
		// WHY?
		if (gamePhysics != NULL)
		gamePhysics->removeObject(this); 
	}

	void AbstractPhysicsObject::setPosition(const VC3 &position)
	{
		this->position = position;
		this->moveToPosition = true;
		this->attemptedPosition = false;
		this->dirty = true;
	}

	void AbstractPhysicsObject::setVelocity(const VC3 &velocity)
	{
		this->velocity = velocity;
		this->moveToVelocity = true;
		this->dirty = true;
	}

	void AbstractPhysicsObject::setAngularVelocity(const VC3 &angularVelocity)
	{
		this->angularVelocity = angularVelocity;
		this->moveToAngularVelocity = true;
		this->dirty = true;
	}

	void AbstractPhysicsObject::setRotation(const QUAT &rotation)
	{
		this->rotation = rotation;
		this->moveToRotation = true;
		this->attemptedRotation = false;
		this->dirty = true;
	}

	void AbstractPhysicsObject::movePosition(const VC3 &position)
	{
		// FIXME: if already forced to some position, this soft non-forcing movement should not override that!
		// for now, just don't ignore the whole call in such case?
		if (!this->moveToPosition)
		{
			this->position = position;
			this->moveToPosition = true;
			this->attemptedPosition = true;
			this->dirty = true;
		}
	}

	void AbstractPhysicsObject::addImpulse(const VC3 &position, const VC3 &impulse)
	{
		// TODO: should queue impulses!
		// now this just syncs multiple impulses all to same position (position of the last impulse)!
		this->impulsePosition = position;
		this->impulse += impulse;
		this->doImpulse = true;
		this->dirty = true;
	}

	void AbstractPhysicsObject::moveRotation(const QUAT &rotation)
	{
		// FIXME: if already forced to some position, this soft non-forcing movement should not override that!
		// for now, just don't ignore the whole call in such case?
		if (!this->moveToRotation)
		{
			this->rotation = rotation;
			this->moveToRotation = true;
			this->attemptedRotation = true;
			this->dirty = true;
		}
	}

	const VC3 &AbstractPhysicsObject::getPosition()
	{
		return position;
	}

	const VC3 &AbstractPhysicsObject::getMassCenterPosition()
	{
		return massCenterPosition;
	}

	const VC3 &AbstractPhysicsObject::getVelocity()
	{
		return velocity;
	}

	const VC3 &AbstractPhysicsObject::getAngularVelocity()
	{
		return angularVelocity;
	}

	const VC3 &AbstractPhysicsObject::getPreviousVelocity()
	{
		return previousVelocity;
	}

	const VC3 &AbstractPhysicsObject::getPreviousAngularVelocity()
	{
		return previousAngularVelocity;
	}

	const VC3 &AbstractPhysicsObject::getAcceleration()
	{
		return acceleration;
	}

	const VC3 &AbstractPhysicsObject::getAngularAcceleration()
	{
		return angularAcceleration;
	}

	const QUAT &AbstractPhysicsObject::getRotation()
	{
		return rotation;
	}

	void AbstractPhysicsObject::syncImplementationObject(PHYSICS_ACTOR &obj)
	{
		if(!dynamicActor)
			return;

		if (this->toSleep)
		{
			this->toSleep = false;
			obj->putToSleep();
		}

		VC3 prevVel = velocity;
		VC3 prevAngVel = angularVelocity;

		if (this->enableYMovementSet)
		{
			this->enableYMovementSet = false;
#ifdef PHYSICS_PHYSX
			obj->enableFeature(frozenbyte::physics::ActorBase::FREEZE_POSITION_Y, false);
#endif
#ifdef PHYSICS_ODE

#pragma message("FIXME: enableYMovement")
//			obj->enableYMovement();
#endif
		}

		if (moveToVelocity)
		{
			obj->setVelocity(velocity);
			moveToVelocity = false;
		} else {
			obj->getVelocity(velocity);
		}

		if (moveToAngularVelocity)
		{
			obj->setAngularVelocity(angularVelocity);
			moveToAngularVelocity = false;
		} else {
			obj->getAngularVelocity(angularVelocity);
		}

		if (doImpulse)
		{
			obj->addImpulse(impulsePosition, impulse);
			impulsePosition = VC3(0,0,0);
			impulse = VC3(0,0,0);
			doImpulse = false;
		}

		if (moveToPosition)
		{
			if (attemptedPosition)
			{
				obj->movePosition(position);
				attemptedPosition = false;
			} else {
				obj->setPosition(position);
			}
			moveToPosition = false;
		} else {
			obj->getPosition(position);
		}

		if (moveToRotation)
		{
			if (attemptedRotation)
			{
				obj->moveRotation(rotation);
				attemptedRotation = false;
			} else {
				obj->setRotation(rotation);
			}
			moveToRotation = false;
		} else {
			obj->getRotation(rotation);
		}

		obj->getMassCenterPosition(massCenterPosition);

		if (this->disableAngularVelocitySet)
		{
			this->disableAngularVelocitySet = false;
#ifdef PHYSICS_PHYSX
			obj->enableFeature(frozenbyte::physics::ActorBase::FREEZE_ROTATION_X, true);
			obj->enableFeature(frozenbyte::physics::ActorBase::FREEZE_ROTATION_Y, true);
			obj->enableFeature(frozenbyte::physics::ActorBase::FREEZE_ROTATION_Z, true);
#endif
#ifdef PHYSICS_ODE
			obj->disableAngularVelocity();
#endif
		}

		if (this->disableYMovementSet)
		{
			this->disableYMovementSet = false;
#ifdef PHYSICS_PHYSX
			obj->enableFeature(frozenbyte::physics::ActorBase::FREEZE_POSITION_Y, true);
#endif
#ifdef PHYSICS_ODE

#pragma message("FIXME: disableYMovement")
//			obj->disableYMovement();
#endif
		}

#ifdef PHYSICS_FEEDBACK
#ifdef PHYSICS_ODE
		obj->getFeedbackNormal(this->feedbackNormal);
		obj->getFeedbackNormalLeft(this->feedbackNormalLeft);
		obj->getFeedbackNormalRight(this->feedbackNormalRight);
#endif
#endif

		// FIXME: should calculate this in correct measures... m/s^2 maybe...
		// now, it's based on the frequency of the syncImplementationObject calls...
		// (m/tick^2?)
		this->acceleration = velocity - prevVel;
		this->angularAcceleration = angularVelocity - prevAngVel;

		this->previousVelocity = prevVel;
		this->previousAngularVelocity = prevAngVel;

		this->dirty = false;
	}


	void AbstractPhysicsObject::syncInactiveImplementationObject(PHYSICS_ACTOR &obj)
	{
		if(!dynamicActor)
			return;

		if (moveToVelocity)
		{
			obj->setVelocity(velocity);
			moveToVelocity = false;
		}

		if (moveToAngularVelocity)
		{
			obj->setAngularVelocity(angularVelocity);
			moveToAngularVelocity = false;
		}

		if (doImpulse)
		{
			obj->addImpulse(impulsePosition, impulse);
			impulsePosition = VC3(0,0,0);
			impulse = VC3(0,0,0);
			doImpulse = false;
		}

		if (moveToPosition)
		{
			if (attemptedPosition)
			{
				obj->movePosition(position);
				attemptedPosition = false;
			} else {
				obj->setPosition(position);
			}
			moveToPosition = false;
		}

		if (moveToRotation)
		{
			if (attemptedRotation)
			{
				obj->moveRotation(rotation);
				attemptedRotation = false;
			} else {
				obj->setRotation(rotation);
			}
			moveToRotation = false;
		}

		this->dirty = false;
	}


	void AbstractPhysicsObject::disableAngularVelocity()
	{
		this->disableAngularVelocitySet = true;
	}

	void AbstractPhysicsObject::disableYMovement()
	{
		// don't do this, it's used for a hack...
		//this->enableYMovementSet = false;

		this->disableYMovementSet = true;
	}

	void AbstractPhysicsObject::enableYMovement()
	{
		// don't do this, it's used for a hack...
		//this->disableYMovementSet = false;

		this->enableYMovementSet = true;
	}

	void AbstractPhysicsObject::setToSleep()
	{
		this->toSleep = true;
	}

#ifdef PHYSICS_FEEDBACK
	void AbstractPhysicsObject::setFeedbackEnabled(bool feedbackEnabled)
	{
		this->feedbackEnabled = feedbackEnabled;
		// TODO: ODE, should pass this flag to the ode implementation instead of using the movement disable flag as feedback flag
	}

	bool AbstractPhysicsObject::isFeedbackEnabled() const
	{
		return this->feedbackEnabled;
	}

#ifdef PHYSICS_PHYSX
	void AbstractPhysicsObject::setFeedbackNormal(const VC3 &vec)
	{
		this->feedbackNormal = vec;
	}

	void AbstractPhysicsObject::setFeedbackNormalLeft(const VC3 &vec)
	{
		this->feedbackNormalLeft = vec;
	}

	void AbstractPhysicsObject::setFeedbackNormalRight(const VC3 &vec)
	{
		this->feedbackNormalRight = vec;
	}
#endif

	const VC3 &AbstractPhysicsObject::getFeedbackNormal() const
	{
		assert(this->feedbackEnabled);
		return this->feedbackNormal;
	}
	const VC3 &AbstractPhysicsObject::getFeedbackNormalLeft() const
	{
		assert(this->feedbackEnabled);
		return this->feedbackNormalLeft;
	}
	const VC3 &AbstractPhysicsObject::getFeedbackNormalRight() const
	{
		assert(this->feedbackEnabled);
		return this->feedbackNormalRight;
	}
#endif

	void AbstractPhysicsObject::setHandle(int objectHandle)
	{
		this->handle = objectHandle;
	}

	int AbstractPhysicsObject::getHandle() const
	{
		return this->handle;
	}

	int AbstractPhysicsObject::getSoundMaterial()
	{
		return this->soundMaterial;
	}

	void AbstractPhysicsObject::setSoundMaterial(int soundMaterialIndex)
	{
		this->soundMaterial = soundMaterialIndex;
	}

	int AbstractPhysicsObject::getDurabilityType()
	{
		return this->durabilityType;
	}

	void AbstractPhysicsObject::setDurabilityType(int durabilityTypeIndex)
	{
		this->durabilityType = durabilityTypeIndex;
	}

	void *AbstractPhysicsObject::getCustomData()
	{
		return this->customData;
	}

	void AbstractPhysicsObject::setCustomData(void *customData)
	{
		this->customData = customData;
	}

	void AbstractPhysicsObject::restorePreviousVelocities(float previousVelocityRatio, float currentVelocityRatio)
	{
		VC3 linvel = this->previousVelocity * previousVelocityRatio + this->velocity * currentVelocityRatio;
		this->setVelocity(linvel);

		VC3 angvel = this->previousAngularVelocity * previousVelocityRatio + this->angularVelocity * currentVelocityRatio;
		this->setAngularVelocity(angvel);
	}

	bool AbstractPhysicsObject::isDynamic()
	{
		return this->dynamicActor;
	}

}



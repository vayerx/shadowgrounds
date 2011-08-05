
#ifndef ABSTRACTPHYSICSOBJECT_H
#define ABSTRACTPHYSICSOBJECT_H

#include "IGamePhysicsObject.h"
#include "DatatypeDef.h"

namespace game
{
	class GamePhysics;

	class AbstractPhysicsObject : public IGamePhysicsObject
	{
	public:
		AbstractPhysicsObject(GamePhysics *gamePhysics);

		virtual ~AbstractPhysicsObject();

		virtual void setPosition(const VC3 &position);
		virtual void setVelocity(const VC3 &velocity);
		virtual void setAngularVelocity(const VC3 &angularVelocity);
		virtual void setRotation(const QUAT &rotation);

		virtual void movePosition(const VC3 &position);
		virtual void moveRotation(const QUAT &rotation);

		void addImpulse(const VC3 &position, const VC3 &impulse);

		virtual const VC3 &getMassCenterPosition();
		virtual const VC3 &getPosition();
		virtual const VC3 &getVelocity();
		virtual const VC3 &getAngularVelocity();
		virtual const QUAT &getRotation();

		virtual const VC3 &getPreviousVelocity();
		virtual const VC3 &getPreviousAngularVelocity();

		virtual const VC3 &getAcceleration();
		virtual const VC3 &getAngularAcceleration();

		virtual void disableAngularVelocity();
		virtual void disableYMovement();
		virtual void enableYMovement();

		virtual void setToSleep();

		virtual int getSoundMaterial();
		virtual void setSoundMaterial(int soundMaterialIndex);
		virtual int getDurabilityType();
		virtual void setDurabilityType(int durabilityTypeIndex);

		virtual int getLastEffectTick() { return this->lastEffectTick; }
		virtual void setLastEffectTick(int tick) { this->lastEffectTick = tick; }

		virtual int getLastEffectSoundTick() { return this->lastEffectSoundTick; }
		virtual void setLastEffectSoundTick( int tick ) { lastEffectSoundTick = tick; }

		virtual void *getCustomData();
		virtual void setCustomData(void *customData);

		virtual void restorePreviousVelocities(float previousVelocityRatio = 1.0f, float currentVelocityRatio = 0.0f);
		virtual bool isDynamic();

#ifdef PHYSICS_FEEDBACK
		virtual const VC3 &getFeedbackNormal() const;
		virtual const VC3 &getFeedbackNormalLeft() const;
		virtual const VC3 &getFeedbackNormalRight() const;

#ifdef PHYSICS_PHYSX
		virtual void setFeedbackNormal(const VC3 &vec);
		virtual void setFeedbackNormalLeft(const VC3 &vec);
		virtual void setFeedbackNormalRight(const VC3 &vec);
#endif

		virtual void setFeedbackEnabled(bool feedbackEnabled);
		virtual bool isFeedbackEnabled() const;
#endif

		virtual int getHandle() const;

//		inline NxActor * getActor() const { return m_actor; }

	protected:
		virtual void setHandle(int objectHandle);

		virtual PHYSICS_ACTOR createImplementationObject() = 0;
		virtual void syncImplementationObject(PHYSICS_ACTOR &obj);

		virtual void syncInactiveImplementationObject(PHYSICS_ACTOR &obj);

		int handle;
		VC3 massCenterPosition;
		VC3 position;
		VC3 velocity;
		VC3 angularVelocity;
		VC3 acceleration;
		VC3 angularAcceleration;
		QUAT rotation;
		bool moveToPosition;
		bool moveToRotation;
		bool moveToVelocity;
		bool moveToAngularVelocity;
		bool attemptedPosition;
		bool attemptedRotation;

#ifdef PHYSICS_PHYSX
		NxActor * m_actor;
#endif

		VC3 previousVelocity;
		VC3 previousAngularVelocity;

		VC3 impulse;
		VC3 impulsePosition;
		bool doImpulse;

		bool disableAngularVelocitySet;
		bool disableYMovementSet;
		bool enableYMovementSet;

public: // public for performance hack.
		bool dynamicActor;
		bool dirty;
protected:

		bool toSleep;

		int soundMaterial;
		int durabilityType;
		int lastEffectTick;
		int lastEffectSoundTick;
		void *customData;

#ifdef PHYSICS_FEEDBACK
		bool feedbackEnabled;
		VC3 feedbackNormal;
		VC3 feedbackNormalLeft;
		VC3 feedbackNormalRight;
#endif

		GamePhysics *gamePhysics;
	};
}

#endif


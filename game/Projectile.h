
#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <DatatypeDef.h>

#include "Part.h"
#include "Bullet.h"
#include "GameObject.h"
#include "../ui/IPointableObject.h"

#include "tracking/ITrackerObject.h"
#include "tracking/ITrackerObjectType.h"
#include "tracking/ITrackableObject.h"
#include "IProjectileTrackerFactory.h"

namespace ui
{
	//class VisualObject;
	class VisualEffect;
}

namespace game
{
	class Unit;

	/**
	 *
	 * A moving (usually airborne) game projectile.
	 * Usually a bullet of some kind.
	 *
	 * @version 1.0, 23.6.2002
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see UnitActor
	 * @see UnitList
	 * @see Part
	 *
	 */

	class Projectile : public GameObject, public ui::IPointableObject, public tracking::ITrackerObject
	{
	public:

		/** 
		 * Creates a new game projectile object.
		 * If projectile has a bullet type, the behaviour and fly path will be
		 * based on the bullet type. If it does not have a bullet type, 
		 * the projectiles behaviour will depend on methods called.
		 * @param shooter  Unit*, the unit that this projectile originated from.
		 * NULL if it has not oginated from a unit.
		 * @param bulletType	Bullet*, the bullet type that this projectile is.
		 * NULL if it is not a bullet, but rather some other type of projectile.
		 */
		Projectile(Unit *shooter, Bullet *bulletType);

		virtual ~Projectile();

		/** 
		 * TODO!
		 * To implement GameObject "interface" class.
		 * @return SaveData, data to be saved. TODO, currently NULL.
		 */
		virtual SaveData *getSaveData() const;

		virtual const char *getStatusInfo() const;

		/**
		 * Trackable by visual effects. Implements the 
		 * IPointableObject interface.
		 */
		virtual const VC3 &getPointerPosition() const;
		virtual const VC3 getPointerMiddleOffset() const;

		/**
		 * Sets a direct path for the projectile.
		 * Makes the projectile move from origin to destination with given velocity.
		 * The lifetime and direction of the projectile are calculated based on 
		 * given values.
		 * @param origin	VC3&, the world starting position for the projectile.
		 * @param destination  VC3&, the world ending position for the projectile.
		 * @param velocity	float, the velocity of the projectile 
		 * (in meters per game ticks, which are 10ms at the moment).
		 */
		void setDirectPath(const VC3 &origin, const VC3 &destination, float velocity);

		void setPathByDirection(const VC3 &origin, const VC3 &direction, float velocity, int lifeTime);

		// sets some spread to the projectile 
		// (does not go exactly to destination)
		void makeSpread(VC3 &spreadVector);

		void setPosition(const VC3 &position);
		void setDestination(const VC3 &destination);
		void setVelocity(const VC3 &velocity);
		void setDirection(const VC3 &direction);
		void setHitNormal(const VC3 &hitNormal);
		//void setScale(VC3 &scale);
		void setRotation(float xAngle, float yAngle, float zAngle);
		void setLifeTime(int lifeTime);
		void setAfterLifeTime(int afterLifeTime);
		void setParabolicPathHeight(float height);

		void setHitTarget(Unit *unit, Part *part); // first param can't be const
		Unit *getHitUnit() const; // can't return const
		Part *getHitPart() const;

		void setOrigin(const VC3 &origin);

		const VC3 &getPosition() const;
		const VC3 &getDestination() const;
		const VC3 &getOrigin() const;
		const VC3 &getVelocity() const;
		const VC3 &getDirection() const;
		const VC3 &getHitNormal() const;
		VC3 getRotation() const;
		//VC3 getScale() const;
		int getLifeTime() const;
		int getAfterLifeTime() const;
		int getOriginalLifeTime() const;
		float getParabolicPathHeight() const;

		int getChainCustomValue() const;
		void setChainCustomValue(int chainCustomValue);

		int getCurrentSplitPosition() const;
		void setCurrentSplitPosition(int splitPosition);

		void setInflictDamage(bool inflict);
		bool doesInflictDamage() const;

		void setShooter(Unit *shooter);
		Unit *getShooter() const;
		Bullet *getBulletType() const;

		//ui::VisualObject *getVisualObject() const;
		//void setVisualObject(ui::VisualObject *visualObject);
		ui::VisualEffect *getVisualEffect() const;
		void setVisualEffect(ui::VisualEffect *visualEffect);

		int getVisualType();

		void setChain(int chain);

		int getChain() const;

		Projectile *getCopy() const;

		int getHandle() const;

		void setParentUnit(Unit *parentUnit);
		void setParentProjectile(Projectile *parentProjectile);

		Projectile *getParentProjectile() const;
		Unit *getParentUnit() const;

		Unit *getOriginUnit() const;
		void setOriginUnit(Unit *originUnit);

		bool doesHitSound() const;
		void setHitSound(bool hitSound);

		void setFollowOrigin(bool followOrigin);
		bool doesFollowOrigin();


		// implementation for the tracking interface...
		virtual tracking::ITrackerObjectType *getType();
		virtual void tick();
		virtual void setTrackablePosition(const VC3 &globalPosition);
		virtual void setTrackableVelocity(const VC3 &velocity);
		virtual void setTrackableRotation(const QUAT &rotation);
		virtual void lostTracked();
		virtual void trackerDeleted();
		virtual void attachedToTrackable(tracking::ITrackableObject *trackable);
		virtual void setTrackerPosition(const VC3 &position);
		virtual VC3 getTrackerPosition() const;
		virtual void iterateTrackables(tracking::ITrackableObjectIterator *iter);
		virtual void trackerSignal(int trackerSignalNumber);

		void setProjectileIsPointedBy(IProjectileTrackerFactory *pointer);

		const VC3 &getOriginOffset() { return originOffset; }
		void setOriginOffset(const VC3 &offset) { originOffset = offset; }

		Unit *getForceGoreExplosionUnit() { return forceGoreExplosionUnit; }
		void setForceGoreExplosionUnit(Unit *unit) { forceGoreExplosionUnit = unit; }


	private:
		Bullet *bulletType;
		Unit *shooter;

		int projectileHandle;

		VC3 position;

		VC3 velocity;

		VC3 direction;

		//VC3 scale;

		// angles
		float xAngle;
		float yAngle;
		float zAngle;

		VC3 destination;
		VC3 origin;
		VC3 hitNormal;

		int lifeTime;

		int visualType;
		int afterLifeTime;
		int originalLifeTime;

		float parabolicPathHeight;

		int chain;

		bool inflictDamage;

		int chainCustomValue;

		int currentSplitPosition;

		Unit *hitUnit;  // can't be const
		Part *hitPart;

		//ui::VisualObject *visualObject;
		ui::VisualEffect *visualEffect;

		Projectile *child;
		Projectile *parentProjectile;
		Unit *parentUnit;

		Unit *originUnit;

		bool doHitSound;

		bool followOrigin;

		VC3 originOffset;

		IProjectileTrackerFactory *projectileIsPointedBy;

		Unit *forceGoreExplosionUnit;

	public:
		int criticalHitDamageMax;
		float criticalHitDamageMultiplier;
		float criticalHitProbabilityMultiplier;
	};

}

#endif


#include "precompiled.h"

#include "Projectile.h"
#include "ProjectileTrackerObjectType.h"
#include "projectilevisdefs.h"
#include "scaledefs.h"
#include "../ui/VisualEffect.h"
#include "../system/Logger.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_game.h"
#include "IProjectileTrackerFactory.h"

#include "tracking/trackable_typeid.h"

#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"


using namespace ui;

namespace game
{

	static int nextProjectileHandle = 1;


	Projectile::Projectile(Unit *shooter, Bullet *bulletType)
	{
		this->projectileHandle = nextProjectileHandle;
		nextProjectileHandle++;

		this->shooter = shooter;
		this->bulletType = bulletType;
		
		position = VC3(0,0,0);
		velocity = VC3(0,0,0);
		direction = VC3(0,0,0);
		xAngle = 0;
		yAngle = 0;
		zAngle = 0;
		destination = VC3(0,0,0);
		origin = VC3(0,0,0);
		hitNormal = VC3(0,1,0);
		//scale = VC3(1,1,1);
		lifeTime = 0;
		afterLifeTime = 0;
		hitUnit = NULL;
		hitPart = NULL;
		//visualObject = NULL;
		visualEffect = NULL;
		chain = HITCHAIN_NOTHING;
		parabolicPathHeight = 0;
		originalLifeTime = 0;
		inflictDamage = true;
		chainCustomValue = 0;

		currentSplitPosition = 0;

		visualType = 0;
		//visualType = PROJECTILE_VIS_NONE;
		if (bulletType != NULL)
		{
			visualType = bulletType->getVisualEffectNumber();
			/*
			if (visualType == PROJECTILE_VIS_GAUSS)
			{
				this->scale = VC3(2,2,8);
			} else {
				if (visualType == PROJECTILE_VIS_RED_PULSE1
					|| visualType == PROJECTILE_VIS_RED_PULSE2)
				{
					if (visualType == PROJECTILE_VIS_RED_PULSE2)
						this->scale = VC3(1,1,1);
					else
						this->scale = VC3(1,1,1);
				} else {
					this->scale = VC3(1,1,1);
				}
			}
			*/
		}

		child = NULL;
		parentProjectile = NULL;
		parentUnit = NULL;

		originUnit = NULL;

		this->doHitSound = true;
		this->followOrigin = false;
		this->projectileIsPointedBy = NULL;
		this->originOffset = VC3(0,0,0);
		this->forceGoreExplosionUnit = NULL;

		this->criticalHitDamageMax = 10000;
		this->criticalHitDamageMultiplier = 100.0f; 
		this->criticalHitProbabilityMultiplier = 1.0f;
	}

	Projectile::~Projectile()
	{
		if (this->projectileIsPointedBy != NULL)
		{
			this->projectileIsPointedBy->projectileDeleted(this);
		}

		//if (visualObject != NULL)
		//  delete visualObject;
		if (visualEffect != NULL)
		{
			visualEffect->setDeleteFlag();
			visualEffect->freeReference();
			visualEffect = NULL;
		}

		if (child != NULL)
		{
			fb_assert(child->parentProjectile == this);
			child->parentProjectile = NULL;
			this->child = NULL;
		}
		if (parentProjectile != NULL)
		{
			fb_assert(parentProjectile->child == this);
			parentProjectile->child = NULL;
			this->parentProjectile = NULL;
		}
	}

	SaveData *Projectile::getSaveData() const
	{
		// TODO
		return NULL;
	}

	const char *Projectile::getStatusInfo() const
	{
		return "Projectile";
	}

	int Projectile::getVisualType()
	{
		return visualType;
	}

	/*
	VisualObject *Projectile::getVisualObject() const
	{
		return visualObject;
	}

	void Projectile::setVisualObject(VisualObject *visualObject)
	{
		this->visualObject = visualObject;
	}
	*/

	VisualEffect *Projectile::getVisualEffect() const
	{
		return visualEffect;
	}

	void Projectile::setVisualEffect(VisualEffect *visualEffect)
	{
		if (this->visualEffect != NULL)
		{
			this->visualEffect->freeReference();
		}
		this->visualEffect = visualEffect;
		if (visualEffect != NULL)
			visualEffect->addReference();
	}

	void Projectile::setHitNormal(const VC3 &hitNormal)
	{
		this->hitNormal = hitNormal;
	}

	const VC3 &Projectile::getHitNormal() const
	{
		return this->hitNormal;
	}

	void Projectile::setDirectPath(const VC3 &origin, const VC3 &destination, float velocity)
	{
		// skipping the first few meters from the directpath...
		VC3 actualOrigin = origin;
		if (bulletType != NULL)
		{
			//if (bulletType->getFlyPath() == Bullet::FLYPATH_DIRECT) 
			if (bulletType->getStartSkipAmount() != 0.0f) 
			{
				VC3 disttmp = destination - origin;
				float distLenSq = disttmp.GetSquareLength();
				//if (distLenSq > 4 * 4) // at least 4 meters distance?
				// umm.... maybe 2 meters?
				if (distLenSq > 2 * 2) 
				{
					// "skip" first X meters
					actualOrigin += disttmp.GetNormalized() * bulletType->getStartSkipAmount();
				}
			}
		}

		setPosition(actualOrigin);
		this->destination = destination;
		this->origin = actualOrigin;

		VC3 dist;
		float distLen;
		if (destination.x != actualOrigin.x 
			|| destination.y != actualOrigin.y
			|| destination.z != actualOrigin.z)
		{
			VC2 destFloat = VC2(
				(float)(destination.x-position.x), (float)(destination.z-position.z));
			float destAngleFloat = destFloat.CalculateAngle();
			float destAngle = -RAD_TO_UNIT_ANGLE(destAngleFloat) + (360+270);
			while (destAngle >= 360) destAngle -= 360;

			VC2 destFloat2 = VC2(
				(float)(destFloat.GetLength()), (float)(destination.y-position.y));
			float destAngleFloat2 = destFloat2.CalculateAngle();
			float destAngle2 = RAD_TO_UNIT_ANGLE(destAngleFloat2) + 360;
			while (destAngle2 >= 360) destAngle2 -= 360;

			setRotation(destAngle2, destAngle, 0);

			dist = destination - actualOrigin;

			distLen = dist.GetLength();
			if (bulletType != NULL)
			{
				if (bulletType->getFlyPath() == Bullet::FLYPATH_DIRECT) 
				{
					if (velocity < 0.01f) velocity = 0.01f;
				}
			}
			if (velocity > 0.0f)
				this->lifeTime = (int)(distLen / velocity);
			else
				this->lifeTime = 1;

			if (bulletType->isEndPadToHit())
			{
				// TODO: if (!hitToUnit || this->lifeTime > 0)
				// {
				if (this->lifeTime > 1) 
				{
					float unPaddedDist = lifeTime * velocity;
					if (unPaddedDist > 0)
					{
						velocity *= (distLen / unPaddedDist);
					}
				} else {
					velocity = distLen / 2.0f;
					this->lifeTime = 2;
				}
				// }
			}
			//if (this->lifeTime <= 0) this->lifeTime = 1;
			if (this->lifeTime <= 0) this->lifeTime = 0;
			this->direction = dist.GetNormalized();
			this->velocity = direction * velocity;
		} else {
			this->direction = VC3(0,0,0);
			this->velocity = VC3(0,0,0);
			//this->lifeTime = 1;
			this->lifeTime = 0;
			dist = VC3(0,0,0);
			distLen = 0;
		}

		if (bulletType != NULL)
		{
			// rays will be set halfway to position and stretched so that 
			// it is between them.
			if (bulletType->getFlyPath() == Bullet::FLYPATH_RAY) 
			{
				VC3 halfWayPosition = origin + (dist / 2);
				//setScale(VC3(1,1,distLen));
				setPosition(halfWayPosition);
				this->velocity = VC3(0,0,0);
				this->lifeTime = bulletType->getLifeTime();
				this->afterLifeTime = bulletType->getAfterLifeTime();
			}
			if (bulletType->getFlyPath() == Bullet::FLYPATH_STATIC
				|| bulletType->getFlyPath() == Bullet::FLYPATH_GRAVITY) 
			{
				this->lifeTime = bulletType->getLifeTime();
				this->afterLifeTime = bulletType->getAfterLifeTime();
			}
			if (bulletType->getFlyPath() == Bullet::FLYPATH_GRAVITY) 
			{
				if (SimpleOptions::getBool(DH_OPT_B_GAME_SIDEWAYS))
				{
					// no hacks for sideways, thank you.
				} else {
					// HACK: some magical constant here!
					//this->velocity.y += 0.08f;
					this->velocity.y += 0.7f * this->velocity.GetLength();
				}
			}
			if( bulletType->getFlyPath() == Bullet::FLYPATH_GOTOTARGET )
			{
				setPosition(destination);
				this->destination = destination;
				this->origin = destination;
				
				this->velocity = VC3(0,0,0);
				this->lifeTime = bulletType->getLifeTime();
				this->afterLifeTime = bulletType->getAfterLifeTime();
			}
		}
		originalLifeTime = lifeTime;
	}

	void Projectile::makeSpread(VC3 &spreadVector)
	{
		// scale to match the spread vector for 100 meter distance...
		VC3 oldDist = destination - position;
		float oldDistLen = oldDist.GetLength();
		spreadVector *= (oldDistLen / 100.0f);

		float vel = velocity.GetLength();

		destination += spreadVector;
		VC3 dist = destination - position;

		velocity = dist.GetNormalized() * vel;
		
		VC2 destFloat = VC2(
			(float)(destination.x-position.x), (float)(destination.z-position.z));
		float destAngleFloat = destFloat.CalculateAngle();
		float destAngle = -RAD_TO_UNIT_ANGLE(destAngleFloat) + (360+270);
		while (destAngle >= 360) destAngle -= 360;

		VC2 destFloat2 = VC2(
			(float)(destFloat.GetLength()), (float)(destination.y-position.y));
		float destAngleFloat2 = destFloat2.CalculateAngle();
		float destAngle2 = RAD_TO_UNIT_ANGLE(destAngleFloat2) + 360;
		while (destAngle2 >= 360) destAngle2 -= 360;

		setRotation(destAngle2, destAngle, 0);
	}

	void Projectile::setPathByDirection(const VC3 &origin, const VC3 &direction, float velocity, int lifeTime)
	{
		// TODO
		assert(0);
	}

	void Projectile::setPosition(const VC3 &position)
	{
		//VC3 offset = position - this->position;
		//this->position += offset;
		this->position = position;
		/*
		if (visualObject != NULL)
		{
			//if (visualType == PROJECTILE_VIS_RED_RAY1
			//  || visualType == PROJECTILE_VIS_RED_RAY2)
			//{
				// quick hack to skip rays...
			//  return;
			//}
			//visualObject->setPosition(position);
		}
		*/
	}

	const VC3 &Projectile::getPointerPosition() const
	{
		return position;
	}

	const VC3 Projectile::getPointerMiddleOffset() const
	{
		return VC3(0,0,0);
	}

	void Projectile::setVelocity(const VC3 &velocity)
	{
		this->velocity = velocity;
	}

	void Projectile::setDestination(const VC3 &destination)
	{
		this->destination = destination;
	}

	void Projectile::setOrigin(const VC3 &origin)
	{
		this->origin = origin;
	}

	void Projectile::setDirection(const VC3 &direction)
	{
		this->direction = direction;
	}

	void Projectile::setRotation(float xAngle, float yAngle, float zAngle)
	{
		this->xAngle = xAngle;
		this->yAngle = yAngle;
		this->zAngle = zAngle;
	}

	/*
	void Projectile::setScale(VC3 &scale)
	{
		this->scale = scale;
		if (visualObject != NULL)
		{
			if (visualObject->model != NULL)
				visualObject->model->SetScale(scale);
		}
	}
	*/

	void Projectile::setLifeTime(int lifeTime)
	{
		this->lifeTime = lifeTime;
	}

	void Projectile::setAfterLifeTime(int afterLifeTime)
	{
		this->afterLifeTime = afterLifeTime;
	}

	const VC3 &Projectile::getPosition() const
	{
		return position;
	}

	const VC3 &Projectile::getDestination() const
	{
		return destination;
	}

	const VC3 &Projectile::getOrigin() const
	{
		return origin;
	}

	const VC3 &Projectile::getVelocity() const
	{
		return velocity;
	}

	const VC3 &Projectile::getDirection() const
	{
		return direction;
	}

/*
	VC3 Projectile::getScale() const
	{
		return scale;
	}
*/

	VC3 Projectile::getRotation() const
	{
		VC3 rotation = VC3(xAngle, yAngle, zAngle);
		return rotation;
	}

	int Projectile::getLifeTime() const
	{
		return lifeTime;
	}

	int Projectile::getOriginalLifeTime() const
	{
		return originalLifeTime;
	}

	int Projectile::getAfterLifeTime() const
	{
		return afterLifeTime;
	}

	void Projectile::setHitTarget(Unit *unit, Part *part)
	{
		hitUnit = unit;
		hitPart = part;
	}

	Unit *Projectile::getHitUnit() const
	{
		return hitUnit;
	}

	Part *Projectile::getHitPart() const
	{
		return hitPart;
	}

	void Projectile::setShooter(Unit *sh)
	{
		shooter = sh;
	}

	Unit *Projectile::getShooter() const
	{
		return shooter;
	}

	Bullet *Projectile::getBulletType() const
	{
		return bulletType;
	}

	void Projectile::setChain(int chain)
	{
		this->chain = chain;
	}

	int Projectile::getChain() const
	{
		return chain;
	}

	void Projectile::setParabolicPathHeight(float height)
	{
		parabolicPathHeight = height;
	}

	float Projectile::getParabolicPathHeight() const
	{
		return parabolicPathHeight;
	}

	int Projectile::getCurrentSplitPosition() const
	{
		return currentSplitPosition;
	}

	void Projectile::setCurrentSplitPosition(int splitPosition)
	{
		this->currentSplitPosition = splitPosition;
	}

	void Projectile::setInflictDamage(bool inflict)
	{
		inflictDamage = inflict;
	}
	
	bool Projectile::doesInflictDamage() const
	{
		return inflictDamage;
	}

	Projectile *Projectile::getCopy() const
	{
		// could just use the clone operator instead of this, but don't
		// like that idea, cos it might get mixer with the usual pointer 
		// setting operators and really would not give any extra usefulness.

		Projectile *c = new Projectile(shooter, bulletType);
		//GameObjectList *tmp = c->gameObjectList;
		//ListNode *tmp2 = c->listSelfPointer;

		//*c = *this;

		c->position = position;
		c->velocity = velocity;
		c->direction = direction;
		c->xAngle = xAngle;
		c->yAngle = yAngle;
		c->zAngle = zAngle;
		c->origin = origin;
		c->destination = destination;
		//c->scale = scale;
		c->lifeTime = lifeTime;
		c->afterLifeTime = afterLifeTime;
		c->hitUnit = hitUnit;
		c->hitPart = hitPart;
		c->chain = chain;
		c->parabolicPathHeight = parabolicPathHeight;
		c->originalLifeTime = originalLifeTime;
		c->visualType = visualType;
		c->chainCustomValue = chainCustomValue;

		//c->currentSplitPosition = currentSplitPosition;
		// NOTE: split position cleared at clone!
		c->currentSplitPosition = 0;

		// NOTE: setting hitsound flag to default at clone!
		c->doHitSound = true;

		//c->gameObjectList = tmp;
		//c->listSelfPointer = tmp2;
		//c->visualObject = NULL;
		c->visualEffect = NULL;

		c->originOffset = originOffset;

		c->forceGoreExplosionUnit = forceGoreExplosionUnit;

		// FIXME: should we clone the parent unit/projectile pointers?
		// could that possibly crash something (as only one parent/child allowed for each projectile)
	
		c->criticalHitDamageMax = criticalHitDamageMax;
		c->criticalHitDamageMultiplier = criticalHitDamageMultiplier; 
		c->criticalHitProbabilityMultiplier = criticalHitProbabilityMultiplier;

		return c;
	}

	int Projectile::getHandle() const
	{
		return this->projectileHandle;
	}

	int Projectile::getChainCustomValue() const
	{
		return this->chainCustomValue;
	}

	void Projectile::setChainCustomValue(int chainCustomValue)
	{
		this->chainCustomValue = chainCustomValue;
	}

	void Projectile::setParentUnit(Unit *parentUnit)
	{
		// parentunit should only be set once... (before any parent projectile)?
		fb_assert(this->parentUnit == NULL);
		fb_assert(this->parentProjectile == NULL);

		this->parentUnit = parentUnit;
	}

	void Projectile::setParentProjectile(Projectile *parentProjectile)
	{
		// parentprojectile should only be set once...?
		fb_assert(this->parentProjectile == NULL);

		// no more parented to unit
		this->parentUnit = NULL;
		this->parentProjectile = parentProjectile;
		parentProjectile->child = this;
	}

	Projectile *Projectile::getParentProjectile() const
	{
		return this->parentProjectile;
	}

	Unit *Projectile::getParentUnit() const
	{
		return this->parentUnit;
	}

	Unit *Projectile::getOriginUnit() const
	{
		return this->originUnit;
	}

	void Projectile::setOriginUnit(Unit *originUnit)
	{
		this->originUnit = originUnit;
	}

	bool Projectile::doesHitSound() const
	{
		return doHitSound;
	}

	void Projectile::setHitSound(bool hitSound)
	{
		this->doHitSound = hitSound;
	}

	void Projectile::setFollowOrigin(bool followOrigin)
	{
		this->followOrigin = followOrigin;
	}

	bool Projectile::doesFollowOrigin()
	{
		return this->followOrigin;
	}


	tracking::ITrackerObjectType *Projectile::getType()
	{
		return &ProjectileTrackerObjectType::instance;
	}

	void Projectile::tick()
	{
		// nop
	}

	void Projectile::setTrackablePosition(const VC3 &globalPosition)
	{
		setTrackerPosition(globalPosition);
	}

	void Projectile::setTrackableRotation(const QUAT &rotation)
	{
		// quat to axis angles...
		QUAT quat = rotation;

		quat.Inverse();
		MAT tm;
		tm.CreateRotationMatrix(quat);

		float heading = 0.f;
		float attitude = 0.f;
		float bank = 0.f;

		float m00 = tm.Get(0);
		float m02 = tm.Get(2);
		float m10 = tm.Get(4);
		float m11 = tm.Get(5);
		float m12 = tm.Get(6);
		float m20 = tm.Get(8);
		float m22 = tm.Get(10);

		if(m10 > 0.998f)
		{
			heading = atan2f(m02, m22);
			attitude = -PI/2.f;
			bank = 0.f;
		}
		else if(m10 < -0.998f)
		{
			heading = atan2f(m02, m22);
			attitude = PI/2;
			bank = 0.f;
		}
		else
		{
			heading = atan2f(-m20, m00);
			attitude = -asinf(m10);
			bank = atan2f(-m12, m11);
		}

		VC3 axisAngle = VC3(bank, heading, attitude);

		axisAngle.x *= 180.0f / 3.1415f;
		axisAngle.y *= 180.0f / 3.1415f;
		axisAngle.z *= 180.0f / 3.1415f;

		this->xAngle = axisAngle.x;
		this->yAngle = axisAngle.y;
		this->zAngle = axisAngle.z;

		if (visualEffect != NULL)
		{
			visualEffect->setRotation(axisAngle);
		}
	}

	void Projectile::setTrackableVelocity(const VC3 &velocity)
	{
		// TODO
	}

	void Projectile::lostTracked()
	{
		if (this->lifeTime > 1)
			this->lifeTime = 1;
	}

	void Projectile::trackerSignal(int trackerSignalNumber)
	{
		// TODO: ...
		// handle KILL signal?
	}


	void Projectile::trackerDeleted()
	{
		if (this->lifeTime > 1)
			this->lifeTime = 1;
	}

	void Projectile::attachedToTrackable(tracking::ITrackableObject *trackable)
	{
		// nop?
		// ...or...
		//if (trackable == NULL)
		//	lostTracked();
	}

	void Projectile::setTrackerPosition(const VC3 &position)
	{
		this->position = position;
		if (visualEffect != NULL)
		{
			visualEffect->setPosition(position);
		}
	}

	VC3 Projectile::getTrackerPosition() const
	{
		return this->position;
	}

	void Projectile::iterateTrackables(tracking::ITrackableObjectIterator *iter)
	{
		// nop?
	}

	void Projectile::setProjectileIsPointedBy(IProjectileTrackerFactory *pointer)
	{
		this->projectileIsPointedBy = pointer;
	}

}



#ifndef PROJECTILETRACKEROBJECTTYPE_H
#define PROJECTILETRACKEROBJECTTYPE_H

#include "tracking/ITrackerObjectType.h"
#include "IProjectileTrackerFactory.h"
#include "Projectile.h"

namespace game
{
	namespace tracking
	{
		class ITrackerObject;
	};

	class ProjectileTrackerObjectType : public tracking::ITrackerObjectType
	{
	public:
		virtual std::string getTrackerTypeName() const
		{
			return "projectile";
		}

		virtual bool doesGiveOwnershipToObjectTracker() const
		{
			return false;
		}

		virtual void *getTypeId() const
		{
			return &ProjectileTrackerObjectType::instance;
		}

		virtual int getTickInterval() const 
		{
			return 1000;
		}

		virtual bool doesAllowTickBalancing() const
		{
			return true;
		}

		// the area within which the tracker is interested in trackables...
		virtual float getAreaOfInterestRadius() const
		{
			return 0.0f;
		}

		// the type of trackables which the tracker is interested in (may be multi bit mask)
		virtual TRACKABLE_TYPEID_DATATYPE getTrackablesTypeOfInterest() const
		{
			return 0;
		}

		virtual tracking::ITrackerObject *createNewObjectInstance()
		{
			assert(projectileTrackerFactory != NULL);

			return projectileTrackerFactory->createNewProjectileTrackerInstance();
		}

		static void setProjectileTrackerFactory(IProjectileTrackerFactory *factory)
		{
			projectileTrackerFactory = factory;
		}

	private:
		static IProjectileTrackerFactory *projectileTrackerFactory;
		static ProjectileTrackerObjectType instance;

		friend class Projectile;
	};
}

#endif


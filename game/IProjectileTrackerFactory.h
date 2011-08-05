
#ifndef IPROJECTILETRACKERFACTORY_H
#define IPROJECTILETRACKERFACTORY_H

//#include "Projectile.h"

namespace game
{
	class Projectile;

	class IProjectileTrackerFactory
	{
	public:
		virtual Projectile *createNewProjectileTrackerInstance() = 0;
		virtual void projectileDeleted(Projectile *projectile) = 0;
		virtual ~IProjectileTrackerFactory() {};
	};
}

#endif


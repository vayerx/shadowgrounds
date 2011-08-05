
#include "precompiled.h"

#include "ProjectileTrackerObjectType.h"

#include "IProjectileTrackerFactory.h"
#include "tracking/trackable_typeid.h"

#include "tracking/ITrackerObject.h"
#include "tracking/ITrackerObjectType.h"
#include "tracking/ITrackableObject.h"

namespace game
{

	ProjectileTrackerObjectType ProjectileTrackerObjectType::instance = ProjectileTrackerObjectType();
	IProjectileTrackerFactory *ProjectileTrackerObjectType::projectileTrackerFactory = NULL;

}


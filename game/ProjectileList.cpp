
#include "precompiled.h"

#include "ProjectileList.h"

#include "Projectile.h"
#include "gamedefs.h"
#include "../container/LinkedList.h"
#include "../util/Debug_MemoryManager.h"

namespace game
{

	ProjectileList::ProjectileList()
	{
		allProjectiles = new LinkedList();
	}

	// NOTE, does not delete the projectiles inside this but just the list of them
	ProjectileList::~ProjectileList()
	{
		while (!allProjectiles->isEmpty())
		{
			allProjectiles->popLast();
		}
		delete allProjectiles;
	}

	SaveData *ProjectileList::getSaveData() const
	{
		// TODO
		return NULL;
	}

	const char *ProjectileList::getStatusInfo() const
	{
		return "ProjectileList";
	}

	int ProjectileList::getAllProjectileAmount()
	{
		// TODO, optimize, not thread safe! (caller may not be using iterator)
		int count = 0;
		allProjectiles->resetIterate();
		while (allProjectiles->iterateAvailable())
		{
			allProjectiles->iterateNext();
			count++;
		}
		return count;
	}

	LinkedList *ProjectileList::getAllProjectiles()
	{
		return allProjectiles;
	}

	void ProjectileList::addProjectile(Projectile *projectile)
	{
		allProjectiles->append(projectile);
	}

	// does not delete the projectile, just removes it from the list
	void ProjectileList::removeProjectile(Projectile *projectile)
	{
		allProjectiles->remove(projectile);
	}

	Projectile *ProjectileList::getProjectileByHandle(int handle)
	{
		Projectile *ret = NULL;

		allProjectiles->resetIterate();
		while (allProjectiles->iterateAvailable())
		{
			Projectile *p = (Projectile *)allProjectiles->iterateNext();
			if (p->getHandle() == handle)
				ret = p;
		}
		return ret;
	}

}


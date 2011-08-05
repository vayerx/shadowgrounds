
#ifndef PROJECTILELIST_H
#define PROJECTILELIST_H

#include "GameObject.h"

class LinkedList;

namespace game
{
	class Projectile;

	/**
	 * A class holding game projectiles.
	 * Projectiles are usually bullets of some kind.
	 * 
	 * @version 1.0, 25.6.2002
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see Projectile
	 * @see Game
	 */

	class ProjectileList : public GameObject
	{
	public:
		ProjectileList();
		~ProjectileList();

		virtual SaveData *getSaveData() const;

		virtual const char *getStatusInfo() const;

		int getAllProjectileAmount();

		LinkedList *getAllProjectiles();

		void addProjectile(Projectile *projectile);
		void removeProjectile(Projectile *projectile);

		Projectile *getProjectileByHandle(int handle);

	private:
		LinkedList *allProjectiles;
	};

}

#endif


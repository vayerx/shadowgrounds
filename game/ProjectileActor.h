
#ifndef PROJECTILEACTOR_H
#define PROJECTILEACTOR_H

#include "../container/LinkedList.h"

struct TerrainObstacle;
struct ExplosionEvent;

namespace game
{
  class Game;
  class Unit;
  class Part;
  class Projectile;
	class Bullet;

/**
 *
 * A class that moves projectiles. 
 *
 * @version 0.5, 25.6.2002
 * @author Jukka Kokkonen <jukka@frozenbyte.com>
 * @see Projectile
 *
 */

class ProjectileActor
{
public:

  ProjectileActor(Game *game);

  /** 
   * Move the projectile and check for collisions with units and stuff.
   * @param projectile  Projectile to move.
   */
  void act(Projectile *projectile);

  void createVisualForProjectile(Projectile *projectile, bool originToSpecialUnit = false, Unit *specialUnit = NULL);

  Projectile *createChainedProjectile(Projectile *projectile, const VC3 &position, 
    int hitchain, const VC3 &direction, Unit *indirectHitUnit = 0);

	// targetUnit can't be const
	void doProjectileRaytrace(Unit *shooter, Unit *noCollisionUnit, Projectile *projectile,
		Bullet *bulletType, const VC3 &weaponPosition, const VC3 &weaponRayPosition, 
		const VC3 &targetPosition, const VC3 &direction, float maxRange, Unit *targetUnit, float velocityFactor = 1.0f);

	static void handleTerrainBreaking(Game *game, std::vector<TerrainObstacle> &removedObjects, std::vector<ExplosionEvent> &events);

private:
  Game *game;

  // hitUnit can't be const
  void doUnitHit(Projectile *projectile, Unit *hitUnit, Part *hitPart, 
    VC3 &pushVector, float damageFactor, bool directHit);

  void doHitMisses(Projectile *projectile, const Unit *hitUnit);

  bool doGore(Projectile *projectile, Unit *hitUnit, bool onlyPartial, float probabilityFactor, int additionalProbability);
	
  void getCollisionDisableList(Unit *shooter, LinkedList &noCollUnits, const VC3 &weaponPosition);

};

}

#endif

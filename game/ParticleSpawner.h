
#ifndef PARTICLESPAWNER_H
#define PARTICLESPAWNER_H

//namespace ui
//{
//  class ParticleSystem;
//  class ParticleManager;
//}

#include <DatatypeDef.h>

namespace game
{
  class Projectile;
  class Weapon;
  class Bullet;
  class Game;

  class ParticleSpawnerManager;

  // Actually, this is a projectile (bullet) spawner...

  class ParticleSpawner
  {
    public:
      ParticleSpawner(Game *game);

      ~ParticleSpawner();

      void setName(const char *name);

      void setPosition(const VC3 &position);
      
      void setDirection(const VC3 &direction);

      void setSpawnerWeapon(const Weapon *weapon);

      void disable();
     
      void enable();

      void run();

			const char *getName() const;

			static Projectile *spawnProjectileWithWeapon(Game *game, const Weapon *weapon,
				const VC3 &position, const VC3 &direction, float distanceToListener = 0.0f);

    private:
      Game *game;

      char *name;

      VC3 position;
      VC3 direction;

      const Weapon *weapon;

      bool enabled;

			int waitTime;

			// this should be set by manager, to indicate that the particle
			// spawner is outside screen (thus, disabled)
			bool outsideScreen;
			// this should be set by manager to tell the distance _squared_ to listener position
			float distanceToListenerSq;

			int projectileHandle;
   
      friend class ParticleSpawnerManager;
  };
}

#endif



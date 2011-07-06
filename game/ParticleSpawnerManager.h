
#ifndef PARTICLESPAWNERMANAGER_H
#define PARTICLESPAWNERMANAGER_H

#include "../game/unified_handle_type.h"
#include "../editor/UniqueEditorObjectHandle.h"

#include <DatatypeDef.h>

class LinkedList;

namespace game
{
  class Game;

	// bad dependency
	class UnifiedHandleManager;

  class ParticleSpawner;

  class ParticleSpawnerManager
  {
    public:
      ParticleSpawnerManager(Game *game);

      ~ParticleSpawnerManager();

      ParticleSpawner *createParticleSpawner();

      void deleteParticleSpawner(ParticleSpawner *spawner);

      ParticleSpawner *getParticleSpawnerByName(const char *name);

      void deleteAllParticleSpawners();
      void disableAllParticleSpawners();

			int getParticleSpawnerAmount();

      void run();

			void attachParticleSpawner(UnifiedHandle particleSpawner, UnifiedHandle toObject);

			UnifiedHandle findUnifiedHandleByUniqueEditorObjectHandle(UniqueEditorObjectHandle ueoh) const;

			UnifiedHandle getUnifiedHandle(ParticleSpawner *spawner) const;
			ParticleSpawner *getParticleSpawnerByUnifiedHandle(UnifiedHandle handle) const;
			bool doesParticleSpawnerExist(UnifiedHandle handle) const;
			UnifiedHandle getFirstParticleSpawner() const;
			UnifiedHandle getNextParticleSpawner(UnifiedHandle handle) const;

			VC3 getParticleSpawnerPosition(UnifiedHandle unifiedHandle) const;
			void setParticleSpawnerPosition(UnifiedHandle unifiedHandle, const VC3 &position);

			UnifiedHandle findClosestLight(const VC3 &position);
			UnifiedHandle findClosestDetachedLight(const VC3 &position);
			void attachLight(UnifiedHandle light, UnifiedHandle toObject);

			void setPlayerPosition(const VC3 &playerPosition);

    private:
      Game *game;

      LinkedList *spawnerList;

			VC3 playerPosition;

  };
}

#endif



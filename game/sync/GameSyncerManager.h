
#ifndef GAMESYNCERMANAGER_H
#define GAMESYNCERMANAGER_H

#include "IGameSyncer.h"

namespace game
{
namespace sync
{
	class GameSyncerManagerImpl;

	/**
	 * A class for overall management of different game syncers.
	 * Intended to be used as a singleton.
	 *
	 * This bound to the actual game, not usable in editor. 
	 * Editor needs to use the EditorGameSyncerManager instead.
	 *
	 * @author <jukka.kokkonen@postiloota.net>
	 */
	class GameSyncerManager
	{
	public:
		GameSyncerManager(game::Game *game);

		void run();

	private:
		GameSyncerManagerImpl *impl;
	};

}
}

#endif


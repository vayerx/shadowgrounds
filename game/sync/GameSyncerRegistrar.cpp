
#ifndef GAMESYNCERREGISTRAR_H
#define GAMESYNCERREGISTRAR_H

#include "IGameSyncer.h"

namespace game
{
namespace sync
{
	class GameSyncerRegistrarImpl;

	/**
	 * A class for holding different game syncers.
	 * Intended to be used as a singleton.
	 *
	 * @author <jukka.kokkonen@postiloota.net>
	 */
	class GameSyncerRegistrar
	{
	public:
		static GameSyncerRegistrar *getInstance();

		static void cleanInstance();

		void registerGameSyncer(IGameSyncer *syncer, const char *name);

		IGameSyncer *getSyncer(const char *name);

	private:
		GameSyncerRegistrarImpl *impl;
	};

}
}

#endif


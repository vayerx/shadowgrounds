
#ifndef PERMANENTGAMESYNCER_H
#define PERMANENTGAMESYNCER_H

namespace game
{
namespace sync
{
	/**
	 * This class synchronizes the game permanently, in other words, it saves any changes to a file that
	 * will be applied during next same mission startup. Also, that file will be read in by the editor
	 * to sync changes made in the game to the editor file.
	 *
	 * @author Jukka Kokkonen <jukka.kokkonen@postiloota.net>
	 * @version 1.0, 25.8.2007
	 */
	class PermanentGameSyncer
	{
	public:
		PermanentGameSyncer(const char *filename);

		virtual ~PermanentGameSyncer();

		virtual void run();

		// on server, broadcast a sync command to clients
		// on client, ask server to broadcast a sync command
		virtual void sendSyncCommand(const IGameSyncCommand &command);

		virtual bool isReceivedSyncCommandAvailable();

		// may return a temporary reference, will be invalidated at next isReceived.../receive call .
		virtual const IGameSyncCommand &receiveSyncCommand();

	private:
		PermanentGameSyncerImpl *impl;
	};

}
}

#endif


#ifndef IGAMESYNCER_H
#define IGAMESYNCER_H

namespace game
{
namespace sync
{

	class IGameSyncer
	{
	public:
		virtual ~IGameSyncer() { }

		virtual void run() = 0;

		// on server, broadcast a sync command to clients
		// on client, ask server to broadcast a sync command
		virtual void sendSyncCommand(const IGameSyncCommand &command) = 0;

		virtual bool isReceivedSyncCommandAvailable() = 0;

		// may return a temporary reference, will be invalidated at next isReceived.../receive call .
		virtual const IGameSyncCommand &receiveSyncCommand() = 0;
	};

}
}

#endif

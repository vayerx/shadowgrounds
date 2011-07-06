
#ifndef GAME_SYNCER_TOOLS_H
#define GAME_SYNCER_TOOLS_H

namespace game
{
namespace sync
{

// some tools for handling sync commands properly in the game syncer classes

#define SYNC_COMMAND_FLAG_CACHED_CLASS_ID (1<<0)
#define SYNC_COMMAND_FLAG_CACHED_DATA_ID (1<<1)
//#define _RESERVED_SYNC_COMMAND_FLAG_NO_CLASS_ID (1<<2)  - this would be very silly?
#define SYNC_COMMAND_FLAG_NO_DATA_ID (1<<3)
#define SYNC_COMMAND_FLAG_CACHED_STRING_DATA (1<<4)
#define SYNC_COMMAND_FLAG_DATA_ID_32BIT (1<<5)

typedef unsigned char sync_packet_flags;

class SyncCommandManager
{
public:

	SyncCommandManager *getInstance();

	void cleanInstance();

	void addSyncCommandFactor(ISyncCommandFactory *syncCommandFactory);
	
	//void removeSyncCommandFactor(ISyncCommandFactory *syncCommandFactory);

	// returns a pointer to temporary buffer that will be overridden by next call
	const unsigned char *convert_sync_command_to_stream_bytes(ISyncCommand *command);

	// returns a temporary object that may be overridden by next call
	ISyncCommand &convert_stream_bytes_to_sync_command(const unsigned char *databytes);

private:
	SyncCommandManagerImpl *impl;
};

}
}

#endif


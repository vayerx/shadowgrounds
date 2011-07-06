
#include "precompiled.h"

#include "PermanentGameSyncer.h"
#include "../net/NetDriverManager.h"
#include "commands/InvalidCommand.h"

namespace game
{
namespace sync
{

class PermanentGameSyncerImpl
{
public:

	PermanentGameSyncerImpl(const char *filename)
	{
	}

	~PermanentGameSyncerImpl()
	{
	}


};

PermanentGameSyncer::PermanentGameSyncer(const char *filename)
{
	this->impl = new PermanentGameSyncerImpl(filename);
}

PermanentGameSyncer::~PermanentGameSyncer()
{
	delete impl;
}

void PermanentGameSyncer::run()
{
}

void PermanentGameSyncer::sendSyncCommand(const ISyncCommand &command)
{
}

bool PermanentGameSyncer::isReceivedSyncCommandAvailable()
{
	return true;
}

const ISyncCommand &PermanentGameSyncer::receiveSyncCommand()
{
	assert(isReceivedSyncCommandAvailable());

	return InvalidCommandFactory::invalidCommandFactory.getCommand(0,0,NULL);
}

}
}

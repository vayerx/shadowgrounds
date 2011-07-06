
#ifndef INVALIDCOMMAND_H
#define INVALIDCOMMAND_H

#include "../ISyncCommand.h"
#include "../ISyncCommandFactory.h"

namespace game
{
namespace sync
{

class InvalidCommand : public ISyncCommand
{
public:
	InvalidCommand() { }

	virtual sync_command_class_id getSyncCommandClassId() const { return SYNC_COMMAND_IDSTRING_TO_ID("INVA"); }

	virtual sync_command_data_id getDataId() { return 0; }

	virtual int getDataSize() { return 0; }
	virtual const unsigned char *getDataBuffer() { return 0; /* NULL */ }

	virtual sync_command_allow_flags getAllowFlags() const { return SYNC_COMMAND_FLAG_ALLOW_NONE; }
};


class InvalidCommandFactory : public ISyncCommandFactory
{
public:
	InvalidCommandFactory() { }

	virtual ISyncCommand &getCommand(sync_command_data_id dataId, int dataSize, const unsigned char *data) { return representation; }

	static InvalidCommandFactory invalidCommandFactory;

private:
	InvalidCommand representation;
};

}
}

#endif


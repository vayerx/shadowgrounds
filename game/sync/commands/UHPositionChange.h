
#ifndef UHPOSITIONCHANGE_H
#define UHPOSITIONCHANGE_H

#include "ISyncCommand.h"
#include "unified_handle_type.h"
#include <DatatypeDef.h>

namespace game
{
namespace sync
{

class UHPositionChange : public ISyncCommand
{
public:
	UHPositionChange(game::UnifiedHandleManager *uhman);

	void init(int dataId, int dataSize, const unsigned char *data);

	virtual sync_command_class_id getSyncCommandClassId() const;

	virtual sync_command_data_id getDataId();

	virtual int getDataSize();
	virtual const unsigned char *getDataBuffer();

	virtual sync_command_allow_flags getAllowFlags() const;	

private:
	UnifiedHandle unifiedHandle;
	struct
	{
		VC3 position;
	} data;
};


class UHPositionChangeFactory : public ISyncCommandFactory
{
public:
	UHPositionChangeFactory(game::UnifiedHandleManager *uhman);

	virtual ISyncCommand &getCommand(int dataId, int dataSize, const unsigned char *data);

private:
	game::UnifiedHandleManager *uhman;
};

}
}

#endif


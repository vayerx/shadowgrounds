
#ifndef UHPOSITIONXZCHANGE_H
#define UHPOSITIONXZCHANGE_H

#include "ISyncCommand.h"
#include "unified_handle_type.h"
#include <DatatypeDef.h>

namespace game
{
namespace sync
{

class UHPositionXZChange : public ISyncCommand
{
public:
	UHPositionXZChange(game::UnifiedHandleManager *uhman);

	void init(sync_command_data_id dataId, int dataSize, const unsigned char *data);

	virtual sync_command_class_id getSyncCommandClassId() const;

	virtual sync_command_data_id getDataId();

	virtual int getDataSize();
	virtual const unsigned char *getDataBuffer();

	virtual sync_command_allow_flags getAllowFlags() const;	

private:
	UnifiedHandle unifiedHandle;
	struct
	{
		VC2 positionXZ;
	} data;
};


class UHPositionXZChangeFactory : public ISyncCommandFactory
{
public:
	UHPositionXZChangeFactory(game::UnifiedHandleManager *uhman);

	virtual ISyncCommand &getCommand(int dataId, int dataSize, const unsigned char *data);

private:
	game::UnifiedHandleManager *uhman;
};

}
}

#endif


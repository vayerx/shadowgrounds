
#include "precompiled.h"

#include "UHPositionChange.h"

namespace game
{
namespace sync
{

sync_command_class_id UHPositionChange::getSyncCommandClassId() const
{
	return SYNC_COMMAND_IDSTRING_TO_ID("UHPC");
}

sync_command_data_id UHPositionChange::getDataId()
{
	return (sync_command_data_id)unifiedHandle;
}

int UHPositionChange::getDataSize()
{
	return sizeof(data);
}

const unsigned char *UHPositionChange::getDataBuffer()
{
	return (const unsigned char *)data;
}

sync_command_allow_flags UHPositionChange::getAllowFlags() const
{
}

}
}


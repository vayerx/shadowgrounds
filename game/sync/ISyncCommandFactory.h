
#ifndef ISYNCCOMMANDFACTORY_H
#define ISYNCCOMMANDFACTORY_H

#include "ISyncCommand.h"

namespace game
{
namespace sync
{

	class ISyncCommandFactory
	{
	public:
		virtual ISyncCommand &getCommand(sync_command_data_id dataId, int dataSize, const unsigned char *data) = 0;
	};

}
}

#endif


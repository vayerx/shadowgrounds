
#ifndef IGAMESYNCCOMMAND_H
#define IGAMESYNCCOMMAND_H

namespace game
{
namespace sync
{
	class ISyncCommand
	{


		virtual int getDataId();
		virtual int getDataSize();
		virtual const unsigned char *getDataBuffer();
	};
}
}

#endif


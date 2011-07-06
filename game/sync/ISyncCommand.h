
#ifndef ISYNCCOMMAND_H
#define ISYNCCOMMAND_H

#include "sync_command_macros.h"

#define SYNC_COMMAND_FLAG_ALLOW_NONE 0

#define SYNC_COMMAND_FLAG_ALLOW_CACHED_CLASS_ID (1<<0)
#define SYNC_COMMAND_FLAG_ALLOW_CACHED_DATA_ID (1<<1)
#define SYNC_COMMAND_FLAG_ALLOW_CACHED_STRING_DATA (1<<2)

namespace game
{
namespace sync
{
	typedef unsigned char sync_command_allow_flags;
	typedef unsigned int sync_command_class_id;
	typedef unsigned __int64 sync_command_data_id;

	/**
	 * Interface for networked game synchronization commands.
	 *
	 * @author <jukka.kokkonen@postiloota.net>
	 */

	class ISyncCommand
	{
	public:
		/**
		 * This method should return the class id of the command. (which should effectively be a FOURCC value)
		 * Use the provided SYNC_COMMAND_IDSTRING_TO_ID macro to construct the ids.
		 *
		 * @return sync_command_class_id
		 */
		virtual sync_command_class_id getSyncCommandClassId() const = 0;

		/**
		 * This method should return the data id of the command. 
		 * The data id is most likely the "unique editor object handle" of the object in question
		 */
		virtual sync_command_data_id getDataId() = 0;

		virtual int getDataSize() = 0;
		virtual const unsigned char *getDataBuffer() = 0;

		/**
		 * This method should return the flags that define how this command is allowed to be handled.
		 * (for example, can this command's class id be cached and so on)
		 */
		virtual sync_command_allow_flags getAllowFlags() const = 0;
	};


	// (access to this should be done using the SYNC_COMMAND_UNIQUE_DATA_ID macro)
	//sync_command_data_id unique_data_id_iterator;
}
}

#endif


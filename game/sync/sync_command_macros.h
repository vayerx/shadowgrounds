
#ifndef SYNC_COMMAND_MACROS_H
#define SYNC_COMMAND_MACROS_H

#include <assert.h>
// use like: SYNC_COMMAND_IDSTRING_TO_ID("FOOB")
#define SYNC_COMMAND_IDSTRING_TO_ID(str_4char) \
	( \
		assert(strlen(str_4char) == 4), \
		( \
			(game::sync::sync_command_class_id)str_4char[0] \
			| ((game::sync::sync_command_class_id)str_4char[1]<<8) \
			| ((game::sync::sync_command_class_id)str_4char[2]<<16) \
			| ((game::sync::sync_command_class_id)str_4char[3]<<24) \
		) \
	)

#define SYNC_COMMAND_UNIQUE_DATA_ID() (unique_data_id_iterator++)


#endif

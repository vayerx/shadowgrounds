

#ifndef SCRIPTING_MACROS_START_H
#error scripting_macros_end header included without including start.
#endif

#define GS_MAX_COMMANDS 99999999

#ifdef GS_EXPAND_GAMESCRIPTING_LIST

	{ GS_MAX_COMMANDS, "***", GS_DATATYPE_ARR_END } };	

#endif


#undef SCRIPTING_MACROS_START_H

#undef GS_CMD_SIMPLE
#undef GS_CMD

#undef STRING
#undef INT
#undef NONE
#undef EXPR
#undef FLOAT

#undef VECTOR_XYZ
#undef COLOR_RGB
#undef PATHNAME
#undef FILENAME




#ifdef SCRIPTING_MACROS_START_H
#error scripting_macros_start header included multiple times without including end.
#endif

#define SCRIPTING_MACROS_START_H

#ifdef NONE
#error Already a macro named NONE defined, cannot continue..
#endif
#ifdef STRING
#error Already a macro named STRING defined, cannot continue..
#endif
#ifdef INT
#error Already a macro named INT defined, cannot continue..
#endif
#ifdef FLOAT
#error Already a macro named FLOAT defined, cannot continue..
#endif
#ifdef EXPR
#error Already a macro named EXPR defined, cannot continue..
#endif

// --- aliases macro checks (for string) ---
#ifdef VECTOR_XYZ
#error Already a macro named VECTOR_XYZ defined, cannot continue..
#endif
#ifdef COLOR_RGB
#error Already a macro named COLOR_RGB defined, cannot continue..
#endif
#ifdef PATHNAME
#error Already a macro named PATHNAME defined, cannot continue..
#endif
#ifdef FILENAME
#error Already a macro named FILENAME defined, cannot continue..
#endif
// ---

#define STRING SCRIPT_DATATYPE_STRING
#define INT SCRIPT_DATATYPE_INT
#define NONE SCRIPT_DATATYPE_NONE
#define EXPR SCRIPT_DATATYPE_EXPRESSION
#define FLOAT SCRIPT_DATATYPE_FLOAT

// --- aliases (for string) ---
#define VECTOR_XYZ SCRIPT_DATATYPE_STRING
#define COLOR_RGB SCRIPT_DATATYPE_STRING
#define LEGACY_FLOAT_STRING SCRIPT_DATATYPE_STRING
#define PATHNAME SCRIPT_DATATYPE_STRING
#define FILENAME SCRIPT_DATATYPE_STRING
// ---

#ifdef GS_EXPAND_GAMESCRIPTING_LIST

	gs_commands_listtype gs_commands[GS_CMD_AMOUNT + 1] = {

#endif


#ifdef GS_EXPAND_GAMESCRIPTING_LIST

#define GS_CMD(NUM, ID, NAME, DATATYPE) { (GS_CMD_BASE + NUM), NAME, DATATYPE },

#elif defined(GS_EXPAND_GAMESCRIPTING_CASE)

#define GS_CMD(NUM, ID, NAME, DATATYPE) case ID:

#else

#define GS_CMD(NUM, ID, NAME, DATATYPE) static const int ID = (GS_CMD_BASE + NUM);

#endif

#define GS_CMD_SIMPLE(NUM, NAMEID, DATATYPE) GS_CMD(NUM, GS_CMD_##NAMEID, #NAMEID, DATATYPE)


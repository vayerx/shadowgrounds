
#ifdef GS_EXPAND_GAMESCRIPTING_LIST
#undef SCRIPTCOMMANDS_H
#endif
#ifdef GS_EXPAND_GAMESCRIPTING_CASE
#error This should not happen
#endif

#ifndef SCRIPTCOMMANDS_H
#define SCRIPTCOMMANDS_H

// NOTE: Defines moved to appropriate scripting class headers

#include "scripting_macros_start.h"

#include "camera_script_commands.h"
#include "sound_script_commands.h"
#include "cinematic_script_commands.h"
#include "decor_script_commands.h"
#include "water_script_commands.h"
#include "math_script_commands.h"
#include "mission_script_commands.h"
#include "position_script_commands.h"
#include "option_script_commands.h"
#include "dev_script_commands.h"
#include "string_script_commands.h"
#include "misc_script_commands.h"
#include "unit_script_commands.h"
#include "light_script_commands.h"
#include "animation_script_commands.h"
#include "environment_script_commands.h"
#include "hitchain_script_commands.h"
#include "map_script_commands.h"
#include "item_script_commands.h"
#include "tracking_script_commands.h"
#include "sync_script_commands.h"
#include "directcontrol_script_commands.h"

#define GS_CMD_BASE 0
GS_CMD(0, GS_CMD_INVALIDGSCOMMAND, "invalidGSCommand", NONE)
#undef GS_CMD_BASE

#include "scripting_macros_end.h"

#endif


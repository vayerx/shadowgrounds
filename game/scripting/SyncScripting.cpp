
// Copyright(C) Jukka Kokkonen, 2007

#include "precompiled.h"

#include "SyncScripting.h"

#include "scripting_macros_start.h"
#include "sync_script_commands.h"
#include "scripting_macros_end.h"

#include <DatatypeDef.h>

#include "GameScriptData.h"
#include "GameScriptingUtils.h"
#include "../Game.h"
#include "../GameUI.h"
#include "../UnitSelections.h"
#include "../../util/assert.h"
#include "GameScripting.h"

#include "../../convert/str2int.h"
#include "../../util/ScriptProcess.h"
#include "../../util/ScriptManager.h"
#include "../../system/Logger.h"

#include "../../util/Debug_MemoryManager.h"

namespace game
{
	void SyncScripting::process(util::ScriptProcess *sp, 
		int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue,
		GameScriptData *gsd, Game *game, bool *pause)
	{
		switch(command)
		{
		case GS_CMD_editSendMoveUnifiedHandleObjectToPosition:
			if (VALIDATE_UNIFIED_HANDLE_BITS(gsd->unifiedHandle))
			{

			} else {
				sp->error("SyncScripting::process - editSendMoveUnifiedHandleObjectToPosition, unified handle not valid.");
			}
			break;

		case GS_CMD_editSendDeleteUnifiedHandleObject:
			break;
		case GS_CMD_editSendDuplicateUnifiedHandleObject:
			break;
		case GS_CMD_editSendRotateUnifiedHandleObjectX:
			break;
		case GS_CMD_editSendRotateUnifiedHandleObjectY:
			break;
		case GS_CMD_editSendRotateUnifiedHandleObjectZ:
			break;

		case GS_CMD_editRecvMoveUnifiedHandleObjectToPosition:
			break;
		case GS_CMD_editRecvDeleteUnifiedHandleObject:
			break;
		case GS_CMD_editRecvDuplicateUnifiedHandleObject:
			break;
		case GS_CMD_editRecvRotateUnifiedHandleObjectX:
			break;
		case GS_CMD_editRecvRotateUnifiedHandleObjectY:
			break;
		case GS_CMD_editRecvRotateUnifiedHandleObjectZ:
			break;

		default:
			sp->error("SyncScripting::process - Unknown command.");
			assert(0);
		}
	}
}



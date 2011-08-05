
#include "precompiled.h"

#include "SelectionVisualizer.h"
#include "DebugVisualizerTextUtil.h"
#include "DebugUnitVisualizer.h"
#include "../game/unified_handle.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/UnitList.h"

#include <vector>
#include <Storm3D_UI.h>

using namespace game;

namespace ui
{
	static std::vector<UnifiedHandle> selectionUnifiedHandles;

	void SelectionVisualizer::clearSelectionForUnifiedHandle(UnifiedHandle uh)
	{
		if (!VALIDATE_UNIFIED_HANDLE_BITS(uh))
		{
			LOG_WARNING_W_DEBUG("SelectionVisualizer::clearSelectionForUnifiedHandle - Given unified handle is not valid.", int2str(uh));
			return;
		}

		for (int i = 0; i < (int)selectionUnifiedHandles.size(); i++)
		{
			if (selectionUnifiedHandles[i] == uh)
			{
				selectionUnifiedHandles.erase(selectionUnifiedHandles.begin() + i);
				return;
			}
		}

		LOG_WARNING_W_DEBUG("SelectionVisualizer::clearSelectionForUnifiedHandle - Given unified handle not found in list.", int2str(uh));
	}



	void SelectionVisualizer::setSelectionForUnifiedHandle(UnifiedHandle uh)
	{
		if (!VALIDATE_UNIFIED_HANDLE_BITS(uh))
		{
			LOG_WARNING_W_DEBUG("SelectionVisualizer::setSelectionForUnifiedHandle - Given unified handle is not valid.", int2str(uh));
			return;
		}

		for (int i = 0; i < (int)selectionUnifiedHandles.size(); i++)
		{
			if (selectionUnifiedHandles[i] == uh)
			{
				LOG_WARNING_W_DEBUG("SelectionVisualizer::setSelectionForUnifiedHandle - Given unified handle already found in list.", int2str(uh));
				return;
			}
		}

		selectionUnifiedHandles.push_back(uh);
	}



	void SelectionVisualizer::visualizeSelections(game::Game *game)
	{
		VC3 campos = game->gameUI->getGameCamera()->getActualInterpolatedPosition();

		for (int i = 0; i < (int)selectionUnifiedHandles.size(); i++)
		{
			if (IS_UNIFIED_HANDLE_TERRAIN_OBJECT(selectionUnifiedHandles[i]))
			{
				// TODO: ...
				//DebugUnitVisFlags flags = DEBUGTERRAINOBJECTVISUALIZER_FLAG_SELECTED;
				//DebugTerrainObjectVisualizer::visualizeTerrainObject(selectionUnifiedHandles[i], campos, flags);
			}
			else if (IS_UNIFIED_HANDLE_UNIT(selectionUnifiedHandles[i]))
			{
				Unit *u = game->units->getUnitById(game->units->unifiedHandleToUnitId(selectionUnifiedHandles[i]));
				DebugUnitVisFlags flags = DEBUGUNITVISUALIZER_FLAG_SELECTED;
				DebugUnitVisualizer::visualizeUnit(u, campos, flags);
			}
		}
	}

}

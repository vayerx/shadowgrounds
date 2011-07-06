
#include "precompiled.h"

#include "DebugTerrainObjectVisualizer.h"
#include "DebugVisualizerTextUtil.h"
#include "../ui/Terrain.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/UnifiedHandleManager.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"
#include "../ui/uidefaults.h"
#include "../ogui/OguiStormDriver.h"
#include "../system/Timer.h"

#include <vector>
#include <Storm3D_UI.h>

#define DEBUGTERRAINOBJECTVISUALIZER_MAX_DIST 30.0f

#define DEBUGTERRAINOBJECTVISUALIZER_EXTENDED_MAX_DIST 10.0f

using namespace game;

extern IStorm3D_Scene *disposable_scene;

namespace ui
{

	void DebugTerrainObjectVisualizer::visualizeTerrainObject(game::Game *game, UnifiedHandle terrainObject, const VC3 &cameraPosition, DebugTerrainObjectVisFlags visFlags)
	{
		char textbuf[128];

		assert(VALIDATE_UNIFIED_HANDLE_BITS(terrainObject));

		if (!game->unifiedHandleManager->doesObjectExist(terrainObject))
		{
			return;
		}

		VC3 cullpos = game->unifiedHandleManager->getObjectPosition(terrainObject);
		VC3 distToCamVec = cullpos - cameraPosition;
		// 2D RANGE
		distToCamVec.y = 0;
		float distToCamSq = distToCamVec.GetSquareLength();
		if (distToCamSq < (DEBUGTERRAINOBJECTVISUALIZER_MAX_DIST * DEBUGTERRAINOBJECTVISUALIZER_MAX_DIST))
		{
			// ok
		} else {
			return;
		}

		std::string fname = game->gameUI->getTerrain()->getTypeFilenameByUnifiedHandle(terrainObject);
		std::string fnamecut;
		for (int i = (int)fname.length() - 1; i >= 0; i--)
		{
			if (fname[i] == '\\')
			{
				fname[i] = '/';
			}
			if (fname[i] == '/')
			{
				if (fnamecut.empty())
				{
					fnamecut = fname.substr(i + 1, fname.length() - (i + 1));
				}
			}
		}

		const char *totypename = game::SimpleOptions::getString(DH_OPT_S_DEBUG_VISUALIZE_TERRAINOBJECTS_OF_NAME);
		if (totypename != NULL
			&& totypename[0] != '\0')
		{
			if (strstr(fname.c_str(), totypename) != NULL)
			{
				// ok
			} else {
				return;
			}
		}

		COL col = COL(0.5f,0.5f,1.0f);

		if (visFlags & DEBUGTERRAINOBJECTVISUALIZER_FLAG_SELECTED)
		{
			// ok with that color?
		} else {
			col.r = col.r / 2.0f;
			col.g = col.g / 2.0f;
			col.b = col.b / 2.0f;
		}

		/*
		VC3 pos = game->unifiedHandleManager->getObjectPosition(terrainObject);
		pos.y += 0.03f;

		VC3 sizes = VC3(1.00f, 1.00f, 1.00f);

		VC3 c1 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c2 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c3 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 c4 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 cb1 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb2 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb3 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z + sizes.z);
		VC3 cb4 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z + sizes.z);
		*/

		OOBB oobb = game->gameUI->getTerrain()->getOOBB(terrainObject);

		VC3 pos = oobb.center;

		VC3 c1 = pos - (oobb.axes[0] * oobb.extents.x) + (oobb.axes[1] * oobb.extents.y) - (oobb.axes[2] * oobb.extents.z);
		VC3 c2 = pos + (oobb.axes[0] * oobb.extents.x) + (oobb.axes[1] * oobb.extents.y) - (oobb.axes[2] * oobb.extents.z);
		VC3 c3 = pos + (oobb.axes[0] * oobb.extents.x) + (oobb.axes[1] * oobb.extents.y) + (oobb.axes[2] * oobb.extents.z);
		VC3 c4 = pos - (oobb.axes[0] * oobb.extents.x) + (oobb.axes[1] * oobb.extents.y) + (oobb.axes[2] * oobb.extents.z);
		VC3 cb1 = pos - (oobb.axes[0] * oobb.extents.x) - (oobb.axes[1] * oobb.extents.y) - (oobb.axes[2] * oobb.extents.z);
		VC3 cb2 = pos + (oobb.axes[0] * oobb.extents.x) - (oobb.axes[1] * oobb.extents.y) - (oobb.axes[2] * oobb.extents.z);
		VC3 cb3 = pos + (oobb.axes[0] * oobb.extents.x) - (oobb.axes[1] * oobb.extents.y) + (oobb.axes[2] * oobb.extents.z);
		VC3 cb4 = pos - (oobb.axes[0] * oobb.extents.x) - (oobb.axes[1] * oobb.extents.y) + (oobb.axes[2] * oobb.extents.z);

		/*
		VC3 pos = game->unifiedHandleManager->getObjectPosition(terrainObject);
		OOBB oobb = game->gameUI->getTerrain()->getOOBB(terrainObject);

		pos = oobb.center;

		VC3 sizes = oobb.extents;

		VC3 c1 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c2 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c3 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 c4 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 cb1 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb2 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb3 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z + sizes.z);
		VC3 cb4 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z + sizes.z);
		*/

		disposable_scene->AddLine(c1, c2, col);
		disposable_scene->AddLine(c2, c3, col);
		disposable_scene->AddLine(c3, c4, col);
		disposable_scene->AddLine(c4, c1, col);
		disposable_scene->AddLine(cb1, cb2, col);
		disposable_scene->AddLine(cb2, cb3, col);
		disposable_scene->AddLine(cb3, cb4, col);
		disposable_scene->AddLine(cb4, cb1, col);
		disposable_scene->AddLine(c1, cb1, col);
		disposable_scene->AddLine(c2, cb2, col);
		disposable_scene->AddLine(c3, cb3, col);
		disposable_scene->AddLine(c4, cb4, col);
		disposable_scene->AddLine(c4, cb4, col);

		int textoffy = 0;

		if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_TERRAINOBJECTS_EXTENDED)
			&& distToCamSq < (DEBUGTERRAINOBJECTVISUALIZER_EXTENDED_MAX_DIST * DEBUGTERRAINOBJECTVISUALIZER_EXTENDED_MAX_DIST))
		{
			sprintf(textbuf, "uh: %d", terrainObject);
			DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
			textoffy += 16;

			sprintf(textbuf, "filename: %s", fnamecut.c_str());
			DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
			textoffy += 16;
		}

	}

	void DebugTerrainObjectVisualizer::visualizeTerrainObjects(game::Game *game, const VC3 &cameraPosition)
	{
		assert(game != NULL);

		UnifiedHandle uh = game->gameUI->getTerrain()->getFirstTerrainObject();

		while (uh != UNIFIED_HANDLE_NONE)
		{
			visualizeTerrainObject(game, uh, cameraPosition, 0);

			uh = game->gameUI->getTerrain()->getNextTerrainObject(uh);
		}
	}

}

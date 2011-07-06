
#include "precompiled.h"

#include "DebugLightVisualizer.h"
#include "DebugVisualizerTextUtil.h"
#include "../ui/LightManager.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/UnifiedHandleManager.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"
#include "../ui/uidefaults.h"
#include "../ogui/StormDriver.h"
#include "../system/Timer.h"

#include <vector>
#include <Storm3D_UI.h>
#include "../util/Debug_MemoryManager.h"

#define DEBUGLIGHTVISUALIZER_MAX_DIST 20.0f

#define DEBUGLIGHTVISUALIZER_EXTENDED_MAX_DIST 10.0f

using namespace game;

extern IStorm3D_Scene *disposable_scene;

namespace ui
{

	void DebugLightVisualizer::visualizeLight(game::Game *game, UnifiedHandle light, const VC3 &cameraPosition, DebugLightVisFlags visFlags)
	{
//		char textbuf[128];

		assert(VALIDATE_UNIFIED_HANDLE_BITS(light));

		if (!game->unifiedHandleManager->doesObjectExist(light))
		{
			return;
		}

		if (visFlags & DEBUGLIGHTVISUALIZER_FLAG_SELECTED)
		{
			// no range culling
		} else {
			VC3 cullpos = game->unifiedHandleManager->getObjectPosition(light);
			VC3 distToCamVec = cullpos - cameraPosition;
			// 2D RANGE
			distToCamVec.y = 0;
			float distToCamSq = distToCamVec.GetSquareLength();
			if (distToCamSq < (DEBUGLIGHTVISUALIZER_MAX_DIST * DEBUGLIGHTVISUALIZER_MAX_DIST))
			{
				// ok
			} else {
				return;
			}
		}

		/*
		std::string fname = game->gameUI->getLightManager()->getTypeNameByUnifiedHandle(light);
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

		const char *totypename = game::SimpleOptions::getString(DH_OPT_S_DEBUG_VISUALIZE_LIGHTS_OF_NAME);
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
		*/

		COL col = COL(1.0f,1.0f,0.5f);

		if (IS_UNIFIED_HANDLE_LIGHT_SPOTLIGHT(light))
		{
			col = COL(1.0f,1.0f,1.0f);
		}
		else if (IS_UNIFIED_HANDLE_LIGHT_SHADOWCASTER(light))
		{
			col = COL(1.0f,0.0f,0.0f);
		}

		if (visFlags & DEBUGLIGHTVISUALIZER_FLAG_SELECTED)
		{
			// ok with that color?
		} else {
			col.r = col.r / 2.0f;
			col.g = col.g / 2.0f;
			col.b = col.b / 2.0f;
		}

		VC3 pos = game->unifiedHandleManager->getObjectPosition(light);
		pos.y += 0.01f;

		bool center = false;
		if (IS_UNIFIED_HANDLE_LIGHT_SPOTLIGHT(light))
		{
			if (game->gameUI->getLightManager()->getSpotSubId(light) == LIGHT_SUB_ID_CENTER)
			{
				center = true;
			}
		}
		else if (IS_UNIFIED_HANDLE_LIGHT_SHADOWCASTER(light))
		{
			if (game->gameUI->getLightManager()->getShadowcasterSubId(light) == LIGHT_SUB_ID_CENTER)
			{
				center = true;
			}
		} else {
			if (game->gameUI->getLightManager()->getLightSubId(light) == LIGHT_SUB_ID_CENTER)
			{
				center = true;
			}
		}

		VC3 sizes = VC3(0.30f, 0.30f, 0.30f);
		if (!center)
		{
			sizes *= 0.5f;
			if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
			{
				pos.y = game->gameMap->getScaledHeightAt(pos.x, pos.z) + 0.02f;
			}
		}

		VC3 c1 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c2 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c3 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 c4 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 cb1 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb2 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb3 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z + sizes.z);
		VC3 cb4 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z + sizes.z);

		VC3 foldpos = pos;

		disposable_scene->transformPositionForWorldFoldUsingPosition(c1, foldpos);
		disposable_scene->transformPositionForWorldFoldUsingPosition(c2, foldpos);
		disposable_scene->transformPositionForWorldFoldUsingPosition(c3, foldpos);
		disposable_scene->transformPositionForWorldFoldUsingPosition(c4, foldpos);
		disposable_scene->transformPositionForWorldFoldUsingPosition(cb1, foldpos);
		disposable_scene->transformPositionForWorldFoldUsingPosition(cb2, foldpos);
		disposable_scene->transformPositionForWorldFoldUsingPosition(cb3, foldpos);
		disposable_scene->transformPositionForWorldFoldUsingPosition(cb4, foldpos);

		if (center)
		{
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
		} else {
			if (IS_UNIFIED_HANDLE_LIGHT_SPOTLIGHT(light))
			{
        // nop.?
			}
			else if (IS_UNIFIED_HANDLE_LIGHT_SHADOWCASTER(light))
			{
				if (game->gameUI->getLightManager()->getShadowcasterSubId(light) == LIGHT_SUB_ID_MIN_Z)
				{
					disposable_scene->AddLine((c1 + c2) * 0.5f, c3, col);
					disposable_scene->AddLine((c1 + c2) * 0.5f, c4, col);
				}
				if (game->gameUI->getLightManager()->getShadowcasterSubId(light) == LIGHT_SUB_ID_MAX_Z)
				{
					disposable_scene->AddLine((c3 + c4) * 0.5f, c1, col);
					disposable_scene->AddLine((c3 + c4) * 0.5f, c2, col);
				}
				if (game->gameUI->getLightManager()->getShadowcasterSubId(light) == LIGHT_SUB_ID_MIN_X)
				{
					disposable_scene->AddLine((c1 + c4) * 0.5f, c2, col);
					disposable_scene->AddLine((c1 + c4) * 0.5f, c3, col);
				}
				if (game->gameUI->getLightManager()->getShadowcasterSubId(light) == LIGHT_SUB_ID_MAX_X)
				{
					disposable_scene->AddLine((c2 + c3) * 0.5f, c1, col);
					disposable_scene->AddLine((c2 + c3) * 0.5f, c4, col);
				}
			} else {
				if (game->gameUI->getLightManager()->getLightSubId(light) == LIGHT_SUB_ID_MIN_Z)
				{
					disposable_scene->AddLine((c1 + c2) * 0.5f, c3, col);
					disposable_scene->AddLine((c1 + c2) * 0.5f, c4, col);
				}
				if (game->gameUI->getLightManager()->getLightSubId(light) == LIGHT_SUB_ID_MAX_Z)
				{
					disposable_scene->AddLine((c3 + c4) * 0.5f, c1, col);
					disposable_scene->AddLine((c3 + c4) * 0.5f, c2, col);
				}
				if (game->gameUI->getLightManager()->getLightSubId(light) == LIGHT_SUB_ID_MIN_X)
				{
					disposable_scene->AddLine((c1 + c4) * 0.5f, c2, col);
					disposable_scene->AddLine((c1 + c4) * 0.5f, c3, col);
				}
				if (game->gameUI->getLightManager()->getLightSubId(light) == LIGHT_SUB_ID_MAX_X)
				{
					disposable_scene->AddLine((c2 + c3) * 0.5f, c1, col);
					disposable_scene->AddLine((c2 + c3) * 0.5f, c4, col);
				}
			}
		}

		if (center)
		{
			VC2 minPlane = VC2(0.0f, 0.0f);
			VC2 maxPlane = VC2(0.0f, 0.0f);

			minPlane.x = game->gameUI->getLightManager()->getLightPropertyValueFloat(light, "minx");
			maxPlane.x = game->gameUI->getLightManager()->getLightPropertyValueFloat(light, "maxx");
			minPlane.y = game->gameUI->getLightManager()->getLightPropertyValueFloat(light, "miny");
			maxPlane.y = game->gameUI->getLightManager()->getLightPropertyValueFloat(light, "maxy");

			float groundHeight = pos.y;
			if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
			{
				groundHeight = game->gameMap->getScaledHeightAt(pos.x, pos.z) + 0.02f;
			}
			VC3 plane1 = VC3(minPlane.x, groundHeight, minPlane.y);
			VC3 plane2 = VC3(maxPlane.x, groundHeight, minPlane.y);
			VC3 plane3 = VC3(maxPlane.x, groundHeight, maxPlane.y);
			VC3 plane4 = VC3(minPlane.x, groundHeight, maxPlane.y);

			disposable_scene->transformPositionForWorldFoldUsingPosition(plane1, foldpos);
			disposable_scene->transformPositionForWorldFoldUsingPosition(plane2, foldpos);
			disposable_scene->transformPositionForWorldFoldUsingPosition(plane3, foldpos);
			disposable_scene->transformPositionForWorldFoldUsingPosition(plane4, foldpos);

			disposable_scene->AddLine(plane1, plane2, col);
			disposable_scene->AddLine(plane2, plane3, col);
			disposable_scene->AddLine(plane3, plane4, col);
			disposable_scene->AddLine(plane4, plane1, col);
		}

		/*
		int textoffy = 0;

		if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_LIGHT_EXTENDED)
			&& distToCamSq < (DEBUGLIGHTVISUALIZER_EXTENDED_MAX_DIST * DEBUGLIGHTVISUALIZER_EXTENDED_MAX_DIST))
		{
			sprintf(textbuf, "uh: %d", terrainObject);
			DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
			textoffy += 16;

			sprintf(textbuf, "filename: %s", fnamecut.c_str());
			DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
			textoffy += 16;
		}
		*/

	}

	void DebugLightVisualizer::visualizeLights(game::Game *game, const VC3 &cameraPosition)
	{
		assert(game != NULL);

		UnifiedHandle uh = game->gameUI->getLightManager()->getFirstLight();

		while (uh != UNIFIED_HANDLE_NONE)
		{
			for (int i = 0; i < LIGHTMANAGER_LIGHT_SUB_HANDLES; i++)
			{
				visualizeLight(game, game->gameUI->getLightManager()->getSubHandle(uh, i), cameraPosition, 0);
			}

			uh = game->gameUI->getLightManager()->getNextLight(uh);
		}
	}

}

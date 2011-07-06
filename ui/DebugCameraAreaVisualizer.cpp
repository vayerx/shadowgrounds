
#include "precompiled.h"

#include "DebugCameraAreaVisualizer.h"
#include "DebugVisualizerTextUtil.h"
#include "../game/areas/GameAreaManager.h"
#include "../game/areas/CameraArea.h"
#include "../game/areas/TriggerArea.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/UnifiedHandleManager.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"
#include "../ui/uidefaults.h"
#include "../ogui/StormDriver.h"
#include "../system/Timer.h"
#include "../editor/ueoh_to_id_string.h"
#include "../convert/int64_to_hex.h"

#include <vector>
#include <Storm3D_UI.h>
#include "../util/Debug_MemoryManager.h"

#define DEBUGCAMERAAREAVISUALIZER_MAX_DIST 60.0f

#define DEBUGCAMERAAREAVISUALIZER_EXTENDED_MAX_DIST 30.0f

using namespace game;

extern IStorm3D_Scene *disposable_scene;

namespace ui
{

	void DebugCameraAreaVisualizer::visualizeCameraArea(game::Game *game, UnifiedHandle area, const VC3 &cameraPosition, DebugCameraAreaVisFlags visFlags)
	{
//		char textbuf[128];

		assert(VALIDATE_UNIFIED_HANDLE_BITS(area));

		if (!game->unifiedHandleManager->doesObjectExist(area))
		{
			return;
		}

		bool classok = false;
		if (game->getAreaManager()->getAreaByUnifiedHandle(area) != NULL)
		{
			if (game->getAreaManager()->getAreaByUnifiedHandle(area)->getAreaClassId() == AREA_CLASS_ID_CAMERA)
			{
				classok = true;
			}
		}
		if (!classok)
		{
			return;
		}

		if (visFlags & DEBUGCAMERAAREAVISUALIZER_FLAG_SELECTED)
		{
			// no range culling
		} else {
			VC3 cullpos = game->unifiedHandleManager->getObjectPosition(area);
			VC3 distToCamVec = cullpos - cameraPosition;
			// 2D RANGE
			distToCamVec.y = 0;
			float distToCamSq = distToCamVec.GetSquareLength();
			if (distToCamSq < (DEBUGCAMERAAREAVISUALIZER_MAX_DIST * DEBUGCAMERAAREAVISUALIZER_MAX_DIST))
			{
				// ok
			} else {
				return;
			}
		}

		COL col = COL(0.5f,0.0f,0.5f);
		if (((CameraArea *)game->getAreaManager()->getAreaByUnifiedHandle(area))->getDirectionMask() & CAMERAAREA_DIRECTION_MASK_LEFT)
		{
			col.r = 1.0f;
		}
		if (((CameraArea *)game->getAreaManager()->getAreaByUnifiedHandle(area))->getDirectionMask() & CAMERAAREA_DIRECTION_MASK_RIGHT)
		{
			col.b = 1.0f;
		}
		COL camcol = COL(0.0f,1.0f,0.0f);
		COL targcol = COL(1.0f,0.0f,0.0f);

		if (visFlags & DEBUGCAMERAAREAVISUALIZER_FLAG_SELECTED)
		{
			// ok with that color?
		} else {
			col.r = col.r / 2.0f;
			col.g = col.g / 2.0f;
			col.b = col.b / 2.0f;

			camcol.r = camcol.r / 2.0f;
			camcol.g = camcol.g / 2.0f;
			camcol.b = camcol.b / 2.0f;
			targcol.r = targcol.r / 2.0f;
			targcol.g = targcol.g / 2.0f;
			targcol.b = targcol.b / 2.0f;
		}

		VC3 pos = game->unifiedHandleManager->getObjectPosition(area);
		pos.y += 0.01f;

		bool center = false;
		if (game->getAreaManager()->getAreaSubHandleNumber(area) == CAMERAAREA_SUBHANDLE_CENTER)
		{
			center = true;
		}

		VC3 sizes = VC3(0.30f, 0.30f, 0.30f);
		if (!center)
		{
			sizes *= 0.5f;
			//if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
			//{
			//	pos.y = game->gameMap->getScaledHeightAt(pos.x, pos.z) + 0.02f;
			//}
		}

		VC3 c1 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c2 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c3 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 c4 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 cb1 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb2 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb3 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z + sizes.z);
		VC3 cb4 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z + sizes.z);

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

			CameraArea *cam = static_cast<CameraArea *>(game->getAreaManager()->getAreaByUnifiedHandle(area));

			if (cam->isLeftRightCamera())
			{
				VC3 end1 = (c1+c4)*0.5f;
				VC3 end2 = (c2+c3)*0.5f;
				end1.x -= sizes.x;
				end2.x += sizes.x;
				disposable_scene->AddLine(end1, end2, col);
				disposable_scene->AddLine(end1, c1, col);
				disposable_scene->AddLine(end1, c4, col);
				disposable_scene->AddLine(end2, c2, col);
				disposable_scene->AddLine(end2, c3, col);
			}
			if (cam->isUpDownCamera())
			{
				VC3 end1 = (c1+c2)*0.5f;
				VC3 end2 = (c3+c4)*0.5f;
				end1.z -= sizes.z;
				end2.z += sizes.z;
				disposable_scene->AddLine(end1, end2, col);
				disposable_scene->AddLine(end1, c1, col);
				disposable_scene->AddLine(end1, c2, col);
				disposable_scene->AddLine(end2, c3, col);
				disposable_scene->AddLine(end2, c4, col);
			}
		} else {
			int subNum = game->getAreaManager()->getAreaSubHandleNumber(area);
			if (subNum == CAMERAAREA_SUBHANDLE_UPPER_LEFT)
			{
				disposable_scene->AddLine(c1, c2, col);
				disposable_scene->AddLine(c1, c4, col);
			}
			else if (subNum == CAMERAAREA_SUBHANDLE_UPPER_RIGHT)
			{
				disposable_scene->AddLine(c1, c2, col);
				disposable_scene->AddLine(c2, c3, col);
			}
			else if (subNum == CAMERAAREA_SUBHANDLE_LOWER_RIGHT)
			{
				disposable_scene->AddLine(c2, c3, col);
				disposable_scene->AddLine(c3, c4, col);
			}
			else if (subNum == CAMERAAREA_SUBHANDLE_LOWER_LEFT)
			{
				disposable_scene->AddLine(c1, c4, col);
				disposable_scene->AddLine(c3, c4, col);
			}
			else 
			{
				if ((visFlags & DEBUGCAMERAAREAVISUALIZER_FLAG_SELECTED) != 0
					|| (visFlags & DEBUGCAMERAAREAVISUALIZER_FLAG_PARENTSELECTED) != 0)
				{
					if (subNum >= CAMERAAREA_SUBHANDLE_UPPER_LEFT_CAMERA_POSITION
						&& subNum <= CAMERAAREA_SUBHANDLE_LOWER_LEFT_CAMERA_POSITION)
					{
						disposable_scene->AddLine(c1, c2, camcol);
						disposable_scene->AddLine(c2, c3, camcol);
						disposable_scene->AddLine(c3, c4, camcol);
						disposable_scene->AddLine(c4, c1, camcol);
						disposable_scene->AddLine(cb1, cb2, camcol);
						disposable_scene->AddLine(cb2, cb3, camcol);
						disposable_scene->AddLine(cb3, cb4, camcol);
						disposable_scene->AddLine(cb4, cb1, camcol);
						disposable_scene->AddLine(c1, cb1, camcol);
						disposable_scene->AddLine(c2, cb2, camcol);
						disposable_scene->AddLine(c3, cb3, camcol);
						disposable_scene->AddLine(c4, cb4, camcol);
					} else {
						disposable_scene->AddLine(c1, c2, targcol);
						disposable_scene->AddLine(c2, c3, targcol);
						disposable_scene->AddLine(c3, c4, targcol);
						disposable_scene->AddLine(c4, c1, targcol);
						disposable_scene->AddLine(cb1, cb2, targcol);
						disposable_scene->AddLine(cb2, cb3, targcol);
						disposable_scene->AddLine(cb3, cb4, targcol);
						disposable_scene->AddLine(cb4, cb1, targcol);
						disposable_scene->AddLine(c1, cb1, targcol);
						disposable_scene->AddLine(c2, cb2, targcol);
						disposable_scene->AddLine(c3, cb3, targcol);
						disposable_scene->AddLine(c4, cb4, targcol);
					}
				}
			}

		}

		if (center)
		{
			VC3 subp[CAMERAAREA_SUBHANDLE_AMOUNT];
			//for (int i = 0; i < CAMERAAREA_SUBHANDLE_AMOUNT; i++)
			// skip center, we're already processing that.
			for (int i = 1; i < CAMERAAREA_SUBHANDLE_AMOUNT; i++)
			{
				UnifiedHandle subuh = game->getAreaManager()->getSubHandle(area, i);
				subp[i] = game->unifiedHandleManager->getObjectPosition(subuh);

				if (visFlags & DEBUGCAMERAAREAVISUALIZER_FLAG_SELECTED)
				{
					if (i >= CAMERAAREA_SUBHANDLE_UPPER_LEFT_CAMERA_POSITION
						&& i <= CAMERAAREA_SUBHANDLE_LOWER_LEFT_TARGET_POSITION)
					{
						visualizeCameraArea(game, subuh, cameraPosition, DEBUGCAMERAAREAVISUALIZER_FLAG_PARENTSELECTED);
					}
				}
			}
				
			disposable_scene->AddLine(subp[CAMERAAREA_SUBHANDLE_UPPER_LEFT], subp[CAMERAAREA_SUBHANDLE_UPPER_RIGHT], col);
			disposable_scene->AddLine(subp[CAMERAAREA_SUBHANDLE_UPPER_RIGHT], subp[CAMERAAREA_SUBHANDLE_LOWER_RIGHT], col);
			disposable_scene->AddLine(subp[CAMERAAREA_SUBHANDLE_LOWER_RIGHT], subp[CAMERAAREA_SUBHANDLE_LOWER_LEFT], col);
			disposable_scene->AddLine(subp[CAMERAAREA_SUBHANDLE_LOWER_LEFT], subp[CAMERAAREA_SUBHANDLE_UPPER_LEFT], col);

			disposable_scene->AddLine(subp[CAMERAAREA_SUBHANDLE_UPPER_LEFT_CAMERA_POSITION], subp[CAMERAAREA_SUBHANDLE_UPPER_LEFT_TARGET_POSITION], col);
			disposable_scene->AddLine(subp[CAMERAAREA_SUBHANDLE_UPPER_RIGHT_CAMERA_POSITION], subp[CAMERAAREA_SUBHANDLE_UPPER_RIGHT_TARGET_POSITION], col);
			disposable_scene->AddLine(subp[CAMERAAREA_SUBHANDLE_LOWER_RIGHT_CAMERA_POSITION], subp[CAMERAAREA_SUBHANDLE_LOWER_RIGHT_TARGET_POSITION], col);
			disposable_scene->AddLine(subp[CAMERAAREA_SUBHANDLE_LOWER_LEFT_CAMERA_POSITION], subp[CAMERAAREA_SUBHANDLE_LOWER_LEFT_TARGET_POSITION], col);
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

	void DebugCameraAreaVisualizer::visualizeCameraAreas(game::Game *game, const VC3 &cameraPosition)
	{
		assert(game != NULL);

		UnifiedHandle uh = game->getAreaManager()->getFirstArea();

		while (uh != UNIFIED_HANDLE_NONE)
		{
			if (game->getAreaManager()->getAreaByUnifiedHandle(uh) != NULL)
			{
				if (game->getAreaManager()->getAreaByUnifiedHandle(uh)->getAreaClassId() == AREA_CLASS_ID_CAMERA)
				{
					for (int i = 0; i < CAMERAAREA_SUBHANDLE_AMOUNT; i++)
					{
						visualizeCameraArea(game, game->getAreaManager()->getSubHandle(uh, i), cameraPosition, 0);
					}
				}
			}

			uh = game->getAreaManager()->getNextArea(uh);
		}
	}


	void DebugCameraAreaVisualizer::visualizeTriggerArea(game::Game *game, UnifiedHandle area, const VC3 &cameraPosition, DebugCameraAreaVisFlags visFlags)
	{
//		char textbuf[128];

		assert(VALIDATE_UNIFIED_HANDLE_BITS(area));

		if (!game->unifiedHandleManager->doesObjectExist(area))
		{
			return;
		}

		bool classok = false;
		if (game->getAreaManager()->getAreaByUnifiedHandle(area) != NULL)
		{
			if (game->getAreaManager()->getAreaByUnifiedHandle(area)->getAreaClassId() == AREA_CLASS_ID_TRIGGER)
			{
				classok = true;
			}
		}
		if (!classok)
		{
			return;
		}

		if (visFlags & DEBUGCAMERAAREAVISUALIZER_FLAG_SELECTED)
		{
			// no range culling
		} else {
			VC3 cullpos = game->unifiedHandleManager->getObjectPosition(area);
			VC3 distToCamVec = cullpos - cameraPosition;
			// 2D RANGE
			distToCamVec.y = 0;
			float distToCamSq = distToCamVec.GetSquareLength();
			if (distToCamSq < (DEBUGCAMERAAREAVISUALIZER_MAX_DIST * DEBUGCAMERAAREAVISUALIZER_MAX_DIST))
			{
				// ok
			} else {
				return;
			}
		}

		COL col = COL(0.0f,0.5f,0.5f);

		if (visFlags & DEBUGCAMERAAREAVISUALIZER_FLAG_SELECTED)
		{
			// ok with that color?
		} else {
			col.r = col.r / 2.0f;
			col.g = col.g / 2.0f;
			col.b = col.b / 2.0f;
		}

		VC3 pos = game->unifiedHandleManager->getObjectPosition(area);
		pos.y += 0.01f;

		bool center = false;
		if (game->getAreaManager()->getAreaSubHandleNumber(area) == QUADAREA_SUBHANDLE_CENTER)
		{
			center = true;
		}

		VC3 sizes = VC3(0.30f, 0.30f, 0.30f);
		if (!center)
		{
			sizes *= 0.5f;
			//if (game->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
			//{
			//	pos.y = game->gameMap->getScaledHeightAt(pos.x, pos.z) + 0.02f;
			//}
		}

		VC3 c1 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c2 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c3 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 c4 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 cb1 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb2 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb3 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z + sizes.z);
		VC3 cb4 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z + sizes.z);

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
			int subNum = game->getAreaManager()->getAreaSubHandleNumber(area);
			if (subNum == QUADAREA_SUBHANDLE_UPPER_LEFT)
			{
				disposable_scene->AddLine(c1, c2, col);
				disposable_scene->AddLine(c1, c4, col);
			}
			else if (subNum == QUADAREA_SUBHANDLE_UPPER_RIGHT)
			{
				disposable_scene->AddLine(c1, c2, col);
				disposable_scene->AddLine(c2, c3, col);
			}
			else if (subNum == QUADAREA_SUBHANDLE_LOWER_RIGHT)
			{
				disposable_scene->AddLine(c2, c3, col);
				disposable_scene->AddLine(c3, c4, col);
			}
			else if (subNum == QUADAREA_SUBHANDLE_LOWER_LEFT)
			{
				disposable_scene->AddLine(c1, c4, col);
				disposable_scene->AddLine(c3, c4, col);
			}
			else
			{
				TriggerArea *triggerArea = (TriggerArea*)game->getAreaManager()->getAreaByUnifiedHandle(area);
				if(triggerArea->isHelperEnabled(subNum - QUADAREA_SUBHANDLE_AMOUNT))
				{
					disposable_scene->AddLine(c1, c3, col);
					disposable_scene->AddLine(c2, c4, col);
				}
			}
		}

		if (center)
		{
			VC3 subp[QUADAREA_SUBHANDLE_AMOUNT];
			for (int i = 1; i < QUADAREA_SUBHANDLE_AMOUNT; i++)
			{
				UnifiedHandle subuh = game->getAreaManager()->getSubHandle(area, i);
				subp[i] = game->unifiedHandleManager->getObjectPosition(subuh);
			}
				
			disposable_scene->AddLine(subp[QUADAREA_SUBHANDLE_UPPER_LEFT], subp[QUADAREA_SUBHANDLE_UPPER_RIGHT], col);
			disposable_scene->AddLine(subp[QUADAREA_SUBHANDLE_UPPER_RIGHT], subp[QUADAREA_SUBHANDLE_LOWER_RIGHT], col);
			disposable_scene->AddLine(subp[QUADAREA_SUBHANDLE_LOWER_RIGHT], subp[QUADAREA_SUBHANDLE_LOWER_LEFT], col);
			disposable_scene->AddLine(subp[QUADAREA_SUBHANDLE_LOWER_LEFT], subp[QUADAREA_SUBHANDLE_UPPER_LEFT], col);

			VC3 distToCamVec = pos - cameraPosition;
			float distToCamSq = distToCamVec.GetSquareLength();

			if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_TRIGGER_AREAS_EXTENDED)
				&& distToCamSq < (DEBUGCAMERAAREAVISUALIZER_EXTENDED_MAX_DIST *DEBUGCAMERAAREAVISUALIZER_EXTENDED_MAX_DIST))
			{
				UniqueEditorObjectHandle ueoh = game->getAreaManager()->getUniqueEditorObjectHandleForUnifiedHandle( area );
				char textbuf[256];
				if (ueoh != 0)
				{
					std::string ueohstr;
					if (ueoh & 0x8000000000000000LL)
					{
						ueohstr = ueoh_to_id_string(ueoh);
					} else {
						ueohstr = int64_to_hex(ueoh);
					}
					sprintf(textbuf, "ueoh: %s", ueohstr.c_str());
				} else {
					sprintf(textbuf, "ueoh: (missing)");
				}
				DebugVisualizerTextUtil::renderText(pos, 0, 0, textbuf);
			}
		}
	}

	void DebugCameraAreaVisualizer::visualizeTriggerAreas(game::Game *game, const VC3 &cameraPosition)
	{
		assert(game != NULL);

		UnifiedHandle uh = game->getAreaManager()->getFirstArea();

		while (uh != UNIFIED_HANDLE_NONE)
		{
			if (game->getAreaManager()->getAreaByUnifiedHandle(uh) != NULL)
			{
				if (game->getAreaManager()->getAreaByUnifiedHandle(uh)->getAreaClassId() == AREA_CLASS_ID_TRIGGER)
				{
					for (int i = 0; i < TRIGGERAREA_SUBHANDLE_AMOUNT; i++)
					{
						visualizeTriggerArea(game, game->getAreaManager()->getSubHandle(uh, i), cameraPosition, 0);
					}
				}
			}

			uh = game->getAreaManager()->getNextArea(uh);
		}
	}

}

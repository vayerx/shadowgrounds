
#include "precompiled.h"

#include "DebugUnitVisualizer.h"
#include "DebugVisualizerTextUtil.h"
#include "../game/UnitList.h"
#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "../game/GameRandom.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_debug.h"
#include "../ui/uidefaults.h"
#include "../ogui/OguiStormDriver.h"
#include "../system/Timer.h"

#include <vector>
#include <Storm3D_UI.h>

#define DEBUGUNITVISUALIZER_EXTENDED_MAX_DIST 100.0f

using namespace game;

extern IStorm3D_Scene *disposable_scene;

namespace ui
{

	void DebugUnitVisualizer::visualizeUnit(game::Unit *unit, const VC3 &cameraPosition, DebugUnitVisFlags visFlags)
	{
		char textbuf[128];

		const char *utypename = game::SimpleOptions::getString(DH_OPT_S_DEBUG_VISUALIZE_UNITS_OF_TYPE);
		if (utypename != NULL
			&& utypename[0] != '\0')
		{
			if (unit->getUnitType()->getName() != NULL
				&& strcmp(unit->getUnitType()->getName(), utypename) == 0)
			{
				// ok
			} else {
				return;
			}
		}

		COL col = COL(1.0f,1.0f,0.5f);
		if (unit->getOwner() == 0)
			col = COL(0.5f,1.0f,0.5f);
		if (unit->getOwner() == 1)
			col = COL(1.0f,0.5f,0.5f);

		if (!unit->isActive())
		{
			col.r = col.r / 5.0f;
			col.g = col.g / 5.0f;
			col.b = col.b / 5.0f;
		} else {
			if (unit->isDestroyed())
			{
				col.r = col.r / 2.0f;
				col.g = col.g / 2.0f;
				col.b = col.b / 2.0f;
			}
		}

		VC3 pos = unit->getPosition();
		pos.y += 0.03f;
		VC3 sizes = VC3(0.25f, 0.25f, 0.25f);

		VC3 c1 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c2 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z - sizes.z);
		VC3 c3 = VC3(pos.x + sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 c4 = VC3(pos.x - sizes.x, pos.y + sizes.y, pos.z + sizes.z);
		VC3 cb1 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb2 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z - sizes.z);
		VC3 cb3 = VC3(pos.x + sizes.x, pos.y - sizes.y, pos.z + sizes.z);
		VC3 cb4 = VC3(pos.x - sizes.x, pos.y - sizes.y, pos.z + sizes.z);

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

		if (unit->isDirectControl())
		{
			VC3 aimpos = unit->targeting.getAimingPosition();
			aimpos.y += 0.03f;
			disposable_scene->AddLine(pos, aimpos, col);
		}
		if (unit->targeting.hasTarget())
		{
			VC3 targpos = VC3(0,0,0);
			targpos.y += 0.03f;
			if (unit->targeting.getTargetUnit() != NULL)
			{
				targpos = unit->targeting.getTargetUnit()->getPosition();
			} else {
				targpos = unit->targeting.getTargetPosition();
			}
			disposable_scene->AddLine(pos, targpos, col);
		}

		int textoffy = 0;

		VC3 distToCamVec = pos - cameraPosition;
		float distToCamSq = distToCamVec.GetSquareLength();

		if (game::SimpleOptions::getBool(DH_OPT_B_DEBUG_VISUALIZE_UNITS_EXTENDED)
			&& distToCamSq < (DEBUGUNITVISUALIZER_EXTENDED_MAX_DIST *DEBUGUNITVISUALIZER_EXTENDED_MAX_DIST))
		{
			sprintf(textbuf, "id/uh: %d", unit->getIdNumber());
			DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
			textoffy += 16;

			if (unit->getIdString() != NULL)
			{
				if (strlen(unit->getIdString()) < 128 - 20)
					sprintf(textbuf, "idstr: %s", unit->getIdString());
				else
					sprintf(textbuf, "idstr: ...");
				DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
				textoffy += 16;
			}

			sprintf(textbuf, "type: %s", unit->getUnitType()->getName());
			DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
			textoffy += 16;

			sprintf(textbuf, "script: %s", unit->getScript());
			DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
			textoffy += 16;
		}

		const char *uvarname = game::SimpleOptions::getString(DH_OPT_S_DEBUG_VISUALIZE_UNITS_VARIABLE);
		if (uvarname != NULL
			&& uvarname[0] != '\0'
			&& distToCamSq < (DEBUGUNITVISUALIZER_EXTENDED_MAX_DIST *DEBUGUNITVISUALIZER_EXTENDED_MAX_DIST))
		{
			int uvarNum = unit->variables.getVariableNumberByName(uvarname);
			if (uvarNum != -1)
			{
				if (strlen(uvarname) < 128 - 30)
					sprintf(textbuf, "%s: %d", uvarname, unit->variables.getVariable(uvarNum));
				else
					sprintf(textbuf, "...: %d", unit->variables.getVariable(uvarNum));
			} else {
				if (strlen(uvarname) < 128 - 30)
					sprintf(textbuf, "%s: (invalid)", uvarname);
				else
					sprintf(textbuf, "...: (invalid)");
			}
			DebugVisualizerTextUtil::renderText(pos, 0, textoffy, textbuf);
			textoffy += 16;
		}

	}

	void DebugUnitVisualizer::visualizeUnits(game::UnitList *units, const VC3 &cameraPosition)
	{
		assert(units != NULL);

		LinkedList *unitList = units->getAllUnits();
		LinkedListIterator iter(unitList);

		//int index = 0;
		while (iter.iterateAvailable())
		{
			Unit *unit = (Unit *)iter.iterateNext();

			visualizeUnit(unit, cameraPosition, 0);

			//index++;
		}
	}

}

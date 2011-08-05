
#include "precompiled.h"

#include "OffscreenUnitPointers.h"

#include <Storm3D_UI.h>

#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/GameScene.h"
#include "cursordefs.h"
#include "../ogui/Ogui.h"
#include "../container/LinkedList.h"


#define OFFSCREEN_UNIT_POINTERS_MAX_UNITS 50

// button id's
// MUST NOT OVERLAP ONES DEFINED IN COMBATWINDOW!!!
//#define COMBATW_OFFSCREENPOINTERS 600
#define COMBATW_OFFSCREEN_UNIT_START 700
#define COMBATW_OFFSCREEN_UNIT_END 799


using namespace game;

namespace ui
{
	OffscreenUnitPointers::OffscreenUnitPointers(Ogui *ogui, game::Game *game, int player, OguiWindow *window)
	{
		this->ogui = ogui;
		this->game = game;
		this->player = player;
		this->win = window;
		this->enabled = false;

		checkUnits = new LinkedList();

		pointedUnitsAmount = 0;
		pointerButs = new OguiButton *[OFFSCREEN_UNIT_POINTERS_MAX_UNITS];
		pointedUnits = new Unit *[OFFSCREEN_UNIT_POINTERS_MAX_UNITS];

		int i;
		for (i = 0; i < OFFSCREEN_UNIT_POINTERS_MAX_UNITS; i++)
		{ 
			pointerButs[i] = NULL;
			pointedUnits[i] = NULL;
		}

#ifdef LEGACY_FILES
		upImage = ogui->LoadOguiImage("Data/GUI/Ingame/unitpointers/unit_up.tga");
		downImage = ogui->LoadOguiImage("Data/GUI/Ingame/unitpointers/unit_down.tga");
		leftImage = ogui->LoadOguiImage("Data/GUI/Ingame/unitpointers/unit_left.tga");
		rightImage = ogui->LoadOguiImage("Data/GUI/Ingame/unitpointers/unit_right.tga");
#else
		upImage = ogui->LoadOguiImage("data/gui/hud/unitpointer/unit_up.tga");
		downImage = ogui->LoadOguiImage("data/gui/hud/unitpointer/unit_down.tga");
		leftImage = ogui->LoadOguiImage("data/gui/hud/unitpointer/unit_left.tga");
		rightImage = ogui->LoadOguiImage("data/gui/hud/unitpointer/unit_right.tga");
#endif
	}


	OffscreenUnitPointers::~OffscreenUnitPointers()
	{
		for (int i = 0; i < OFFSCREEN_UNIT_POINTERS_MAX_UNITS; i++)
		{ 
			if (pointerButs[i] != NULL)
				delete pointerButs[i];
		}
		if (upImage != NULL) delete upImage;
		if (downImage != NULL) delete downImage;
		if (leftImage != NULL) delete leftImage;
		if (rightImage != NULL) delete rightImage;

		delete checkUnits;
		delete [] pointerButs;
		delete [] pointedUnits;
	}


	void OffscreenUnitPointers::setEnabled(bool enabled)
	{
		this->enabled = enabled;
	}


	void OffscreenUnitPointers::addUnitForChecklist(game::Unit *unit)
	{ 
		LinkedListIterator iter = LinkedListIterator(checkUnits);
		while (iter.iterateAvailable())
		{
			Unit *u = (Unit *)iter.iterateNext();
			if (unit == u)
			{
				assert(!"OffscreenUnitPointers - Unit already in checklist.");
				return;
			}
		}
		checkUnits->append(unit);
	}


	void OffscreenUnitPointers::removeUnitFromChecklist(game::Unit *unit)
	{
		// TODO: check that the unit actually is in the list.
		checkUnits->remove(unit);
	}


	void OffscreenUnitPointers::update()
	{
		IStorm3D_Scene *scene = game->getGameScene()->getStormScene();
		IStorm3D_Camera *cam = scene->GetCamera();

		// create pointers for all offscreen units in checklist...
		// reusing existing buttons, or creating new ones if necessary.
		int num = 0;
		LinkedListIterator iter = LinkedListIterator(checkUnits);
		while (iter.iterateAvailable())
		{
			// if not enabled, just break now.
			if (!enabled) break;

			Unit *u = (Unit *)iter.iterateNext();

			if (u->isDestroyed()) continue;

			VC3 pos = u->getPosition();
			VC3 result = VC3(0,0,0);
			float rhw = 0;
			float real_z = 0;
			bool infront = cam->GetTransformedToScreen(pos, result, rhw, real_z);
			if (infront)
			{
				bool offscreen = false;
				int x = (int)(result.x * 1024.0f);
				int y = (int)(result.y * 768.0f);
				if (x < 0 || y < 0 || x >= 1024 || y >= 768)
					offscreen = true;

				if (offscreen)
				{
					OguiButton *b;
					if (pointerButs[num] == NULL)
					{
						// create a new button for this unit.
						b = ogui->CreateSimpleImageButton(win, 
							0, 0, 16, 16, 
							NULL, NULL, NULL, COMBATW_OFFSCREEN_UNIT_START + num);
						pointerButs[num] = b;
					} else {
						// reuse an existing button.
						b = pointerButs[num];
					}
					b->SetListener(this);
					b->SetReactMask(0);
					b->SetEventMask(OGUI_EMASK_CLICK | OGUI_EMASK_OVER);
					// HACK: disable offscreen pointers
					b->SetDisabled(true);

					IOguiImage *butDir = NULL;
					if (y < 0) 
					{
						y = 0;
						butDir = upImage;
					}
					if (y >= 768) 
					{
						y = 1024-1;
						butDir = downImage;
					}
					if (x < 0) 
					{
						x = 0;
						if (abs(x-1024/2) > abs(y-768/2))
							butDir = leftImage;
					}
					if (x >= 1024) 
					{
						x = 1024-1;
						if (abs(x-1024/2) > abs(y-768/2))
							butDir = rightImage;
					}

					b->SetImage(butDir);
					b->Move(x - 8, y - 8);

					pointedUnits[num] = u;
					num++;

					if (num >= OFFSCREEN_UNIT_POINTERS_MAX_UNITS) break;
				}
			}
		}
		// delete leftover buttons...
		for (int i = num; i < pointedUnitsAmount; i++)
		{
			if (pointerButs[i] != NULL)
			{
				delete pointerButs[i];
				pointerButs[i] = NULL;
			}
		}
		pointedUnitsAmount = num;
	}


	void OffscreenUnitPointers::CursorEvent(OguiButtonEvent *eve)
	{
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_OVER)
		{
			// TODO: split screen, solve correct cursor number
#ifdef PROJECT_SHADOWGROUNDS
			ogui->SetCursorImageState(0, DH_CURSOR_AIM);
#endif
		}
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			int unum = eve->triggerButton->GetId() - COMBATW_OFFSCREEN_UNIT_START;
			assert(unum >= 0 && unum < OFFSCREEN_UNIT_POINTERS_MAX_UNITS);
			game->gameUI->doUnitClick(player, pointedUnits[unum]);
		}
	}

}

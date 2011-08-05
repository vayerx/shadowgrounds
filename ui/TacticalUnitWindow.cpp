
#include "precompiled.h"

#include "TacticalUnitWindow.h"

#include "../system/Logger.h"
#include "../game/Game.h"
#include "../game/Unit.h"
#include "../game/Weapon.h"
#include "../game/GameUI.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_tactical.h"
#include "uidefaults.h"
#include "cursordefs.h"

namespace ui {
namespace  {
		const int NORMAL_MOVE_BUTTON  = 1;
		const int FAST_MOVE_BUTTON    = 2;
		const int STEALTH_MOVE_BUTTON = 3;

		const int PRIMARY_FIRE_BUTTON = 4;
		const int SECONDARY_FIRE_BUTTON = 5;
		const int BOTH_FIRE_BUTTON = 6;

		const int STOPMOVE_BUTTON = 7;
		const int STOPFIRE_BUTTON = 8;

} // end of unnamed namespace

struct TacticalUnitWindowData
{
	Ogui &ogui;
	game::Game &game;

	OguiWindow *window;

	// States
	bool isVisible;
	bool gameWasRunning;
	bool closedAck;
	std::vector<game::Unit *> units;

	IOguiImage *highlightImage;

	OguiButton *stealthBut;
	OguiButton *stealthIconBut;
	OguiButton *firePrimaryBut;
	OguiButton *firePrimaryIconBut;
	OguiButton *fireSecondaryBut;
	OguiButton *fireSecondaryIconBut;
	OguiButton *fireAllBut;
	OguiButton *fireAllIconBut;

	// if true, firing with multiple units, thus "basic" or "heavy"
	// if false, firing with one unit, thus "X" (left) or "Y" (right)
	bool fireMultipleUnits;

	bool primaryDropWeap;
	bool secondaryDropWeap;

	VC3 clickPosition;

	// the button (command) id that was selected from the menu
	int clickedButton;

	// Target
	Vector2D targetPosition;  // ACTUALLY UNUSED!
	game::Unit *targetUnit;

	TacticalUnitWindowData(Ogui &ogui_, game::Game &game_)
	:	ogui(ogui_),
		game(game_),

		window(0),

		isVisible(false),
		gameWasRunning(false),
		closedAck(true),

		stealthBut(0),
		stealthIconBut(0),
		firePrimaryBut(0),
		firePrimaryIconBut(0),
		fireSecondaryBut(0),
		fireSecondaryIconBut(0),
		fireAllBut(0),
		fireAllIconBut(0),
		fireMultipleUnits(false),

		primaryDropWeap(false),
		secondaryDropWeap(false),
		clickPosition(0,0,0),
		targetUnit(0)
		
	{
	}

	~TacticalUnitWindowData()
	{
		// TODO: Should delete the buttons!
		// The window deletion works too, but that may leak memory
		// or may be buggy in some other way (not recommended).
		// The right way is to first delete buttons, then the containing
		// window.

		delete window;

		delete highlightImage;
	}

	void showWindowAt(int x, int y)
	{
		window->MoveTo(x - 5, y - 5);
		window->Show();
		isVisible = true;
		closedAck = false;

		if (game::SimpleOptions::getBool(DH_OPT_B_MENU_AUTOPAUSE))
		{
			// Pause game
			if(!game.isTacticalMode())
			{
				gameWasRunning = true;
				game.gameUI->setPointersChangedFlag(game.singlePlayerNumber);
			}
			else
				gameWasRunning = false;

			game.setTacticalMode(true);
		}

		// TODO: client number
		game::SceneSelection *sel = game.gameUI->getSceneSelection(0);

		clickPosition = VC3(sel->scaledMapX, 
			game.gameMap->getScaledHeightAt(sel->scaledMapX, sel->scaledMapY), 
			sel->scaledMapY);

		if (game::SimpleOptions::getBool(DH_OPT_B_MENU_TWO_CLICK))
		{
			// TODO: correct player...
			if (sel->unit != NULL
				&& sel->unit->visibility.isSeenByPlayer(game.singlePlayerNumber))
			{
				game.gameUI->setUnitHighlight(game.singlePlayerNumber, sel->unit);
			} else {
				game.gameUI->setTerrainHighlight(game.singlePlayerNumber, clickPosition);
			}
			game.gameUI->lockHighlight(game.singlePlayerNumber);
		}
	}

	void updateWindowButtons(std::vector<game::Unit *> *unitsSelected)
	{
		primaryDropWeap = false;
		secondaryDropWeap = false;
		if (unitsSelected->size() == 0)
		{
			firePrimaryBut->SetText("");
			fireSecondaryBut->SetText("");
			fireAllBut->SetText("");
			firePrimaryBut->SetDisabled(true);
			fireSecondaryBut->SetDisabled(true);
			fireAllBut->SetDisabled(true);
			firePrimaryIconBut->SetDisabled(true);
			fireSecondaryIconBut->SetDisabled(true);
			fireAllIconBut->SetDisabled(true);
			fireMultipleUnits = true; // well... :)
		}
		else if (unitsSelected->size() == 1)
		{
			game::Weapon *w1 = (*unitsSelected)[0]->getWeaponType(0);
			game::Weapon *w2 = (*unitsSelected)[0]->getWeaponType(1);
			if (w1 != NULL)
			{
				// NOTE: copy&paste ahead (w1 & w2)...
				std::string firetext = "    Fire ";
				if (w1->isDropWeapon())
				{
					firetext = "    Lay "; 
					primaryDropWeap = true;
				}
				// TODO: should be "Setup " for timebomb.
				if (w1->getShortName() != NULL)
					firetext += w1->getShortName();
				const char *t = firetext.c_str();
				firePrimaryBut->SetText(t);
				firePrimaryBut->SetDisabled(false);
				firePrimaryIconBut->SetDisabled(false);
			} else {
				firePrimaryBut->SetText("");
				firePrimaryBut->SetDisabled(true);
				firePrimaryIconBut->SetDisabled(true);
			}
			if (w2 != NULL)
			{
				std::string firetext = "    Fire ";
				if (w2->isDropWeapon())
				{
					firetext = "    Lay "; 
					secondaryDropWeap = true;
				}
				// TODO: should be "Setup " for timebomb.
				if (w2->getShortName() != NULL)
					firetext += w2->getShortName();
				const char *t = firetext.c_str();
				fireSecondaryBut->SetText(t);
				fireSecondaryBut->SetDisabled(false);
				fireSecondaryIconBut->SetDisabled(false);
			} else {
				fireSecondaryBut->SetText("");
				fireSecondaryBut->SetDisabled(true);
				fireSecondaryIconBut->SetDisabled(true);
			}
			if (w1 != NULL && w2 != NULL)
			{
				fireAllBut->SetText("    Fire all");
				fireAllBut->SetDisabled(false);
				fireAllIconBut->SetDisabled(false);
			} else {
				fireAllBut->SetText("");
				fireAllBut->SetDisabled(true);
				fireAllIconBut->SetDisabled(true);
			}
			fireMultipleUnits = false;
			if ((*unitsSelected)[0]->getStealthValue() > 0)
			{
				stealthBut->SetText("    Stealth");
				stealthBut->SetDisabled(false);
				stealthIconBut->SetDisabled(false);
			} else {
				stealthBut->SetText("");
				stealthBut->SetDisabled(true);
				stealthIconBut->SetDisabled(true);
			}
		} else {
			stealthBut->SetText("");
			firePrimaryBut->SetText("    Fire basic");
			fireSecondaryBut->SetText("    Fire heavy");
			fireAllBut->SetText("    Fire all");
			stealthBut->SetDisabled(true);
			stealthIconBut->SetDisabled(true);
			firePrimaryBut->SetDisabled(false);
			firePrimaryIconBut->SetDisabled(false);
			fireSecondaryBut->SetDisabled(false);
			fireSecondaryIconBut->SetDisabled(false);
			fireAllBut->SetDisabled(false);
			fireAllIconBut->SetDisabled(false);
			fireMultipleUnits = true;
		}
	}

};

TacticalUnitWindow::TacticalUnitWindow(Ogui *ogui, game::Game *game)
{
	data = new TacticalUnitWindowData(*ogui, *game);
	data->window = data->ogui.CreateSimpleWindow(0, 0, 111, 150, "Data/GUI/Ingame/commandmenu.tga");
	data->window->SetPopup();

	data->highlightImage = ogui->LoadOguiImage("Data/GUI/Ingame/menu_highlight.tga");

	// Buttons
	OguiButton *b;
	b = data->ogui.CreateSimpleImageButton(data->window, 8, 5 + 17*3, 24, 16, "Data/GUI/Ingame/Menuicons/action_stealth.tga", 0, 0, STEALTH_MOVE_BUTTON);
	data->stealthIconBut = b;
	b->SetReactMask(0);
	b->SetListener(this);
	b = data->ogui.CreateSimpleImageButton(data->window, 8, 5 + 17*1, 24, 16, "Data/GUI/Ingame/Menuicons/action_normal.tga",  0, 0, NORMAL_MOVE_BUTTON);
	b->SetReactMask(0);
	b->SetListener(this);
	b = data->ogui.CreateSimpleImageButton(data->window, 8, 5 + 17*2, 24, 16, "Data/GUI/Ingame/Menuicons/action_fast.tga",    0, 0, FAST_MOVE_BUTTON);
	b->SetReactMask(0);
  b->SetListener(this);

	b = data->ogui.CreateSimpleImageButton(data->window, 8, 5+5 + 17*5, 24, 16, "Data/GUI/Ingame/Menuicons/action_firebasic.tga",  0, 0, PRIMARY_FIRE_BUTTON);
	b->SetReactMask(0);
	b->SetListener(this);
	data->firePrimaryIconBut = b;
	b = data->ogui.CreateSimpleImageButton(data->window, 8, 5+5 + 17*6, 24, 16, "Data/GUI/Ingame/Menuicons/action_fireheavy.tga",  0, 0, SECONDARY_FIRE_BUTTON);
	b->SetReactMask(0);
	b->SetListener(this);
	data->fireSecondaryIconBut = b;
	b = data->ogui.CreateSimpleImageButton(data->window, 8, 5+5 + 17*7, 24, 16, "Data/GUI/Ingame/Menuicons/action_fireall.tga",  0, 0, BOTH_FIRE_BUTTON);
	b->SetReactMask(0);
	b->SetListener(this);
	data->fireAllIconBut = b;
	b = data->ogui.CreateSimpleImageButton(data->window, 8, 5 + 17*0, 24, 16, "Data/GUI/Ingame/Menuicons/action_stop.tga",  0, 0, STOPMOVE_BUTTON);
	b->SetReactMask(0);
	b->SetListener(this);
	b = data->ogui.CreateSimpleImageButton(data->window, 8, 5+5 + 17*4, 24, 15, "Data/GUI/Ingame/Menuicons/action_ceasefire.tga",  0, 0, STOPFIRE_BUTTON);
	b->SetReactMask(0);
	b->SetListener(this);

	// Texts
	b = data->ogui.CreateSimpleTextButton(data->window, 8, 6 + 17*3, 96, 16, 0, 0, 0, "    Sneak", STEALTH_MOVE_BUTTON);    b->SetListener(this); b->SetFont(defaultSmallIngameFont); b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
	b->SetHighlightedImage(data->highlightImage);
	data->stealthBut = b;
	b = data->ogui.CreateSimpleTextButton(data->window, 8, 6 + 17*1, 96, 16, 0, 0, 0, "    Move", NORMAL_MOVE_BUTTON);      b->SetListener(this); b->SetFont(defaultSmallIngameFont); b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
	b->SetHighlightedImage(data->highlightImage);
	b = data->ogui.CreateSimpleTextButton(data->window, 8, 6 + 17*2, 96, 16, 0, 0, 0, "    Move fast", FAST_MOVE_BUTTON);   b->SetListener(this); b->SetFont(defaultSmallIngameFont); b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
	b->SetHighlightedImage(data->highlightImage);
	b = data->ogui.CreateSimpleTextButton(data->window, 8, 5+6 + 17*5, 96, 16, 0, 0, 0, "    Fire basic", PRIMARY_FIRE_BUTTON);   b->SetListener(this); b->SetFont(defaultSmallIngameFont); b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
	b->SetHighlightedImage(data->highlightImage);
	data->firePrimaryBut = b;
	b = data->ogui.CreateSimpleTextButton(data->window, 8, 5+6 + 17*6, 96, 16, 0, 0, 0, "    Fire heavy", SECONDARY_FIRE_BUTTON); b->SetListener(this); b->SetFont(defaultSmallIngameFont); b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
	b->SetHighlightedImage(data->highlightImage);
	data->fireSecondaryBut = b;
	b = data->ogui.CreateSimpleTextButton(data->window, 8, 5+6 + 17*7, 96, 16, 0, 0, 0, "    Fire all", BOTH_FIRE_BUTTON);    b->SetListener(this); b->SetFont(defaultSmallIngameFont); b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
	b->SetHighlightedImage(data->highlightImage);
	data->fireAllBut = b;
	b = data->ogui.CreateSimpleTextButton(data->window, 8, 6 + 17*0, 96, 16, 0, 0, 0, "    Stop", STOPMOVE_BUTTON);    b->SetListener(this); b->SetFont(defaultSmallIngameFont); b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
	b->SetHighlightedImage(data->highlightImage);
	b = data->ogui.CreateSimpleTextButton(data->window, 8, 5+6 + 17*4, 96, 16, 0, 0, 0, "    Cease fire", STOPFIRE_BUTTON);    b->SetListener(this); b->SetFont(defaultSmallIngameFont); b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
	b->SetHighlightedImage(data->highlightImage);

	data->window->Hide();

}

TacticalUnitWindow::~TacticalUnitWindow()
{
	delete data;
}

void TacticalUnitWindow::keepVisible()
{
	if (!data->closedAck)
	{
		data->isVisible = true;
		data->closedAck = false;
		data->window->Show();
	}
}

void TacticalUnitWindow::doTacticalClick(VC3 &position, game::Unit *targetUnit)
{
	if (game::SimpleOptions::getBool(DH_OPT_B_MENU_TWO_CLICK))
	{
		// this should only happen if we have three click menu selected
		// this may possibly happen if option is changed in the middle of the game.
		Logger::getInstance()->debug("TacticalUnitWindow - doTacticalClick called while in two click mode.");
	}

	data->clickPosition = position;
	data->targetUnit = targetUnit;
	executeCommand();
}

void TacticalUnitWindow::executeCommand()
{
	int buttonId = data->clickedButton;

	// Movement
	if(buttonId == NORMAL_MOVE_BUTTON)
	{
		data->game.formations.addMovePoint(&data->units, data->clickPosition, game::Unit::MoveTypeNormal);
	}
	else if(buttonId == FAST_MOVE_BUTTON)
	{
		data->game.formations.addMovePoint(&data->units, data->clickPosition, game::Unit::MoveTypeFast);
	}
	else if(buttonId == STEALTH_MOVE_BUTTON)
	{
		data->game.formations.addMovePoint(&data->units, data->clickPosition, game::Unit::MoveTypeStealth);
	}

	// Shooting
	if(buttonId == PRIMARY_FIRE_BUTTON)
	{
		game::Unit::FireType fireType = game::Unit::FireTypePrimary;
		if (data->fireMultipleUnits)
		{
			fireType = game::Unit::FireTypeBasic;
		} else {
			// was this a drop weapon? if so, walk near the target...
			if (data->primaryDropWeap)
				data->game.formations.addMovePoint(&data->units, data->clickPosition, game::Unit::MoveTypeNormal);
		}

		if(data->targetUnit)
			data->game.formations.setTarget(&data->units, data->targetUnit, fireType);
		else
			data->game.formations.setTarget(&data->units, data->clickPosition, fireType);
	}
	else if(buttonId == SECONDARY_FIRE_BUTTON)
	{
		game::Unit::FireType fireType = game::Unit::FireTypeSecondary;
		if (data->fireMultipleUnits)
		{
			fireType = game::Unit::FireTypeHeavy;
		} else {
			// was this a drop weapon? if so, walk near the target...
			if (data->secondaryDropWeap)
				data->game.formations.addMovePoint(&data->units, data->clickPosition, game::Unit::MoveTypeNormal);
		}

		if(data->targetUnit)
			data->game.formations.setTarget(&data->units, data->targetUnit, fireType);
		else
			data->game.formations.setTarget(&data->units, data->clickPosition, fireType);
	}
	else if(buttonId == BOTH_FIRE_BUTTON)
	{
		if(data->targetUnit)
			data->game.formations.setTarget(&data->units, data->targetUnit, game::Unit::FireTypeAll);
		else
			data->game.formations.setTarget(&data->units, data->clickPosition, game::Unit::FireTypeAll);
	}

	// Cancel, delete this waypoint
	if(buttonId == STOPMOVE_BUTTON)
	{
		// lose destination
		data->game.formations.clearMovePoint(&data->units);
	}

	// Cancel, delete this waypoint
	if(buttonId == STOPFIRE_BUTTON)
	{
		// lose target...
		data->game.formations.clearTarget(&data->units);
	}

	tacticalDone();

	// TODO: correct player...
	data->game.gameUI->setPointersChangedFlag(data->game.singlePlayerNumber);
}

void TacticalUnitWindow::CursorEvent(OguiButtonEvent *event)
{
	//VC2I cursorPosition(event->cursorScreenX, event->cursorScreenY);

	data->clickedButton = event->triggerButton->GetId();

	if (game::SimpleOptions::getBool(DH_OPT_B_MENU_TWO_CLICK)
		|| data->clickedButton == STOPFIRE_BUTTON
		|| data->clickedButton == STOPMOVE_BUTTON)
	{
		// 2 click menu, or stop / cease fire button
		executeCommand();
	} else {
		// 3 click menu
		int cursor = DH_CURSOR_MOVE_TO;
		if (data->fireMultipleUnits)
		{
#ifdef PROJECT_SHADOWGROUNDS
			if (data->clickedButton == PRIMARY_FIRE_BUTTON)
				cursor = DH_CURSOR_AIM;			
			if (data->clickedButton == SECONDARY_FIRE_BUTTON)
				cursor = DH_CURSOR_AIM_HEAVY;
			if (data->clickedButton == BOTH_FIRE_BUTTON)
				cursor = DH_CURSOR_AIM_ALL;
#endif
		} else {
			assert(data->units.size() == 1);

#ifdef PROJECT_SHADOWGROUNDS
			game::Weapon *w1 = (data->units)[0]->getWeaponType(0);
			game::Weapon *w2 = (data->units)[0]->getWeaponType(1);
			if (data->clickedButton == PRIMARY_FIRE_BUTTON)
			{
				if (w1->isHeavyWeapon())
					cursor = DH_CURSOR_AIM_HEAVY;
				else
					cursor = DH_CURSOR_AIM;
			}
			if (data->clickedButton == SECONDARY_FIRE_BUTTON)
			{
				if (w2->isHeavyWeapon())
					cursor = DH_CURSOR_AIM_HEAVY;	
				else
					cursor = DH_CURSOR_AIM;
			}
			if (data->clickedButton == BOTH_FIRE_BUTTON)
			{
				if (w1->isHeavyWeapon() || w2->isHeavyWeapon())
				{
					if (w1->isHeavyWeapon() && w2->isHeavyWeapon())
					{
						cursor = DH_CURSOR_AIM_HEAVY;	
					} else {
						cursor = DH_CURSOR_AIM_ALL;
					}
				} else {
					cursor = DH_CURSOR_AIM;	
				}
			}
#endif
		}
		// TODO: correct player number...
		data->game.gameUI->setTacticalClickExpected(data->game.singlePlayerNumber, cursor);
	}

	// ToDo: handling of existing waypoints. Do we even need it?

  setClosedAck();

	hide();
}

void TacticalUnitWindow::update()
{
	if((data->isVisible == true) && (data->window->IsVisible() == false))
		hide();
}

bool TacticalUnitWindow::isVisible()
{
	// note: may lag behind by one frame, if update not called before this
  return data->isVisible;
}

bool TacticalUnitWindow::isClosedAck()
{
	// note: may lag behind by one frame, if update not called before this
  return data->closedAck;
}

void TacticalUnitWindow::setClosedAck()
{
  data->closedAck = true;
}

void TacticalUnitWindow::showAt(int xPosition, int yPosition, std::vector<game::Unit *> *unitsSelected, game::Unit *targetUnit)
{
	data->updateWindowButtons(unitsSelected);

	data->units.swap(*unitsSelected);
	data->targetUnit = targetUnit;

	// ToDo: map settings from units to window

	data->showWindowAt(xPosition, yPosition);
}

void TacticalUnitWindow::showAt(int xPosition, int yPosition, std::vector<game::Unit *> *unitsSelected, const VC2 &targetPosition)
{
	data->updateWindowButtons(unitsSelected);

	data->units.swap(*unitsSelected);
	data->targetUnit = 0;

	// ToDo: map settings from units to window

	data->showWindowAt(xPosition, yPosition);
}
/*
void TacticalUnitWindow::showAt(int xPosition, int yPosition, game::Unit *unit, int waypointIndex)
{
	data->units.clear();
	data->targetUnit = 0;

	// ToDo: map settings from units to window

	data->showWindowAt(xPosition, yPosition);
}
*/

void TacticalUnitWindow::tacticalDone()
{
	// TODO: correct player...
	if (game::SimpleOptions::getBool(DH_OPT_B_MENU_TWO_CLICK))
		data->game.gameUI->unlockHighlight(data->game.singlePlayerNumber);

	if (game::SimpleOptions::getBool(DH_OPT_B_MENU_AUTOPAUSE))
	{
		// Continue
		if(data->gameWasRunning)
		{
			data->game.gameUI->setPointersChangedFlag(data->game.singlePlayerNumber);
			data->game.setTacticalMode(false);
			data->gameWasRunning = false;
		}
	}
}

void TacticalUnitWindow::hide()
{
	data->isVisible = false;
	data->window->Hide();
}

} // end of namespace ui


#include "precompiled.h"

#include "OptionsWindow.h"

#include <assert.h>
#include "uidefaults.h"
#include "cursordefs.h"
#include "../ogui/Ogui.h"
#include "../game/SimpleOptions.h"
#include "../game/Game.h"
#include "../game/scripting/GameScripting.h"
#include "../game/options/options_players.h"
#include "../game/DHLocaleManager.h"

// button id's for options window
#define OPTIONSW_PLAYERENABLED_FIRST 1
#define OPTIONSW_PLAYERENABLED_AMOUNT (MAX_PLAYERS_PER_CLIENT)
#define OPTIONSW_PLAYERCONTROLLER_FIRST (OPTIONSW_PLAYERENABLED_FIRST + OPTIONSW_PLAYERENABLED_AMOUNT)
#define OPTIONSW_PLAYERCONTROLLER_AMOUNT (MAX_PLAYERS_PER_CLIENT)

#define OPTIONSW_DIFFICULTY_LEVEL 101
#define OPTIONSW_AUTOADJUST 102

using namespace game;

namespace ui
{
	OptionsWindow::OptionsWindow(game::Game *game, Ogui *ogui, int player)
	{
		this->ogui = ogui;
		this->game = game;
		this->player = player;
		
//		win = ogui->CreateSimpleWindow(1024-256, 768-128-16-40, 256, 120, NULL);
		win = ogui->CreateSimpleWindow(1024-220, 16, 220, 120, NULL);
		win->SetUnmovable();
		win->SetReactMask(OGUI_WIN_REACT_MASK_CURSOR_1);

		for (int ci = 0; ci < MAX_PLAYERS_PER_CLIENT; ci++)		
		{
			playerTextLabel[ci] = NULL;
			playerEnabledButton[ci] = NULL;
			playerControllerButton[ci] = NULL;
		}

    OguiButton *b;

		/*
		//TEMP! only 1 player
		//for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)		
		for (int i = 0; i < 1; i++)		
		{
			const char *pltxt = "?";
			if (i == 0) pltxt = getLocaleGuiString("gui_options_player_1"); //"Player 1";
			if (i == 1) pltxt = getLocaleGuiString("gui_options_player_2"); //"Player 2";
			if (i == 2) pltxt = getLocaleGuiString("gui_options_player_3"); //"Player 3";
			if (i == 3) pltxt = getLocaleGuiString("gui_options_player_4"); //"Player 4";

			playerTextLabel[i] = ogui->CreateTextLabel(win, 0, i * 20, 64, 18, pltxt);
			playerTextLabel[i]->SetFont(ui::defaultMediumIngameFont);
			playerTextLabel[i]->SetTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);

			// player on/off button
			b = ogui->CreateSimpleTextButton(win, 64, i * 20, 32, 20, "Data/GUI/Buttons/playerenabled.tga", "Data/GUI/Buttons/playerenabled_down.tga", "Data/GUI/Buttons/playerenabled_highlight.tga", "", OPTIONSW_PLAYERENABLED_FIRST + i);
			b->SetReactMask(OGUI_REACT_MASK_OVER | OGUI_REACT_MASK_BUT_1);
			b->SetListener(this);
			b->SetEventMask(OGUI_EMASK_CLICK);
			b->SetFont(ui::defaultMediumIngameFont);
			b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
			b->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);

			playerEnabledButton[i] = b;

			// player controller button
			b = ogui->CreateSimpleTextButton(win, 64+32, i * 20, 96, 20, "Data/GUI/Buttons/playercontroller.tga", "Data/GUI/Buttons/playercontroller_down.tga", "Data/GUI/Buttons/playercontroller_highlight.tga", "", OPTIONSW_PLAYERCONTROLLER_FIRST + i);
			b->SetReactMask(OGUI_REACT_MASK_OVER | OGUI_REACT_MASK_BUT_1);
			b->SetListener(this);
			b->SetEventMask(OGUI_EMASK_CLICK);
			b->SetFont(ui::defaultMediumIngameFont);
			b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
			b->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);

			playerControllerButton[i] = b;
		}
		*/

		// difficulty label
		difficultyTextLabel = ogui->CreateTextLabel(win, 0, 3 * 20, 64, 18, getLocaleGuiString("gui_options_difficulty"));
		difficultyTextLabel->SetFont(ui::defaultMediumIngameFont);
		difficultyTextLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);

		// autoadjust difficulty label
		/*
		autoadjustTextLabel = ogui->CreateTextLabel(win, 0, 4 * 20, 64, 18, getLocaleGuiString("gui_options_auto_balance"));
		autoadjustTextLabel->SetFont(ui::defaultMediumIngameFont);
		autoadjustTextLabel->SetTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);
		*/

		// difficulty level
		b = ogui->CreateSimpleTextButton(win, 64, 3 * 20, 96, 20, "Data/GUI/Buttons/playercontroller.tga", "Data/GUI/Buttons/playercontroller_down.tga", "Data/GUI/Buttons/playercontroller_highlight.tga", "", OPTIONSW_DIFFICULTY_LEVEL);
		b->SetReactMask(OGUI_REACT_MASK_OVER | OGUI_REACT_MASK_BUT_1);
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK);
		b->SetFont(ui::defaultMediumIngameFont);
		b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
		b->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);

		difficultyButton = b;

		// autoadjust difficulty		
		/*
		b = ogui->CreateSimpleTextButton(win, 64, 4 * 20, 20, 20, "Data/GUI/Buttons/playerenabled.tga", "Data/GUI/Buttons/playerenabled_down.tga", "Data/GUI/Buttons/playerenabled_highlight.tga", "", OPTIONSW_AUTOADJUST);
		b->SetReactMask(OGUI_REACT_MASK_OVER | OGUI_REACT_MASK_BUT_1);
		b->SetListener(this);
		b->SetEventMask(OGUI_EMASK_CLICK);
		b->SetFont(ui::defaultMediumIngameFont);
		b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
		b->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);

		autoadjustButton = b;
		*/

		// HACK: SET DEFAULT DIFFICULTY TO EASY
		int difficultyLevel = 25;
		game->gameScripting->setGlobalIntVariableValue("general_difficulty_level", difficultyLevel);
		game->gameScripting->setGlobalIntVariableValue("damage_amount_level", difficultyLevel);
		game->gameScripting->setGlobalIntVariableValue("item_amount_level", difficultyLevel);
		game->gameScripting->setGlobalIntVariableValue("hostile_amount_level", difficultyLevel);

		refresh();
	}


	OptionsWindow::~OptionsWindow()
	{
		for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			if (playerEnabledButton[i] != NULL)
				delete playerEnabledButton[i];
			if (playerControllerButton[i] != NULL)
				delete playerControllerButton[i];
			if (playerTextLabel[i] != NULL)
				delete playerTextLabel[i];
		}
		delete win;
	}


	void OptionsWindow::refresh()
	{
		for (int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			bool playerOn = false;
			if (playerEnabledButton[i] != NULL)
			{
				const char *txt = getLocaleGuiString("gui_options_off");//"Off";
				if ((i == 0 && game::SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED))
					|| (i == 1 && game::SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
					|| (i == 2 && game::SimpleOptions::getBool(DH_OPT_B_3RD_PLAYER_ENABLED))
					|| (i == 3 && game::SimpleOptions::getBool(DH_OPT_B_4TH_PLAYER_ENABLED)))
				{
					txt = getLocaleGuiString("gui_options_on");//"On";
					playerOn = true;
				}
				playerEnabledButton[i]->SetText(txt);
			}

			if (playerControllerButton[i] != NULL)
			{
				const char *txt = getLocaleGuiString("gui_options_none");//" None";
				const char *keybinds = "";
				if (i == 0) keybinds = game::SimpleOptions::getString(DH_OPT_S_1ST_PLAYER_KEYBINDS);
				if (i == 1) keybinds = game::SimpleOptions::getString(DH_OPT_S_2ND_PLAYER_KEYBINDS);
				if (i == 2) keybinds = game::SimpleOptions::getString(DH_OPT_S_3RD_PLAYER_KEYBINDS);
				if (i == 3) keybinds = game::SimpleOptions::getString(DH_OPT_S_4TH_PLAYER_KEYBINDS);

				if (keybinds != NULL && keybinds[0] != '\0')
				{
					txt = getLocaleGuiString("gui_options_custom"); //" Custom";

					if (strcmp(keybinds, "Config/keybinds_mouse.txt") == 0)
						txt = getLocaleGuiString("gui_options_kb_mouse");//" Kb+Mouse";
					if (strcmp(keybinds, "Config/keybinds_joypad1.txt") == 0)
						txt = getLocaleGuiString("gui_options_joypad1");//" Joypad 1";
					if (strcmp(keybinds, "Config/keybinds_joypad2.txt") == 0)
						txt = getLocaleGuiString("gui_options_joypad2");//" Joypad 2";
				}

				if (!playerOn)
				{
					txt = getLocaleGuiString("gui_options_disabled"); //" Disabled";
					playerControllerButton[i]->SetDisabled(true);
				} else {
					playerControllerButton[i]->SetDisabled(false);
				}

				playerControllerButton[i]->SetText(txt);
			}
		}

		const char *lvlName = getLocaleGuiString("gui_custom");
		int difficultyLevel = game->gameScripting->getGlobalIntVariableValue("general_difficulty_level");
		if (difficultyLevel < 25)
		{
			lvlName = getLocaleGuiString("gui_options_very_easy");
		}
		else if (difficultyLevel < 50)
		{
			lvlName = getLocaleGuiString("gui_options_easy");
		}
		else if (difficultyLevel < 75)
		{
			lvlName = getLocaleGuiString("gui_options_normal");
		}
		else if (difficultyLevel < 100)
		{
			lvlName = getLocaleGuiString("gui_options_hard");
		}
		else if (difficultyLevel == 100)
		{
			lvlName = getLocaleGuiString("gui_options_very_hard");
		}
		difficultyButton->SetText(lvlName);

		/*
		int autoadjust = game->gameScripting->getGlobalIntVariableValue("autoadjust_difficulty_level");

		if (autoadjust != 0)
			autoadjustButton->SetText("X");
		else
			autoadjustButton->SetText("");
		*/

	}

		
	void OptionsWindow::CursorEvent(OguiButtonEvent *eve)
	{
		/*
		int changePlayerKeybind = -1;

		if (eve->triggerButton->GetId() >= OPTIONSW_PLAYERENABLED_FIRST
			&& eve->triggerButton->GetId() < OPTIONSW_PLAYERENABLED_FIRST + OPTIONSW_PLAYERENABLED_AMOUNT)
		{
			int num = eve->triggerButton->GetId() - OPTIONSW_PLAYERENABLED_FIRST;
			assert(num >= 0 && num < MAX_PLAYERS_PER_CLIENT);

			bool playerOn = false;
			if (playerEnabledButton[num] != NULL)
			{
				if ((num == 0 && game::SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED))
					|| (num == 1 && game::SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED))
					|| (num == 2 && game::SimpleOptions::getBool(DH_OPT_B_3RD_PLAYER_ENABLED))
					|| (num == 3 && game::SimpleOptions::getBool(DH_OPT_B_4TH_PLAYER_ENABLED)))
				{
					playerOn = true;
				}

				playerOn = !playerOn;

				// player 1 always enabled... 
				if (num == 0) playerOn = true;

				if (num == 0)
					game::SimpleOptions::setBool(DH_OPT_B_1ST_PLAYER_ENABLED, playerOn);
				if (num == 1)
					game::SimpleOptions::setBool(DH_OPT_B_2ND_PLAYER_ENABLED, playerOn);
				if (num == 2)
					game::SimpleOptions::setBool(DH_OPT_B_3RD_PLAYER_ENABLED, playerOn);
				if (num == 3)
					game::SimpleOptions::setBool(DH_OPT_B_4TH_PLAYER_ENABLED, playerOn);

				// HACK: prevent same keybinds
				if (num == 1 
					&& game::SimpleOptions::getString(DH_OPT_S_1ST_PLAYER_KEYBINDS) != NULL
					&& game::SimpleOptions::getString(DH_OPT_S_2ND_PLAYER_KEYBINDS) != NULL
					&& strcmp(game::SimpleOptions::getString(DH_OPT_S_1ST_PLAYER_KEYBINDS), game::SimpleOptions::getString(DH_OPT_S_2ND_PLAYER_KEYBINDS)) == 0)
					changePlayerKeybind = 1;
			}
		}

		if (eve->triggerButton->GetId() >= OPTIONSW_PLAYERCONTROLLER_FIRST
			&& eve->triggerButton->GetId() < OPTIONSW_PLAYERCONTROLLER_FIRST + OPTIONSW_PLAYERCONTROLLER_AMOUNT)
		{
			int num = eve->triggerButton->GetId() - OPTIONSW_PLAYERCONTROLLER_FIRST;
			assert(num >= 0 && num < MAX_PLAYERS_PER_CLIENT);

			changePlayerKeybind = num;
		}

		if (changePlayerKeybind != -1)
		{
			int num = changePlayerKeybind;

			char *keybinds = "";
			int controlScheme = 0;
			bool hasCursor = false;
			if (num == 0) keybinds = game::SimpleOptions::getString(DH_OPT_S_1ST_PLAYER_KEYBINDS);
			if (num == 1) keybinds = game::SimpleOptions::getString(DH_OPT_S_2ND_PLAYER_KEYBINDS);
			if (num == 2) keybinds = game::SimpleOptions::getString(DH_OPT_S_3RD_PLAYER_KEYBINDS);
			if (num == 3) keybinds = game::SimpleOptions::getString(DH_OPT_S_4TH_PLAYER_KEYBINDS);

			bool keybindValid = false;
			int failcount = 0;
			while (!keybindValid)
			{
				if (keybinds != NULL && keybinds[0] != '\0')
				{
					if (strcmp(keybinds, "Config/keybinds_mouse.txt") == 0)
					{
						keybinds = "Config/keybinds_joypad1.txt";
						controlScheme = 1;
						hasCursor = false;
					}
					else if (strcmp(keybinds, "Config/keybinds_joypad1.txt") == 0)
					{
						keybinds = "Config/keybinds_joypad2.txt";
						controlScheme = 4;
						hasCursor = false;
					}
					else if (strcmp(keybinds, "Config/keybinds_joypad2.txt") == 0)
					{
						keybinds = "Config/keybinds_mouse.txt";
						controlScheme = 0;
						hasCursor = true;
					} else {
						keybinds = "Config/keybinds_mouse.txt";
						controlScheme = 0;
						hasCursor = true;
					}
				} else {
					keybinds = "Config/keybinds_mouse.txt";
					controlScheme = 0;
					hasCursor = true;
				}

				// HACK: prevent same keybinds...
				keybindValid = true;
				if (num == 0 
					&& game::SimpleOptions::getString(DH_OPT_S_2ND_PLAYER_KEYBINDS) != NULL
					&& game::SimpleOptions::getBool(DH_OPT_B_2ND_PLAYER_ENABLED)
					&& strcmp(keybinds, game::SimpleOptions::getString(DH_OPT_S_2ND_PLAYER_KEYBINDS)) == 0)
					keybindValid = false;
				if (num == 1 
					&& game::SimpleOptions::getString(DH_OPT_S_1ST_PLAYER_KEYBINDS) != NULL
					&& game::SimpleOptions::getBool(DH_OPT_B_1ST_PLAYER_ENABLED)
					&& strcmp(keybinds, game::SimpleOptions::getString(DH_OPT_S_1ST_PLAYER_KEYBINDS)) == 0)
					keybindValid = false;

				failcount++;
				if (failcount > 10) break;
			}

			if (num == 0)
			{
				game::SimpleOptions::setString(DH_OPT_S_1ST_PLAYER_KEYBINDS, keybinds);
				game::SimpleOptions::setBool(DH_OPT_B_1ST_PLAYER_HAS_CURSOR, hasCursor);
				game::SimpleOptions::setInt(DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME, controlScheme);
			}
			if (num == 1)
			{
				game::SimpleOptions::setString(DH_OPT_S_2ND_PLAYER_KEYBINDS, keybinds);
				game::SimpleOptions::setBool(DH_OPT_B_2ND_PLAYER_HAS_CURSOR, hasCursor);
				game::SimpleOptions::setInt(DH_OPT_I_2ND_PLAYER_CONTROL_SCHEME, controlScheme);
			}
			if (num == 2)
			{
				game::SimpleOptions::setString(DH_OPT_S_3RD_PLAYER_KEYBINDS, keybinds);
				game::SimpleOptions::setBool(DH_OPT_B_3RD_PLAYER_HAS_CURSOR, hasCursor);
				game::SimpleOptions::setInt(DH_OPT_I_3RD_PLAYER_CONTROL_SCHEME, controlScheme);
			}
			if (num == 3)
			{
				game::SimpleOptions::setString(DH_OPT_S_4TH_PLAYER_KEYBINDS, keybinds);
				game::SimpleOptions::setBool(DH_OPT_B_4TH_PLAYER_HAS_CURSOR, hasCursor);
				game::SimpleOptions::setInt(DH_OPT_I_4TH_PLAYER_CONTROL_SCHEME, controlScheme);
			}
		}
		*/

		if (eve->triggerButton->GetId() == OPTIONSW_DIFFICULTY_LEVEL)
		{
			int difficultyLevel = game->gameScripting->getGlobalIntVariableValue("general_difficulty_level");
			// NOTE: unnecessarily complex to fix possible "round up" errors
			// causing a possible very hard level skip. 
			if (difficultyLevel == 100)
			{
				difficultyLevel = 0;
			} else {
				difficultyLevel += 25;
				if (difficultyLevel > 75)
				{
					difficultyLevel = 25;
				}
				/*
				if (difficultyLevel > 100)
				{
					difficultyLevel = 100;
				}
				*/
			}
			game->gameScripting->setGlobalIntVariableValue("general_difficulty_level", difficultyLevel);
			game->gameScripting->setGlobalIntVariableValue("damage_amount_level", difficultyLevel);
			game->gameScripting->setGlobalIntVariableValue("item_amount_level", difficultyLevel);
			game->gameScripting->setGlobalIntVariableValue("hostile_amount_level", difficultyLevel);
		}

		/*
		if (eve->triggerButton->GetId() == OPTIONSW_AUTOADJUST)
		{
			int autoadjust = game->gameScripting->getGlobalIntVariableValue("autoadjust_difficulty_level");
			if (autoadjust != 0)
			{
				autoadjust = 0;
			} else {
				autoadjust = 1;
			}
			game->gameScripting->setGlobalIntVariableValue("autoadjust_difficulty_level", autoadjust);
		}
		*/

		refresh();
	}

}



#include "precompiled.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string>
#include <sstream>
#include <algorithm>

#include "CharacterSelectionWindow.h"

#include <IStorm3D.h>
#include <istorm3d_videostreamer.h>
#include "../util/assert.h"

#include "uidefaults.h"
#include "../game/Game.h"
#include "../game/GameProfiles.h"
#include "../ui/CombatWindow.h"
#include "../game/GameUI.h"
#include "../game/GameScene.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_players.h"
#include "../game/options/options_video.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiLocaleWrapper.h"
#include "../system/Logger.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_graphics.h"
#include "../game/scripting/GameScripting.h"
#include "../util/LipsyncManager.h"
#include "../sound/SoundMixer.h"
#include "../ogui/OguiAligner.h"

#include "../game/DHLocaleManager.h"

#include "../util/Debug_MemoryManager.h"
#include "../game/scripting/UnitScripting.h"
#include "../game/UnitType.h"
#include "../game/UnitList.h"
#include "../game/UnitSpawner.h"
#include "../game/unittypes.h"
#include "../util/Script.h"
#include "../ui/GameController.h"
#include <keyb3.h>

#include "../survivor/SurvivorConfig.h"

extern int scr_width;
extern int scr_height;

using namespace game;

namespace ui
{
	const char *CharacterSelectionWindow::characterNames[3] = {"Marine", "Napalm", "Sniper"};
	bool CharacterSelectionWindow::characterEnabled[3] = { true, true, true };
	int CharacterSelectionWindow::characterForced = -1;


	CharacterSelectionWindow::CharacterSelectionWindow( Ogui *ogui, game::Game *game ) :
      ogui( ogui ),
	  game( game )
	{
		FB_ASSERT( ogui != NULL );
		FB_ASSERT( game != NULL );
		oguiLoader = new OguiLocaleWrapper(ogui);
		win = oguiLoader->LoadWindow("characterselectionwindow");
		FB_ASSERT( win != NULL );

		frame = 0;

		// header label
		headerText = oguiLoader->LoadButton("header", win, 0);


		characterImages.resize(3);
		characterVideoStreams.resize(3, NULL);
		characterVideos.resize(6, NULL);
		switchToVideoInFrame.resize(3, 0);
		for(int i = 0; i < 3; i++)
		{
			std::string buttonname = characterNames[i];
			// lower case plz
			std::transform(buttonname.begin(),buttonname.end(),buttonname.begin(),(int(*)(int))tolower);

			characterImages[i] = oguiLoader->LoadButton(buttonname, win, i);
#ifdef PROJECT_SURVIVOR
			OguiAligner::align(characterImages[i], OguiAligner::WIDESCREEN_FIX_CENTER, ogui);
#endif

			characterVideos[i + 3] = ogui->LoadOguiImage(getLocaleGuiString(("gui_characterselectionwindow_" + buttonname + "_idle").c_str()));
			characterImages[i]->SetImage(characterVideos[i + 3]);
			characterImages[i]->SetDisabledImage(characterVideos[i + 3]);
			characterImages[i]->SetHighlightedImage(characterVideos[i + 3]);
			characterImages[i]->SetDisabled(true);
		}
		characterButtons.resize(3);
		characterButtonImages.resize(3*2);
		for(unsigned int i = 0; i < 3; i++)
		{
			std::string buttonname = characterNames[i];
			// lower case plz
			std::transform(buttonname.begin(),buttonname.end(),buttonname.begin(),(int(*)(int))tolower);

			characterButtons[i] = oguiLoader->LoadButton(buttonname, win, i);
			characterButtons[i]->SetListener(this);
#ifdef PROJECT_SURVIVOR
			OguiAligner::align(characterButtons[i], OguiAligner::WIDESCREEN_FIX_CENTER, ogui);
#endif

			IOguiImage *image = NULL;
			IOguiImage *imageHigh = NULL;
			characterButtons[i]->GetImages(&image, 0, 0, &imageHigh);
			bool autodel[4];
			characterButtons[i]->GetImageAutoDelete(&autodel[0], &autodel[1], &autodel[2], &autodel[3]);
			characterButtons[i]->SetImageAutoDelete(false, autodel[1], autodel[2], false);
			characterButtonImages[i*2 + 0] = image;
			characterButtonImages[i*2 + 1] = imageHigh;

			if(!characterEnabled[i])
				characterButtons[i]->SetDisabled(true);
		}

		profileButtonOffset = getLocaleGuiInt("gui_characterselectionwindow_profile_offset_y", 20);

		for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			chosenCharacter[i] = -1;
			chosenCharacterLocked[i] = false;
			profileDisplayOrder[i] = i;
		}

		int firstEnabledCharacter = 0;
		for(int i = 0; i < 3; i++)
		{
			if(characterEnabled[i])
			{
				firstEnabledCharacter = i;
				break;
			}
		}

		profileButtons.resize(getNumberOfPlayers());
		for(unsigned int i = 0; i < profileButtons.size(); i++)
		{
			chosenCharacter[i] = firstEnabledCharacter;
			profileButtons[i] = oguiLoader->LoadButton("profile", win, 0);
			profileButtons[i]->SetText(game->getGameProfiles()->getCurrentProfile(i));
			profileButtons[i]->MoveBy(0, profileButtonOffset * i);
#ifdef PROJECT_SURVIVOR
			OguiAligner::align(profileButtons[i], OguiAligner::WIDESCREEN_FIX_CENTER, ogui);
#endif
		}

		chooseSound = getLocaleGuiString( "gui_characterselectionwindow_sound_choose" );
		lockSound = getLocaleGuiString( "gui_characterselectionwindow_sound_lock" );
		errorSound = getLocaleGuiString( "gui_characterselectionwindow_sound_error" );

		mustChooseError = NULL;

		if(characterForced != -1)
		{
			std::string locale = characterNames[characterForced];
			std::transform(locale.begin(),locale.end(),locale.begin(),(int(*)(int))tolower);
			locale = "gui_characterselectionwindow_mustchoose_text_" + locale;
			mustChooseError = oguiLoader->LoadButton("mustchoose", win, 0);
			mustChooseError->SetDisabled(true);
			mustChooseError->SetText(getLocaleGuiString(locale.c_str()));
#ifdef PROJECT_SURVIVOR
			OguiAligner::align(mustChooseError, OguiAligner::WIDESCREEN_FIX_CENTER, ogui);
#endif
		}

		updateProfileButtons();

		readyToCloseTimer = INT_MAX;

#ifdef PROJECT_SURVIVOR
		OguiAligner::align(win, OguiAligner::WIDESCREEN_FIX_CENTER, ogui);
		OguiAligner::align(headerText, OguiAligner::WIDESCREEN_FIX_CENTER, ogui);
#endif
		win->Hide();

#ifndef PROJECT_SURVIVOR_DEMO
		int numChoices = 0;
		for(int i = 0; i < 3; i++)
		{
			if(characterEnabled[i])
			{
				numChoices++;
			}
		}
		// if you can't choose from more than 1
		if(numChoices <= 1)
		{
			// just pick it directly
			closeWindow();
			readyToCloseTimer = game->gameTimer;
		}
#endif
	}

	CharacterSelectionWindow::~CharacterSelectionWindow()
	{
		for(unsigned int i = 0; i < characterVideoStreams.size(); i++)
		{
			delete characterVideoStreams[i];
		}

		for(unsigned int i = 0; i < characterVideos.size(); i++)
		{
			delete characterVideos[i];
		}

		for(unsigned int i = 0; i < characterImages.size(); i++)
		{
			delete characterImages[i];
		}
		for(unsigned int i = 0; i < characterButtonImages.size(); i++)
		{
			delete characterButtonImages[i];
		}
		for(unsigned int i = 0; i < characterButtons.size(); i++)
		{
			delete characterButtons[i];
		}
		for(unsigned int i = 0; i < profileButtons.size(); i++)
		{
			delete profileButtons[i];
		}
		delete mustChooseError;
		delete headerText;
		delete win;
		delete oguiLoader;
	}

	void CharacterSelectionWindow::raise()
	{
		win->Raise();
	}

	void CharacterSelectionWindow::update()
	{
		frame++;
		if(frame == 2)
		{
			win->Show();
		}

		for(unsigned int i = 0; i < switchToVideoInFrame.size(); i++)
		{
			if(switchToVideoInFrame[i] != 0 && frame == switchToVideoInFrame[i])
			{
				characterImages[i]->SetImage(characterVideos[i]);
				characterImages[i]->SetDisabledImage(characterVideos[i]);
				characterImages[i]->SetHighlightedImage(characterVideos[i]);
			}
		}
	}

	void CharacterSelectionWindow::updateProfileButtons()
	{
		for(unsigned int k = 0; k < MAX_PLAYERS_PER_CLIENT; k++)
		{
			// ugly hack to preserve display order
			unsigned int i = profileDisplayOrder[k];
			if(i >= profileButtons.size()) continue;

			if(chosenCharacter[i] != -1)
			{
				// calculate offset
				int totalOffset = 0;
				int offsetY = 0;
				for(unsigned int j = 0; j < MAX_PLAYERS_PER_CLIENT; j++)
				{
					if(chosenCharacter[profileDisplayOrder[j]] == chosenCharacter[i])
					{
						if(j < k)	offsetY += profileButtonOffset;
						if(j != k) totalOffset += profileButtonOffset;
					}
				}
				// center
				offsetY -= totalOffset / 2;

				OguiButton *move_over_button = characterButtons[chosenCharacter[i]];
				profileButtons[i]->Move(move_over_button->GetX() + move_over_button->GetSizeX()/2 - profileButtons[i]->GetSizeX()/2,
																move_over_button->GetY() + move_over_button->GetSizeY()/2 - profileButtons[i]->GetSizeY()/2 + offsetY);

				profileButtons[i]->SetDisabled(chosenCharacterLocked[i]);
			}
		}
	}

	bool CharacterSelectionWindow::canStart()
	{
		bool forced_character_chosen = false;
		for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			if(!SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + i )) continue;

			// all must be locked
			if(!chosenCharacterLocked[i] )
				return false;

			if(chosenCharacter[i] == characterForced)
				forced_character_chosen = true;
		}

		// if forced character is not chosen
		if(!forced_character_chosen && characterForced != -1)
		{
			// unlock everyone
			//for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			//	chosenCharacterLocked[i] = false;
			//updateProfileButtons();

			// change appearance
			if(mustChooseError) mustChooseError->SetDisabled(false);
			game->gameUI->playGUISound(errorSound.c_str());

			return false;
		}

		return true;
	}

	void CharacterSelectionWindow::CursorEvent(OguiButtonEvent *eve)
	{
		// already closing
		if(readyToCloseTimer != INT_MAX) return;

		if(eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			// whoever has mouse can choose by clicking directly
			int i = eve->cursorNumber;
			if(game->gameUI->getController( i )->getBoundKey(DH_CTRL_ATTACK, 0) == KEYCODE_MOUSE_BUTTON1
		    || game->gameUI->getController( i )->getBoundKey(DH_CTRL_ATTACK, 0) == KEYCODE_MOUSE0_BUTTON1
				|| game->gameUI->getController( i )->getBoundKey(DH_CTRL_ATTACK, 0) == KEYCODE_MOUSE1_BUTTON1
				|| game->gameUI->getController( i )->getBoundKey(DH_CTRL_ATTACK, 0) == KEYCODE_MOUSE2_BUTTON1
				|| game->gameUI->getController( i )->getBoundKey(DH_CTRL_ATTACK, 0) == KEYCODE_MOUSE3_BUTTON1)
			{
				// clicking on the same button
				if(chosenCharacter[i] == eve->triggerButton->GetId())
				{
					// lock
					lockChosenCharacter(i);
					updateProfileButtons();
				}
				else
				{
					// choose
					chosenCharacterLocked[i] = false;
					updateCharacterButtonChosen(chosenCharacter[i]);

					int dir = eve->triggerButton->GetId() - chosenCharacter[i];
					chooseCharacterInDir(dir, i);
					updateProfileButtons();
				}
			}
		}
	}

	inline Unit *getNextUnitOfType(const char *name, Unit *unit, Game *game)
	{
		UnitType *ut = getUnitTypeByName((char *)name);
		unit = UnitScripting::nextOwnedUnit(game, Vector(0,0,0), 0, unit, false );
		while (unit != NULL)
		{
			if (unit->getUnitType() == ut)
				break;
			unit = UnitScripting::nextOwnedUnit(game, Vector(0,0,0), 0, unit, false );
		}
		return unit;
	}

	void CharacterSelectionWindow::closeWindow()
	{
		// construct character type strings
		std::string characterTypes[3];
		for(unsigned int i = 0; i < 3; i++)
		{
			characterTypes[i] = std::string("Surv_") + characterNames[i];
		}

		// reset unit id array
		for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			util::Script::setGlobalArrayVariableValue("player_units", i, 0);
		}

		// take-first-free-one logic
		//
		/*// for each character type
		for(int charNum = 0; charNum < 3; charNum++)
		{
			Unit *unit = NULL;

			// for each player
			for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			{
				// player disabled
				if(!SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + i )) continue;

				// player is using this character
				if(chosenCharacter[i] == charNum)
				{
					// get next free unit
					unit = getNextUnitOfType(characterTypes[charNum].c_str(), unit, game);
					if(unit != NULL)
					{
						// set unit id
						util::Script::setGlobalArrayVariableValue("player_units", i, game->units->getIdForUnit(unit));

						// set singleplayer unit id too
						if(i == 0)
							game->gameScripting->setGlobalIntVariableValue("singleplayer_unitid", game->units->getIdForUnit(unit));				
					}
					else
					{
						util::Script::setGlobalArrayVariableValue("player_units", i, 0);
					}
				}
			}
		}*/

		// each player uses their own character copies
		//
		// for each player
		for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			// player disabled
			if(!SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + i )) continue;

			// get the unit
			Unit *unit = NULL;
			for(int c = 0; c <= i; c++)
			{
				unit = getNextUnitOfType(characterTypes[chosenCharacter[i]].c_str(), unit, game);
			}

			if(unit != NULL)
			{
				// set unit id
				util::Script::setGlobalArrayVariableValue("player_units", i, game->units->getIdForUnit(unit));

				// set singleplayer unit id too
				if(i == 0)
					game->gameScripting->setGlobalIntVariableValue("singleplayer_unitid", game->units->getIdForUnit(unit));				
			}
			else
			{
				assert(!"CharacterSelectionWindow::parseChoices - wanted unit not found.");
				util::Script::setGlobalArrayVariableValue("player_units", i, 0);
			}
		}

		// recreate combat windows
		game->gameUI->closeCombatWindow(game->singlePlayerNumber);
		game->gameUI->openCombatWindow(game->singlePlayerNumber, true);

		// run script
		game->gameScripting->runMissionScript("player_selection", "new_character_selected");

		// keep combat windows hidden
		game->gameUI->getCombatWindow(game->singlePlayerNumber)->startGUIModeTempInvisible();

		readyToCloseTimer = game->gameTimer + GAME_TICKS_PER_SECOND * getLocaleGuiInt("gui_characterselectionwindow_wait_before_close", 0) / 1000;
	}

	void CharacterSelectionWindow::parseChoices(const char *params)
	{
		if(params == NULL) return;

		// transform to lower case
		std::string chars = params;
		std::transform(chars.begin(),chars.end(),chars.begin(),(int(*)(int))tolower);

		CharacterSelectionWindow::characterForced = -1;

		// loop through character names
		for(unsigned int i = 0; i < 3; i++)
		{
			// transform to lower case
			std::string name = CharacterSelectionWindow::characterNames[i];
			std::transform(name.begin(),name.end(),name.begin(),(int(*)(int))tolower);

			// find name in string
			const char *pos = strstr(chars.c_str(), name.c_str());
			CharacterSelectionWindow::characterEnabled[i] = pos != NULL;

			// find "force_" before string
			const int force_len = strlen("force_");
			if(pos != NULL && pos - force_len >= chars.c_str() &&
				memcmp(pos - force_len, "force_", force_len) == 0)
			{
				CharacterSelectionWindow::characterForced = i;
			}
		}
	}

	void CharacterSelectionWindow::chooseCharacterInDir(int dir, int player)
	{
		assert(player >= 0 && player < MAX_PLAYERS_PER_CLIENT);
		
		// already closing
		if(readyToCloseTimer != INT_MAX) return;
		
		// locked
		if(chosenCharacterLocked[player]) return;

		int oldChoice = chosenCharacter[player];

		// no character chosen yet
		if(chosenCharacter[player] == -1)
		{
			// choose the middle one
			if(dir == 0) 
				chosenCharacter[player] = 1;
			// choose the right one
			else if(dir == 1)
				chosenCharacter[player] = 2;
			// choose the left one
			else if(dir == -1)
				chosenCharacter[player] = 0;
		}
		else
		{
			// choose whatever was pointed to
			chosenCharacter[player] += dir;
		}

		// clamp
		if(chosenCharacter[player] < 0) chosenCharacter[player] = 0;
		else if(chosenCharacter[player] > 2) chosenCharacter[player] = 2;

		// character not enabled
		if(!characterEnabled[chosenCharacter[player]])
		{
			// keep the same choise
			chosenCharacter[player] = oldChoice;
			game->gameUI->playGUISound(errorSound.c_str());
		}
		
		
		if(chosenCharacter[player] != oldChoice)
		{
			game->gameUI->playGUISound(chooseSound.c_str());

			// hack: last moved goes to bottom of list
			for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
			{
				if(profileDisplayOrder[i] == player)
				{
					for(int j = i + 1; j < MAX_PLAYERS_PER_CLIENT; j++)
					{
						profileDisplayOrder[j - 1] = profileDisplayOrder[j];
					}
					break;
				}
			}
			profileDisplayOrder[MAX_PLAYERS_PER_CLIENT - 1] = player;
		}
	}

	void CharacterSelectionWindow::updateCharacterButtonChosen(int charNum)
	{
		// check if at least one player has chosen this character and locked it
		bool chosen = false;
		for(unsigned int i = 0; i < profileButtons.size(); i++)
		{
			if(chosenCharacter[i] == charNum && chosenCharacterLocked[i])
			{
				chosen = true;
				break;
			}
		}

		std::string characterName = characterNames[charNum] + std::string("Selection");
		std::string animName;
		if(chosen)
		{
			animName = characterNames[charNum] + std::string("_selected");
			std::transform(animName.begin(),animName.end(),animName.begin(),(int(*)(int))tolower);


			IOguiImage *downImage = NULL;
			characterButtons[charNum]->GetImages( 0, &downImage, 0, 0);
			characterButtons[charNum]->SetImage( downImage );
			characterButtons[charNum]->SetHighlightedImage( downImage );
		}
		else
		{
			animName = characterNames[charNum] + std::string("_idle");
			std::transform(animName.begin(),animName.end(),animName.begin(),(int(*)(int))tolower);

			characterButtons[charNum]->SetImage( characterButtonImages[charNum*2 + 0] );
			characterButtons[charNum]->SetHighlightedImage( characterButtonImages[charNum*2 + 1] );
		}

		if(SimpleOptions::getBool( DH_OPT_B_VIDEO_ENABLED ) && chosen)
		{
			sfx::SoundMixer *mixer = game->gameUI->getSoundMixer();
			IStorm3D_StreamBuilder *builder = 0;
			if(mixer)
				builder = mixer->getStreamBuilder();

			delete characterVideoStreams[charNum];
			delete characterVideos[charNum];

			characterVideoStreams[charNum] = game->gameScene->getStorm3D()->CreateVideoStreamer( ("Data/Videos/" + animName + ".wmv").c_str(), builder, !chosen );
			characterVideos[charNum] = ogui->ConvertVideoToImage( characterVideoStreams[charNum], builder );

			switchToVideoInFrame[charNum] = frame + 2;
			characterImages[charNum]->SetDisabled(true);
		}
		else
		{
			characterImages[charNum]->SetImage(characterVideos[charNum + 3]);
			characterImages[charNum]->SetDisabledImage(characterVideos[charNum + 3]);
			characterImages[charNum]->SetHighlightedImage(characterVideos[charNum + 3]);
			characterImages[charNum]->SetDisabled(true);
		}
	}
	
	void CharacterSelectionWindow::lockChosenCharacter(int player)
	{
		assert(player >= 0 && player < MAX_PLAYERS_PER_CLIENT);

		// already closing
		if(readyToCloseTimer != INT_MAX) return;

		// can't lock without choosing, duh
		if(chosenCharacter[player] == -1) return;

		// lock
		chosenCharacterLocked[player] = !chosenCharacterLocked[player];
		game->gameUI->playGUISound(lockSound.c_str());

		updateCharacterButtonChosen(chosenCharacter[player]);

		// all ready - go
		if(canStart()) closeWindow();
	}

	void CharacterSelectionWindow::forceStart()
	{
		// already closing
		if(readyToCloseTimer != INT_MAX)
		{
			// quick start
			readyToCloseTimer = game->gameTimer;
			return;
		}

		// lock all
		bool forced_character_chosen = false;
		for(int i = 0; i < MAX_PLAYERS_PER_CLIENT; i++)
		{
			if(!SimpleOptions::getBool( DH_OPT_B_1ST_PLAYER_ENABLED + i )) continue;

			chosenCharacterLocked[i] = true;

			if(chosenCharacter[i] == characterForced)
				forced_character_chosen = true;
		}

		for(unsigned int i = 0; i < characterButtons.size(); i++)
		{
			updateCharacterButtonChosen(i);
		}

		updateProfileButtons();

		// if forced character is not chosen
		/*if(!forced_character_chosen && characterForced != -1)
		{
			// randomly pick someone to fill the hole of doom
			int player = rand()%getNumberOfPlayers();
			chosenCharacter[player] = characterForced;
		}*/

	  // all ready - go
		if(canStart()) closeWindow();
	}

	bool CharacterSelectionWindow::shouldClose()
	{
		return game->gameTimer > readyToCloseTimer;
	}
} // end of namespace game


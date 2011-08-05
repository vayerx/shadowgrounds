
#include "precompiled.h"

#include <assert.h>
#include <limits.h>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <SDL.h>

#include "SurvivorLoadGameMenu.h"
#include "LoadGameMenu.h"

#include "../system/Timer.h"
#include "../ogui/Ogui.h"
#include "../game/Game.h"
#include "../game/savegamevars.h"
#include "../game/GameUI.h"
#include "MenuCollection.h"
#include "../game/DHLocaleManager.h"
#include "../ui/GameController.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_game.h"
#include "../game/GameProfiles.h"
#include "../filesystem/input_stream_wrapper.h"

#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"
#include "../util/StringUtil.h"

#include "../ui/LoadingMessage.h"

#include "../ogui/OguiLocaleWrapper.h"
#include "../ogui/OguiCheckBox.h"
#include "../ogui/OguiFormattedText.h"

#include "CoopMenu.h"
#include "NewGameMenu.h"
#include "../game/BonusManager.h"

using namespace game;
using namespace frozenbyte;

namespace ui
{
	
SurvivorLoadGameMenu::SurvivorLoadGameMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, Game* g ) :
	MenuBaseImpl( NULL ),
	menuCollection( menu ),
	fonts( fonts ),
	bonusOptionWindow( NULL ),
	doubleClickHack( -1 ),
	doubleClickTimeHack( 0 ),
	//faderLeft( NULL ),
	//faderRight( NULL ),
	//faderBg( NULL ),
	lastValidMission( 0 ),
	firstMission( 0 ),
	missionButtonScrollAmount( 0 ),
	holdingScroll( false ),
	startedHoldingScroll( -1 )
{
	ogui = o_gui;
	game = g;

	menu->setBackgroundImage( getLocaleGuiString( "gui_survivorloadgamemenu_background_image" ) );

	// get last savegame
	//
	std::string filename = std::string(game->getGameProfiles()->getProfileDirectory( 0 ));
	if(LoadGameMenu::startAsCoop)
		filename += "/Save/lastsave_coop.txt";
	else
		filename += "/Save/lastsave.txt";

	filesystem::FB_FILE *f = filesystem::fb_fopen(filename.c_str(), "rb");
	if (f != NULL)
	{
		char buf[32];
		memset(buf, 0, 32);
		int length = filesystem::fb_fread(buf, 1, 31, f);
		buf[length] = 0;
		if(sscanf(buf, "%i", &firstMission) == 1)
		{
			firstMission--;
		}
		filesystem::fb_fclose(f);
	}
	if(firstMission < 0)
	{
		firstMission = 0;
	}

	win = ogui->CreateSimpleWindow( getLocaleGuiInt( "gui_survivorloadgamemenu_window_x", 0 ),
		getLocaleGuiInt( "gui_survivorloadgamemenu_window_y", 0 ),
		getLocaleGuiInt( "gui_survivorloadgamemenu_window_w", 1024 ),
		getLocaleGuiInt( "gui_survivorloadgamemenu_window_h", 768 ), NULL );

	win->Hide();
	win->SetUnmovable();
	loader = new OguiLocaleWrapper(ogui);
	loader->SetWindowName("survivorloadgamemenu");

	/*// fader background
	{
		const int x = getLocaleGuiInt( "gui_survivorloadgamemenu_fader_bg_x", 0 );
		const int y = getLocaleGuiInt( "gui_survivorloadgamemenu_fader_bg_y", 0 );
		const int w = getLocaleGuiInt( "gui_survivorloadgamemenu_fader_bg_w", 0 );
		const int h = getLocaleGuiInt( "gui_survivorloadgamemenu_fader_bg_h", 0 );
		const char *img_name = getLocaleGuiString( "gui_survivorloadgamemenu_fader_bg_img" );

		faderBg = ogui->CreateSimpleImageButton(win, x, y, w, h, NULL, NULL, NULL, img_name);
		faderBg->SetDisabled(true);
	}*/

	// add header
	//
	{
		headerTextX = getLocaleGuiInt( "gui_survivorloadgamemenu_header_x", getLocaleGuiInt( "gui_menu_common_header_x", 0 ) );
		headerTextY = getLocaleGuiInt( "gui_survivorloadgamemenu_header_y", getLocaleGuiInt( "gui_menu_common_header_y", 0 ) );

		if(LoadGameMenu::startAsCoop)
			addHeaderText( getLocaleGuiString( "gui_lgm_header_coop" ), fonts->big.normal );
		else
			addHeaderText( getLocaleGuiString( "gui_lgm_header" ), fonts->big.normal );
	}

	// add load button
	//
	{
		const int x = getLocaleGuiInt( "gui_survivorloadgamemenu_loadbutton_x", 0 );
		const int y = getLocaleGuiInt( "gui_survivorloadgamemenu_loadbutton_y", 0 );
		const int w = getLocaleGuiInt( "gui_survivorloadgamemenu_loadbutton_w", 0 );
		const int h = getLocaleGuiInt( "gui_survivorloadgamemenu_loadbutton_h", 0 );

		buttonX = x;
		buttonY = y;
		buttonW = w;
		buttonH = h;
		OguiButton *but = addButton( getLocaleGuiString( "gui_lgm_load" ), COMMANDS_LOAD, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, OguiButton::TEXT_H_ALIGN_LEFT );
		but->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
	}

	// add bonus options button
	//
	if(game->bonusManager->getNumOptions() > 0 && game->bonusManager->areOptionsAvailable())
	{
		const int x = getLocaleGuiInt( "gui_survivorloadgamemenu_bonusoptionsbutton_x", 0 );
		const int y = getLocaleGuiInt( "gui_survivorloadgamemenu_bonusoptionsbutton_y", 0 );
		const int w = getLocaleGuiInt( "gui_survivorloadgamemenu_bonusoptionsbutton_w", 0 );
		const int h = getLocaleGuiInt( "gui_survivorloadgamemenu_bonusoptionsbutton_h", 0 );

		buttonX = x;
		buttonY = y;
		buttonW = w;
		buttonH = h;
		addButton( getLocaleGuiString( "gui_lgm_bonusoptions" ), COMMANDS_BONUSOPTIONS, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, OguiButton::TEXT_H_ALIGN_LEFT );
	}

	// load images
	int time_loading_start = SDL_GetTicks();
	int mission_max = SimpleOptions::getInt(DH_OPT_I_SAVEGAME_SLOT_AMOUNT);
	bool show_loading_bar = false;
	for(int savegame_id = 1; ;savegame_id++)
	{
		std::string temp = std::string( "gui_survivorloadgamemenu_mission_" ) + boost::lexical_cast< std::string >( savegame_id ) + "_image_norm";

		if(!::DHLocaleManager::getInstance()->hasString( ::DHLocaleManager::BANK_GUI, temp.c_str() ))
		{
			break;
		}

		const char *image_norm = getLocaleGuiString( temp.c_str() );

		/*temp = std::string( "gui_survivorloadgamemenu_mission_" ) + boost::lexical_cast< std::string >( savegame_id ) + "_image_high";
		const char *image_high = getLocaleGuiString( temp.c_str() );

		temp = std::string( "gui_survivorloadgamemenu_mission_" ) + boost::lexical_cast<std::string >( savegame_id ) + "_image_down";
		const char *image_down = getLocaleGuiString( temp.c_str() );

		temp = std::string( "gui_survivorloadgamemenu_mission_" ) + boost::lexical_cast< std::string >( savegame_id ) + "_image_disa";
		const char *image_disabled = getLocaleGuiString( temp.c_str() );

		temp = std::string( "gui_survivorloadgamemenu_mission_" ) + boost::lexical_cast< std::string >( savegame_id ) + "_image_selected_norm";
		const char *image_selected_norm = getLocaleGuiString( temp.c_str() );

		temp = std::string( "gui_survivorloadgamemenu_mission_" ) + boost::lexical_cast< std::string >( savegame_id ) + "_image_selected_high";
		const char *image_selected_high = getLocaleGuiString( temp.c_str() );*/

		Thumbnail tb;
		tb.normal = ogui->LoadOguiImage( image_norm );
		thumbnails.push_back(tb);

		if(savegame_id == 5)
		{
			// loading first 5 entries took longer than 100 ms
			if(SDL_GetTicks() - time_loading_start > 100)
			{
				// show loading bar
				show_loading_bar = true;
			}
		}

		if(savegame_id%5 == 0 && show_loading_bar)
		{
			SET_LOADING_BAR_TEXT(getLocaleGuiString("gui_loadingbar_loading"));
			SHOW_LOADING_BAR(savegame_id*100/mission_max);
		}
	}

	// create mission buttons
	{
		const int x = getLocaleGuiInt( "gui_survivorloadgamemenu_missionimage_x", 0 );
		const int y = getLocaleGuiInt( "gui_survivorloadgamemenu_missionimage_y", 0 );
		const int w = getLocaleGuiInt( "gui_survivorloadgamemenu_missionimage_w", 0 );
		const int h = getLocaleGuiInt( "gui_survivorloadgamemenu_missionimage_h", 0 );

		missionButtonStartPosX = x;
		missionButtonStartPosY = y;
		missionButtonOffsetX = getLocaleGuiInt( "gui_survivorloadgamemenu_missionimage_offset", 10 );
	
		originalInfoText = getLocaleGuiString("gui_survivorloadgamemenu_infotext_text");

		for(int i = 0; i < 4; i++)
		{
			missionInfos[i].button = ogui->CreateSimpleImageButton( win, x, y, w, h, NULL, NULL, NULL, NULL, COMMANDS_MISSION, 0, false );
			missionInfos[i].button->SetEventMask(OguiButtonEvent::EVENT_TYPE_CLICK | OguiButtonEvent::EVENT_TYPE_OVER | OguiButtonEvent::EVENT_TYPE_LEAVE );
			missionInfos[i].button->SetListener(this);
			missionInfos[i].button->SetClipToWindow(false);
			missionInfos[i].button->Move(missionButtonStartPosX + missionButtonOffsetX * i, missionButtonStartPosY);
			missionInfos[i].highlighted = false;
			missionInfos[i].highlight_button = ogui->CreateSimpleImageButton( win, x, y, w, h, NULL, NULL, NULL, getLocaleGuiString("gui_survivorloadgamemenu_mission_highlight"), 0, 0, false );
			missionInfos[i].highlight_button->SetDisabled(true);
			loadMissionInfo(&missionInfos[i], firstMission + i);
		}
		updateHighlightButtons();

		// loading latest savegame failed, so better scroll to beginning
		if(missionInfos[1].empty && firstMission != 0)
		{
			firstMission = 0;
			for(int i = 0; i < 4; i++)
			{
				loadMissionInfo(&missionInfos[i], firstMission + i);
			}
		}

		
		missionButtonSwapLeft = x - missionButtonOffsetX;
		missionButtonSwapRight = x + missionButtonOffsetX * 3;
	}

	// create faders
	{
		const int x = getLocaleGuiInt( "gui_survivorloadgamemenu_fader_left_x", 0 );
		const int w = getLocaleGuiInt( "gui_survivorloadgamemenu_fader_left_w", 0 );
		fadeLeftStart = x + w;
		missionButtonClipLeft = x + 2;
	}
	{
		const int x = getLocaleGuiInt( "gui_survivorloadgamemenu_fader_right_x", 0 );
		const int w = getLocaleGuiInt( "gui_survivorloadgamemenu_fader_right_w", 0 );
		fadeRightStart = x;
		missionButtonClipRight = x + w - 2;
	}

	// scroll buttons
	{
		const int x = getLocaleGuiInt( "gui_survivorloadgamemenu_arrow_next_x", 0 );
		const int y = getLocaleGuiInt( "gui_survivorloadgamemenu_arrow_next_y", 0 );
		const int w = getLocaleGuiInt( "gui_survivorloadgamemenu_arrow_next_w", 0 );
		const int h = getLocaleGuiInt( "gui_survivorloadgamemenu_arrow_next_h", 0 );

		const char *image_norm = getLocaleGuiString( "gui_survivorloadgamemenu_arrow_next_image_norm" );
		const char *image_down = getLocaleGuiString( "gui_survivorloadgamemenu_arrow_next_image_down" );
		const char *image_high = getLocaleGuiString( "gui_survivorloadgamemenu_arrow_next_image_high" );
		const char *image_disa = getLocaleGuiString( "gui_survivorloadgamemenu_arrow_next_image_disa" );

		nextButton = ogui->CreateSimpleImageButton(win, x, y, w, h, image_norm, image_down, image_high, image_disa, COMMANDS_NEXT);
		nextButton->SetEventMask(OGUI_EMASK_CLICK|OGUI_EMASK_OVER|OGUI_EMASK_HOLD|OGUI_EMASK_PRESS);
		nextButton->SetListener(this);
	}

	{
		const int x = getLocaleGuiInt( "gui_survivorloadgamemenu_arrow_prev_x", 0 );
		const int y = getLocaleGuiInt( "gui_survivorloadgamemenu_arrow_prev_y", 0 );
		const int w = getLocaleGuiInt( "gui_survivorloadgamemenu_arrow_prev_w", 0 );
		const int h = getLocaleGuiInt( "gui_survivorloadgamemenu_arrow_prev_h", 0 );

		const char *image_norm = getLocaleGuiString( "gui_survivorloadgamemenu_arrow_prev_image_norm" );
		const char *image_down = getLocaleGuiString( "gui_survivorloadgamemenu_arrow_prev_image_down" );
		const char *image_high = getLocaleGuiString( "gui_survivorloadgamemenu_arrow_prev_image_high" );
		const char *image_disa = getLocaleGuiString( "gui_survivorloadgamemenu_arrow_prev_image_disa" );

		prevButton = ogui->CreateSimpleImageButton(win, x, y, w, h, image_norm, image_down, image_high, image_disa, COMMANDS_PREV);
		prevButton->SetEventMask(OGUI_EMASK_CLICK|OGUI_EMASK_OVER|OGUI_EMASK_HOLD|OGUI_EMASK_PRESS);
		prevButton->SetListener(this);
	}

	{
		missionTitleText = loader->LoadButton("missiontitle", win, 0);
		missionTitleText->SetDisabled(true);
		missionTitleText->SetText("");
	}
	{
		missionText = loader->LoadFormattedText("missiondesc", win, 0);
		missionText->setText("");
	}

	/*{
		infoTitle = loader->LoadButton("infotitle", win, 0);
		infoTitle->SetDisabled(true);
	}*/
	infoTitle = NULL;

	{
		infoText = loader->LoadFormattedText("infotext", win, 0);
		infoText->setText("");
	}
	{
		std::string text = getLocaleGuiString("gui_survivorloadgamemenu_profiletext_text");
		profileText = loader->LoadFormattedText("profiletext", win, 0);
		if(LoadGameMenu::startAsCoop)
		{
			profileText->move(getLocaleGuiInt( "gui_survivorloadgamemenu_profiletext_coop_x", 0 ),
												getLocaleGuiInt( "gui_survivorloadgamemenu_profiletext_coop_y", 0 ) );
		}
		text = util::StringReplace("($name)", game->getGameProfiles()->getCurrentProfile( 0 ), text);
		profileText->setText(text.c_str());
	}

	if( game->inCombat )
	{
		closeMenuByEsc = false;
	}
	else
	{
		closeMenuByEsc = true;
		editHandle = game->gameUI->getController(0)->addKeyreader( this );
		debugKeyreader( editHandle, false, "SurvivorLoadGameMenu::SurvivorLoadGameMenu()" );
	}
	clipMissionButtons();
}

SurvivorLoadGameMenu::~SurvivorLoadGameMenu()
{
	if( closeMenuByEsc )
	{
		game->gameUI->getController(0)->removeKeyreader( editHandle );
		debugKeyreader( editHandle, true, "SurvivorLoadGameMenu::~SurvivorLoadGameMenu()" );
	}

	for(unsigned int i = 0; i < thumbnails.size(); i++)
	{
		delete thumbnails[i].normal;
	}

	delete profileText;
	delete infoText;
	//delete infoTitle;

	delete missionTitleText;
	delete missionText;
	//delete faderBg;
	//delete faderRight;
	//delete faderLeft;
	delete missionInfos[0].button;
	delete missionInfos[1].button;
	delete missionInfos[2].button;
	delete missionInfos[3].button;
	delete missionInfos[0].highlight_button;
	delete missionInfos[1].highlight_button;
	delete missionInfos[2].highlight_button;
	delete missionInfos[3].highlight_button;
	delete nextButton;
	delete prevButton;

	// only apply if settings actually exist..
	if(bonusOptionBoxes.size() > 0)
	{
		NewGameMenu::applyBonusOptions(game, bonusOptionBoxes);
	}

	for(unsigned int i = 0; i < bonusOptionButtons.size(); i++)
	{
		delete bonusOptionButtons[i];
	}

	for(unsigned int i = 0; i < bonusOptionBoxes.size(); i++)
	{
		delete bonusOptionBoxes[i];
	}

	std::list<OguiTextLabel*>::iterator it;
	for(it = bonusOptionTexts.end(); it != bonusOptionTexts.end(); it++)
	{
		delete (*it);
	}

	if(bonusOptionWindow)
	{
		delete bonusOptionWindow;
	}

	delete loader;
	delete win;
}

int SurvivorLoadGameMenu::getType() const
{
	return MenuCollection::MENU_TYPE_LOADGAMEMENU;
}

void SurvivorLoadGameMenu::openMenu( int menu )
{
	assert( menuCollection );
	menuCollection->openMenu( menu );
}

void SurvivorLoadGameMenu::closeMenu()
{
	assert( menuCollection );
	menuCollection->closeMenu();
}

void SurvivorLoadGameMenu::applyChanges()
{
}

void SurvivorLoadGameMenu::CursorEvent( OguiButtonEvent* eve )
{
	if(eve->triggerButton->GetId() == COMMANDS_NEXT || eve->triggerButton->GetId() == COMMANDS_PREV)
	{
		// click sound for next/previous buttons on PRESS, not CLICK
		OguiButtonEvent eve2 = *eve;
		
		if(eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
			eve2.eventType = OguiButtonEvent::EVENT_TYPE_PRESS;
		else if(eve->eventType == OguiButtonEvent::EVENT_TYPE_PRESS)
			eve2.eventType = OguiButtonEvent::EVENT_TYPE_CLICK;

		MenuBaseImpl::CursorEvent( &eve2 );
	}
	else
	{
		MenuBaseImpl::CursorEvent( eve );
	}

	holdingScroll = false;

	if( eve->eventType == OguiButtonEvent::EVENT_TYPE_OVER )
	{
		if(eve->triggerButton->GetId() == COMMANDS_MISSION)
		{
			// find button
			for(int i = 0; i < 4; i++)
			{
				if(missionInfos[i].button == eve->triggerButton)
				{
					// set highlight
					missionInfos[i].highlighted = true;
					break;
				}
			}
			updateHighlightButtons();
		}
	}
	else if( eve->eventType == OguiButtonEvent::EVENT_TYPE_LEAVE )
	{
		if(eve->triggerButton->GetId() == COMMANDS_MISSION)
		{
			// find button
			for(int i = 0; i < 4; i++)
			{
				if(missionInfos[i].button == eve->triggerButton)
				{
					// set highlight
					missionInfos[i].highlighted = false;
					break;
				}
			}
			updateHighlightButtons();
		}
	}
	else if( eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK )
	{
		if(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_UP_MASK)
		{
			startedHoldingScroll = Timer::getTime();
			holdingScroll = true;
			scrollMissions(-1);
			holdingScroll = false;
			startedHoldingScroll = -1;
		}
		else if(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_DOWN_MASK)
		{
			startedHoldingScroll = Timer::getTime();
			holdingScroll = true;
			scrollMissions(1);
			holdingScroll = false;
			startedHoldingScroll = -1;
		}
		else if( eve->triggerButton->GetId() >= COMMANDS_BONUSOPTION_BUTTONS )
		{
			game->bonusManager->buttonPressed( eve->triggerButton->GetId() - COMMANDS_BONUSOPTION_BUTTONS );
		}
		else switch( eve->triggerButton->GetId() )
		{
		case COMMANDS_LOAD:
			menuLoad();
			break;

		case COMMANDS_BONUSOPTIONS:
			menuBonusOptions();
			break;

		case COMMANDS_APPLYBONUSOPTIONS:
			menuCloseBonusOptions();
			break;

		case COMMANDS_NEXT:
			scrollMissions(1);
			break;
		
		case COMMANDS_PREV:
			scrollMissions(-1);
			break;

		case COMMANDS_MISSION:
			{
				// not middle button
				if( centerMission == NULL || eve->triggerButton != centerMission->button )
				{
					// scroll so this button is middle
					int middle_position = missionButtonStartPosX + missionButtonOffsetX * 1;
					missionButtonScrollAmount = middle_position - eve->triggerButton->GetX();
					break;
				}

				if( doubleClickHack == eve->triggerButton->GetId() && ( Timer::getTime() - doubleClickTimeHack ) < 500  )
				{
					menuLoad();
				} else {
					MenuBaseImpl::CursorEvent( eve );
					doubleClickHack = eve->triggerButton->GetId();
					doubleClickTimeHack = Timer::getTime();
				}
				return;
			}

		default:
			break;
		}
	}
	if(eve->eventType == OguiButtonEvent::EVENT_TYPE_PRESS || eve->eventType == OguiButtonEvent::EVENT_TYPE_HOLD)
	{
		switch( eve->triggerButton->GetId() )
		{
		case COMMANDS_NEXT:
			{
				if(startedHoldingScroll < 0)
					startedHoldingScroll = Timer::getTime();
				holdingScroll = true;
				scrollMissions(1);
			}
			break;
		
		case COMMANDS_PREV:
			{
				if(startedHoldingScroll < 0)
					startedHoldingScroll = Timer::getTime();
				holdingScroll = true;
				scrollMissions(-1);
			}
			break;

		default:
			break;
		}
	}
	if(!holdingScroll)
	{
		startedHoldingScroll = -1;
	}
}

void SurvivorLoadGameMenu::update()
{
	static int lastUpdate = 0;
	int delta_time = 1;
	if(lastUpdate != 0)
	{
		delta_time = 3*(Timer::getTime() - lastUpdate)/4;

		// really fast scrolling
		if(holdingScroll && startedHoldingScroll > 0)
		{
			float factor = (Timer::getTime() - startedHoldingScroll) / 250.0f;
			if(factor < 1.0f)
				factor = 1.0f;
			delta_time = (int)(delta_time * factor);
		}

		if(delta_time < 1)
			delta_time = 1;
	}
	lastUpdate = Timer::getTime();

	int dir = 0;
	if(missionButtonScrollAmount < 0)
	{
		dir = -delta_time;
		if(missionButtonScrollAmount - dir > 0)
			dir = missionButtonScrollAmount;
	}
	if(missionButtonScrollAmount > 0)
	{
		dir = delta_time;
		if(missionButtonScrollAmount - dir < 0)
			dir = missionButtonScrollAmount;
	}
	missionButtonScrollAmount -= dir;

	// mission is centered
	if(missionButtonScrollAmount == 0 && !holdingScroll)
	{
		// find center button
		MissionInfo *mi = NULL;
		int smallest = INT_MAX;
		int middle_position = missionButtonStartPosX + missionButtonOffsetX * 1;
		for(int i = 0; i < 4; i++)
		{
			int dist = middle_position - missionInfos[i].button->GetX();
			if(dist * dist <= smallest)
			{
				smallest = dist * dist;
				mi = &missionInfos[i];
				missionButtonScrollAmount = dist;
			}
		}

		// close enough to center
		if(smallest <= 2)
		{
			// just force to center
			for(int i = 0; i < 4; i++)
			{
				missionInfos[i].button->MoveBy(missionButtonScrollAmount, 0);
			}
			missionButtonScrollAmount = 0;
		}

		centerMission = mi;
		if(missionButtonScrollAmount == 0 && mi != NULL)
		{
			//mi->button->SetSelected(true);
			missionTitleText->SetText(mi->title.c_str());
			missionText->setText(mi->desc.c_str());
			infoText->setText(mi->gamestats.c_str());
		}
	}
	else
	{
		centerMission = NULL;
		for(int i = 0; i < 4; i++)
		{
			//if(missionInfos[i].button->IsSelected())
				//missionInfos[i].button->SetSelected(false);
		}
		missionTitleText->SetText("");
		missionText->setText("");
		infoText->setText("");
	}

	for(int i = 0; i < 4; i++)
	{
		missionInfos[i].button->MoveBy(dir, 0);
	}

	// left most button scrolled out
	if(missionInfos[0].button->GetX() < missionButtonSwapLeft)
	{
		MissionInfo swap = missionInfos[0];
		for(int i = 0; i < 3; i++)
		{
			missionInfos[i] = missionInfos[i+1];
		}
		missionInfos[3] = swap;

		for(int i = 0; i < 4; i++)
		{
			missionInfos[i].button->Move(missionButtonStartPosX + missionButtonOffsetX * i, missionButtonStartPosY);
		}

		firstMission++;
		loadMissionInfo(&missionInfos[3], firstMission + 3);
	}
	// right most button scrolled out
	else if(missionInfos[3].button->GetX() > missionButtonSwapRight)
	{
		MissionInfo swap = missionInfos[3];
		for(int i = 3; i > 0; i--)
		{
			missionInfos[i] = missionInfos[i-1];
		}
		missionInfos[0] = swap;

		for(int i = 0; i < 4; i++)
		{
			missionInfos[i].button->Move(missionButtonStartPosX + missionButtonOffsetX * (i - 1), missionButtonStartPosY);
		}

		firstMission--;
		loadMissionInfo(&missionInfos[0], firstMission + 0);
	}

	clipMissionButtons();
	updateHighlightButtons();

	MenuBaseImpl::update();
}

inline void addQuad(OguiButton::Vertex *vertices, int x, int y, int size_x, int size_y, unsigned char alpha_left, unsigned char alpha_right)
{
	vertices[0].color = 0xFFFFFF | (alpha_left << 24);
	vertices[0].x = x;
	vertices[0].y = y;

	vertices[1].color = 0xFFFFFF | (alpha_right << 24);
	vertices[1].x = x + size_x;
	vertices[1].y = y + size_y;

	vertices[2].color = 0xFFFFFF | (alpha_left << 24);
	vertices[2].x = x;
	vertices[2].y = y + size_y;

	vertices[3].color = 0xFFFFFF | (alpha_left << 24);
	vertices[3].x = x;
	vertices[3].y = y;

	vertices[4].color = 0xFFFFFF | (alpha_right << 24);
	vertices[4].x = x + size_x;
	vertices[4].y = y;

	vertices[5].color = 0xFFFFFF | (alpha_right << 24);
	vertices[5].x = x + size_x;
	vertices[5].y = y + size_y;
}

inline unsigned char getAlphaAtPosition(int point, int fade_start, int fade_end)
{
	float fade_amount = 255.0f * (point - fade_start) / (float)(fade_end - fade_start);
	if(fade_amount < 0.0f)
		fade_amount = 0.0f;
	else if(fade_amount > 255.0f)
		fade_amount = 255.0f;
	return (unsigned char)fade_amount;
}

void SurvivorLoadGameMenu::clipMissionButtons()
{
	for(int i = 0; i < 4; i++)
	{
		float clip_left = 0.0f;
		int deltaLeft = missionButtonClipLeft - missionInfos[i].button->GetX();
		if(deltaLeft > 0)
		{
			clip_left = deltaLeft / (float)missionInfos[i].button->GetSizeX();
			if(clip_left > 1)
			{
				clip_left = 1;
			}
		}

		float clip_right = 1.0f;
		int deltaRight = missionInfos[i].button->GetX() + missionInfos[i].button->GetSizeX() - missionButtonClipRight;
		if(deltaRight > 0)
		{
			clip_right = 1 - deltaRight / (float)missionInfos[i].button->GetSizeX();
			if(clip_right < 0)
			{
				clip_right = 0;
			}
		}

		missionInfos[i].button->SetClip(clip_left * 100.0f, 0.0f, clip_right * 100.0f, 100.0f);

		

		int pos_x = missionInfos[i].button->GetX();
		//int pos_y = missionInfos[i].button->GetY();
		int size_x = missionInfos[i].button->GetSizeX();
		int size_y = missionInfos[i].button->GetSizeY();

		///////////////////////
		// LEFT FADER
		//

		// case 1:
		//
		// clip    fader
		//  |    IIII|IIIIIII
		//
		if(pos_x <= fadeLeftStart
			&& pos_x > missionButtonClipLeft
			&& pos_x + size_x > fadeLeftStart)
		{
			unsigned char alpha = getAlphaAtPosition(pos_x, missionButtonClipLeft, fadeLeftStart);
			unsigned char alpha2 = getAlphaAtPosition(pos_x + size_x, missionButtonClipLeft, fadeLeftStart);

			int split_1 = fadeLeftStart - pos_x;
			addQuad(&vertices[0],        0, 0,          split_1, size_y,  alpha, alpha2);
			addQuad(&vertices[6],  split_1, 0, size_x - split_1, size_y, alpha2, alpha2);
			missionInfos[i].button->SetCustomShape(vertices, 12);
		}

		// case 2:
		//
		// clip    fader
		//  | IIIIII |
		//
		else if(pos_x < fadeLeftStart
						&& pos_x >= missionButtonClipLeft
						&& pos_x + size_x <= fadeLeftStart)
		{
			unsigned char alpha = getAlphaAtPosition(pos_x, missionButtonClipLeft, fadeLeftStart);
			unsigned char alpha2 = getAlphaAtPosition(pos_x + size_x, missionButtonClipLeft, fadeLeftStart);

			//int split_1 = fadeLeftStart - pos_x;
			addQuad(&vertices[0], 0, 0, size_x, size_y, alpha, alpha2);
			missionInfos[i].button->SetCustomShape(vertices, 6);
		}

		// case 3:
		//
		// clip    fader
		//II|IIIII  |
		//
		else if(pos_x <= missionButtonClipLeft
						&& pos_x + size_x >= missionButtonClipLeft
						&& pos_x + size_x <= fadeLeftStart)
		{
			unsigned char alpha2 = getAlphaAtPosition(pos_x + size_x, missionButtonClipLeft, fadeLeftStart);

			int split_1 = missionButtonClipLeft - pos_x;
			addQuad(&vertices[0], split_1, 0, size_x - split_1, size_y, 0, alpha2);
			missionInfos[i].button->SetCustomShape(vertices, 6);
		}

		// case 4:
		//
		// clip    fader
		//II|IIIIIII|II
		//
		else if(pos_x < missionButtonClipLeft
						&& pos_x + size_x > fadeLeftStart)
		{
			int split_1 = missionButtonClipLeft - pos_x;
			int split_2 = fadeLeftStart - pos_x;
			addQuad(&vertices[0], split_1,  0, split_2 - split_1, size_y,  0, 255);
			addQuad(&vertices[6], split_2,  0, size_x - split_2, size_y, 255, 255);
			missionInfos[i].button->SetCustomShape(vertices, 12);
		}

		// case 5:
		//
		// completely clipped
		//
		else if(pos_x <= missionButtonClipLeft && pos_x + size_x <= missionButtonClipLeft)
		{
			// dummy shape
			memset(vertices, 0, sizeof(OguiButton::Vertex) * 3);
			missionInfos[i].button->SetCustomShape(&vertices[0], 3);
		}

		///////////////////////
		// RIGHT FADER
		//

		// case 1:
		//
		//   fader     clip
		// IIII|IIIII   |
		//
		else if(pos_x + size_x >= fadeRightStart
						&& pos_x + size_x <= missionButtonClipRight
						&& pos_x <= fadeRightStart)
		{
			unsigned char alpha = 255 - getAlphaAtPosition(pos_x, fadeRightStart, missionButtonClipRight);
			unsigned char alpha2 = 255 - getAlphaAtPosition(pos_x + size_x, fadeRightStart, missionButtonClipRight);

			int split_1 = fadeRightStart - pos_x;
			addQuad(&vertices[0],        0, 0,          split_1, size_y, alpha, alpha);
			addQuad(&vertices[6],  split_1, 0, size_x - split_1, size_y, alpha, alpha2);
			missionInfos[i].button->SetCustomShape(vertices, 12);
		}

		// case 2:
		//
		//   fader     clip
		//    |  IIIII  |
		//
		else if(pos_x + size_x >= fadeRightStart
						&& pos_x + size_x <= missionButtonClipRight
						&& pos_x >= fadeRightStart)
		{
			unsigned char alpha = 255 - getAlphaAtPosition(pos_x, fadeRightStart, missionButtonClipRight);
			unsigned char alpha2 = 255 - getAlphaAtPosition(pos_x + size_x, fadeRightStart, missionButtonClipRight);

			addQuad(&vertices[0], 0, 0, size_x, size_y, alpha, alpha2);
			missionInfos[i].button->SetCustomShape(vertices, 6);
		}

		// case 3:
		//
		//   fader     clip
		//    |    IIIII|III
		//
		else if(pos_x + size_x >= missionButtonClipRight
						&& pos_x >= fadeRightStart
						&& pos_x <= missionButtonClipRight)
		{
			int split_1 = missionButtonClipRight - pos_x;
			unsigned char alpha2 = 255 - getAlphaAtPosition(pos_x, fadeRightStart, missionButtonClipRight);
			addQuad(&vertices[0], 0,  0, split_1, size_y, alpha2, 0);
			missionInfos[i].button->SetCustomShape(vertices, 6);
		}

		// case 4:
		//
		//   fader     clip
		//  III|IIIIIIII|III
		//
		else if(pos_x + size_x >= missionButtonClipRight
						&& pos_x <= fadeRightStart)
		{
			int split_1 = fadeRightStart - pos_x;
			int split_2 = missionButtonClipRight - pos_x;
			addQuad(&vertices[0], split_1,  0, split_2 - split_1, size_y, 255, 0);
			addQuad(&vertices[6], 0,  0, split_1, size_y, 255, 255);
			missionInfos[i].button->SetCustomShape(vertices, 12);
		}

		// case 5:
		//
		// completely clipped
		//
		else if(pos_x >= missionButtonClipRight && pos_x + size_x >= missionButtonClipRight)
		{
			// dummy shape
			memset(vertices, 0, sizeof(OguiButton::Vertex) * 3);
			missionInfos[i].button->SetCustomShape(&vertices[0], 3);
		}

		// case 0:
		//
		// not inside any fader
		//
		else if(pos_x >= fadeLeftStart && pos_x + size_x <= fadeRightStart)
		{
			missionInfos[i].button->SetCustomShape(NULL, 0);
		}
	}
}

void SurvivorLoadGameMenu::scrollMissions(int dir)
{
	// find closest candidate for center button
	// taking into account the direction of scrolling
	int center_mission = 0;
	int smallest = 10000;
	int middle_position = missionButtonStartPosX + missionButtonOffsetX * 1;
	for(int i = 0; i < 4; i++)
	{
		int dist = middle_position - missionInfos[i].button->GetX();
		if(dir > 0 && dist > 0)
			continue;
		if(dir < 0 && dist < 0)
			continue;

		if(dist * dist <= smallest * smallest)
		{
			smallest = dist;
			center_mission = i;
		}
	}

	// scroll to that button
	missionButtonScrollAmount = smallest;

	if(!holdingScroll)
		return;

	if(center_mission >= 4)
		return;

	if(dir < 0 && missionInfos[center_mission-1].empty)
		return;

	if(dir > 0 && missionInfos[center_mission+1].empty)
		return;

	// if distance is smaller than half the offset
	if(missionButtonScrollAmount * missionButtonScrollAmount < missionButtonOffsetX * missionButtonOffsetX / 2)
	{
		// scroll one further
		missionButtonScrollAmount -= dir * missionButtonOffsetX;
	}
}

bool SurvivorLoadGameMenu::loadMissionInfo(MissionInfo *mi, int savegame_id)
{
	OguiButton *button = mi->button;

	std::stringstream savegame;
	// only show coop saves
	if(LoadGameMenu::startAsCoop)
	{
		savegame << "coop_";
	}
	savegame << savegame_id;
	if(!game->getInfoForSavegame( savegame.str().c_str(), "savegame" ))
	{
		button->SetImage( NULL );
		button->SetDownImage( NULL );
		button->SetHighlightedImage( NULL );
		button->SetDisabledImage( NULL );
		//if(savegame_id > 0)
		{
			button->SetDisabledImage( ogui->LoadOguiImage(getLocaleGuiString("gui_survivorloadgamemenu_missionimage_empty")) );
			button->SetImageAutoDelete( false, false, true, false );
		}
		button->SetSelectedImages( NULL, NULL );
		button->SetDisabled(true);
		mi->title.clear();
		mi->desc.clear();
		mi->gamestats.clear();
		mi->empty = true;
		return false;
	}

	if(savegame_id > lastValidMission)
		lastValidMission = savegame_id;

	mi->empty = false;

	// get title
	//
	std::string titlelocale = ( savegame_mission_id + "_header_text" );
	if(::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI, titlelocale.c_str()))
		mi->title = getLocaleGuiString( titlelocale.c_str() );
	else
		mi->title = "[" + titlelocale + "]";

	// get description
	//
	std::string desclocale = ( savegame_mission_id + "_brief" );
	if(::game::DHLocaleManager::getInstance()->hasString( ::game::DHLocaleManager::BANK_GUI, desclocale.c_str()))
		mi->desc = getLocaleGuiString( desclocale.c_str() );
	else
		mi->desc = "[" + desclocale + "]";

	// get stats
	//
	mi->gamestats = originalInfoText;
	// parse stats string
	char varname[256];
	char value[256];
	memset(varname, '\0', 256);
	memset(value, '\0', 256);
	const char *stats = savegame_stats.c_str();
	while(true)
	{
		// scan for variable
		if(sscanf(stats, "%s %s", varname, value) != 2)
			break;
		// next plz
		stats += strlen(varname) + strlen(value) + 2;
		// replace
		mi->gamestats = util::StringReplace(std::string("($") + varname + std::string(")"), value, mi->gamestats);
	}
	mi->gamestats = util::StringReplace("($savetime)", savegame_time, mi->gamestats);


	Thumbnail tn = { NULL };
	if(savegame_id > 0 && savegame_id - 1 < (int)thumbnails.size())
	{
		tn = thumbnails[savegame_id - 1];
	}

	button->SetImage( tn.normal );
	button->SetDownImage( tn.normal );
	button->SetDisabledImage( tn.normal );
	button->SetHighlightedImage( tn.normal );
	button->SetSelectedImages( NULL, NULL );
	button->SetDisabled(false);
	return true;
}

void SurvivorLoadGameMenu::menuLoad()
{
	int i;
	for(i = 0; i < 4; i++)
	{
		if(centerMission != NULL && missionInfos[i].button == centerMission->button)
		{
			break;
		}
	}
	int missionId = i + firstMission;

	if(LoadGameMenu::startAsCoop)
	{
		CoopMenu::enableCoopGameSettings(game);

		// load coop savegame
		std::string mission = std::string("coop_") + int2str( missionId );
		if(game->loadGame(mission.c_str()))
		{
			LoadGameMenu::startAsCoop = false;
		}
	}
	else
	{
		CoopMenu::disableCoopGameSettings(game);
		menuCollection->loadMission( missionId );
	}

}

void SurvivorLoadGameMenu::menuBonusOptions()
{
	hide();

	// not created yet
	if(bonusOptionWindow == NULL)
	{
		bonusOptionWindow = ogui->CreateSimpleWindow( win->GetPositionX(), win->GetPositionY(), win->GetSizeX(), win->GetSizeY(), NULL );
		bonusOptionWindow->SetUnmovable();

		const int offset_x = getLocaleGuiInt( "gui_survivorloadgamemenu_bonusoptions_offset_x", 0 );
		const int offset_y = getLocaleGuiInt( "gui_survivorloadgamemenu_bonusoptions_offset_y", 0 );
		NewGameMenu::createBonusOptions(game, bonusOptionWindow, ogui, fonts, bonusOptionBoxes, bonusOptionButtons, bonusOptionTexts, this, COMMANDS_BONUSOPTION_BUTTONS, offset_x, offset_y);

		// apply button
		{
			const int x = getLocaleGuiInt( "gui_survivorloadgamemenu_applybonusoptionsbutton_x", 0 );
			const int y = getLocaleGuiInt( "gui_survivorloadgamemenu_applybonusoptionsbutton_y", 0 );
			const int w = getLocaleGuiInt( "gui_survivorloadgamemenu_applybonusoptionsbutton_w", 0 );
			const int h = getLocaleGuiInt( "gui_survivorloadgamemenu_applybonusoptionsbutton_h", 0 );

			OguiButton *but = ogui->CreateSimpleTextButton(bonusOptionWindow, x, y, w, h, "", "", "", getLocaleGuiString( "gui_lgm_applybonusoptions" ), COMMANDS_APPLYBONUSOPTIONS);
			but->SetFont(fonts->medium.normal);
			but->SetHighlightedFont(fonts->medium.highlighted);
			but->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );
			but->SetListener(this);
		}

	}
	else
	{
		// otherwise just show it again
		bonusOptionWindow->Show();
	}
}

void SurvivorLoadGameMenu::menuCloseBonusOptions()
{
	// only apply if settings actually exist..
	if(bonusOptionBoxes.size() > 0)
	{
		NewGameMenu::applyBonusOptions(game, bonusOptionBoxes);
	}

	// don't delete yet - just hide
	if(bonusOptionWindow)
	{
		bonusOptionWindow->Hide();
	}
	show();
}

void SurvivorLoadGameMenu::updateHighlightButtons()
{
	for(int i = 0; i < 4; i++)
	{
		if(missionInfos[i].highlighted && !missionInfos[i].empty)
		{
			missionInfos[i].highlight_button->Move(missionInfos[i].button->GetX(), missionInfos[i].button->GetY());
			float x,y,w,h;
			missionInfos[i].button->GetClip(x,y,w,h);
			missionInfos[i].highlight_button->SetClip(x,y,w,h);
		}
		else
		{
			missionInfos[i].highlight_button->Move(10000,10000);
			missionInfos[i].highlight_button->SetClip(0,0,0,0);
		}
	}

}
}// namespace ui

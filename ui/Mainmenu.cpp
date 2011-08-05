#include "precompiled.h"

#include <boost/lexical_cast.hpp>
#include <assert.h>

#include "Mainmenu.h"

#include "../ogui/Ogui.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/scripting/GameScripting.h"
#include "../game/savegamevars.h"
#include "MenuCollection.h"
#include "../game/DHLocaleManager.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiTypeEffectListener.h"
#include "../game/options/options_game.h"
// #include "../ogui/OguiFormatedText.h"

#include "LoadGameMenu.h"
#ifdef PROJECT_SURVIVOR
	#include "NewGameMenu.h"
	#include "CoopMenu.h"
	#include "SurvivalMenu.h"
	#include "../game/GameProfiles.h"
	#include "../filesystem/input_stream_wrapper.h"
#endif

using namespace game;

namespace ui {
namespace {

bool DoWeHaveAnySaveGames( Game* game )
{
	if (mission_max == 0)
	{
		mission_max = SimpleOptions::getInt(DH_OPT_I_SAVEGAME_SLOT_AMOUNT);
	}

	int i = 0;
	std::string temp = boost::lexical_cast< std::string >( char( 0 ) );
	for( i = 1; i <= mission_max; i++ )
	{
		std::stringstream foo;

		foo << i;
		bool add = game->getInfoForSavegame( foo.str().c_str(), "savegame" );

		if ( add )
			return true;		
	}
	return false;
}

}

MainMenu::MainMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, Game* g, bool from_game ) :
  MenuBaseImpl( NULL ),
  menuCollection( menu ),
  fonts( fonts ),
  abortGame( NULL ),
  abortGameYes( NULL ),
  abortGameNo( NULL ),
  captureAllEvents( NULL ),
  continueButton( NULL ),
  currentActive( NULL ),
#ifdef PROJECT_SURVIVOR
  numberOfButtons( 9 ),
#else
	numberOfButtons( 7 ),
#endif
  fromGame( from_game ),
  quitPressed( false ),
  playerName( "Pertti" )
{
	assert( o_gui );
	assert( menu );
	assert( fonts );
	assert( g );
	game = g;

	ogui = o_gui;
	{
		int x = getLocaleGuiInt( "gui_mainmenu_window_x", 0 );
		int y = getLocaleGuiInt( "gui_mainmenu_window_y", 0 );
		int w = getLocaleGuiInt( "gui_mainmenu_window_w", 1024 );
		int h = getLocaleGuiInt( "gui_mainmenu_window_h", 150 );
		win = ogui->CreateSimpleWindow( x, y, w, h, NULL );
		win->Hide();
		win->SetUnmovable();
	}
	{
		int x = getLocaleGuiInt( "gui_mainmenu_window2_x", 0 );
		int y = getLocaleGuiInt( "gui_mainmenu_window2_y", 0 );
		int w = getLocaleGuiInt( "gui_mainmenu_window2_w", 0 );
		int h = getLocaleGuiInt( "gui_mainmenu_window2_h", 0 );
		win2 = ogui->CreateSimpleWindow( x, y, w, h, NULL );
		win2->Hide();
		win2->SetUnmovable();
	}
	fromGame = game->inCombat;

	// Main menu buttons
	buttonX	= getLocaleGuiInt( "gui_mainmenu_button_x", 65 );
	if ( fromGame )
			buttonY = getLocaleGuiInt( "gui_mainmenu_button_y_fromgame", 226 );
	else	buttonY = getLocaleGuiInt( "gui_mainmenu_button_y", 275 );

	buttonAddX = getLocaleGuiInt( "gui_mainmenu_button_add_x", getLocaleGuiInt( "gui_menu_common_button_add_x", 0 ) );
	buttonAddY = getLocaleGuiInt( "gui_mainmenu_button_add_y", getLocaleGuiInt( "gui_menu_common_button_add_y", 28 ) );
	buttonW = getLocaleGuiInt( "gui_mainmenu_button_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
	buttonH = getLocaleGuiInt( "gui_mainmenu_button_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );

	// Adding the buttons
	if ( fromGame )
	{
		addButton( getLocaleGuiString( "gui_mm_resume" ),	COMMANDS_RESUME,	fonts->medium.normal, fonts->medium.highlighted );
		addSeparator();
	}
	else
	{
		continueButton = addButton( getLocaleGuiString( "gui_mm_continue" ),		COMMANDS_CONTINUE,	fonts->medium.normal, fonts->medium.highlighted );
		checkContinue();
		addSeparator();
	}

	

	addButton( getLocaleGuiString( "gui_mm_new_game" ),		COMMANDS_NEW_GAME,	fonts->medium.normal, fonts->medium.highlighted );
	addButton( getLocaleGuiString( "gui_mm_load_game" ),	COMMANDS_LOAD_GAME, fonts->medium.normal, fonts->medium.highlighted );
#ifdef PROJECT_SURVIVOR
	addButton( getLocaleGuiString( "gui_mm_survival" ),	COMMANDS_SURVIVAL, fonts->medium.normal, fonts->medium.highlighted );
	addButton( getLocaleGuiString( "gui_mm_coop" ),	COMMANDS_COOP, fonts->medium.normal, fonts->medium.highlighted );
#endif
	addButton( getLocaleGuiString( "gui_mm_profiles" ),		COMMANDS_PROFILES,	fonts->medium.normal, fonts->medium.highlighted );
	addButton( getLocaleGuiString( "gui_mm_options" ),		COMMANDS_OPTIONS,	fonts->medium.normal, fonts->medium.highlighted );
	addSeparator();
	addButton( getLocaleGuiString( "gui_mm_credits" ),		COMMANDS_CREDITS,	fonts->medium.normal, fonts->medium.highlighted );
	addButton( getLocaleGuiString( "gui_mm_quit" ),			COMMANDS_QUIT,		fonts->medium.normal, fonts->medium.highlighted );

	closeMenuByEsc = false;

	// disable profiles menu if from coop
	if( game->inCombat && game->isCooperative() )
	{
		std::list< OguiButton* >::iterator i = buttons.begin();
		for( i = buttons.begin(); i != buttons.end(); ++i )
		{
			if( (*i)->GetId() == COMMANDS_PROFILES )
			{
				(*i)->SetDisabled( true );
#ifdef PROJECT_SURVIVOR
				(*i)->SetText("");
#endif
			}
		}
	}

	firstUpdate = true;
}

MainMenu::~MainMenu()
{

	while( !buttons.empty() )
	{
		delete *(buttons.begin());
		buttons.pop_front();
	}

	delete abortGame;
	delete abortGameYes;
	delete abortGameNo;
	delete captureAllEvents;

	delete win;
	delete win2;
}

//.............................................................................

void MainMenu::checkContinue()
{
	if( continueButton )
	{
		if( DoWeHaveAnySaveGames( game ) )
		{
			continueButton->SetDisabled( false );
			continueButton->SetText( getLocaleGuiString( "gui_mm_continue" ) );
		}
		else
		{
			continueButton->SetDisabled( true );
			continueButton->SetText( "" );
		}
	}
}

int MainMenu::getType() const
{
	return MenuCollection::MENU_TYPE_MAINMENU;
}


//.............................................................................

void MainMenu::openMenu( int m )
{
	menuCollection->openMenu( m );
}

void MainMenu::closeMenu()
{
	menuCollection->closeMenu();
}

void MainMenu::applyChanges()
{
}

//.............................................................................

void MainMenu::CursorEvent( OguiButtonEvent* eve )
{
	if( abortGame )
	{
		MenuBaseImpl::CursorEvent( eve );
		if( eve->eventType == OGUI_EMASK_CLICK )
		{
			switch( eve->triggerButton->GetId() )
			{
				case COMMANDS_YES:
					menuCollection->newMission();
					break;

				case COMMANDS_NO:
					closeAbortMenu();
					break;
			}
		}
	}
	else 
	{
		MenuBaseImpl::CursorEvent( eve );
		
		if( eve->eventType == OGUI_EMASK_CLICK )
		{
			if( currentActive && eve->triggerButton->GetId() != COMMANDS_CONTINUE )
			{
				currentActive->SetFont( fonts->medium.normal );

				if( currentActive == eve->triggerButton )
				{
					menuCollection->closeMenu();
					return;
				}
				else
				{
					menuCollection->closeMenu();
				}
			}
			
			currentActive = eve->triggerButton;
			if( currentActive )
				currentActive->SetFont( fonts->medium.highlighted );

			switch( eve->triggerButton->GetId() )
			{
			case COMMANDS_RESUME:
				menuResume();
				break;

			case COMMANDS_CONTINUE:
				menuContinue();
				break;

			case COMMANDS_NEW_GAME:
				menuNewGame();
				break;
			case COMMANDS_LOAD_GAME:
				menuLoadGame();
				break;
#ifdef PROJECT_SURVIVOR
			case COMMANDS_SURVIVAL:
				menuSurvival();
				break;
			case COMMANDS_COOP:
				menuCoop();
				break;
#endif
			case COMMANDS_PROFILES:
				menuProfiles();
				break;
			case COMMANDS_OPTIONS:
				menuOptions();
				break;
			
			case COMMANDS_CREDITS:
				menuCredits();
				break;

			case COMMANDS_QUIT:
				menuQuit();
				break;
			
			case COMMANDS_EASY:
			case COMMANDS_NORMAL:
			case COMMANDS_HARD:
				selectDifficultButton( eve->triggerButton->GetId() );
				break;

			default:
				// This should not happen
				assert( false );
				break;
			};
		}
	}

}

void MainMenu::handleEsc()
{
	if( fromGame )
	{
		menuResume();
	}
}

//.............................................................................

void MainMenu::menuResume()
{
	game->gameUI->resumeGame();
}

void MainMenu::menuContinue()
{

	
	int newest_mission = 0;

#ifdef PROJECT_SURVIVOR
	// get last savegame
	//
	std::string filename = std::string(game->getGameProfiles()->getProfileDirectory( 0 )) + "/Save/lastsave.txt";
	frozenbyte::filesystem::FB_FILE *f = frozenbyte::filesystem::fb_fopen(filename.c_str(), "rb");
	if (f != NULL)
	{
		char buf[32];
		int length = frozenbyte::filesystem::fb_fread(buf, 1, 31, f);
		buf[length] = 0;
		sscanf(buf, "%i", &newest_mission);
		frozenbyte::filesystem::fb_fclose(f);
	}	
#else
	int i = 0;
	std::string temp = boost::lexical_cast< std::string >( char( 0 ) );
	for( i = 1; i <= mission_max; i++ )
	{
		std::stringstream foo;

		foo << i;
		bool add = game->getInfoForSavegame( foo.str().c_str(), "savegame" );

		if ( add )
		{
				
			if( savegame_time >= temp )
			{
				temp = savegame_time;
				newest_mission = i;
			}

		} 
		else 
		{
		}
		
	}
#endif

	if( newest_mission )
		menuCollection->loadMission( newest_mission );
	else 
		menuCollection->newMission();
}

void MainMenu::menuNewGame()
{
#ifdef PROJECT_SURVIVOR
	// start as single player when clicked from mainmenu
	NewGameMenu::selectedGameplaySelection = NewGameMenu::COMMANDS_SINGLEPLAYER;
#endif

	openMenu( MenuCollection::MENU_TYPE_NEWGAMEMENU );
	/*if( fromGame ) 
	{
		// abortCurrentGame();
	}
	else
	{
		// setDifficulty( difficultActiveSelection );
		// menuCollection->loadMission( 5 );
		menuCollection->newMission();
	}*/
	//menuCollection->loadMission( 1 );
	// game->gameUI->openLoadingWindow( game->singlePlayerNumber ); 
	// hide();
}

void MainMenu::menuLoadGame()
{
#ifdef PROJECT_SURVIVOR
	// start as single player when clicked from mainmenu
	LoadGameMenu::startAsCoop = false;
#endif
	openMenu( MenuCollection::MENU_TYPE_LOADGAMEMENU );
}

void MainMenu::menuSurvival()
{
#ifdef PROJECT_SURVIVOR
	// start as single player when clicked from mainmenu
	SurvivalMenu::startAsCoop = false;
#endif
	openMenu( MenuCollection::MENU_TYPE_SURVIVALMENU );
}

void MainMenu::menuCoop()
{
	openMenu( MenuCollection::MENU_TYPE_COOPMENU );
}

void MainMenu::menuProfiles()
{
	openMenu( MenuCollection::MENU_TYPE_PROFILESMENU );
}

void MainMenu::menuOptions()
{
	openMenu( MenuCollection::MENU_TYPE_OPTIONSMENU );
}

void MainMenu::menuCredits()
{
	openMenu( MenuCollection::MENU_TYPE_CREDITSMENU );
	// assert( false && "Credits:" );
}

void MainMenu::menuQuit()
{
	game->gameUI->setQuitRequested();
	// quitPressed = true;
}

//.............................................................................

void MainMenu::abortCurrentGame()
{
	closeAbortMenu();

	captureAllEvents = ogui->CreateSimpleWindow( 0, 0, 1024, 768, NULL ); 
	captureAllEvents->SetUnmovable();
	captureAllEvents->Show();

	{
		int x = getLocaleGuiInt( "gui_mainmenu_abortbox_x", 0 );
		int y = getLocaleGuiInt( "gui_mainmenu_abortbox_y", 0 );
		int w = getLocaleGuiInt( "gui_mainmenu_abortbox_w", 0 );
		int h = getLocaleGuiInt( "gui_mainmenu_abortbox_h", 0 );
		int id = 0;
		abortGame = new OguiFormattedText( captureAllEvents, ogui, x, y, w, h, id );
		abortGame->setFont( fonts->medium.normal );
		abortGame->setText( getLocaleGuiString( "gui_mainmenu_abortbox_text" ) );
	}
	
	{
		int x = getLocaleGuiInt( "gui_mainmenu_abort_yes_x", 0 );
		int y = getLocaleGuiInt( "gui_mainmenu_abort_yes_y", 0 );
		int w = getLocaleGuiInt( "gui_mainmenu_abort_yes_w", 0 );
		int h = getLocaleGuiInt( "gui_mainmenu_abort_yes_h", 0 );

		abortGameYes = ogui->CreateSimpleTextButton( captureAllEvents, x, y, w, h, 
			getLocaleGuiString( "gui_mainmenu_abort_yes_norm" ), getLocaleGuiString( "gui_mainmenu_abort_yes_down" ), getLocaleGuiString( "gui_mainmenu_abort_yes_high" ), 
			getLocaleGuiString( "gui_mainmenu_abort_yes_text" ), COMMANDS_YES );

		abortGameYes->SetFont( fonts->medium.normal );
		abortGameYes->SetListener( this );
	}

	{
		int x = getLocaleGuiInt( "gui_mainmenu_abort_no_x", 0 );
		int y = getLocaleGuiInt( "gui_mainmenu_abort_no_y", 0 );
		int w = getLocaleGuiInt( "gui_mainmenu_abort_no_w", 0 );
		int h = getLocaleGuiInt( "gui_mainmenu_abort_no_h", 0 );

		abortGameNo = ogui->CreateSimpleTextButton( captureAllEvents, x, y, w, h, 
			getLocaleGuiString( "gui_mainmenu_abort_no_norm" ), getLocaleGuiString( "gui_mainmenu_abort_no_down" ), getLocaleGuiString( "gui_mainmenu_abort_no_high" ), 
			getLocaleGuiString( "gui_mainmenu_abort_no_text" ), COMMANDS_NO );

		abortGameNo->SetFont( fonts->medium.normal );
		abortGameNo->SetListener( this );
	}
	
}

void MainMenu::closeAbortMenu()
{
	delete abortGame;			abortGame = NULL;
	delete abortGameYes;		abortGameYes = NULL;
	delete abortGameNo;			abortGameNo = NULL;
	delete captureAllEvents;	captureAllEvents = NULL;
}

//.............................................................................

void MainMenu::createDifficultyButtons()
{
	int difficultButtonX = getLocaleGuiInt( "gui_mainmenu_difficult_button_x", 0 );
	int difficultButtonY = getLocaleGuiInt( "gui_mainmenu_difficult_button_y", 0 );
	int difficultButtonW = getLocaleGuiInt( "gui_optionsmenu_difficult_button_w", 0 );
	int difficultButtonH = getLocaleGuiInt( "gui_optionsmenu_difficult_button_h", 0 );
	int difficultButtonAddX = getLocaleGuiInt( "gui_optionsmenu_difficult_button_add_x", 0 );
	int difficultButtonAddY = getLocaleGuiInt( "gui_optionsmenu_difficult_button_add_y", 0 );

	std::string optionsDifficultButtonNormal =	getLocaleGuiString( "gui_optionsmenu_difficult_image_normal" );
	std::string optionsDifficultButtonDown =	getLocaleGuiString( "gui_optionsmenu_difficult_image_down" );
	std::string optionsDifficultButtonHigh =	getLocaleGuiString( "gui_optionsmenu_difficult_image_high" );
	
	std::string optionsEasyText =	getLocaleGuiString( "gui_optionsmenu_text_easy" );
	std::string optionsNormalText = getLocaleGuiString( "gui_optionsmenu_text_normal" );
	std::string optionsHardText =	getLocaleGuiString( "gui_optionsmenu_text_hard" );

	difficultImageSelectDown = ogui->LoadOguiImage( optionsDifficultButtonHigh.c_str() );
	difficultImageSelectNorm = ogui->LoadOguiImage( optionsDifficultButtonNormal.c_str() );

	addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonW, difficultButtonH, 
		optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
		fonts->little.normal, optionsEasyText, COMMANDS_EASY );

	difficultButtonX += difficultButtonAddX;
	difficultButtonY += difficultButtonAddY;

	addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonW, difficultButtonH, 
		optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
		fonts->little.normal, optionsNormalText, COMMANDS_NORMAL );
	
	difficultButtonX += difficultButtonAddX;
	difficultButtonY += difficultButtonAddY;

	addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonW, difficultButtonH, 
		optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
		fonts->little.normal, optionsHardText, COMMANDS_HARD );

	int diff = game->gameScripting->getGlobalIntVariableValue( "general_difficulty_level" );
	if( diff < 50 )						selectDifficultButton( COMMANDS_EASY );
	else if ( diff >= 50 && diff < 75 )	selectDifficultButton( COMMANDS_NORMAL );
	else if ( diff >= 75 )				selectDifficultButton( COMMANDS_HARD );
}

void MainMenu::setDifficulty( int difficulty )
{
	// HACK: SET DEFAULT DIFFICULTY TO EASY
	int difficultyLevel = 25;

	switch( difficulty )
	{
	case COMMANDS_EASY:
		difficultyLevel = 25;
		break;

	case COMMANDS_NORMAL:
		difficultyLevel = 50;
		break;

	case COMMANDS_HARD:
		difficultyLevel = 75;
		break;

	default:
		break;
	}

	game->gameScripting->setGlobalIntVariableValue("general_difficulty_level", difficultyLevel);
	game->gameScripting->setGlobalIntVariableValue("damage_amount_level", difficultyLevel);
	game->gameScripting->setGlobalIntVariableValue("item_amount_level", difficultyLevel);
	game->gameScripting->setGlobalIntVariableValue("hostile_amount_level", difficultyLevel);
}

//.............................................................................

void MainMenu::selectDifficultButton( int i )
{
	if ( !difficultButtons.empty() )
	{
		if ( difficultButtons.find( i ) != difficultButtons.end() )
		{
			std::map< int, OguiButton* >::iterator it;

			if ( difficultActiveSelection != -1 )
			{
				it = difficultButtons.find( difficultActiveSelection );
				if ( it != difficultButtons.end() )
				{
					assert( difficultImageSelectNorm );
					it->second->SetImage( difficultImageSelectNorm );
				}

			}

			it = difficultButtons.find( i );

			if ( it != difficultButtons.end() )
			{
				assert( difficultImageSelectDown );
				it->second->SetImage( difficultImageSelectDown );	
				difficultActiveSelection = i;
			}
		}
	}	
}

//.............................................................................

void MainMenu::addDifficultButton( int x, int y, int w, int h, 
		const std::string& button_norm, const std::string& button_down, const std::string& button_high, 
		IOguiFont* font, const std::string& text, int command )
{
	assert( ogui );
	assert( win );
	assert( command >= 0 );

	OguiButton* b;
	b = ogui->CreateSimpleTextButton( win, x, y, w, h, 
		button_norm.empty()?NULL:button_norm.c_str(), 
		button_down.empty()?NULL:button_down.c_str(), 
		button_high.empty()?NULL:button_high.c_str(), 
		text.c_str(), command );
	
	b->SetListener( this );
	
	if ( font ) b->SetFont( font );

	difficultButtons.insert( std::pair< int, OguiButton* >( command, b ) );
}

///////////////////////////////////////////////////////////////////////////////

OguiButton* MainMenu::addButton( const std::string& text, int command, IOguiFont* font, IOguiFont* high, IOguiFont* down, IOguiFont* disa, OguiButton::TEXT_H_ALIGN halign)
{
	assert( ogui );
	assert( win );

	static int count = 1;

	std::string preHeader = "gui_mainmenu_button_pos_";

	if (!text.empty()) {
		OguiButton* b;
		
		int x = getLocaleGuiInt( ( preHeader + "x_" + boost::lexical_cast< std::string >( count ) ).c_str(), buttonX );
		int y = getLocaleGuiInt( ( preHeader + "y_" + boost::lexical_cast< std::string >( count ) ).c_str(), buttonY );
		int w = getLocaleGuiInt( ( preHeader + "w_" + boost::lexical_cast< std::string >( count ) ).c_str(), buttonW );
		int h = getLocaleGuiInt( ( preHeader + "h_" + boost::lexical_cast< std::string >( count ) ).c_str(), buttonH );

		OguiWindow *parent_win = win;
		if(getLocaleGuiInt( ( preHeader + "window_" + boost::lexical_cast< std::string >( count ) ).c_str(), 1 ) == 2)
		{
			parent_win = win2;
		}
		b = ogui->CreateSimpleTextButton( parent_win, x, y, w, h, 
			buttonNormal.c_str(), buttonDown.c_str(), buttonHigh.c_str(), 
			( buttonPaddingString + text ).c_str(), command );
		b->SetListener( this );
		if ( font ) b->SetFont( font );
		if ( high ) b->SetHighlightedFont( high );
		if ( down ) b->SetDownFont( down );
		if ( disa ) b->SetDisabledFont( disa );

		b->SetTextHAlign( halign );
		// b->SetTextVAlign( verticalAlign );

		b->SetEventMask( OGUI_EMASK_CLICK |  OGUI_EMASK_OVER );

		buttonX += buttonAddX;
		buttonY += buttonAddY;

		buttons.push_back( b );
		count++;
		if( count > numberOfButtons ) count = 1;

		return b;
	}
	count++;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void MainMenu::show()
{
	// assert( false );
	
	if( currentActive )
	{
		currentActive->SetFont( fonts->medium.normal );
		menuCollection->setBackgroundImage( "" );
	}
	
	currentActive = NULL;

	checkContinue();

	MenuBaseImpl::show();
	win2->Show();
}

void MainMenu::hide()
{
	MenuBaseImpl::show();
	win2->Hide();
}

void MainMenu::raise()
{
	MenuBaseImpl::raise();
	win2->Raise();
}

void MainMenu::update()
{
#ifdef PROJECT_SURVIVOR
	//#define GC_BUILD_2007 1
#endif
#ifdef GC_BUILD_2007
	// open loadgame menu
	if(firstUpdate && currentActive == NULL)
	{
		firstUpdate = false;

		std::list< OguiButton* >::iterator i = buttons.begin();
		for( i = buttons.begin(); i != buttons.end(); ++i )
		{
			if( (*i)->GetId() == COMMANDS_LOAD_GAME )
			{
				currentActive = (*i);
			}
		}
		if( currentActive )
		{
			openMenu( MenuCollection::MENU_TYPE_LOADGAMEMENU );
			currentActive->SetFont( fonts->medium.highlighted );
		}
	}
#endif
}
///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui


#include "precompiled.h"

#include "CoopMenu.h"
#include "NewGameMenu.h"
#include "SurvivalMenu.h"
#include "LoadGameMenu.h"
#include "OptionsMenu.h"

#include "../util/assert.h"
#include "../ogui/Ogui.h"
#include "../ui/MenuCollection.h"
#include "../game/DHLocaleManager.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/scripting/GameScripting.h"
#include "../game/savegamevars.h"
#include "../game/GameProfilesEnumeration.h"
#include "../game/GameProfiles.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_players.h"
#include "../game/options/options_game.h"
#include "../ui/GameController.h"
#include "../ogui/OguiCheckBox.h"

#include <boost/lexical_cast.hpp>

#include "../game/userdata.h"

using namespace game;

namespace ui {

std::map< int, std::string >	CoopMenu::coopPlayerNames;
std::string CoopMenu::none;


// Converts player number into a running number
int CoopMenu::convertToRunningNum( int i )
{
	switch( i )
	{
	case CoopMenu::COMMANDS_PLAYER1:
		return 0;
		break;
	case CoopMenu::COMMANDS_PLAYER2:
		return 1;
		break;
	case CoopMenu::COMMANDS_PLAYER3:
		return 2;
		break;
	case CoopMenu::COMMANDS_PLAYER4:
		return 3;
		break;
	}

	return -1;
}

CoopMenu::COMMANDS CoopMenu::convertToPlayerNum( int i )
{
	switch( i )
	{
	case 0:
		return CoopMenu::COMMANDS_PLAYER1;
		break;
	case 1:
		return CoopMenu::COMMANDS_PLAYER2;
		break;
	case 2:
		return CoopMenu::COMMANDS_PLAYER3;
		break;
	case 3:
		return CoopMenu::COMMANDS_PLAYER4;
		break;
	}

	return CoopMenu::COMMANDS_STARTGAME;
}


///////////////////////////////////////////////////////////////////////////////

CoopMenu::CoopMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, Game* g ) :
	MenuBaseImpl( NULL ),

	coopBigText( NULL ),
	coopProfileList( NULL ),
	coopProfileListSaver( NULL ),
	coopCaptureEvents( NULL ),
	coopCurrentSelection( -1 ),

	gameProfiles( NULL ),

	menuCollection( menu ),
	fonts( fonts ),

	styles(),
	selectListStyle( NULL ),

	textLabels(),
	startGame( NULL ),

	optionsBigText( NULL )
{
	///////////////////////////////////////////////////////////////////////////
	
	FB_ASSERT( o_gui );
	FB_ASSERT( menu );
	FB_ASSERT( fonts );
	FB_ASSERT( g );
	game = g;

	none = getLocaleGuiString( "gui_newgame_text_none" );

	ogui = o_gui;
	win = ogui->CreateSimpleWindow( getLocaleGuiInt( "gui_coopmenu_window_x", 0 ), getLocaleGuiInt( "gui_coopmenu_window_y", 0 ), 
									getLocaleGuiInt( "gui_coopmenu_window_w", 1024 ), getLocaleGuiInt( "gui_coopmenu_window_h", 768 ), NULL );
	win->Hide();
	win->SetUnmovable();

	imageSelectNorm = ogui->LoadOguiImage( buttonNormal.c_str() );
	imageSelectDown = ogui->LoadOguiImage( buttonDown.c_str() );

	menu->setBackgroundImage( getLocaleGuiString( "gui_coopmenu_background_image" ) );

	fontSelectNorm	= ogui->LoadFont( buttonFontSelectNormal.c_str() );
	fontSelectDown	= ogui->LoadFont( buttonFontSelectDown.c_str() );
	fontDescNorm	= ogui->LoadFont( buttonFontDescNormal.c_str() );
	fontDescDown	= ogui->LoadFont( buttonFontDescDown.c_str() );

	gameProfiles = game->getGameProfiles();


	{
		int i;
		for( i = 0; i < 4; i++ )
		{
			std::string name = gameProfiles->getCurrentProfile( i );
			if( name.empty() ) 
				name = none;
			
			if( coopPlayerNames.find( convertToPlayerNum( i ) ) == coopPlayerNames.end() )
				coopPlayerNames.insert( std::pair< int, std::string >( convertToPlayerNum( i ), name ) );
			else coopPlayerNames[ convertToPlayerNum( i ) ] = name;

		}
	}
	
	createCooperativeMenu();

	///////////////////////////////////////////////////////////////////////////
	{
		startGame = ogui->CreateSimpleTextButton( win,	getLocaleGuiInt( "gui_coopmenu_startgame_x", 0 ), getLocaleGuiInt( "gui_coopmenu_startgame_y", 0 ), 
														getLocaleGuiInt( "gui_coopmenu_startgame_w", 0 ), getLocaleGuiInt( "gui_coopmenu_startgame_h", 0 ), 
														getLocaleGuiString( "gui_coopmenu_startgame_norm" ), getLocaleGuiString( "gui_coopmenu_startgame_high" ), getLocaleGuiString( "gui_coopmenu_startgame_high" ), NULL, COMMANDS_STARTGAME );

		startGame->SetListener( this );
		startGame->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );

		startGame->SetFont( fonts->medium.normal );
		startGame->SetDisabledFont( fonts->medium.disabled );
		startGame->SetDownFont( fonts->medium.down );
		startGame->SetHighlightedFont( fonts->medium.highlighted );
		startGame->SetText( getLocaleGuiString( "gui_coop_startgame_text" ) );
		startGame->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	}
	{
		loadGame = ogui->CreateSimpleTextButton( win,	getLocaleGuiInt( "gui_coopmenu_loadgame_x", 0 ), getLocaleGuiInt( "gui_coopmenu_loadgame_y", 0 ), 
														getLocaleGuiInt( "gui_coopmenu_loadgame_w", 0 ), getLocaleGuiInt( "gui_coopmenu_loadgame_h", 0 ), 
														getLocaleGuiString( "gui_coopmenu_loadgame_norm" ), getLocaleGuiString( "gui_coopmenu_loadgame_high" ), getLocaleGuiString( "gui_coopmenu_loadgame_high" ), NULL, COMMANDS_LOADGAME );

		loadGame->SetListener( this );
		loadGame->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );

		loadGame->SetFont( fonts->medium.normal );
		loadGame->SetDisabledFont( fonts->medium.disabled );
		loadGame->SetDownFont( fonts->medium.down );
		loadGame->SetHighlightedFont( fonts->medium.highlighted );
		loadGame->SetText( getLocaleGuiString( "gui_coop_loadgame_text" ) );
		loadGame->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	}
	{
		startSurvival = ogui->CreateSimpleTextButton( win,	getLocaleGuiInt( "gui_coopmenu_startsurvival_x", 0 ), getLocaleGuiInt( "gui_coopmenu_startsurvival_y", 0 ), 
														getLocaleGuiInt( "gui_coopmenu_startsurvival_w", 0 ), getLocaleGuiInt( "gui_coopmenu_startsurvival_h", 0 ), 
														getLocaleGuiString( "gui_coopmenu_startsurvival_norm" ), getLocaleGuiString( "gui_coopmenu_startsurvival_high" ), getLocaleGuiString( "gui_coopmenu_startsurvival_high" ), NULL, COMMANDS_STARTSURVIVAL );

		startSurvival->SetListener( this );
		startSurvival->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );

		startSurvival->SetFont( fonts->medium.normal );
		startSurvival->SetDisabledFont( fonts->medium.disabled );
		startSurvival->SetDownFont( fonts->medium.down );
		startSurvival->SetHighlightedFont( fonts->medium.highlighted );
		startSurvival->SetText( getLocaleGuiString( "gui_coop_startsurvival_text" ) );
		startSurvival->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	}
	///////////////////////////////////////////////////////////////////////////

	assert( game  );
	assert( game->gameUI ); 
	assert( game->gameUI->getController(0) );

	if( game->inCombat )
	{
		closeMenuByEsc = false;
	} 
	else
	{
		
		closeMenuByEsc = true;
		editHandle = game->gameUI->getController(0)->addKeyreader( this );
		// debugKeyreader( editHandle, false, "CreditsMenu::CreditsMenu()" );
	}
	
	buttonPaddingString = "";

	options.resize(NUM_OPTIONS);
	{
		int i = OPTION_FRIENDLY_FIRE;
		int	x = getLocaleGuiInt( "gui_coopmenu_friendlyfireoption_x", 0 );
		int y = getLocaleGuiInt( "gui_coopmenu_friendlyfireoption_y", 0 );
		int w = getLocaleGuiInt( "gui_coopmenu_friendlyfireoption_w", 0 );
		int h = getLocaleGuiInt( "gui_coopmenu_friendlyfireoption_h", 0 );
		std::string img_norm = getLocaleGuiString( "gui_coopmenu_friendlyfireoption_img_norm" );
		std::string img_fill = getLocaleGuiString( "gui_coopmenu_friendlyfireoption_img_fill" );
		std::string	text = getLocaleGuiString( "gui_coop_friendlyfire_text" );

		options[i] = new OguiCheckBox( win, ogui, x, y, w, h, 
																		img_norm.c_str(), "", "", 
																		img_fill.c_str(),	"", "", i );

		options[i]->setText( text.c_str(), OguiCheckBox::TEXT_ALIGN_RIGHT, 300, fonts->medium.highlighted, OguiButton::TEXT_V_ALIGN_TOP  );
		options[i]->setValue( SimpleOptions::getBool(DH_OPT_B_GAME_FRIENDLY_FIRE) );
	}

	{
		std::string	text = getLocaleGuiString( "gui_coop_options_text" );
			
		int x = getLocaleGuiInt( "gui_coopmenu_text_options_x", 0 );
		int y = getLocaleGuiInt( "gui_coopmenu_text_options_y", 0 );
		int w = getLocaleGuiInt( "gui_coopmenu_text_options_w", 0 );
		int h = getLocaleGuiInt( "gui_coopmenu_text_options_h", 0 );

		optionsBigText = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
		optionsBigText->SetFont( fonts->big.normal );
		optionsBigText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	}
}

//=============================================================================

CoopMenu::~CoopMenu()
{
	for(unsigned int i = 0; i < options.size(); i++)
	{
		delete options[i];
	}

	if( closeMenuByEsc )
	{
		game->gameUI->getController(0)->removeKeyreader( editHandle );
		debugKeyreader( editHandle, true, "CreditsMenu::~CreditsMenu()" );
	}

	delete startGame;

	while( !buttons.empty() )
	{
		delete *(buttons.begin());
		buttons.pop_front();
	}

	
	while( textLabels.empty() == false )
	{
		delete textLabels.front();
		textLabels.pop_front();
	}
	
	{
		delete coopCaptureEvents;
		coopCaptureEvents = NULL;

		delete coopProfileListSaver;
		coopProfileListSaver = NULL;

		delete coopProfileList;
		coopProfileList = NULL;

		delete coopBigText;
		coopBigText = NULL;
	}
	
	{
		std::list< OguiButtonStyle* >::iterator i;
		for( i = styles.begin(); i != styles.end(); ++i )
		{
			delete (*i)->image;
			delete (*i)->imageDown;
			delete (*i)->imageDisabled;
			delete (*i)->imageHighlighted;
			delete (*i)->textFont;
			delete (*i)->textFontHighlighted;
			delete (*i)->textFontDown;
			delete (*i)->textFontDisabled;

			delete (*i);
		}

		delete selectListStyle;

	}

	delete optionsBigText;
	delete fontSelectNorm;
	delete fontSelectDown;
	delete fontDescNorm;
	delete fontDescDown;
	delete win;
}

///////////////////////////////////////////////////////////////////////////////

int CoopMenu::getType() const
{
	return MenuCollection::MENU_TYPE_COOPMENU;
}

//=============================================================================

void CoopMenu::closeMenu()
{
	assert( menuCollection );

	menuCollection->closeMenu();
}

//=============================================================================

void CoopMenu::openMenu( int m )
{
	assert( menuCollection );
	menuCollection->openMenu( m );
}

//=============================================================================

void CoopMenu::applyChanges()
{
	SimpleOptions::setBool(DH_OPT_B_GAME_FRIENDLY_FIRE, options[OPTION_FRIENDLY_FIRE]->getValue());

	{
		int i;
		for( i = 0; i < 4; i++ )
		{
			std::string profile = coopPlayerNames[ convertToPlayerNum( i ) ];
			if( profile == none ) 
				profile = "";

			gameProfiles->setCurrentProfile( profile.c_str(), i );

			if(profile.empty() == false )
			{
#ifdef LEGACY_FILES
				std::string tmp = "Profiles/";
				tmp += game->getGameProfiles()->getCurrentProfile( i );
				tmp += "/Config/keybinds.txt";
#else
				std::string tmp = "profiles/";
				tmp += game->getGameProfiles()->getCurrentProfile( i );
				tmp += "/config/keybinds.txt";
#endif
				game->gameUI->getController( i )->loadConfiguration( igios_mapUserDataPrefix(tmp).c_str() );
			}

		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CoopMenu::CursorEvent( OguiButtonEvent* eve )
{
	
	if( eve->eventType == OGUI_EMASK_CLICK )
	{
		
		if( eve->triggerButton == coopCaptureEvents )
		{
			
			closeCoopProfileMenu();
			return;
		}

		switch( eve->triggerButton->GetId() )
		{
		case COMMANDS_STARTGAME:
			menuStartGame();
			break;

		case COMMANDS_LOADGAME:
			menuLoadGame();
			break;
		case COMMANDS_STARTSURVIVAL:
			menuStartSurvival();
			break;

	
		case COMMANDS_PLAYER1:
		case COMMANDS_PLAYER2:
		case COMMANDS_PLAYER3:
		case COMMANDS_PLAYER4:
			openCoopProfileMenu( eve->triggerButton->GetId() );
			break;

	
		case COMMANDS_PLAYER1_OPTIONS:
		case COMMANDS_PLAYER2_OPTIONS:
		case COMMANDS_PLAYER3_OPTIONS:
		case COMMANDS_PLAYER4_OPTIONS:
			menuOptions( eve->triggerButton->GetId() - COMMANDS_PLAYER1_OPTIONS  );
			break;

		default:
			break;
		}
		
	}
	else
	{
		MenuBaseImpl::CursorEvent( eve );
	}
}

///////////////////////////////////////////////////////////////////////////////

void CoopMenu::SelectEvent( OguiSelectListEvent* eve )
{
	if( eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT )
	{
		setCoopPlayer( coopCurrentSelection, eve->selectionValue?eve->selectionValue:"" );
		closeCoopProfileMenu();
	}
}

///////////////////////////////////////////////////////////////////////////////
void CoopMenu::disableCoopGameSettings(Game *game)
{
	SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_ENABLED, true );

	for( int c = 1; c < MAX_PLAYERS_PER_CLIENT; c++ )
	{
		SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_ENABLED + c, false );
	}
}

bool CoopMenu::prepareCoopGameRestart(Game *game)
{
	if(!enableCoopGameSettings(game))
	{
		return false;
	}
	return true;
}

bool CoopMenu::enableCoopGameSettings(Game *game, bool test_only)
{
	if( game->getGameProfiles()->doesProfileExist( 0 ) == false )
		return false;

	int num_of_players = 0;
	{
		std::vector< std::string > temp;

		std::map< int, std::string >::iterator i;
		for( i = coopPlayerNames.begin(); i != coopPlayerNames.end(); ++i )
		{
			if( i->second != none )
			{
				temp.push_back( i->second );
			}

			i->second = none;
		}
		
		int j;
		num_of_players = (int)temp.size();
		for( j = 0; j < (int)temp.size(); j++ )
		{
			coopPlayerNames[ convertToPlayerNum( j ) ] = temp[ j ];
		}
	
		if( num_of_players < 2 )
			return false;
	}
	
	if(test_only)
	{
		// we don't want to change any settings, just test
		// if coop can be enabled with the given ones
		return true;
	}

	setSinglePlayer( game );
	{
		int i;
		for( i = 0; i < 4; i++ )
		{
			std::string profile = coopPlayerNames[ convertToPlayerNum( i ) ];
			if( profile == none ) 
				profile = "";

			game->getGameProfiles()->setCurrentProfile( profile.c_str(), i );

			if(profile.empty() == false )
			{
#ifdef LEGACY_FILES
				std::string tmp = "Profiles/";
				tmp += game->getGameProfiles()->getCurrentProfile( i );
				tmp += "/Config/keybinds.txt";
#else
				std::string tmp = "profiles/";
				tmp += game->getGameProfiles()->getCurrentProfile( i );
				tmp += "/config/keybinds.txt";
#endif
				game->gameUI->getController( i )->loadConfiguration( igios_mapUserDataPrefix(tmp).c_str() );
			}

		}
	}

	{
		game->setCooperative( true );
		int c;
		for( c = 0; c < MAX_PLAYERS_PER_CLIENT; c++ )
		{
			if( num_of_players > c )
			{
				SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_ENABLED + c, true );
				
				GameController* gameController = game->getGameUI()->getController( c );

				{
#ifndef NDEBUG
					int foo = gameController->getControllerType();
#endif
					gameController->reloadConfiguration();

#ifndef NDEBUG
					assert( foo == gameController->getControllerType() );
#endif
				}
				
				if( gameController->controllerTypeHasMouse() )
				{
					SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR + c, true );
				}
				else
				{
					SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR + c, false );
				}
				SimpleOptions::setInt( DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME + c, gameController->getControllerType() );
				
			}
		}
	}
	return true;
}

void CoopMenu::menuStartGame()
{
	applyChanges();
	if(!enableCoopGameSettings(game, true)) return;

	NewGameMenu::selectedGameplaySelection = NewGameMenu::COMMANDS_MULTIPLAYER;
	menuCollection->changeMenu( MenuCollection::MENU_TYPE_NEWGAMEMENU );
}

void CoopMenu::menuLoadGame()
{
	applyChanges();
	if(!enableCoopGameSettings(game, true)) return;

	LoadGameMenu::startAsCoop = true;
	menuCollection->changeMenu( MenuCollection::MENU_TYPE_LOADGAMEMENU );
}

void CoopMenu::menuStartSurvival()
{
	applyChanges();
	if(!enableCoopGameSettings(game, true)) return;

	SurvivalMenu::startAsCoop = true;
	menuCollection->changeMenu( MenuCollection::MENU_TYPE_SURVIVALMENU );
}

void CoopMenu::menuOptions(int player)
{
	applyChanges();

	OptionsMenu::returnToCoopMenu = true;
	OptionsMenu::cooperativeSelection = player;
	menuCollection->changeMenu( MenuCollection::MENU_TYPE_OPTIONSMENU );
}

///////////////////////////////////////////////////////////////////////////////

void CoopMenu::addText( const std::string& text, int x, int y, int w, int h, IOguiFont* font )
{
	assert( win );
	assert( ogui );

	OguiTextLabel* foo;
	foo = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
	if ( font ) foo->SetFont( font );

	foo->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );

	textLabels.push_back( foo );
}

void CoopMenu::createCooperativeMenu()
{

	{
		std::string	text = getLocaleGuiString( "gui_newgamemenu_text_coopprofile" );
			
		int x = getLocaleGuiInt( "gui_coopmenu_text_coopprofile_x", 0 );
		int y = getLocaleGuiInt( "gui_coopmenu_text_coopprofile_y", 0 );
		int w = getLocaleGuiInt( "gui_coopmenu_text_coopprofile_w", 0 );
		int h = getLocaleGuiInt( "gui_coopmenu_text_coopprofile_h", 0 );

		coopBigText = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
		coopBigText->SetFont( fonts->big.normal );
		coopBigText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	}

	// Coop profile buttons
	buttonX	= getLocaleGuiInt( "gui_coopmenu_coopprofile_x", 0 );
	buttonY	= getLocaleGuiInt( "gui_coopmenu_coopprofile_y", 0 );
	buttonW	= getLocaleGuiInt( "gui_coopmenu_coopprofile_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
	buttonH	= getLocaleGuiInt( "gui_coopmenu_coopprofile_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );
	
	buttonPaddingString = "";

	/*int profile_button_x = buttonX;
	int profile_button_y = buttonY;*/
	addSelectionButton( getCoopPlayerName( COMMANDS_PLAYER1 ), COMMANDS_PLAYER1, fonts->medium.normal );
	addSelectionButton( getCoopPlayerName( COMMANDS_PLAYER2 ), COMMANDS_PLAYER2, fonts->medium.normal );
	addSelectionButton( getCoopPlayerName( COMMANDS_PLAYER3 ), COMMANDS_PLAYER3, fonts->medium.normal );
	addSelectionButton( getCoopPlayerName( COMMANDS_PLAYER4 ), COMMANDS_PLAYER4, fonts->medium.normal );

	// Profile options buttons

	buttonX	= getLocaleGuiInt( "gui_coopmenu_coopprofileoptions_x", 0 );
	buttonY	= getLocaleGuiInt( "gui_coopmenu_coopprofileoptions_y", 0 );
	buttonW	= getLocaleGuiInt( "gui_coopmenu_coopprofileoptions_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
	buttonH	= getLocaleGuiInt( "gui_coopmenu_coopprofileoptions_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );

	optionsButtons.resize(4);
	optionsButtons[0] = addButton( "", COMMANDS_PLAYER1_OPTIONS, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down );
	optionsButtons[1] = addButton( "", COMMANDS_PLAYER2_OPTIONS, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down );
	optionsButtons[2] = addButton( "", COMMANDS_PLAYER3_OPTIONS, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down );
	optionsButtons[3] = addButton( "", COMMANDS_PLAYER4_OPTIONS, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down );

	// update options button states
	for(int i = 0; i < 4; i++)
	{
		bool dis = getCoopPlayerName(i + COMMANDS_PLAYER1) == none;
		optionsButtons[i]->SetDisabled(dis);
		if(dis)
		{
			optionsButtons[i]->SetText("");
		}
		else
		{
			optionsButtons[i]->SetText(getLocaleGuiString("gui_coopmenu_coopprofileoptions_text"));
		}
	}
}

//.............................................................................

std::string CoopMenu::getCoopPlayerName( int num )
{
	std::map< int, std::string >::iterator i;

	i = coopPlayerNames.find( num );

	if( i == coopPlayerNames.end() )
	{
		coopPlayerNames.insert( std::pair< int, std::string >( num, none ) );
	}

	return coopPlayerNames[ num ];
}

//.............................................................................

void CoopMenu::freeCooperativeMenu()
{
	delete coopBigText;
	coopBigText = NULL;

	std::map< int, OguiButton* >::iterator i;
	
	for(int id = COMMANDS_PLAYER1; id <= COMMANDS_PLAYER4; id++)
	{
		i = selectButtons.find( id );
		if( i != selectButtons.end() )
		{
			delete i->second;
			selectButtons.erase( i );
			continue;
		}
	}
}

//.............................................................................

OguiButtonStyle* CoopMenu::loadStyle( const std::string& button_name )
{
	IOguiImage* norm = ogui->LoadOguiImage( getLocaleGuiString( ( button_name + "_norm" ).c_str() ) );
	IOguiImage* high = ogui->LoadOguiImage( getLocaleGuiString( ( button_name + "_high" ).c_str() ) );
	IOguiImage* down = ogui->LoadOguiImage( getLocaleGuiString( ( button_name + "_down" ).c_str() ) );
	IOguiImage* disa = ogui->LoadOguiImage( getLocaleGuiString( ( button_name + "_disa" ).c_str() ) );
	IOguiFont*  font = ogui->LoadFont( getLocaleGuiString( ( button_name + "_font" ).c_str() ) );
	IOguiFont*  font_down = ogui->LoadFont( getLocaleGuiString( ( button_name + "_font_down" ).c_str() ) );
	IOguiFont*  font_disa = ogui->LoadFont( getLocaleGuiString( ( button_name + "_font_disa" ).c_str() ) );
	IOguiFont*  font_high = ogui->LoadFont( getLocaleGuiString( ( button_name + "_font_high" ).c_str() ) );


	int w = getLocaleGuiInt( ( button_name + "_w" ).c_str(), 0 );
	int h = getLocaleGuiInt( ( button_name + "_h" ).c_str(), 0 );

	OguiButtonStyle* result = new OguiButtonStyle( norm, down, disa, high, font, w, h );
	result->textFontDisabled	= font_disa;
	result->textFontDown		= font_down;
	result->textFontHighlighted = font_high;

	styles.push_back( result );
	return result;

}

//=============================================================================

void CoopMenu::openCoopProfileMenu( int i )
{
	coopCaptureEvents = ogui->CreateSimpleImageButton( win, 0, 0, getLocaleGuiInt( "gui_coopmenu_window_w", 1024 ), getLocaleGuiInt( "gui_coopmenu_window_h", 768 ), NULL, NULL, NULL, 0 );
	coopCaptureEvents->SetListener( this );

	int scroll_button_w = 0;

	if( selectListStyle == NULL )
	{
		OguiButtonStyle* unselStyle = loadStyle( "gui_coopmenu_unselected_item" );
		OguiButtonStyle* selStyle = loadStyle( "gui_coopmenu_selected_item" );

		OguiButtonStyle* newStyle = loadStyle( "gui_coopmenu_unselected_item" );
		OguiButtonStyle* newUnStyle = loadStyle( "gui_coopmenu_selected_item" );

		OguiButtonStyle* scrollUpStyle = loadStyle( "gui_coopmenu_arrow_up" );
		OguiButtonStyle* scrollDownStyle = loadStyle( "gui_coopmenu_arrow_down" );

		scroll_button_w = scrollUpStyle->sizeX>scrollDownStyle->sizeX?scrollUpStyle->sizeX:scrollDownStyle->sizeX;

		int num_of_elements = getLocaleGuiInt( "gui_coopmenu_number_of_items", 0 );
	
		selectListStyle = new OguiSelectListStyle( unselStyle, selStyle, newStyle, newUnStyle, scrollUpStyle, scrollDownStyle, unselStyle->sizeX, unselStyle->sizeY * num_of_elements, scrollUpStyle->sizeX, scrollUpStyle->sizeY );

	}
	
	{
		// buttonW	= getLocaleGuiInt( "gui_coopmenu_coopprofile_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
		// buttonH	= getLocaleGuiInt( "gui_coopmenu_coopprofile_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );

		int x	= getLocaleGuiInt( "gui_coopmenu_coopprofile_x", 0 );
		int y	= getLocaleGuiInt( "gui_coopmenu_coopprofile_y", 0 );

		x += buttonAddX * convertToRunningNum( i );
		y += buttonAddY * convertToRunningNum( i );
		y += buttonH;

		int w	= buttonW + scroll_button_w;
		int h	= getLocaleGuiInt( "gui_coopmenu_number_of_items", 0 ) * buttonAddY;

		coopProfileListSaver = ogui->CreateSimpleImageButton( win, x, y, w, h, NULL, NULL, NULL );

		coopProfileList = ogui->CreateSelectList( win, x, y, selectListStyle, 0, NULL, NULL );
		coopProfileList->setListener( this );
	}

	coopProfileList->addItem( none.c_str(), none.c_str() );
	
	{
		const std::string &current_player = coopPlayerNames.find( i ) != coopPlayerNames.end()?coopPlayerNames[ i ]:none;
		GameProfilesEnumeration* enum1 = gameProfiles->getProfileList();
		while( enum1->isNextProfileAvailable() )
		{
			
			const std::string& temp = enum1->getNextProfile();
			bool selected = false;
			if( temp == current_player ) selected = true;
			coopProfileList->addItem( temp.c_str(), temp.c_str(), selected );
		}
		delete enum1;
	}
	
	

	coopCurrentSelection = i;
}

//.............................................................................

void CoopMenu::setCoopPlayer( int player_num, const std::string& nam )
{

	std::string name( nam );

	{
		std::map< int, std::string >::iterator i;
		
		for( i = coopPlayerNames.begin(); i != coopPlayerNames.end(); ++i )
		{
			if( i->second == name && i->first != player_num )
			{
				name = none;
			}
		}
	}

	{
		std::map< int, OguiButton* >::iterator i;
		i = selectButtons.find( player_num );

		if( i != selectButtons.end() && i->second != NULL )
		{
			i->second->SetText( ( buttonPaddingString + name ).c_str() );
		}
	}

	{
		std::map< int, std::string >::iterator i;
		i = coopPlayerNames.find( player_num );

		if( i != coopPlayerNames.end() )
		{
			i->second = name;
		}
	}
	
	// update options button states
	{
		int i = convertToRunningNum(player_num);
		bool dis = getCoopPlayerName(i + COMMANDS_PLAYER1) == none;
		optionsButtons[i]->SetDisabled(dis);
		if(dis)
		{
			optionsButtons[i]->SetText("");
		}
		else
		{
			optionsButtons[i]->SetText(getLocaleGuiString("gui_coopmenu_coopprofileoptions_text"));
		}
	}
}

//.............................................................................

void CoopMenu::closeCoopProfileMenu()
{
	delete coopCaptureEvents;
	coopCaptureEvents = NULL;

	delete coopProfileListSaver;
	coopProfileListSaver = NULL;

	delete coopProfileList;
	coopProfileList = NULL;
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui

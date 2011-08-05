
#include "precompiled.h"

#include "NewGameMenu.h"

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
#include "../game/userdata.h"

#include <boost/lexical_cast.hpp>

#ifdef PROJECT_SURVIVOR
	#include "CoopMenu.h"
	#include "../game/BonusManager.h"
#else
	#define GAME_MODE_OPTION_ENABLED
#endif

using namespace game;

#ifdef PROJECT_SURVIVOR
	#define EASY_VALUE -15
	#define NORMAL_VALUE 0
	#define HARD_VALUE 25
	#define VERY_HARD_VALUE 50
	#define EXTREMELY_HARD_VALUE 75
#else
	#define EASY_VALUE 25
	#define NORMAL_VALUE 50
	#define HARD_VALUE 75
	#define VERY_HARD_VALUE 100
	#define EXTREMELY_HARD_VALUE 125
#endif

namespace ui {

int NewGameMenu::selectedDifficultSelection = -1;
int	NewGameMenu::selectedGameplaySelection = -1;
std::map< int, std::string >	NewGameMenu::coopPlayerNames;


// Converts player number into a running number
int NewGameMenu::convertToRunningNum( int i )
{
	switch( i )
	{
	case NewGameMenu::COMMANDS_PLAYER1:
		return 0;
		break;
	case NewGameMenu::COMMANDS_PLAYER2:
		return 1;
		break;
	case NewGameMenu::COMMANDS_PLAYER3:
		return 2;
		break;
	case NewGameMenu::COMMANDS_PLAYER4:
		return 3;
		break;
	}

	return -1;
}

NewGameMenu::COMMANDS NewGameMenu::convertToPlayerNum( int i )
{
	switch( i )
	{
	case 0:
		return NewGameMenu::COMMANDS_PLAYER1;
		break;
	case 1:
		return NewGameMenu::COMMANDS_PLAYER2;
		break;
	case 2:
		return NewGameMenu::COMMANDS_PLAYER3;
		break;
	case 3:
		return NewGameMenu::COMMANDS_PLAYER4;
		break;
	}

	return NewGameMenu::COMMANDS_STARTGAME;
}

void setSinglePlayer( const Game* game )
{
	SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_ENABLED, true );
	SimpleOptions::setBool( DH_OPT_B_2ND_PLAYER_ENABLED, false );
	SimpleOptions::setBool( DH_OPT_B_3RD_PLAYER_ENABLED, false );
	SimpleOptions::setBool( DH_OPT_B_4TH_PLAYER_ENABLED, false );

	if( game->getGameUI()->getController( 0 )->controllerTypeHasMouse() )
	{
		SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR, true );
	}
	else
	{
		SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR, false );
	}
	SimpleOptions::setInt( DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME, game->getGameUI()->getController( 0 )->getControllerType() );

	game->gameUI->getController( 1 )->unloadConfiguration();
	game->gameUI->getController( 2 )->unloadConfiguration();
	game->gameUI->getController( 3 )->unloadConfiguration();
}

///////////////////////////////////////////////////////////////////////////////

NewGameMenu::NewGameMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, Game* g ) :
	MenuBaseImpl( NULL ),
	difficultActiveSelection( 0 ),

	tutorialHintsButton( NULL ),
	gameModeButtons(),
	gameModeActiveSelection( 0 ),

	coopBigText( NULL ),
	coopProfileList( NULL ),
	coopProfileListSaver( NULL ),
	coopCaptureEvents( NULL ),
	coopCurrentSelection( -1 ),

	none( getLocaleGuiString( "gui_newgame_text_none" ) ),
	gameProfiles( NULL ),
	menuCollection( menu ),
	fonts( fonts ),

	styles(),
	selectListStyle( NULL ),
	textLabels(),
	startGame( NULL )
{
	///////////////////////////////////////////////////////////////////////////
	
	FB_ASSERT( o_gui );
	FB_ASSERT( menu );
	FB_ASSERT( fonts );
	FB_ASSERT( g );
	game = g;

	ogui = o_gui;
	win = ogui->CreateSimpleWindow( getLocaleGuiInt( "gui_newgamemenu_window_x", 0 ), getLocaleGuiInt( "gui_newgamemenu_window_y", 0 ), 
									getLocaleGuiInt( "gui_newgamemenu_window_w", 1024 ), getLocaleGuiInt( "gui_newgamemenu_window_h", 768 ), NULL );
	win->Hide();
	win->SetUnmovable();

	imageSelectNorm = ogui->LoadOguiImage( buttonNormal.c_str() );
	imageSelectDown = ogui->LoadOguiImage( buttonDown.c_str() );

	menu->setBackgroundImage( getLocaleGuiString( "gui_newgamemenu_background_image" ) );

	fontSelectNorm	= ogui->LoadFont( buttonFontSelectNormal.c_str() );
	fontSelectDown	= ogui->LoadFont( buttonFontSelectDown.c_str() );
	fontDescNorm	= ogui->LoadFont( buttonFontDescNormal.c_str() );
	fontDescDown	= ogui->LoadFont( buttonFontDescDown.c_str() );

	gameProfiles = game->getGameProfiles();

	///////////////////////////////////////////////////////////////////////////
	// The big text's in the new game menu
	{
		std::string textGameMode =	getLocaleGuiString( "gui_newgamemenu_text_gamemode" );
		int			textBigX =		getLocaleGuiInt( "gui_newgamemenu_text_gamemode_x", 0 );
		int			textBigY =		getLocaleGuiInt( "gui_newgamemenu_text_gamemode_y", 0 );
		int			textBigW =		getLocaleGuiInt( "gui_newgamemenu_text_gamemode_w", 0 );
		int			textBigH =		getLocaleGuiInt( "gui_newgamemenu_text_gamemode_h", 0 );

#ifdef GAME_MODE_OPTION_ENABLED
		addText( textGameMode, textBigX, textBigY, textBigW, textBigH, fonts->big.normal );
#endif
		

		std::string	textDifficultyLevel = getLocaleGuiString( "gui_newgamemenu_text_difficultylevel" );
		
		textBigX = getLocaleGuiInt( "gui_newgamemenu_text_difficultylevel_x", 0 );
		textBigY = getLocaleGuiInt( "gui_newgamemenu_text_difficultylevel_y", 0 );
		textBigW = getLocaleGuiInt( "gui_newgamemenu_text_difficultylevel_w", 0 );
		textBigH = getLocaleGuiInt( "gui_newgamemenu_text_difficultylevel_h", 0 );

		addText( textDifficultyLevel, textBigX, textBigY, textBigW, textBigH, fonts->big.normal );

	}

	///////////////////////////////////////////////////////////////////////////
#ifdef GAME_MODE_OPTION_ENABLED
	createGameModeButtons();
#endif
	createDifficultyButtons();

#ifdef PROJECT_SURVIVOR
	createBonusOptions(game, win, ogui, fonts, bonusOptionBoxes, bonusOptionButtons, textLabels, this, COMMANDS_BONUSOPTION_BUTTONS);
#endif


	if(DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, "gui_newgamemenu_text_tooltip_easy" ))
	{
		int difficultToolTipX = getLocaleGuiInt( "gui_newgamemenu_text_difficultytooltip_x", 0 );
		int difficultToolTipY = getLocaleGuiInt( "gui_newgamemenu_text_difficultytooltip_y", 0 );
		int difficultToolTipW = getLocaleGuiInt( "gui_newgamemenu_text_difficultytooltip_w", 0 );
		int difficultToolTipH = getLocaleGuiInt( "gui_newgamemenu_text_difficultytooltip_h", 0 );

		difficultToolTips.insert( std::pair<int, std::string>(COMMANDS_EASY, getLocaleGuiString( "gui_newgamemenu_text_tooltip_easy" )));
		difficultToolTips.insert( std::pair<int, std::string>(COMMANDS_NORMAL, getLocaleGuiString( "gui_newgamemenu_text_tooltip_normal" )));
		difficultToolTips.insert( std::pair<int, std::string>(COMMANDS_HARD, getLocaleGuiString( "gui_newgamemenu_text_tooltip_hard" )));
		difficultToolTips.insert( std::pair<int, std::string>(COMMANDS_VERY_HARD, getLocaleGuiString( "gui_newgamemenu_text_tooltip_very_hard" )));
		difficultToolTips.insert( std::pair<int, std::string>(COMMANDS_EXTREMELY_HARD, getLocaleGuiString( "gui_newgamemenu_text_tooltip_extremely_hard" )));
		difficultToolTipText = ogui->CreateTextArea( win, difficultToolTipX, difficultToolTipY, difficultToolTipW, difficultToolTipH, "");
		difficultToolTipFont = ogui->LoadFont(getLocaleGuiString("gui_newgamemenu_text_difficultytooltip_font"));
		difficultToolTipText->SetFont(difficultToolTipFont);
		difficultToolTipText->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
	}
	else
	{
		difficultToolTipFont = NULL;
		difficultToolTipText = NULL;
	}

	if( selectedDifficultSelection != -1 ) 
		selectDifficultButton( selectedDifficultSelection  ); 

#ifdef GAME_MODE_OPTION_ENABLED
	if( selectedGameplaySelection != -1 )
		selectGameModeButton( selectedGameplaySelection );
#endif

	
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
	
#ifdef GAME_MODE_OPTION_ENABLED
	if( selectedGameplaySelection == COMMANDS_MULTIPLAYER )
		createCooperativeMenu();
#endif

	///////////////////////////////////////////////////////////////////////////
	{
		startGame = ogui->CreateSimpleTextButton( win,	getLocaleGuiInt( "gui_newgamemenu_startgame_x", 0 ), getLocaleGuiInt( "gui_newgamemenu_startgame_y", 0 ), 
														getLocaleGuiInt( "gui_newgamemenu_startgame_w", 0 ), getLocaleGuiInt( "gui_newgamemenu_startgame_h", 0 ), 
														getLocaleGuiString( "gui_newgamemenu_startgame_norm" ), getLocaleGuiString( "gui_newgamemenu_startgame_high" ), getLocaleGuiString( "gui_newgamemenu_startgame_high" ), NULL, COMMANDS_STARTGAME );

		startGame->SetListener( this );

		startGame->SetFont( fonts->big.normal );
		startGame->SetDisabledFont( fonts->big.disabled );
		startGame->SetDownFont( fonts->big.down );
		startGame->SetHighlightedFont( fonts->big.highlighted );
		startGame->SetText( getLocaleGuiString( "gui_newgame_startgame_text" ) );
		startGame->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
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

#ifdef PROJECT_SURVIVOR
	{
		int	x = getLocaleGuiInt( "gui_newgamemenu_tutorialhints_x", 0 );
		int y = getLocaleGuiInt( "gui_newgamemenu_tutorialhints_y", 0 );
		int w = getLocaleGuiInt( "gui_newgamemenu_tutorialhints_w", 0 );
		int h = getLocaleGuiInt( "gui_newgamemenu_tutorialhints_h", 0 );
		int text_w = getLocaleGuiInt( "gui_newgamemenu_tutorialhints_text_w", 0 );
		const char *img_norm = getLocaleGuiString( "gui_newgamemenu_tutorialhints_img_norm" );
		const char *img_fill = getLocaleGuiString( "gui_newgamemenu_tutorialhints_img_fill" );

		const char *text = getLocaleGuiString( "gui_newgamemenu_tutorialhints_text" );

		tutorialHintsButton = new OguiCheckBox( win, ogui, x, y, w, h, 
			img_norm, "", "", 
			img_fill,	"", "", 0 );
		
		tutorialHintsButton->setText( text, OguiCheckBox::TEXT_ALIGN_RIGHT, text_w, fonts->medium.highlighted, OguiButton::TEXT_V_ALIGN_TOP  );
		tutorialHintsButton->setValue( SimpleOptions::getBool(DH_OPT_B_SHOW_TUTORIAL_HINTS) );
	}
#endif
	
	buttonPaddingString = "";
}

//=============================================================================

NewGameMenu::~NewGameMenu()
{
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
		std::map< int, OguiButton* >::iterator i;
		for( i = difficultButtons.begin(); i != difficultButtons.end(); ++i )
		{
			delete i->second;
		}

		delete difficultToolTipFont;
		delete difficultToolTipText;

		for( i = gameModeButtons.begin(); i != gameModeButtons.end(); ++i )
		{
			delete i->second;
		}

		for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
		{
			delete i->second;
		}

	}

	{
		for(unsigned int i = 0; i < bonusOptionBoxes.size(); i++)
		{
			delete bonusOptionBoxes[i];
		}
		for(unsigned int i = 0; i < bonusOptionButtons.size(); i++)
		{
			delete bonusOptionButtons[i];
		}
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

	delete tutorialHintsButton;

	delete fontSelectNorm;
	delete fontSelectDown;
	delete fontDescNorm;
	delete fontDescDown;
	delete win;
}

///////////////////////////////////////////////////////////////////////////////

int NewGameMenu::getType() const
{
	return MenuCollection::MENU_TYPE_NEWGAMEMENU;
}

//=============================================================================

void NewGameMenu::closeMenu()
{
	assert( menuCollection );

	menuCollection->closeMenu();
}

//=============================================================================

void NewGameMenu::openMenu( int m )
{
	assert( menuCollection );
	menuCollection->openMenu( m );
}

//=============================================================================

void NewGameMenu::applyChanges()
{
	applyBonusOptions(game, bonusOptionBoxes);

	if(tutorialHintsButton)
	{
		SimpleOptions::setBool(DH_OPT_B_SHOW_TUTORIAL_HINTS, tutorialHintsButton->getValue());
	}

	selectedDifficultSelection = difficultActiveSelection;
	selectedGameplaySelection = gameModeActiveSelection;

#ifndef PROJECT_SURVIVOR
	{
		int i;
		for( i = 0; i < 4; i++ )
		{
			std::string profile = coopPlayerNames[ convertToPlayerNum( i ) ];
			if( profile == none ) 
				profile = "";

			gameProfiles->setCurrentProfile( profile.c_str(), i );

			if( selectedGameplaySelection == COMMANDS_MULTIPLAYER && profile.empty() == false )
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
#endif
}

///////////////////////////////////////////////////////////////////////////////

void NewGameMenu::CursorEvent( OguiButtonEvent* eve )
{
	
	if( eve->eventType == OGUI_EMASK_CLICK )
	{
		if( eve->triggerButton->GetId() >= COMMANDS_BONUSOPTION_BUTTONS )
		{
#ifdef PROJECT_SURVIVOR
			game->bonusManager->buttonPressed( eve->triggerButton->GetId() - COMMANDS_BONUSOPTION_BUTTONS );
			return;
#endif
		}
		
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


		case COMMANDS_SINGLEPLAYER:
			if( gameModeActiveSelection == COMMANDS_MULTIPLAYER )
				freeCooperativeMenu();
			
			selectGameModeButton( eve->triggerButton->GetId() );
			break;

		case COMMANDS_MULTIPLAYER:
			if( gameModeActiveSelection == COMMANDS_SINGLEPLAYER )
				createCooperativeMenu();

			selectGameModeButton( eve->triggerButton->GetId() );
			break;

		case COMMANDS_EASY:
		case COMMANDS_NORMAL:
		case COMMANDS_HARD:
		case COMMANDS_VERY_HARD:
		case COMMANDS_EXTREMELY_HARD:
			selectDifficultButton( eve->triggerButton->GetId() );
			break;
		
		case COMMANDS_PLAYER1:
		case COMMANDS_PLAYER2:
		case COMMANDS_PLAYER3:
		case COMMANDS_PLAYER4:
			openCoopProfileMenu( eve->triggerButton->GetId() );
			break;


		default:
			break;
		}
		
	}
	else if(eve->eventType == OGUI_EMASK_OVER || eve->eventType == OGUI_EMASK_LEAVE)
	{
		switch( eve->triggerButton->GetId() )
		{
		case COMMANDS_EASY:
		case COMMANDS_NORMAL:
		case COMMANDS_HARD:
		case COMMANDS_VERY_HARD:
		case COMMANDS_EXTREMELY_HARD:
			if(eve->eventType == OGUI_EMASK_OVER)
				showDifficultyToolTip(eve->triggerButton->GetId());
			else
				hideDifficultyToolTip();
		default:
			MenuBaseImpl::CursorEvent( eve );
			break;
		}
	}
	else
	{
		MenuBaseImpl::CursorEvent( eve );
	}
}

///////////////////////////////////////////////////////////////////////////////

void NewGameMenu::SelectEvent( OguiSelectListEvent* eve )
{
	if( eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT )
	{
		setCoopPlayer( coopCurrentSelection, eve->selectionValue?eve->selectionValue:"" );
		closeCoopProfileMenu();
	}
}

///////////////////////////////////////////////////////////////////////////////

void NewGameMenu::menuStartGame()
{

#ifdef PROJECT_SURVIVOR
	if( selectedGameplaySelection == COMMANDS_MULTIPLAYER )
	{
		if(!CoopMenu::enableCoopGameSettings(game)) return;

		setDifficulty( difficultActiveSelection );
		menuCollection->newMission();

		// reset back to single-player
		selectedGameplaySelection = COMMANDS_SINGLEPLAYER;
		gameModeActiveSelection = COMMANDS_SINGLEPLAYER;
		return;
	}
	else
	{
		CoopMenu::disableCoopGameSettings(game);
	}
#endif

	if( game->getGameProfiles()->doesProfileExist( 0 ) == false ) {
		game->getGameProfiles()->createNewProfile("Player 1");
		game->getGameProfiles()->setCurrentProfile("Player 1", 0, true);
		game->getGameUI()->getController(0)->loadConfiguration( "Data/Misc/keybinds.txt" );
	}
	//return;

	int num_of_players = 0;
	setSinglePlayer( game );
	if( gameModeActiveSelection == COMMANDS_MULTIPLAYER )
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
			return;
	}
	
	applyChanges();

	if( gameModeActiveSelection == COMMANDS_MULTIPLAYER )
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

					assert( foo == gameController->getControllerType() );
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
	else
	{
		game->setCooperative( false );
	}

	setDifficulty( difficultActiveSelection );
	menuCollection->newMission();
}

///////////////////////////////////////////////////////////////////////////////

void NewGameMenu::addText( const std::string& text, int x, int y, int w, int h, IOguiFont* font )
{
	assert( win );
	assert( ogui );

	OguiTextLabel* foo;
	foo = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
	if ( font ) foo->SetFont( font );

	foo->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );

	textLabels.push_back( foo );
}

///////////////////////////////////////////////////////////////////////////////

void NewGameMenu::createDifficultyButtons()
{
	int difficultButtonX = getLocaleGuiInt( "gui_newgamemenu_difficult_button_x", 0 );
	int difficultButtonY = getLocaleGuiInt( "gui_newgamemenu_difficult_button_y", 0 );
	int difficultButtonW = getLocaleGuiInt( "gui_optionsmenu_difficult_button_w", 0 );
	int difficultButtonH = getLocaleGuiInt( "gui_optionsmenu_difficult_button_h", 0 );
	int difficultButtonAddX = getLocaleGuiInt( "gui_optionsmenu_difficult_button_add_x", 0 );
	int difficultButtonAddY = getLocaleGuiInt( "gui_optionsmenu_difficult_button_add_y", 0 );

	int difficultButtonLine2X = getLocaleGuiInt( "gui_newgamemenu_difficult_line2_button_x", 0 );
	int difficultButtonLine2Y = getLocaleGuiInt( "gui_newgamemenu_difficult_line2_button_y", 0 );
	int difficultButtonLine2W = getLocaleGuiInt( "gui_optionsmenu_difficult_line2_button_w", 0 );
	int difficultButtonLine2H = getLocaleGuiInt( "gui_optionsmenu_difficult_line2_button_h", 0 );
	int difficultButtonLine2AddX = getLocaleGuiInt( "gui_optionsmenu_difficult_line2_button_add_x", 0 );
	int difficultButtonLine2AddY = getLocaleGuiInt( "gui_optionsmenu_difficult_line2_button_add_y", 0 );

	std::string optionsDifficultButtonNormal =	getLocaleGuiString( "gui_optionsmenu_difficult_image_normal" );
	std::string optionsDifficultButtonDown =	getLocaleGuiString( "gui_optionsmenu_difficult_image_down" );
	std::string optionsDifficultButtonHigh =	getLocaleGuiString( "gui_optionsmenu_difficult_image_high" );
	
	std::string optionsEasyText =	getLocaleGuiString( "gui_optionsmenu_text_easy" );
	std::string optionsNormalText = getLocaleGuiString( "gui_optionsmenu_text_normal" );
	std::string optionsHardText =	getLocaleGuiString( "gui_optionsmenu_text_hard" );
	std::string optionsVeryHardText =	getLocaleGuiString( "gui_optionsmenu_text_very_hard" );
	std::string optionsExtremelyHardText =	getLocaleGuiString( "gui_optionsmenu_text_extremely_hard" );

	difficultImageSelectDown = ogui->LoadOguiImage( optionsDifficultButtonHigh.c_str() );
	difficultImageSelectNorm = ogui->LoadOguiImage( optionsDifficultButtonNormal.c_str() );

	addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonW, difficultButtonH, 
		optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
		fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, optionsEasyText, COMMANDS_EASY );

	difficultButtonX += difficultButtonAddX;
	difficultButtonY += difficultButtonAddY;

	addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonW, difficultButtonH, 
		optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
		fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, optionsNormalText, COMMANDS_NORMAL );
	
	difficultButtonX += difficultButtonAddX;
	difficultButtonY += difficultButtonAddY;

	addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonW, difficultButtonH, 
		optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
		fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, optionsHardText, COMMANDS_HARD );

	bool veryHardAvail = SimpleOptions::getBool(DH_OPT_B_GAME_VERY_HARD_AVAILABLE); 
	bool extremelyHardAvail = false;
	if (veryHardAvail)
	{
		difficultButtonX = difficultButtonLine2X;
		difficultButtonY = difficultButtonLine2Y;

		addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonLine2W, difficultButtonLine2H, 
			optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
			fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, optionsVeryHardText, COMMANDS_VERY_HARD );

		extremelyHardAvail = SimpleOptions::getBool(DH_OPT_B_GAME_EXTREMELY_HARD_AVAILABLE);
		if (extremelyHardAvail)
		{
			difficultButtonX += difficultButtonLine2AddX;
			difficultButtonY += difficultButtonLine2AddY;

			addDifficultButton( difficultButtonX, difficultButtonY, difficultButtonLine2W, difficultButtonLine2H, 
				optionsDifficultButtonNormal, optionsDifficultButtonDown, optionsDifficultButtonHigh,
				fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, optionsExtremelyHardText, COMMANDS_EXTREMELY_HARD );
		}
	}

	int diff = game->gameScripting->getGlobalIntVariableValue( "general_difficulty_level" );
	if( diff < NORMAL_VALUE )						selectDifficultButton( COMMANDS_EASY );
	else if ( diff >= NORMAL_VALUE && diff < HARD_VALUE )	selectDifficultButton( COMMANDS_NORMAL );
	else if ( diff >= HARD_VALUE && diff < VERY_HARD_VALUE) selectDifficultButton( COMMANDS_HARD );
	else if ( diff >= VERY_HARD_VALUE && diff < EXTREMELY_HARD_VALUE && veryHardAvail) selectDifficultButton( COMMANDS_VERY_HARD );
	else if ( diff >= EXTREMELY_HARD_VALUE && extremelyHardAvail) selectDifficultButton( COMMANDS_EXTREMELY_HARD );
}

void NewGameMenu::setDifficulty( int difficulty )
{
	// HACK: SET DEFAULT DIFFICULTY TO EASY
	int difficultyLevel = EASY_VALUE;

	switch( difficulty )
	{
	case COMMANDS_EASY:
		difficultyLevel = EASY_VALUE;
		break;

	case COMMANDS_NORMAL:
		difficultyLevel = NORMAL_VALUE;
		break;

	case COMMANDS_HARD:
		difficultyLevel = HARD_VALUE;
		break;

	case COMMANDS_VERY_HARD:
		difficultyLevel = VERY_HARD_VALUE;
		break;

	case COMMANDS_EXTREMELY_HARD:
		difficultyLevel = EXTREMELY_HARD_VALUE;
		break;

	default:
		break;
	}

	game->gameScripting->newGlobalIntVariable( "general_difficulty_level", true );
	game->gameScripting->newGlobalIntVariable( "damage_amount_level", true );
	game->gameScripting->newGlobalIntVariable( "item_amount_level", true );
	game->gameScripting->newGlobalIntVariable( "hostile_amount_level", true );

	game->gameScripting->setGlobalIntVariableValue("general_difficulty_level", difficultyLevel);
	game->gameScripting->setGlobalIntVariableValue("damage_amount_level", difficultyLevel);
	game->gameScripting->setGlobalIntVariableValue("item_amount_level", difficultyLevel);
	game->gameScripting->setGlobalIntVariableValue("hostile_amount_level", difficultyLevel);

}

//.............................................................................

void NewGameMenu::selectDifficultButton( int i )
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
					FB_ASSERT( it->second );
					it->second->SetImage( difficultImageSelectNorm );

					if( fontSelectNorm )
						it->second->SetFont( fontSelectNorm );
				}

			}

			it = difficultButtons.find( i );

			if ( it != difficultButtons.end() )
			{
				FB_ASSERT( it->second );
				it->second->SetImage( difficultImageSelectDown );	

				if( fontSelectDown )
						it->second->SetFont( fontSelectDown );

				difficultActiveSelection = i;
			}
		}
	}	
}

//.............................................................................

void NewGameMenu::addDifficultButton( int x, int y, int w, int h, 
		const std::string& button_norm, const std::string& button_down, const std::string& button_high, 
		IOguiFont* font, IOguiFont* high, IOguiFont* down, IOguiFont* disa, const std::string& text, int command )
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
	b->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );
	// b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	
	if ( font ) b->SetFont( font );
	if ( high ) b->SetHighlightedFont( high );
	if ( down ) b->SetDownFont( down );
	if ( disa ) b->SetDisabledFont( disa );

	difficultButtons.insert( std::pair< int, OguiButton* >( command, b ) );
}

///////////////////////////////////////////////////////////////////////////////

void NewGameMenu::showDifficultyToolTip( int difficulty )
{
	if(difficultToolTipText == NULL) return;
	std::map<int, std::string>::iterator it = difficultToolTips.find( difficulty );
	if( it != difficultToolTips.end() )
	{
		difficultToolTipText->SetText(it->second.c_str());
	}
}

///////////////////////////////////////////////////////////////////////////////

void NewGameMenu::hideDifficultyToolTip( void )
{
	if(difficultToolTipText == NULL) return;
	difficultToolTipText->SetText("");
}

///////////////////////////////////////////////////////////////////////////////

void NewGameMenu::createGameModeButtons()
{
	int x = getLocaleGuiInt( "gui_newgamemenu_gamemode_button_x", 0 );
	int y = getLocaleGuiInt( "gui_newgamemenu_gamemode_button_y", 0 );
	int w = getLocaleGuiInt( "gui_newgamemenu_gamemode_button_w", 0 );
	int h = getLocaleGuiInt( "gui_newgamemenu_gamemode_button_h", 0 );
	int addX = getLocaleGuiInt( "gui_newgamemenu_gamemode_button_add_x", 0 );
	int addY = getLocaleGuiInt( "gui_newgamemenu_gamemode_button_add_y", 0 );

	std::string img_norm = ""; // getLocaleGuiString( "" );
	std::string img_down = ""; // getLocaleGuiString( "" );
	std::string img_high = ""; // getLocaleGuiString( "" );

	std::string modeSingle =	getLocaleGuiString( "gui_newgamemenu_gamemode_singleplayer" );
	std::string modeCoop =		getLocaleGuiString( "gui_newgamemenu_gamemode_cooperative" );

	addGameModeButton( x, y, w, h, img_norm, img_down, img_high, 
		fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, 
		modeSingle, COMMANDS_SINGLEPLAYER );

	x += addX;
	y += addY;

	addGameModeButton( x, y, w, h, img_norm, img_down, img_high, 
		fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, 
		modeCoop, COMMANDS_MULTIPLAYER );

	selectGameModeButton( COMMANDS_SINGLEPLAYER );
	
}

//.............................................................................

void NewGameMenu::setGameMode( int gamemode )
{
	
}

//.............................................................................

void NewGameMenu::selectGameModeButton( int i )
{
	if ( !gameModeButtons.empty() )
	{
		if ( gameModeButtons.find( i ) != gameModeButtons.end() )
		{
			std::map< int, OguiButton* >::iterator it;

			if ( gameModeActiveSelection != -1 )
			{
				it = gameModeButtons.find( gameModeActiveSelection );
				if ( it != gameModeButtons.end() )
				{
					FB_ASSERT( it->second );
					// it->second->SetImage( difficultImageSelectNorm );

					if( fontSelectNorm )
						it->second->SetFont( fontSelectNorm );
				}

			}

			it = gameModeButtons.find( i );

			if ( it != gameModeButtons.end() )
			{
				FB_ASSERT( it->second );
				// it->second->SetImage( difficultImageSelectDown );	

				if( fontSelectDown )
					it->second->SetFont( fontSelectDown );

				gameModeActiveSelection = i;
			}
		}
	}
}

//.............................................................................

void NewGameMenu::addGameModeButton( int x, int y, int w, int h, const std::string& button_norm, const std::string& button_down, const std::string& button_high, 
					   	IOguiFont* font, IOguiFont* high, IOguiFont* down, IOguiFont* disa, const std::string& text, int command )
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
	b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	
	if ( font ) b->SetFont( font );
	if ( high ) b->SetHighlightedFont( high );
	if ( down ) b->SetDownFont( down );
	if ( disa ) b->SetDisabledFont( disa );

	gameModeButtons.insert( std::pair< int, OguiButton* >( command, b ) );
}

///////////////////////////////////////////////////////////////////////////////

void NewGameMenu::createCooperativeMenu()
{

	{
		std::string	text = getLocaleGuiString( "gui_newgamemenu_text_coopprofile" );
			
		int x = getLocaleGuiInt( "gui_newgamemenu_text_coopprofile_x", 0 );
		int y = getLocaleGuiInt( "gui_newgamemenu_text_coopprofile_y", 0 );
		int w = getLocaleGuiInt( "gui_newgamemenu_text_coopprofile_w", 0 );
		int h = getLocaleGuiInt( "gui_newgamemenu_text_coopprofile_h", 0 );

		coopBigText = ogui->CreateTextLabel( win, x, y, w, h, text.c_str() );
		coopBigText->SetFont( fonts->big.normal );
		coopBigText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	}

	// Coop profile buttons
	buttonX	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_x", 0 );
	buttonY	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_y", 0 );
	buttonW	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
	buttonH	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );
	
	buttonPaddingString = "";

	addSelectionButton( getCoopPlayerName( COMMANDS_PLAYER1 ), COMMANDS_PLAYER1, fonts->medium.normal );
	addSelectionButton( getCoopPlayerName( COMMANDS_PLAYER2 ), COMMANDS_PLAYER2, fonts->medium.normal );
	addSelectionButton( getCoopPlayerName( COMMANDS_PLAYER3 ), COMMANDS_PLAYER3, fonts->medium.normal );
	addSelectionButton( getCoopPlayerName( COMMANDS_PLAYER4 ), COMMANDS_PLAYER4, fonts->medium.normal );
}

//.............................................................................

std::string NewGameMenu::getCoopPlayerName( int num )
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

void NewGameMenu::freeCooperativeMenu()
{
	delete coopBigText;
	coopBigText = NULL;

	std::map< int, OguiButton* >::iterator i;
	
	i = selectButtons.find( COMMANDS_PLAYER1 );
	if( i != selectButtons.end() )
	{
		delete i->second;
		selectButtons.erase( i );
	}

	i = selectButtons.find( COMMANDS_PLAYER2 );
	if( i != selectButtons.end() )
	{
		delete i->second;
		selectButtons.erase( i );
	}
	
	i = selectButtons.find( COMMANDS_PLAYER3 );
	if( i != selectButtons.end() )
	{
		delete i->second;
		selectButtons.erase( i );
	}
	
	i = selectButtons.find( COMMANDS_PLAYER4 );
	if( i != selectButtons.end() )
	{
		delete i->second;
		selectButtons.erase( i );
	}
	
}

//.............................................................................

OguiButtonStyle* NewGameMenu::loadStyle( const std::string& button_name )
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

void NewGameMenu::openCoopProfileMenu( int i )
{
	coopCaptureEvents = ogui->CreateSimpleImageButton( win, 0, 0, getLocaleGuiInt( "gui_newgamemenu_window_w", 1024 ), getLocaleGuiInt( "gui_newgamemenu_window_h", 768 ), NULL, NULL, NULL, 0 );
	coopCaptureEvents->SetListener( this );

	int scroll_button_w = 0;

	if( selectListStyle == NULL )
	{
		OguiButtonStyle* unselStyle = loadStyle( "gui_newgamemenu_unselected_item" );
		OguiButtonStyle* selStyle = loadStyle( "gui_newgamemenu_selected_item" );

		OguiButtonStyle* newStyle = loadStyle( "gui_newgamemenu_unselected_item" );
		OguiButtonStyle* newUnStyle = loadStyle( "gui_newgamemenu_selected_item" );

		OguiButtonStyle* scrollUpStyle = loadStyle( "gui_newgamemenu_arrow_up" );
		OguiButtonStyle* scrollDownStyle = loadStyle( "gui_newgamemenu_arrow_down" );

		scroll_button_w = scrollUpStyle->sizeX>scrollDownStyle->sizeX?scrollUpStyle->sizeX:scrollDownStyle->sizeX;

		int num_of_elements = getLocaleGuiInt( "gui_newgamemenu_number_of_items", 0 );
	
		selectListStyle = new OguiSelectListStyle( unselStyle, selStyle, newStyle, newUnStyle, scrollUpStyle, scrollDownStyle, unselStyle->sizeX, unselStyle->sizeY * num_of_elements, scrollUpStyle->sizeX, scrollUpStyle->sizeY );

	}
	
	{
		// buttonW	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
		// buttonH	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );

		int x	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_x", 0 );
		int y	= getLocaleGuiInt( "gui_newgamemenu_coopprofile_y", 0 );

		x += buttonAddX * convertToRunningNum( i );
		y += buttonAddY * convertToRunningNum( i );
		y += buttonH;

		int w	= buttonW + scroll_button_w;
		int h	= getLocaleGuiInt( "gui_newgamemenu_number_of_items", 0 ) * buttonAddY;

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

void NewGameMenu::setCoopPlayer( int player_num, const std::string& nam )
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
	
}

//.............................................................................

void NewGameMenu::closeCoopProfileMenu()
{
	delete coopCaptureEvents;
	coopCaptureEvents = NULL;

	delete coopProfileListSaver;
	coopProfileListSaver = NULL;

	delete coopProfileList;
	coopProfileList = NULL;
}

void NewGameMenu::createBonusOptions(game::Game *game, OguiWindow *win, Ogui *ogui, MenuCollection::Fonts *fonts,
												std::vector<OguiCheckBox *> &boxes, std::vector<OguiButton *> &buttons, std::list<OguiTextLabel *> &texts,
												IOguiButtonListener *button_listener, int button_id_offset,
												int offset_x, int offset_y)
{
#ifdef PROJECT_SURVIVOR
	if(game->bonusManager && !game->bonusManager->areOptionsAvailable())
		return;

	OguiTextLabel* foo;

	std::string	header = getLocaleGuiString( "gui_newgamemenu_text_bonusoptions" );

	int headerX = getLocaleGuiInt( "gui_newgamemenu_text_bonusoptions_x", 0 ) + offset_x;
	int headerY = getLocaleGuiInt( "gui_newgamemenu_text_bonusoptions_y", 0 ) + offset_y;
	int headerW = getLocaleGuiInt( "gui_newgamemenu_text_bonusoptions_w", 0 );
	int headerH = getLocaleGuiInt( "gui_newgamemenu_text_bonusoptions_h", 0 );


	foo = ogui->CreateTextLabel( win, headerX, headerY, headerW, headerH, header.c_str() );
	foo->SetFont( fonts->big.normal );
	foo->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	texts.push_back(foo);

	int numOptions = game->bonusManager->getNumOptions();
	if(numOptions == 0)
	{
		int	optionX = getLocaleGuiInt( "gui_newgamemenu_text_nobonusoptions_x", 0 ) + offset_x;
		int optionY = getLocaleGuiInt( "gui_newgamemenu_text_nobonusoptions_y", 0 ) + offset_y;
		int optionW = getLocaleGuiInt( "gui_newgamemenu_text_nobonusoptions_w", 0 );
		int optionH = getLocaleGuiInt( "gui_newgamemenu_text_nobonusoptions_h", 0 );

		std::string	text = getLocaleGuiString( "gui_newgamemenu_text_nobonusoptions" );
		foo = ogui->CreateTextLabel( win, optionX, optionY, optionW, optionH, text.c_str() );
		foo->SetFont( fonts->little.normal );
		foo->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
		texts.push_back(foo);
	}
	else
	{
		int	x = getLocaleGuiInt( "gui_newgamemenu_bonusoptions_x", 0 ) + offset_x;
		int y = getLocaleGuiInt( "gui_newgamemenu_bonusoptions_y", 0 ) + offset_y;
		int w = getLocaleGuiInt( "gui_newgamemenu_bonusoptions_w", 0 );
		int h = getLocaleGuiInt( "gui_newgamemenu_bonusoptions_h", 0 );
		int addX = getLocaleGuiInt( "gui_newgamemenu_bonusoptions_add_x", 0);
		int addY = getLocaleGuiInt( "gui_newgamemenu_bonusoptions_add_y", 0);
		int text_w = getLocaleGuiInt( "gui_newgamemenu_bonusoptions_text_w", 0 );
		std::string img_norm = getLocaleGuiString( "gui_newgamemenu_bonusoptions_img_norm" );
		std::string img_fill = getLocaleGuiString( "gui_newgamemenu_bonusoptions_img_fill" );

		for(int i = 0; i < numOptions; i++)
		{
			const std::string &name = game->bonusManager->getOptionName(i);

			std::string	text = getLocaleGuiString( ("gui_newgamemenu_bonus_" + name).c_str() );

			if(game->bonusManager->shouldCreateAsButton(i))
			{
				OguiButton *b;
				b = ogui->CreateSimpleTextButton(win, x+w, y+1, text_w, h, NULL, NULL, NULL, "", button_id_offset + i, 0, false);
				b->SetFont(fonts->medium.highlighted);
				b->SetHighlightedFont(fonts->medium.normal);
				b->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
				b->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
				b->SetText(text.c_str());
				b->SetListener(button_listener);
				buttons.push_back( b );
			}
			else
			{
				OguiCheckBox* b;
				b = new OguiCheckBox( win, ogui, x, y, w, h, 
					img_norm.c_str(), "", "", 
					img_fill.c_str(),	"", "", i );
				
				b->setText( text.c_str(), OguiCheckBox::TEXT_ALIGN_RIGHT, text_w, fonts->medium.highlighted, OguiButton::TEXT_V_ALIGN_TOP  );
				b->setValue(game->bonusManager->isActive(i));
				boxes.push_back( b );
			}

			x += addX;
			y += addY;
		}
	}
#endif
}

void NewGameMenu::applyBonusOptions(game::Game *game, std::vector<OguiCheckBox *> &boxes)
{
#ifdef PROJECT_SURVIVOR
	game->bonusManager->deactivateAll();

	for( unsigned int i = 0; i < boxes.size(); i++ )
	{
		if(boxes[i]->getValue())
			game->bonusManager->activateOption(boxes[i]->getId());
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui

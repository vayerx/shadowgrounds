
#include "precompiled.h"

#include <sstream>
#include <assert.h>
#include <boost/lexical_cast.hpp>

#include "LoadGameMenu.h"

#ifdef PROJECT_SURVIVOR
	#include "CoopMenu.h"
	#include "NewGameMenu.h"
	#include "../game/BonusManager.h"

	//#define GC_BUILD_2007 1

	#ifdef GC_BUILD_2007
		#include "SurvivalMenu.h"
	#endif
#endif

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

#include "../util/fb_assert.h"
#include "../util/Debug_MemoryManager.h"
#include "../util/StringUtil.h"

#include "../ui/LoadingMessage.h"

#include "../ogui/OguiLocaleWrapper.h"
#include "../ogui/OguiCheckBox.h"
#include "../ogui/OguiFormattedText.h"

using namespace game;

namespace ui
{

bool LoadGameMenu::startAsCoop = false;
int LoadGameMenu::selectedSelection = -1;

	int mission_max = 0;
namespace {
	std::vector< bool >	availableMissions;

/*
	void setSinglePlayer( Game* game )
	{
		SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_ENABLED, true );
		SimpleOptions::setBool( DH_OPT_B_2ND_PLAYER_ENABLED, false );
		SimpleOptions::setBool( DH_OPT_B_3RD_PLAYER_ENABLED, false );
		SimpleOptions::setBool( DH_OPT_B_4TH_PLAYER_ENABLED, false );

		if( game->getGameUI()->getController( 0 )->getControllerType() == GameController::CONTROLLER_TYPE_KEYBOARD )
		{
			SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR, true );
		}
		else
		{
			SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR, false );
			SimpleOptions::setInt( DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME, game->getGameUI()->getController( 0 )->getControllerType() );
		}

		game->gameUI->getController( 1 )->unloadConfiguration();
		game->gameUI->getController( 2 )->unloadConfiguration();
		game->gameUI->getController( 3 )->unloadConfiguration();
	}
*/
}

class LoadGameMenu::SurvivorInfoScreen
{
public:
	///////////////////////////////////////////////////////////////////////

	SurvivorInfoScreen( Ogui* ogui, OguiWindow* window ) :
		ogui( ogui ),
		window( window ),
		oguiFonts(),
		scrollUpArrow( NULL ),
		scrollDownArrow( NULL )
	{
		LoadFormattedText();
	}

	~SurvivorInfoScreen()
	{
		delete theTextBox;
		{
			std::map< std::string, IOguiFont* >::iterator i = oguiFonts.begin();
			for ( i = oguiFonts.begin(); i != oguiFonts.end(); ++i )
			{
				delete i->second;
				i->second = NULL;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////

	void LoadFormattedText()
	{
		const int id = 1010;
		const int x	= getLocaleGuiInt( "gui_loadgamemenu_survivor_infoscreen_x", 0 );
		const int y	= getLocaleGuiInt( "gui_loadgamemenu_survivor_infoscreen_y", 0 );
		const int w	= getLocaleGuiInt( "gui_loadgamemenu_survivor_infoscreen_w", 0 );
		const int h	= getLocaleGuiInt( "gui_loadgamemenu_survivor_infoscreen_h", 0 );

		const std::string font_normal = getLocaleGuiString( "gui_loadgamemenu_survivor_infoscreen_font_normal" );
		const std::string font_b = getLocaleGuiString( "gui_loadgamemenu_survivor_infoscreen_font_bold" );
		const std::string font_i = getLocaleGuiString( "gui_loadgamemenu_survivor_infoscreen_font_italic" );
		const std::string font_h1 = getLocaleGuiString( "gui_loadgamemenu_survivor_infoscreen_font_h1" );
		
		theOriginalText = getLocaleGuiString( "gui_loadgame_survivor_infoscreen_original_text" );
		workingCopyText = theOriginalText;

		OguiFormattedText* return_value = new OguiFormattedText( window, ogui, x, y, w, h, id );
		
		if( font_normal.empty() == false )
			return_value->setFont( LoadFont( font_normal ) );
		
		if( font_b.empty() == false )
			return_value->registerFont( "b", LoadFont( font_b ) );
		
		if( font_i.empty() == false )
			return_value->registerFont( "i", LoadFont( font_i ) );
		
		if( font_h1.empty() == false )
			return_value->registerFont( "h1", LoadFont( font_h1 ) );

		return_value->setText( "" );

		theTextBox = return_value;
	}

	///////////////////////////////////////////////////////////////////////

	IOguiFont* LoadFont( const std::string& name )
	{
		fb_assert( name.empty() == false );

		std::map< std::string, IOguiFont* >::iterator i;
		i = oguiFonts.find( name );
		if( i != oguiFonts.end() )
			return i->second;

		IOguiFont* font = ogui->LoadFont( name.c_str() );
		if( font == NULL )
			return NULL;

		oguiFonts.insert( std::pair< std::string, IOguiFont* >( name, font ) );

		return font;
	}

	///////////////////////////////////////////////////////////////////////

	void ResetText()
	{
		workingCopyText = theOriginalText;
	}

	void SetTextString( const std::string& key, const std::string& value )
	{
		workingCopyText = util::StringReplace( key, value, workingCopyText ); 
	}

	void RenderText()
	{
		theTextBox->setText( workingCopyText );
	}

	///////////////////////////////////////////////////////////////////////

	Ogui* ogui;
	OguiWindow* window;
	
	std::map< std::string, IOguiFont* > oguiFonts;
	
	std::string			theOriginalText;
	std::string			workingCopyText;
	OguiFormattedText*	theTextBox;

	OguiButton*			scrollUpArrow;
	OguiButton*			scrollDownArrow;

};


LoadGameMenu::LoadGameMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, Game* g ) :
  MenuBaseImpl( NULL ),
  menuCollection( menu ),
  fonts( fonts ),
  doubleClickHack( -1 ),
  doubleClickTimeHack( 0 ),
  hideLoadGame( true ),
  scrollCount( 0 ),
  scrollPosition( 0 ),
  buttonYMinLimit( 64 ),
  buttonYMaxLimit( 450 ),
  survivorInfoScreen( NULL ),
	bonusOptionWindow( NULL )
{
	assert( o_gui );
	assert( menu );
	assert( fonts );
	assert( g );
	
#ifdef PROJECT_SURVIVOR
	bool survivor = true;
#else 
	bool survivor = false;
#endif

	// survivor = false;

	if (mission_max == 0)
	{
		mission_max = SimpleOptions::getInt(DH_OPT_I_SAVEGAME_SLOT_AMOUNT);
	}
	// -jpk

	Logger::getInstance()->debug( ((std::string)("LoadGameMenu - Mission amount ") + boost::lexical_cast< std::string >( mission_max )).c_str() );
	
	game = g;

	ogui = o_gui;
	win = ogui->CreateSimpleWindow( getLocaleGuiInt( "gui_loadgamemenu_window_x", 0 ), getLocaleGuiInt( "gui_loadgamemenu_window_y", 0 ), getLocaleGuiInt( "gui_loadgamemenu_window_w", 1024 ), getLocaleGuiInt( "gui_loadgamemenu_window_h", 768 ), NULL );
	win->Hide();
	win->SetUnmovable();

	imageSelectNorm = ogui->LoadOguiImage( buttonNormal.c_str() );
	imageSelectDown = ogui->LoadOguiImage( buttonDown.c_str() );

	menu->setBackgroundImage( getLocaleGuiString( "gui_loadgamemenu_background_image" ) );

	fontSelectNorm	= ogui->LoadFont( buttonFontSelectNormal.c_str() );
	fontSelectDown	= ogui->LoadFont( buttonFontSelectDown.c_str() );
	fontDescNorm	= ogui->LoadFont( buttonFontDescNormal.c_str() );
	fontDescDown	= ogui->LoadFont( buttonFontDescDown.c_str() );


	// Main menu buttons
	buttonX	= getLocaleGuiInt( "gui_loadgamemenu_button_x", 0 );
	buttonY	= getLocaleGuiInt( "gui_loadgamemenu_button_y", 0 );
	buttonW	= getLocaleGuiInt( "gui_loadgamemenu_button_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
	buttonH	= getLocaleGuiInt( "gui_loadgamemenu_button_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );

	buttonXStart = buttonX;
	buttonXLimit= getLocaleGuiInt( "gui_loadgame_menu_buttons_max_width", 0 ); 

	buttonAddX = getLocaleGuiInt( "gui_loadgamemenu_button_add_x", getLocaleGuiInt( "gui_menu_common_button_add_x", 0 ) );
	buttonAddY = getLocaleGuiInt( "gui_loadgamemenu_button_add_y", getLocaleGuiInt( "gui_menu_common_button_add_y", 28 ) );


	buttonDescriptionW = getLocaleGuiInt( "gui_loadgamemenu_desc_w", 0 );
	buttonDescriptionH = getLocaleGuiInt( "gui_loadgamemenu_desc_h", 0 );


	missionDescriptions.resize( mission_max + 1 );
	missionTimes.resize( mission_max + 1 );

	int desc_x_add = getLocaleGuiInt( "gui_loadgamemenu_desc_x_add", 0 );
	int time_x_add = getLocaleGuiInt( "gui_loadgamemenu_time_x_add", 0 );

	buttonYMinLimit = getLocaleGuiInt( "gui_loadgamemenu_button_ymin_limit", 0 );
	buttonYMaxLimit = getLocaleGuiInt( "gui_loadgamemenu_button_ymax_limit", 768 );

	///////////////////////////////////////////////////////////////////////////
	int newst = -1;
	if( survivor == false )
	{
		int i;
		std::string newst_str = "";
		availableMissions.resize( mission_max + 1 );
		for( i = 1; i <= mission_max; i++ )
		{
			std::stringstream foo;

			foo << i;
			bool add = g->getInfoForSavegame( foo.str().c_str(), "savegame" );
			
			availableMissions[ i ] = add;
			missionDescriptions[ i ] = savegame_description;
			missionTimes[ i ] = savegame_time;


			if ( add )
			{
				SelectionButtonDescs* descs = new SelectionButtonDescs;

				descs->first = addDescription( savegame_description, desc_x_add, 0, fonts->little.normal );
				descs->second = addDescription( savegame_time, time_x_add, 0, fonts->little.normal );
				addSelectionButton( getMissionName( i ), i, fonts->medium.normal, descs );

				if( savegame_time > newst_str )
				{
					newst_str = savegame_time;
					newst = i;
				}

				selectionButtonDescs.push_back( descs );

			} else {
				addButton( getLocaleGuiString( "gui_lgm_empty" ), i, fonts->medium.normal, NULL, NULL, NULL, OguiButton::TEXT_H_ALIGN_LEFT );
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////
	else if( survivor )
	{
		int i;
		std::string newst_str = "";
		availableMissions.resize( mission_max + 1 );

		int time_loading_start = Timer::getCurrentTime();
		bool show_loading_bar = false;

		for( i = 1; i <= mission_max; i++ )
		{
			std::stringstream foo;
			// only show coop saves
			if(startAsCoop) foo << "coop_";
			foo << i;
			
			bool add = g->getInfoForSavegame( foo.str().c_str(), "savegame" );

			availableMissions[ i ] = add;
			missionDescriptions[ i ] = savegame_description;
			missionTimes[ i ] = savegame_time;

			if( add )
			{
				std::string temp = std::string( "gui_loadgamemenu_mission_" ) + boost::lexical_cast< std::string >( i ) + "_image_norm";
				const std::string image_norm = getLocaleGuiString( temp.c_str() );

				temp = std::string( "gui_loadgamemenu_mission_" ) + boost::lexical_cast< std::string >( i ) + "_image_high";
				const std::string image_high = getLocaleGuiString( temp.c_str() );

				temp = std::string( "gui_loadgamemenu_mission_" ) + boost::lexical_cast<std::string >( i ) + "_image_down";
				const std::string image_down = getLocaleGuiString( temp.c_str() );

				temp = std::string( "gui_loadgamemenu_mission_" ) + boost::lexical_cast< std::string >( i ) + "_image_disa";
				const std::string image_disabled = getLocaleGuiString( temp.c_str() );

				temp = std::string( "gui_loadgamemenu_mission_" ) + boost::lexical_cast< std::string >( i ) + "_image_selected_norm";
				const std::string image_selected_norm = getLocaleGuiString( temp.c_str() );

				temp = std::string( "gui_loadgamemenu_mission_" ) + boost::lexical_cast< std::string >( i ) + "_image_selected_high";
				const std::string image_selected_high = getLocaleGuiString( temp.c_str() );

				this->addImageSelectionButton( image_norm, image_high, image_down, image_disabled, !add, i, NULL );
				// SelectionButtonDescs* descs = new SelectionButtonDescs;
				
				if( selectButtons[ i ] )
					selectButtons[ i ]->SetSelectedImages( ogui->LoadOguiImage( image_selected_norm.c_str() ), ogui->LoadOguiImage( image_selected_high.c_str() ) );

				// descs->first = addDescription( savegame_description, desc_x_add, 0, fonts->little.normal );
				// descs->second = addDescription( savegame_time, time_x_add, 0, fonts->little.normal );
				// addSelectionButton( getMissionName( i ), i, fonts->medium.normal, descs );

				if( add && savegame_time > newst_str )
				{
					newst_str = savegame_time;
					newst = i;
				}

				// selectionButtonDescs.push_back( descs );

			
			} 

			if(i == 5)
			{
				// loading first 5 entries took longer than 100 ms
				if(Timer::getCurrentTime() - time_loading_start > 100)
				{
					// show loading bar
					show_loading_bar = true;
				}
			}

			if(show_loading_bar && i%5 == 0)
			{
				SET_LOADING_BAR_TEXT(getLocaleGuiString("gui_loadingbar_loading"));
				SHOW_LOADING_BAR(i*100/mission_max);
			}
		}

		if( scrollCount > 0 )
		{
			scrollMissionsDown();
			// scrollMissionsUp();
		}
	}
	///////////////////////////////////////////////////////////////////////////
	
	// addSmallButton( getLocaleGuiString( "gui_lgm_arrow_up" ), COMMANDS_ARROWUP, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled );
	// addSmallButton( getLocaleGuiString( "gui_lgm_arrow_down" ), COMMANDS_ARROWDOWN, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled );

	if( survivor )
	{
		{
			const int x = getLocaleGuiInt( "gui_loadgamemenu_loadbutton_x", 0 );
			const int y = getLocaleGuiInt( "gui_loadgamemenu_loadbutton_y", 0 );
			const int w = getLocaleGuiInt( "gui_loadgamemenu_loadbutton_w", 0 );
			const int h = getLocaleGuiInt( "gui_loadgamemenu_loadbutton_h", 0 );

			buttonX = x;
			buttonY = y;
			buttonW = w;
			buttonH = h;
			addButton( getLocaleGuiString( "gui_lgm_load" ), COMMANDS_LOAD, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, OguiButton::TEXT_H_ALIGN_LEFT );
		}

#ifdef PROJECT_SURVIVOR
		if(game->bonusManager->getNumOptions() > 0 && game->bonusManager->areOptionsAvailable())
		{
			const int x = getLocaleGuiInt( "gui_loadgamemenu_bonusoptionsbutton_x", 0 );
			const int y = getLocaleGuiInt( "gui_loadgamemenu_bonusoptionsbutton_y", 0 );
			const int w = getLocaleGuiInt( "gui_loadgamemenu_bonusoptionsbutton_w", 0 );
			const int h = getLocaleGuiInt( "gui_loadgamemenu_bonusoptionsbutton_h", 0 );

			buttonX = x;
			buttonY = y;
			buttonW = w;
			buttonH = h;
			addButton( getLocaleGuiString( "gui_lgm_bonusoptions" ), COMMANDS_BONUSOPTIONS, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled, OguiButton::TEXT_H_ALIGN_LEFT );
		}
#endif


		survivorInfoScreen = new SurvivorInfoScreen( ogui, win );

		if( scrollCount > 0 )
		{
			const int x = getLocaleGuiInt( "gui_loadgamemenu_arrow_up_x", 0 );
			const int y = getLocaleGuiInt( "gui_loadgamemenu_arrow_up_y", 0 );
			const int w = getLocaleGuiInt( "gui_loadgamemenu_arrow_up_w", 0 );
			const int h = getLocaleGuiInt( "gui_loadgamemenu_arrow_up_h", 0 );

			const std::string image_norm = getLocaleGuiString( "gui_loadgamemenu_arrow_up_image_norm" );
			const std::string image_down = getLocaleGuiString( "gui_loadgamemenu_arrow_up_image_down" );
			const std::string image_high = getLocaleGuiString( "gui_loadgamemenu_arrow_up_image_high" );
			const std::string image_disa = getLocaleGuiString( "gui_loadgamemenu_arrow_up_image_disa" );

			survivorInfoScreen->scrollUpArrow = this->addImageButtton( image_norm, image_down, image_high, image_disa, COMMANDS_ARROWUP, x, y, w, h );
		}

		if( scrollCount > 0 )
		{
			const int x = getLocaleGuiInt( "gui_loadgamemenu_arrow_down_x", 0 );
			const int y = getLocaleGuiInt( "gui_loadgamemenu_arrow_down_y", 0 );
			const int w = getLocaleGuiInt( "gui_loadgamemenu_arrow_down_w", 0 );
			const int h = getLocaleGuiInt( "gui_loadgamemenu_arrow_down_h", 0 );

			const std::string image_norm = getLocaleGuiString( "gui_loadgamemenu_arrow_down_image_norm" );
			const std::string image_down = getLocaleGuiString( "gui_loadgamemenu_arrow_down_image_down" );
			const std::string image_high = getLocaleGuiString( "gui_loadgamemenu_arrow_down_image_high" );
			const std::string image_disa = getLocaleGuiString( "gui_loadgamemenu_arrow_down_image_disa" );

			survivorInfoScreen->scrollDownArrow = this->addImageButtton( image_norm, image_down, image_high, image_disa, COMMANDS_ARROWDOWN, x, y, w, h );
		}

		if( scrollCount > 0 )
			updateArrows();


	}
	else
	{
		addSmallButton( getLocaleGuiString( "gui_lgm_load" ), COMMANDS_LOAD, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled );
	}

	///////////////////////////////////////////////////////////////////////////

	// addCloseButton( getLocaleGuiString( "gui_lgm_closeme" ), COMMANDS_CLOSEME, fonts->medium.normal );

	int to_be_selected = ( selectedSelection == -1 && newst != -1 )?newst:selectedSelection;

	if( to_be_selected >= 0 &&  to_be_selected < (signed)availableMissions.size() )
	{
		if( availableMissions[ to_be_selected ] )
			selectButton( to_be_selected );
	}

	/*
	if ( selectedSelection == -1 && newst != -1 )
		selectButton( newst );
	else 
		selectButton( selectedSelection );
	*/

	{
		headerTextX = getLocaleGuiInt( "gui_loadgamemenu_header_x", getLocaleGuiInt( "gui_menu_common_header_x", 0 ) );
		headerTextY = getLocaleGuiInt( "gui_loadgamemenu_header_y", getLocaleGuiInt( "gui_menu_common_header_y", 0 ) );


		if(startAsCoop) addHeaderText( getLocaleGuiString( "gui_lgm_header_coop" ), fonts->big.normal );
		else addHeaderText( getLocaleGuiString( "gui_lgm_header" ), fonts->big.normal );
		// addCloseButton( "OPEN MENU", COMMANDS_OPENME, fonts->big );
	}
	
	///////////////////////////////////////////////////////////////////////////

	if( hideLoadGame )
	{
		std::list< OguiButton* >::iterator i;
		for( i = buttons.begin(); i != buttons.end(); ++i )
		{
			if( (*i)->GetId() == COMMANDS_LOAD )
			{
				// (*i)->SetTransparency( 50 ); 
				(*i)->SetDisabled( true );

			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	
	if( game->inCombat )
	{
		closeMenuByEsc = false;
	}
	else
	{
		closeMenuByEsc = true;
		editHandle = game->gameUI->getController(0)->addKeyreader( this );
		debugKeyreader( editHandle, false, "LoadGameMenu::LoadGameMenu()" );
	}
	
	///////////////////////////////////////////////////////////////////////////



}

LoadGameMenu::~LoadGameMenu()
{

	if( closeMenuByEsc )
	{
		game->gameUI->getController(0)->removeKeyreader( editHandle );
		debugKeyreader( editHandle, true, "LoadGameMenu::~LoadGameMenu()" );
	}

	delete headerText;

	delete imageSelectNorm;
	delete imageSelectDown;

	std::map< int, OguiButton* >::iterator i;
	for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
	{
		delete i->second;
	}

	while( selectionButtonDescs.empty() == false )
	{
		delete selectionButtonDescs.front();
		selectionButtonDescs.pop_front();
	}

	while( !buttons.empty() )
	{
		delete *(buttons.begin());
		buttons.pop_front();
	}

	delete fontSelectNorm;
	delete fontSelectDown;
	delete fontDescNorm;
	delete fontDescDown;

	{
#ifdef PROJECT_SURVIVOR
		// only apply if settings actually exist..
		if(bonusOptionBoxes.size() > 0)
		{
			NewGameMenu::applyBonusOptions(game, bonusOptionBoxes);
		}
#endif

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
	}

	delete win;
}

//.............................................................................

int LoadGameMenu::getType() const
{
	return MenuCollection::MENU_TYPE_LOADGAMEMENU;
}

//.............................................................................

void LoadGameMenu::closeMenu()
{
	assert( menuCollection );

	menuCollection->closeMenu();
}

void LoadGameMenu::openMenu( int m )
{
	assert( menuCollection );
	menuCollection->openMenu( m );
}

void LoadGameMenu::applyChanges()
{
	selectedSelection = activeSelection;
}

//.............................................................................

void LoadGameMenu::CursorEvent( OguiButtonEvent* eve )
{
	MenuBaseImpl::CursorEvent( eve );

//	assert( !eve->cursorOldButtonMask );

	if( eve->eventType == OGUI_EMASK_CLICK )
	{
		if(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_UP_MASK)
		{
			scrollMissionsUp();
			updateArrows();
		}
		else if(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_DOWN_MASK)
		{
			scrollMissionsDown();
			updateArrows();
		}
		else if( eve->triggerButton->GetId() >= COMMANDS_BONUSOPTION_BUTTONS )
		{
#ifdef PROJECT_SURVIVOR
			game->bonusManager->buttonPressed( eve->triggerButton->GetId() - COMMANDS_BONUSOPTION_BUTTONS );
#endif
		}
		else switch( eve->triggerButton->GetId() )
		{
		case COMMANDS_CLOSEME:
			menuClose();
			break;

		case COMMANDS_LOAD:
			menuLoad();
			break;

		case COMMANDS_BONUSOPTIONS:
			menuBonusOptions();
			break;

		case COMMANDS_APPLYBONUSOPTIONS:
			menuCloseBonusOptions();
			break;

		case COMMANDS_ARROWDOWN:
			scrollMissionsDown();
			updateArrows();
			break;
		
		case COMMANDS_ARROWUP:
			scrollMissionsUp();
			updateArrows();
			break;
		default:
			{
				if( doubleClickHack == eve->triggerButton->GetId() && ( Timer::getTime() - doubleClickTimeHack ) < 500  )
				{
					menuLoad();
				} else {
					MenuBaseImpl::CursorEvent( eve );
					doubleClickHack = activeSelection;
					doubleClickTimeHack = Timer::getTime();
				}
				return;
			}
		}
	}
	
}

void LoadGameMenu::selectButton( int command )
{
	int ex_selection = activeSelection;
	MenuBaseImpl::selectButton( command );

	if( activeSelection != - 1 )
	{
		if( hideLoadGame )
		{
			std::list< OguiButton* >::iterator i;
			for( i = buttons.begin(); i != buttons.end(); ++i )
			{
				if( (*i)->GetId() == COMMANDS_LOAD )
				{
					// (*i)->SetTransparency( 0 );
					(*i)->SetDisabled( false );
					hideLoadGame = false;

				}
			}
		}

		if( activeSelection >= 1 && activeSelection <= mission_max )
		{
			missionSelected( activeSelection, ex_selection );
		}
	}
}

//.............................................................................

void LoadGameMenu::menuClose()
{
	closeMenu(); 
}

void LoadGameMenu::menuLoad()
{
	if( activeSelection > 0 )
	{		
#ifdef GC_BUILD_2007
	game->getInfoForSavegame(int2str( activeSelection ), "savegame" );
	if(strstr(savegame_mission_id.c_str(), "surv_") != 0)
	{
		SurvivalMenu::MissionInfo mi;
		if(SurvivalMenu::loadMissionInfo(savegame_mission_id, mi))
		{
			SurvivalMenu::loadMission(mi, game);
		}
		return;
	}
#endif

		if(startAsCoop)
		{
#ifdef PROJECT_SURVIVOR
			CoopMenu::enableCoopGameSettings(game);

			// load coop savegame
			std::string mission = std::string("coop_") + int2str( activeSelection );
			if(game->loadGame(mission.c_str()))
			{
				startAsCoop = false;
			}
#endif
		}
		else
		{
#ifdef PROJECT_SURVIVOR
			CoopMenu::disableCoopGameSettings(game);
#endif
			menuCollection->loadMission( activeSelection );
		}
	}
}

//.............................................................................

std::string LoadGameMenu::getMissionName( int i )
{
	std::stringstream ss;
	ss << i << ".";
	return ss.str();
}

///////////////////////////////////////////////////////////////////////////////

void LoadGameMenu::missionSelected( int m, int ex_selection )
{
	
	if( survivorInfoScreen )
	{
		std::map< int, OguiButton* >::iterator it = selectButtons.find( ex_selection );
		if( it != selectButtons.end() )
		{
			it->second->SetSelected( false );
		}
		
		it = selectButtons.find( m );
		if( it != selectButtons.end() )
		{
			it->second->SetSelected( true );
		}

		if( true )
		{
			std::stringstream foo;
			// only show coop saves
			if(startAsCoop) foo << "coop_";
			foo << m;

			game->getInfoForSavegame(foo.str().c_str(), "savegame" );
			survivorInfoScreen->ResetText();
			survivorInfoScreen->SetTextString( "($savegametype)", savegame_type );
			survivorInfoScreen->SetTextString( "($savegameversion)", savegame_version );
			survivorInfoScreen->SetTextString( "($savegamedescription)", savegame_description );
			survivorInfoScreen->SetTextString( "($savegametime)", savegame_time );
			survivorInfoScreen->RenderText();
		}
	}

}

///////////////////////////////////////////////////////////////////////////////

void LoadGameMenu::addImageSelectionButton( const std::string& image_norm, const std::string& image_high, const std::string& image_down, const std::string& image_disabled, bool disabled, int command, IOguiFont* font, void* param )
{
	assert( ogui );
	assert( win );
	assert( command >= 0 );

	if( command > numberOfWorkingSelectButtons ) numberOfWorkingSelectButtons = command;

	OguiButton* b;
	b = ogui->CreateSimpleImageButton( win, buttonX, buttonY, buttonW, buttonH, image_norm.c_str(), image_down.c_str(), image_high.c_str(), image_disabled.c_str(), command, param, true );
	/*b = ogui->CreateSimpleTextButton( win, buttonX, buttonY, buttonW, buttonH, 
		buttonNormal.c_str(), buttonDown.c_str(), buttonHigh.c_str(), 
		( buttonPaddingString + text ).c_str(), command, param );
		*/
	b->SetListener( this );
	b->SetDisabled( disabled );
	// if ( font ) b->SetFont( font );
	// b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	b->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );
	b->SetClipToWindow( false );
	b->Move( buttonX, buttonY );
	
	if( buttonX == buttonXStart && buttonY > buttonYMaxLimit )
	{
		scrollCount++;
	}

	if( buttonY > buttonYMaxLimit )
	{
		b->MoveBy( 0, 1000 );
	}

	buttonX += buttonAddX;

	if( buttonX > buttonXLimit )
	{
		buttonY += buttonAddY;

		buttonX = buttonXStart;
	}

	selectButtons.insert( std::pair< int, OguiButton* >( command, b ) );
}

///////////////////////////////////////////////////////////////////////////////

void LoadGameMenu::scrollMissionsDown()
{
	// scroll++;
	if( scrollPosition < scrollCount )
	{
		scrollPosition++;
		std::map< int, OguiButton* >::iterator i;
		for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
		{	
			i->second->MoveBy( 0, -buttonAddY );
			if( i->second->GetY() > buttonYMinLimit - 900 && i->second->GetY() < buttonYMinLimit )
			{
				i->second->MoveBy( 0, -1000 );
			}

			if( i->second->GetY() > buttonYMaxLimit && i->second->GetY() < buttonYMaxLimit + 1000 )
			{
				i->second->MoveBy( 0, -1000 );
			}
		}
	}
}

void LoadGameMenu::scrollMissionsUp()
{
	if( scrollPosition > 0 )
	{
		scrollPosition--;
		std::map< int, OguiButton* >::iterator i;
		for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
		{
			i->second->MoveBy( 0, buttonAddY );
			if( i->second->GetY() < buttonYMaxLimit + 900 && i->second->GetY() > buttonYMaxLimit )
			{
				i->second->MoveBy( 0, +1000 ); 
			}

			if( i->second->GetY() < buttonYMinLimit && i->second->GetY() >= buttonYMinLimit - 1000 )
			{
				i->second->MoveBy( 0, +1000 );
			}
		}
	}
}

void LoadGameMenu::updateArrows()
{
	if( survivorInfoScreen )
	{
		if( survivorInfoScreen->scrollUpArrow )
		{
			if( scrollPosition > 0 )
			{
				survivorInfoScreen->scrollUpArrow->SetDisabled( false );
			}
			else
			{
				survivorInfoScreen->scrollUpArrow->SetDisabled( true );
			}
		}
		
		if( survivorInfoScreen->scrollDownArrow )
		{
			if( scrollPosition < scrollCount )
			{
				survivorInfoScreen->scrollDownArrow->SetDisabled( false );
			}
			else
			{
				survivorInfoScreen->scrollDownArrow->SetDisabled( true );
			}
		}
	}
}

void LoadGameMenu::menuBonusOptions()
{
#ifdef PROJECT_SURVIVOR
	hide();

	// not created yet
	if(bonusOptionWindow == NULL)
	{
		bonusOptionWindow = ogui->CreateSimpleWindow( win->GetPositionX(), win->GetPositionY(), win->GetSizeX(), win->GetSizeY(), NULL );
		bonusOptionWindow->SetUnmovable();

		const int offset_x = getLocaleGuiInt( "gui_loadgamemenu_bonusoptions_offset_x", 0 );
		const int offset_y = getLocaleGuiInt( "gui_loadgamemenu_bonusoptions_offset_y", 0 );
		NewGameMenu::createBonusOptions(game, bonusOptionWindow, ogui, fonts, bonusOptionBoxes, bonusOptionButtons, bonusOptionTexts, this, COMMANDS_BONUSOPTION_BUTTONS, offset_x, offset_y);

		// apply button
		{
			const int x = getLocaleGuiInt( "gui_loadgamemenu_applybonusoptionsbutton_x", 0 );
			const int y = getLocaleGuiInt( "gui_loadgamemenu_applybonusoptionsbutton_y", 0 );
			const int w = getLocaleGuiInt( "gui_loadgamemenu_applybonusoptionsbutton_w", 0 );
			const int h = getLocaleGuiInt( "gui_loadgamemenu_applybonusoptionsbutton_h", 0 );

			OguiButton *but = ogui->CreateSimpleTextButton(bonusOptionWindow, x, y, w, h, "", "", "", getLocaleGuiString( "gui_lgm_applybonusoptions" ), COMMANDS_APPLYBONUSOPTIONS);
			but->SetFont(fonts->medium.normal);
			but->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );
			but->SetListener(this);
			if( COMMANDS_APPLYBONUSOPTIONS > numberOfWorkingSelectButtons ) numberOfWorkingSelectButtons = COMMANDS_APPLYBONUSOPTIONS;
			selectButtons.insert(std::pair<int, OguiButton*>(COMMANDS_APPLYBONUSOPTIONS, but));
		}

	}
	else
	{
		// make sure apply button is in place
		const int x = getLocaleGuiInt( "gui_loadgamemenu_applybonusoptionsbutton_x", 0 );
		const int y = getLocaleGuiInt( "gui_loadgamemenu_applybonusoptionsbutton_y", 0 );
		selectButtons[COMMANDS_APPLYBONUSOPTIONS]->Move(x,y);

		// otherwise just show it again
		bonusOptionWindow->Show();
	}

#endif
}

void LoadGameMenu::menuCloseBonusOptions()
{
#ifdef PROJECT_SURVIVOR
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
#endif
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui




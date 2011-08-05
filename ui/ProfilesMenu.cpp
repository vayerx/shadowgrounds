
#include "precompiled.h"

#include <sstream>
#include <assert.h>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#include <malloc.h>
#endif

#include "ProfilesMenu.h"

#include "../ogui/Ogui.h"
#include "../ogui/OguiFormattedText.h"
#include "../game/Game.h"
#include "../game/GameProfiles.h"
#include "../game/GameProfilesEnumeration.h"
#include "../game/GameUI.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_players.h"
#include "GameController.h"
#include "MenuCollection.h"
#include "../game/DHLocaleManager.h"
#include "../util/StringUtil.h"
#include "../game/savegamevars.h"
#include "../game/options/options_game.h"

#include "../util/Debug_MemoryManager.h"

#include "../game/userdata.h"

using namespace game;

namespace ui
{

const static int profiles_max = 10;
	
ProfilesMenu::ProfilesMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, Game* g ) :
  MenuBaseImpl( NULL ),
  menuCollection( menu ),
  fonts( fonts ),
  doubleClickHack( -1 ),
  doubleClickTimeHack( 0 ),
	profileInfoText( NULL )
{
	assert( o_gui );
	assert( menu );
	assert( fonts );
	assert( g );

	game = g;

	gameProfiles = game->getGameProfiles();
	
	ogui = o_gui;
	win = ogui->CreateSimpleWindow( getLocaleGuiInt( "gui_profilesmenu_window_x", 0 ), 
									getLocaleGuiInt( "gui_profilesmenu_window_y", 0 ) , 
									getLocaleGuiInt( "gui_profilesmenu_window_w", 1024 ), 
									getLocaleGuiInt( "gui_profilesmenu_window_h", 768 ), NULL );
	win->Hide();
	win->SetUnmovable();

	menu->setBackgroundImage( getLocaleGuiString( "gui_profilesmenu_background_image" ) );

	imageSelectNorm = ogui->LoadOguiImage( buttonNormal.c_str() );
	imageSelectDown = ogui->LoadOguiImage( buttonDown.c_str() );
	

	// Main menu buttons
	buttonX	= getLocaleGuiInt( "gui_profilesmenu_button_x", 0 );
	buttonY	= getLocaleGuiInt( "gui_profilesmenu_button_y", 0 );
	buttonW	= getLocaleGuiInt( "gui_profilesmenu_button_w", getLocaleGuiInt( "gui_menu_common_button_w", 0 ) );
	buttonH	= getLocaleGuiInt( "gui_profilesmenu_button_h", getLocaleGuiInt( "gui_menu_common_button_h", 0 ) );


	profilesMax = profiles_max;
	profilesCnt = 0;
	profilesCurrent = 0;

	profileEmpty = getLocaleGuiString( "gui_pm_empty" );
	profilePlayer = getLocaleGuiString( "gui_pm_player" );

	/*
	profileData.push_back( "Profile 1" );
	profileData.push_back( "Profile 2" );
	profileData.push_back( "Profile 3" );
	profileData.push_back( profileEmpty );
	profileData.push_back( profileEmpty );
	*/

	int i = 0;

	GameProfilesEnumeration* enum1 = gameProfiles->getProfileList();
	while( enum1->isNextProfileAvailable() )
	{
		std::string temp = enum1->getNextProfile();
		if( temp == std::string( gameProfiles->getCurrentProfile( 0 ) ) ) 
			profilesCurrent = i;
		
		i++;

		profileData.push_back( temp );
	}
	
	delete enum1;
	
	profilesCnt = profileData.size();
	
	for ( i = profileData.size(); i < profilesMax; i++ )
	{
		profileData.push_back( profileEmpty );
	}


	fontSelectNorm	= ogui->LoadFont( buttonFontSelectNormal.c_str() );
	fontSelectDown	= ogui->LoadFont( buttonFontSelectDown.c_str() );
	fontDescNorm	= ogui->LoadFont( buttonFontDescNormal.c_str() );
	fontDescDown	= ogui->LoadFont( buttonFontDescDown.c_str() );

	// textEditButton = o_gui->CreateSimpleTextButton( win, 

	createProfileButtons();

#ifdef PROJECT_SURVIVOR
	createProfileInfoText();
#endif

	addSmallButton( getLocaleGuiString( "gui_pm_select" ), COMMANDS_SELECT, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled );
	addSmallButton( getLocaleGuiString( "gui_pm_new" ),    COMMANDS_NEW, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled );
	addSmallButton( getLocaleGuiString( "gui_pm_delete" ), COMMANDS_DELETE, fonts->medium.normal, fonts->medium.highlighted, fonts->medium.down, fonts->medium.disabled );
	// addCloseButton( getLocaleGuiString( "gui_pm_closeme" ), COMMANDS_CLOSEME, fonts->medium.normal );

	headerTextX = getLocaleGuiInt( "gui_profilesmenu_header_x", getLocaleGuiInt( "gui_menu_common_header_x", 0 ) );
	headerTextY = getLocaleGuiInt( "gui_profilesmenu_header_y", getLocaleGuiInt( "gui_menu_common_header_y", 0 ) );

	addHeaderText( getLocaleGuiString( "gui_pm_header" ), fonts->big.normal );
	// addCloseButton( "OPEN MENU", COMMANDS_OPENME, fonts->big );
	
	///////////////////////////////////////////////////////////////////////////

	editHandle = game->gameUI->getController(0)->addKeyreader( this );
	closeMenuByEsc = true;
	debugKeyreader( editHandle, false, "ProfilesMenu::ProfilesMenu()" );

	fromGame = game->inCombat;
}

ProfilesMenu::~ProfilesMenu()
{
	if(profileInfoText)
	{
		profileInfoText->deleteRegisteredFonts();
		delete profileInfoText->getFont();
		delete profileInfoText;
	}

	game->gameUI->getController(0)->removeKeyreader( editHandle );
	debugKeyreader( editHandle, true, "ProfilesMenu::~ProfilesMenu()" );


	delete headerText;

	delete imageSelectNorm;
	delete imageSelectDown;

	std::map< int, OguiButton* >::iterator i;
	for( i = selectButtons.begin(); i != selectButtons.end(); ++i )
	{
		delete i->second;
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

	delete win;
}

//.............................................................................

void ProfilesMenu::createProfileButtons()
{
	std::map< int, OguiButton* >::iterator it;
	for( it = selectButtons.begin(); it != selectButtons.end(); ++it )
	{
		delete it->second;
	}

	selectButtons.clear();
	numberOfWorkingSelectButtons = 0;

	buttonX	= getLocaleGuiInt( "gui_profilesmenu_button_x", 0 );
	buttonY	= getLocaleGuiInt( "gui_profilesmenu_button_y", 0 );

 
	int i;
	for( i = 0; i < profilesMax; i++ )
	{
		addSelectionButton( getProfileName( i ), i, fonts->medium.normal );
	}

	if( profilesCnt > 0 )
		selectButton( profilesCurrent );

	numberOfWorkingSelectButtons = profilesCnt - 1;
}

void ProfilesMenu::createProfileInfoText()
{
	const int x	= getLocaleGuiInt( "gui_profilesmenu_survivor_infoscreen_x", 0 );
	const int y	= getLocaleGuiInt( "gui_profilesmenu_survivor_infoscreen_y", 0 );
	const int w	= getLocaleGuiInt( "gui_profilesmenu_survivor_infoscreen_w", 0 );
	const int h	= getLocaleGuiInt( "gui_profilesmenu_survivor_infoscreen_h", 0 );

	const std::string font_normal = getLocaleGuiString( "gui_profilesmenu_survivor_infoscreen_font_normal" );
	const std::string font_b = getLocaleGuiString( "gui_profilesmenu_survivor_infoscreen_font_bold" );
	const std::string font_i = getLocaleGuiString( "gui_profilesmenu_survivor_infoscreen_font_italic" );
	const std::string font_h1 = getLocaleGuiString( "gui_profilesmenu_survivor_infoscreen_font_h1" );
	
	profileInfoString = getLocaleGuiString( "gui_profilesmenu_survivor_infoscreen_text" );
	profileInfoText = new OguiFormattedText( win, ogui, x, y, w, h, 0 );
	
	if( font_normal.empty() == false )
		profileInfoText->setFont( ogui->LoadFont( font_normal.c_str() ) );
	
	if( font_b.empty() == false )
		profileInfoText->registerFont( "b", ogui->LoadFont( font_b.c_str() ) );
	
	if( font_i.empty() == false )
		profileInfoText->registerFont( "i", ogui->LoadFont( font_i.c_str() ) );
	
	if( font_h1.empty() == false )
		profileInfoText->registerFont( "h1", ogui->LoadFont( font_h1.c_str() ) );

	profileInfoCache.resize(profilesCnt);

	for(int profile = 0; profile < profilesCnt; profile++)
	{
		// Cache save game info string
		//

		std::string text = profileInfoString;
		text = util::StringReplace("($profile)", getProfileName(profile), text);

		// change profile
		setProfile(profile, false);

		// find newest game
		int newest = 0;
		int mission_max = SimpleOptions::getInt(DH_OPT_I_SAVEGAME_SLOT_AMOUNT);
		for(int i = mission_max; i > 0; i--)
		{
			// load game
			bool loaded = game->getInfoForSavegame( boost::lexical_cast< std::string >( i ).c_str(), "savegame" );
			if(loaded)
			{
				newest = i;
				break;
			}
		}

		if(newest >= 0)
		{
			// load newest game
			game->getInfoForSavegame( boost::lexical_cast< std::string >( newest ).c_str(), "savegame" );

			// parse stats string
			char varname[256];
			char value[256];
			const char *stats = savegame_stats.c_str();
			while(true)
			{
				// scan for variable
				if(sscanf(stats, "%s %s", varname, value) != 2)
					break;
				// next plz
				stats += strlen(varname) + strlen(value) + 2;
				// replace
				text = util::StringReplace(std::string("($") + varname + std::string(")"), value, text);
			}
		}

		// kill all unset variables
		bool erasing = false;
		char *newText = (char *)alloca(text.length() + 1);
		int newTextLength = 0;
		for(unsigned int s = 0; s < text.length(); s++)
		{
			if(!erasing &&
				 text[s] == '(' &&
				 s + 1 < text.length() &&
				 text[s+1] == '$')
			{
				erasing = true;
			}

			if(!erasing)
			{
				newText[newTextLength] = text[s];
				newTextLength++;
			}
			else if(erasing && text[s] == ')')
			{
				erasing = false;
			}
		}
		newText[newTextLength] = 0;

		profileInfoCache[profile] = newText;
	}

	updateProfileInfo(profilesCurrent);
}

int ProfilesMenu::getType() const
{
	return MenuCollection::MENU_TYPE_PROFILESMENU;
}

void ProfilesMenu::updateProfileInfo(int i)
{
	if(profileInfoText == NULL) return;

	if((unsigned)i < profileInfoCache.size())
		profileInfoText->setText(profileInfoCache[i]);
}
//.............................................................................

void ProfilesMenu::closeMenu()
{
	assert( menuCollection );

	menuCollection->closeMenu();
}

void ProfilesMenu::openMenu( int m )
{
	assert( menuCollection );
	menuCollection->openMenu( m );
}

void ProfilesMenu::setProfile(int profile, bool safetyChecks)
{
	if( profile >= 0 && profile < (int)profileData.size() )
	{
		std::string temp = gameProfiles->getCurrentProfile( 0 );
		int i;
		for( i = 1; i < 4; i++ )
		{
			if( profileData[ profile ] == gameProfiles->getCurrentProfile( i ) )
			{
				gameProfiles->setCurrentProfile( temp.c_str(), i, safetyChecks );
				break;
			}
		}
		gameProfiles->setCurrentProfile( profileData[ profile ].c_str(), 0, safetyChecks );
	}
}

void ProfilesMenu::applyChanges()
{
	// TODO to actually do something
	setProfile(activeSelection);

#ifdef LEGACY_FILES
	std::string tmp = "Profiles/";
	tmp += game->getGameProfiles()->getCurrentProfile( 0 );
	tmp += "/Config/keybinds.txt";
#else
	std::string tmp = "profiles/";
	tmp += game->getGameProfiles()->getCurrentProfile( 0 );
	tmp += "/config/keybinds.txt";
#endif

	game->gameUI->getController( 0 )->loadConfiguration( igios_mapUserDataPrefix(tmp).c_str() );
	if( game->gameUI->getController( 0 )->controllerTypeHasMouse() )
	{
		game::SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR, true );
	}
	else
	{
		game::SimpleOptions::setBool( DH_OPT_B_1ST_PLAYER_HAS_CURSOR, false );
	}
	game::SimpleOptions::setInt( DH_OPT_I_1ST_PLAYER_CONTROL_SCHEME, game->gameUI->getController( 0 )->getControllerType() );

}

//.............................................................................

void ProfilesMenu::CursorEvent( OguiButtonEvent* eve )
{
	MenuBaseImpl::CursorEvent( eve );

	if( eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK )
	{

		if( editButtonP )
		{
			if( editButtonEnterCheck( editBuffer ) == false )
			{
				if( eve->triggerButton->GetId() == COMMANDS_CLOSEME )
					menuClose();
				return;
			}
		}

		switch( eve->triggerButton->GetId() )
		{
		case COMMANDS_CLOSEME:
			menuClose();
			break;

		case COMMANDS_SELECT:
			menuSelect();
			break;

		case COMMANDS_NEW:
			menuNew();
			break;

		case COMMANDS_DELETE:
			menuDelete();
			break;

		default:
			{
				if( doubleClickHack == eve->triggerButton->GetId() && ( Timer::getTime() - doubleClickTimeHack ) < 500  )
				{
					menuSelect();
				} else {
					MenuBaseImpl::CursorEvent( eve );
					doubleClickHack = activeSelection;
					doubleClickTimeHack = Timer::getTime();
					updateProfileInfo(activeSelection);
				}
				return;
			}
		}
	}
	else if( eve->eventType ==  OguiButtonEvent::EVENT_TYPE_OVER )
	{
		int id = eve->triggerButton->GetId();
		if( id >= 0 && id < profilesCnt )
			updateProfileInfo( id );
	}
	else if( eve->eventType ==  OguiButtonEvent::EVENT_TYPE_LEAVE )
	{
		updateProfileInfo( activeSelection );
	}
}

//.............................................................................

void ProfilesMenu::readKey( char ascii, int keycode, const char *keycodeName )
{
	if( fromGame ) 
	{
		if( editButtonP )
		{
			switch( keycode )
			{
			case 14: // backspace
				if( !editBuffer.empty() )
					editBuffer.erase( editBuffer.size() - 1 );
				break;

			case 28: // enter
				editButtonEnter( editBuffer );
				break;

			default:
				if ( ascii != '\0' )
					editBuffer += ascii;
				break;
			}

			if( editButtonP )
				editButtonP->SetText( ( editBufferBefore + editBuffer + editBufferAfter ).c_str() );
		} 
		else if( closeMenuByEsc )
		{
		}
	} 
	else 
	{
		//FOOFOOHAXHAXBETA
		if(editBuffer.size() > 18)
			editBuffer = editBuffer.substr(0, 18);

		MenuBaseImpl::readKey( ascii, keycode, keycodeName );
	}
	
	
}

void ProfilesMenu::escPressed()
{
	if( editButtonP )
		editButtonEnter("");
	else closeMenu();
}

void ProfilesMenu::handleEsc()
{
	if( fromGame == false )
		menuClose();
}
//.............................................................................

void ProfilesMenu::menuClose()
{
	applyChanges();

	closeMenu();
}

void ProfilesMenu::menuSelect()
{
	if( activeSelection >= 0 )
	{
	

		// gameController[c]->loadConfiguration(tmp.c_str());

		menuClose();
	}
}

void ProfilesMenu::menuNew()
{
	// TODO to work in that way that dispables all 
	//      the other buttons until enter is given
	if ( profilesCnt < profilesMax )
	{
		profilesCnt++;
		profilesCurrent = profilesCnt - 1;
			
		createProfileButtons();
		std::map< int, OguiButton* >::iterator i = selectButtons.find( profilesCnt - 1 );

		std::stringstream ss; ss << profilePlayer << " " << ( profilesCurrent + 1 );
		
		if ( i != selectButtons.end() )
			editButton( i->second, ss.str(), buttonPaddingString );

	}
	
}

/*
void ProfilesMenu::editButton( OguiButton* b, const std::string& temp )
{
	MenuBaseImpl::editButton( b, temp );
	editBuffer += "|";
	editButtonP->SetText( editBuffer.c_str() );
}*/

void ProfilesMenu::menuDelete( bool delete_from_profiles )
{
	if( activeSelection >= 0 && activeSelection <= profilesCnt )
	//HAXHAX for UK
	if(profilesCnt >= 2)
	{
		profilesCurrent = activeSelection;
		profilesCnt--;
		// if( profilesCnt < 0 ) profilesCnt = 0;
		
		if( delete_from_profiles ) 
			gameProfiles->deleteProfile( profileData[ activeSelection ].c_str() );
		
		int i;
		for( i = activeSelection; i < (signed)profileData.size() - 1; i++ )
		{
			profileData[ i ] = profileData[ i + 1 ];
		}

		activeSelection = -1;

		profileData[ profileData.size() - 1 ] = profileEmpty;
		
		if( profilesCurrent >= profilesCnt ) profilesCurrent = profilesCnt - 1;
		

		createProfileButtons();
		
		if( activeSelection >= 0 && activeSelection < (signed)profileData.size() )
		{
			gameProfiles->setCurrentProfile( profileData[ activeSelection ].c_str(), 0 );
		}

	}
}

bool ProfilesMenu::editButtonEnterCheck( const std::string& string )
{
	assert( gameProfiles );
	bool add_to_profiles = gameProfiles->isProfileNameValid( string.c_str() );

	if( !string.empty() )
	{
		int i;
		for ( i = 0; i < profilesCnt; i++ )
		{
			if( profileData[ i ] == string )
			{
				add_to_profiles = false;
			}
		}

		if( add_to_profiles )
		{
			profileData[ profilesCurrent ] = string; 
			assert( gameProfiles );
			gameProfiles->createNewProfile( string.c_str() );
			gameProfiles->setCurrentProfile( string.c_str(), 0 );
		}
	}

	// if( add_to_profiles ) 
	MenuBaseImpl::editButtonEnter( profileData[ profilesCurrent ] );

	if( string.empty() || add_to_profiles == false ) 
	{
		menuDelete( false );
	}

	return add_to_profiles;
}


void ProfilesMenu::editButtonEnter( const std::string& string )
{
	editButtonEnterCheck( string );
}

//.............................................................................

std::string ProfilesMenu::getProfileName( int i )
{
	assert( i>= 0 && i < (signed)profileData.size() );
	return profileData[ i ];
}

//.............................................................................





} // end of namespace

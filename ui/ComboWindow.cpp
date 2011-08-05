#include "precompiled.h"

#include "ComboWindow.h"
#include "CombatSubWindowFactory.h"

#include "../game/GameStats.h"
#include "../game/Game.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_game.h"
#include "../game/DHLocaleManager.h"

#include "../ogui/Ogui.h"
#include "../ogui/OguiWindow.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiLocaleWrapper.h"

#include "../util/fb_assert.h"

#include <sstream>
#include <boost/lexical_cast.hpp>

namespace ui { 
///////////////////////////////////////////////////////////////////////////////

namespace {
REGISTER_COMBATSUBWINDOW( ComboWindow );
}

class ComboWindow::ComboWindowImpl : public game::IGameStatsListener
{
public:
	///////////////////////////////////////////////////////////////////////////
	
	ComboWindowImpl( Ogui* ogui, game::Game* game, int player_num ) :
		game( game ),
		playerNum( player_num ),
		lastTimeWeKilled( 0 ),
		timeNeededForACombo( 1000 / GAME_TICK_MSEC ),
		comboCount( 1 ),

		lastTimeShownMessage( 0 ),
		showMessage( 0 ),
		comboMessageLength( 1000 / GAME_TICK_MSEC ),

		window( NULL ),
		textbut( NULL ),
	
		oguiLoader( ogui )

	{
		fb_assert( playerNum >= 0 && playerNum < 4 );
		if( game::GameStats::instances[playerNum] )
			game::GameStats::instances[playerNum]->addListener( this );

		// oguiLoader.SetLogging( true, "combowindow.txt" );

		window = oguiLoader.LoadWindow( "combowindow"  );
		
		if( window )
			window->SetReactMask( 0 );
		
		textbut = oguiLoader.LoadButton( "textbutton", window, 0 );
		if( textbut )
			textbut->SetText( "" );
		showMessage = 0;
	}

	//=========================================================================
	
	~ComboWindowImpl()
	{
		fb_assert( playerNum >= 0 && playerNum < 4 );
		if( game::GameStats::instances[playerNum] )
			game::GameStats::instances[playerNum]->removeListener( this );

		delete textbut;
		delete window;
		
	}

	///////////////////////////////////////////////////////////////////////////

	void onKill( const std::string& enemy ) 
	{ 
		onCombo();
	}
	void onMapChange( const std::string& maplayer ) { }
	void onMarker( const std::string& marker ) { }
	void onPickup( const std::string& pickup ) { }
	void onDeath() { }

	///////////////////////////////////////////////////////////////////////////

	void onCombo()
	{
		timeNeededForACombo = (game::SimpleOptions::getInt( DH_OPT_I_GAME_COMBO_REQUIRED_TIME ) / GAME_TICK_MSEC);
		comboMessageLength = (game::SimpleOptions::getInt( DH_OPT_I_GAME_COMBO_SHOWN_ON_SCREEN_TIME ) / GAME_TICK_MSEC);

		if( ( game->gameTimer - lastTimeWeKilled ) <= timeNeededForACombo  )
		{
			lastTimeWeKilled = game->gameTimer;
			comboCount++;
		}
		else
		{
			lastTimeWeKilled = game->gameTimer;
			comboCount = 1;
		}

		if( comboCount > 1 )
		{
			std::string combo;
			{
				std::string temp = std::string( "gui_combowindow_text_combo_" ) + boost::lexical_cast< std::string >( comboCount );
				combo = game::getLocaleGuiString( temp.c_str()  );
				// ss << combo << " " << comboCount << std::endl;
			}

			if( textbut ) 
				textbut->SetText( combo.c_str() );

			showMessage = 1;
			
			lastTimeShownMessage = game->gameTimer;
			// showingMessage = true;

		}
	}

	//=========================================================================
	
	void update()
	{
		if( showMessage > 0 && ( game->gameTimer - lastTimeShownMessage ) > comboMessageLength )
		{
			showMessage = 0;
			bool result = false;
			
			if( textbut ) 
				result = textbut->SetText( "" );

		}
	}

	///////////////////////////////////////////////////////////////////////////

private:
	game::Game* game;

	int playerNum;

	int	lastTimeWeKilled;
	int timeNeededForACombo;
	int comboCount;
	
	int lastTimeShownMessage;
	int showMessage;
	int comboMessageLength;

	OguiWindow* window;
	OguiButton*	textbut;

	OguiLocaleWrapper	oguiLoader;
};

///////////////////////////////////////////////////////////////////////////////


ComboWindow::ComboWindow( Ogui* ogui, game::Game* game, int player_num ) :
	impl( NULL )
{
	impl = new ComboWindowImpl( ogui, game, player_num );
}

//================================================================================

ComboWindow::~ComboWindow()
{
	delete impl;
	impl = NULL;
}

///////////////////////////////////////////////////////////////////////////////

void ComboWindow::show( int time )
{
}

//================================================================================

void ComboWindow::hide( int time )
{
}

//================================================================================

void ComboWindow::update()
{
	fb_assert( impl );
	impl->update();
}

///////////////////////////////////////////////////////////////////////////////

}

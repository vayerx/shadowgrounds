
#include "precompiled.h"

#include "TerminalManager.h"

#include "../game/Game.h"
#include "../game/scripting/GameScripting.h"
#include "../ogui/Ogui.h"
#include "../game/DHLocaleManager.h"

#include "../ui/TerminalWindow.h"

using namespace game;

namespace ui {

const static std::string	log_style_prefix = "log_entry_style_";
const static std::string	log_text_prefix = "log_entry_text_";

const static std::string	variable_state_prefix = "log_entry_state_";
const static std::string	variable_time_prefix = "log_entry_time_";


///////////////////////////////////////////////////////////////////////////////

class TerminalManagerImpl
{
public:
	TerminalManagerImpl( Ogui* ogui, Game* game ) :
		ogui( ogui ),
		game( game ),
		window( NULL ),
		timeCounter( 1 )
	{
	}

	~TerminalManagerImpl()
	{
	}

	void openTerminalWindow( const std::string& name )
	{
		if( window )
		{
			closeTerminalWindow();
		}

		if( DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, (log_text_prefix + name ).c_str() ) && DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, ( log_style_prefix + name ).c_str() ) )
		{
			const std::string& style = getLocaleGuiString( ( log_style_prefix + name ).c_str() );
			const std::string& text = getLocaleGuiString( (log_text_prefix + name ).c_str() );
			window = new TerminalWindow( ogui, game, style );
			window->setText( text );

			game->gameScripting->newGlobalIntVariable( ( variable_state_prefix + name ).c_str(), true );
			game->gameScripting->newGlobalIntVariable( ( variable_time_prefix + name ).c_str(), true );

			timeCounter = game->gameScripting->getGlobalIntVariableValue( "next_log_entry_time" );

			game->gameScripting->setGlobalIntVariableValue( ( variable_state_prefix + name ).c_str(),	1 );
			game->gameScripting->setGlobalIntVariableValue( ( variable_time_prefix + name ).c_str(),	timeCounter++ );
			game->gameScripting->setGlobalIntVariableValue( "next_log_entry_time", timeCounter );
		}
	}

	void closeTerminalWindow()
	{
		if( window )
		{
			delete window;
			window = NULL;
		}
	}

	void update()
	{
		if( window )
		{
			window->update();
			if( window->isVisible() == false )
			{
				closeTerminalWindow();
			}
		}
	}

	bool isWindowOpen() const
	{
		return ( window != NULL && window->isVisible() );
	}

	Ogui*			ogui;
	Game*			game;
	TerminalWindow*	window;
	int				timeCounter;
	
};

///////////////////////////////////////////////////////////////////////////////

TerminalManager::TerminalManager( Ogui* ogui, game::Game* game ) :
	impl( new TerminalManagerImpl( ogui, game ) )
{
	
}

//=============================================================================

TerminalManager::~TerminalManager()
{
	delete impl;
}

///////////////////////////////////////////////////////////////////////////////

void TerminalManager::openTerminalWindow( const std::string& name )
{
	impl->openTerminalWindow( name );
}

//=============================================================================

void TerminalManager::closeTerminalWindow()
{
	impl->closeTerminalWindow();
}

///////////////////////////////////////////////////////////////////////////////

void TerminalManager::update()
{
	impl->update();
}

bool TerminalManager::isWindowOpen() const
{
	return impl->isWindowOpen();
}

///////////////////////////////////////////////////////////////////////////////
 
} // end of namespace ui

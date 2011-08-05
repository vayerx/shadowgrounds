
#include "precompiled.h"

#include "IngameGuiTabs.h"
#include "../ogui/IOguiButtonListener.h"
#include "../ogui/IOguiEffectListener.h"
#include "../ogui/Ogui.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/DHLocaleManager.h"

#include "../system/Logger.h"
#include <sstream>

#include <vector>
#include <string>

using namespace game;

namespace ui {

static const int GUITABS_FADE_IN_TIME = 500;
static const int GUITABS_FADE_OUT_TIME = 500;

class IngameGuiTabsImpl : public IOguiButtonListener, private IOguiEffectListener
{
public:
	IngameGuiTabsImpl( Ogui* ogui, Game* game ) :
		game( game ),
		ogui( ogui ),
		window(NULL)
		, font(NULL)
		, currentActive(-1)
		, lastClosed( -1 )
		, visible(false)
	{
		init();
	}

	~IngameGuiTabsImpl()
	{
		release();
	}

	void CursorEvent( OguiButtonEvent *eve )
	{
		const int exActive = currentActive;
		if( exActive >= 0 && exActive < (int)buttons.size() && 
			eve->triggerButton->GetId() >= 0 && eve->triggerButton->GetId() < (int)buttons.size() )
		{
			
			if( eve->triggerButton->GetId() >= 0 && eve->triggerButton->GetId() < (int)buttons.size() )
			{
				visible = false;
			}
			GameUI::WINDOW_TYPE ex_active;
			ex_active = (GameUI::WINDOW_TYPE)exActive;
			game->gameUI->closeWindow( ex_active, false );
			lastClosed = ex_active;
		}

		switch( eve->triggerButton->GetId() )
		{
		case GameUI::WINDOW_TYPE_LOG:
			game->gameUI->openWindow( GameUI::WINDOW_TYPE_LOG );
			break;

		case GameUI::WINDOW_TYPE_UPGRADE:
			game->gameUI->openWindow( GameUI::WINDOW_TYPE_UPGRADE );
			break;

		case GameUI::WINDOW_TYPE_MAP:
			game->gameUI->openWindow( GameUI::WINDOW_TYPE_MAP );
			break;
		}




		/*game->gameUI->closeWindow( currentActive );
		game->gameUI->openWindow( eve->triggerButton->GetId() );
		*/

	}

	void EffectEvent( OguiEffectEvent *eve )
	{
	}

	void show()
	{
		if( visible == false )
		{
			visible = true;
			window->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, GUITABS_FADE_IN_TIME);
			window->SetReactMask( OGUI_WIN_REACT_MASK_ALL );
			
			if(buttons[ GameUI::WINDOW_TYPE_LOG ])
				buttons[ GameUI::WINDOW_TYPE_LOG ]->SetDisabled( false );

			buttons[ GameUI::WINDOW_TYPE_UPGRADE ]->SetDisabled( false );
			buttons[ GameUI::WINDOW_TYPE_MAP ]->SetDisabled( false );

#ifdef PROJECT_SURVIVOR
			// in survival mode
			int value = 0;
			if(util::Script::getGlobalIntVariableValue("survival_mode_enabled", &value) && value == 1)
			{
				// hide map button
				buttons[ GameUI::WINDOW_TYPE_MAP ]->SetDisabled( true );
			}
#endif
		}
	}

	void hide()
	{
		if( visible == true )
		{
			visible = false;
			window->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, GUITABS_FADE_OUT_TIME);
			window->SetReactMask( 0 );

			if(buttons[ GameUI::WINDOW_TYPE_LOG ])
				buttons[ GameUI::WINDOW_TYPE_LOG ]->SetDisabled( true );

			buttons[ GameUI::WINDOW_TYPE_UPGRADE ]->SetDisabled( true );
			buttons[ GameUI::WINDOW_TYPE_MAP ]->SetDisabled( true );
	
		}
	}

	void update( int ms )
	{
		if( currentActive == lastClosed && visible == true )
		{
			//visible = false;
		}
	}
	
	void setActive( IngameGuiTabs::IngameWindows current_active )
	{
		// std::stringstream ss;
		// ss << "IngameGuiTabs::setActive(), current_active: " << current_active << ", currentActive: " << currentActive;
		// Logger::getInstance()->error( ss.str().c_str() );

		if( currentActive >= 0 && currentActive < (int)buttons.size() )
		{
			if( buttons[ currentActive ] != NULL )
			{
				delete buttons[ currentActive ];
				buttons[ currentActive ] = loadButton( buttonsUp[ currentActive ], (GameUI::WINDOW_TYPE)currentActive );
			}
		}

		if( current_active >= 0 && current_active < (int)buttons.size() )
		{
			if( buttons[ current_active ] != NULL )
			{
				delete buttons[ current_active ];
				buttons[ current_active ] = loadButton( buttonsDown[ current_active ], -1 );
			}
			// visible = true;
			currentActive = current_active;
		}

	}

	//

	void init()
	{

		int x = getLocaleGuiInt( "gui_tabs_window_x", 0 );
		int y = getLocaleGuiInt( "gui_tabs_window_y", 0 );
		int w = getLocaleGuiInt( "gui_tabs_window_w", 0 );
		int h = getLocaleGuiInt( "gui_tabs_window_h", 0 );
		window = ogui->CreateSimpleWindow( x, y, w, h, NULL, 0 );
		window->SetUnmovable();

		buttonsUp.resize( 3 );
		buttonsDown.resize( 3 );
		buttons.resize( 3 );

		font = ogui->LoadFont( getLocaleGuiString( "gui_tabs_font" ) );

		buttonsUp[ GameUI::WINDOW_TYPE_LOG ]		= ( "gui_tabs_log_normal" );
		buttonsUp[ GameUI::WINDOW_TYPE_UPGRADE ] = ( "gui_tabs_upgrade_normal" );
		buttonsUp[ GameUI::WINDOW_TYPE_MAP ]		= ( "gui_tabs_map_normal" );
		
		buttonsDown[ GameUI::WINDOW_TYPE_LOG ]		= ( "gui_tabs_log_down" );
		buttonsDown[ GameUI::WINDOW_TYPE_UPGRADE ]	= ( "gui_tabs_upgrade_down" );
		buttonsDown[ GameUI::WINDOW_TYPE_MAP ]		= ( "gui_tabs_map_down" );

		buttons[ GameUI::WINDOW_TYPE_LOG ]		= NULL;

#ifndef PROJECT_SURVIVOR
		buttons[ GameUI::WINDOW_TYPE_LOG ]		= loadButton( buttonsUp[ GameUI::WINDOW_TYPE_LOG ],		GameUI::WINDOW_TYPE_LOG  );
#endif

		buttons[ GameUI::WINDOW_TYPE_UPGRADE ]	= loadButton( buttonsUp[ GameUI::WINDOW_TYPE_UPGRADE ],	GameUI::WINDOW_TYPE_UPGRADE );
		buttons[ GameUI::WINDOW_TYPE_MAP ]		= loadButton( buttonsUp[ GameUI::WINDOW_TYPE_MAP ],		GameUI::WINDOW_TYPE_MAP );

		visible = true;
	}

	void release()
	{
		delete font;
		font = NULL;

		delete buttons[ GameUI::WINDOW_TYPE_LOG ];
		buttons[ GameUI::WINDOW_TYPE_LOG ] = NULL;

		delete buttons[ GameUI::WINDOW_TYPE_UPGRADE ];
		buttons[ GameUI::WINDOW_TYPE_UPGRADE ] = NULL;

		delete buttons[ GameUI::WINDOW_TYPE_MAP ];
		buttons[ GameUI::WINDOW_TYPE_MAP ] = NULL;

	}

	OguiButton* loadButton( const std::string& name, int id = 0 )
	{
		int x = getLocaleGuiInt( ( name + "_x" ).c_str(), 0 );
		int y = getLocaleGuiInt( ( name + "_y" ).c_str(), 0 );
		int w = getLocaleGuiInt( ( name + "_w" ).c_str(), 0 );
		int h = getLocaleGuiInt( ( name + "_h" ).c_str(), 0 );
		std::string norm = getLocaleGuiString( ( name + "_norm" ).c_str() );
		std::string high = getLocaleGuiString( ( name + "_high" ).c_str() );
		std::string down = getLocaleGuiString( ( name + "_down" ).c_str() );

		std::string text = getLocaleGuiString( ( name + "_text" ).c_str() );

		OguiButton* result;
		result = ogui->CreateSimpleTextButton( window, x, y, w, h, norm.c_str(), down.c_str(), high.c_str(), text.c_str(), id, NULL );
		result->SetFont( font );

		result->SetListener( this );

		return result;
	}

	void raise()
	{
		window->Raise();
	}

	bool isVisible() const
	{
		return visible;
	}

	Game* game;
	Ogui* ogui;
	OguiWindow* window;
	IOguiFont* font;

	int currentActive;
	int lastClosed;
	bool visible;

	std::vector< OguiButton* > buttons;
	std::vector< std::string > buttonsUp;
	std::vector< std::string > buttonsDown;
};

} // end of namespace ui


namespace ui {

IngameGuiTabs::IngameGuiTabs( Ogui* ogui, Game* game ) 
{
	impl = new IngameGuiTabsImpl( ogui, game );
}

IngameGuiTabs::~IngameGuiTabs()
{
	delete impl;
	impl = NULL;
}

void IngameGuiTabs::show()
{
	impl->show();
}

void IngameGuiTabs::hide()
{
	impl->hide();
}

void IngameGuiTabs::setActive( IngameWindows current_active )
{
	impl->setActive( current_active );
}

bool IngameGuiTabs::isVisible() const
{
	return impl->isVisible();
}

void IngameGuiTabs::raise()
{
	impl->raise();
}

void IngameGuiTabs::update( int ms )
{

}

} // end of namespace ui

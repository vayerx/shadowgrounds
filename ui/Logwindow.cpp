
#include "precompiled.h"

#include "LogWindow.h"
#include "LogManager.h"
#include "LogEntry.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiSelectListEvent.h"
#include "../ui/GUIEffectWindow.h"
#include "../game/DHLocaleManager.h"	
#include "../util/assert.h"
#include "../game/scripting/GameScripting.h"
#include "../ui/uidefaults.h"

#include "../system/Logger.h"

#include <vector>
#include <string>
#include <list>
#include <map>

using namespace game;

namespace ui {
///////////////////////////////////////////////////////////////////////////////

namespace {
	static const int LOG_FADE_IN_TIME = 500;
	static const int LOG_FADE_OUT_TIME = 500;

	static const std::string NEW_LOG_VARIABLE_NAME = "new_log_entry";
	static const std::string LOG_HEADER_BEGIN = "<header>";
	static const std::string LOG_HEADER_END =	"</header>";
}


class LogWindowImpl : public IOguiSelectListListener, public IOguiButtonListener, public IOguiEffectListener
{
public:
	///////////////////////////////////////////////////////////////////////////

	LogWindowImpl( Game& game, Ogui& ogui, LogManager& manager ) :
		window( NULL ),
		selectList( NULL ),
		formattedText( NULL ),
		effectWindow( NULL ),
		textAreaBackground( NULL ),
		selectListBackground( NULL ),
		exitButton( NULL ),
		shown( false ),
		visible( true ),
		game( game ),
		ogui( ogui ),
		manager( manager )
	{
		window = ogui.CreateSimpleWindow( 0, 0, 1024, 768, NULL, 0 );
		window->Hide();
		window->SetUnmovable();
		window->SetEffectListener( this );
		init();

		effectWindow = new GUIEffectWindow( &ogui, getLocaleGuiString( "gui_log_window_effect_layer1_image" ), 
			getLocaleGuiString( "gui_log_window_effect_layer2_image" ), getLocaleGuiString( "gui_log_window_effect_layer3_image" )  );

		// selectList = ogui.CreateSelectList( window, x, y, );
	}

	//=========================================================================

	~LogWindowImpl()
	{
		release();
		delete effectWindow;
		effectWindow = NULL;

		delete window;
		window = NULL;

		// Logger::getInstance()->error( "LogWindow::~LogWindow()" );
	}

	///////////////////////////////////////////////////////////////////////////

	bool getVisibility() const
	{
		return shown;
	}

	void SelectEvent( OguiSelectListEvent *eve )
	{
		if( eve->eventType == OguiSelectListEvent::EVENT_TYPE_SELECT )
		{
			selectItem( eve->selectionNumber );
		}
	}

	void selectItem( int i )
	{
		if( i >= 0 )
		{
			FB_ASSERT( formattedText != NULL );
			FB_ASSERT( i >= 0 && i < (int)entries.size() );

			setText( entries[ i ].getText() );
			entries[ i ].setRead( true, &game );
			selectList->highlightItem( i, false );
			currentSelection = i;
		}
	}

	void EffectEvent( OguiEffectEvent *e )
	{
		if( e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT )
		{
			game.gameUI->closeLogWindow();
			// window->Hide();
			// game.gameUI->closeLogWindow();
			// window->Hide();
		}
	}

	void CursorEvent( OguiButtonEvent* eve )
	{
		if( eve->triggerButton == exitButton )
		{
			game.gameUI->closeWindow( game::GameUI::WINDOW_TYPE_LOG );
		}
	}

	void update( int msecTimeDelta )
	{
		FB_ASSERT( effectWindow != NULL );
		effectWindow->update(msecTimeDelta);
	}

	void fadeOut()
	{
		window->StartEffect( OGUI_WINDOW_EFFECT_FADEOUT, LOG_FADE_OUT_TIME );
		effectWindow->fadeOut( LOG_FADE_OUT_TIME );
		game.gameUI->prepareCloseLogWindow();
		visible = false;
		// game.gameUI->prepareCloseLogWindow();
	}

	void fadeIn()
	{
		effectWindow->raise();
		effectWindow->fadeIn( LOG_FADE_IN_TIME );

		window->Raise();
		window->StartEffect( OGUI_WINDOW_EFFECT_FADEIN, LOG_FADE_IN_TIME );
		window->Show();
		shown = true;
	}

	void loadTypes( int type )
	{
		currentLogType = type;
		
		delete selectList;
		selectList = NULL;

		entries.erase( entries.begin(), entries.end() );
		FB_ASSERT( entries.empty() );

		int x = getLocaleGuiInt( "gui_log_select_list_x", 0 );
		int y = getLocaleGuiInt( "gui_log_select_list_y", 0 );
		FB_ASSERT( selectListStyle != NULL );
		selectList = ogui.CreateSelectList( window, x, y, selectListStyle, 0, NULL, NULL );
		FB_ASSERT( selectList != NULL );
		selectList->setListener( this );

		manager.update( &game );
		std::list< LogEntry > logs = manager.getCollectedLogs();

		std::list< LogEntry >::iterator i;
		int j = 0;
		entries.resize( logs.size() );
		for ( i = logs.begin(); i != logs.end(); ++i )
		{
			if( i->getType() == currentLogType )
			{
				selectList->addItem( i->getDescription().c_str(), i->getDescription().c_str() );
				selectList->highlightItem( j, !i->isRead() );
				entries[ j ] = *i;
				j++;
			}
		}
	}

	void init()
	{

		game.gameScripting->setGlobalIntVariableValue( NEW_LOG_VARIABLE_NAME.c_str(), 0 );
		
		{
			int x = getLocaleGuiInt( "gui_log_text_area_background_x", 0 );
			int y = getLocaleGuiInt( "gui_log_text_area_background_y", 0 );
			int w = getLocaleGuiInt( "gui_log_text_area_background_w", 0 );
			int h = getLocaleGuiInt( "gui_log_text_area_background_h", 0 );
			
			textAreaBackground = ogui.CreateSimpleImageButton( window, x, y, w, h, 
				NULL, NULL, NULL, getLocaleGuiString( "gui_log_text_area_background_image" ), 0 );
			if( textAreaBackground != NULL )
				textAreaBackground->SetDisabled( true );

		}

		{
			int x = getLocaleGuiInt( "gui_log_selectlist_background_x", 0 );
			int y = getLocaleGuiInt( "gui_log_selectlist_background_y", 0 );
			int w = getLocaleGuiInt( "gui_log_selectlist_background_w", 0 );
			int h = getLocaleGuiInt( "gui_log_selectlist_background_h", 0 );
			
			selectListBackground = ogui.CreateSimpleImageButton( window, x, y, w, h, 
				NULL, NULL, NULL, getLocaleGuiString( "gui_log_selectlist_background_image" ), 0 );
			if( selectListBackground != NULL )
				selectListBackground->SetDisabled( true );

		}
		
		{
			int x = getLocaleGuiInt( "gui_log_exit_x", 0 );
			int y = getLocaleGuiInt( "gui_log_exit_y", 0 );
			int w = getLocaleGuiInt( "gui_log_exit_w", 0 );
			int h = getLocaleGuiInt( "gui_log_exit_h", 0 );
			
			exitButton = ogui.CreateSimpleTextButton( window, x, y, w, h, 
				getLocaleGuiString( "gui_log_exit_image" ), getLocaleGuiString( "gui_log_exit_image_down" ), getLocaleGuiString( "gui_log_exit_image_highlight" ), getLocaleGuiString( "gui_map_exit" ) );
			exitButton->SetListener( this );

			exitButton->SetFont( ui::defaultIngameFont );

		}

		{
			OguiButtonStyle* unselStyle = loadStyle( "gui_log_unselected_item" );
			OguiButtonStyle* selStyle = loadStyle( "gui_log_selected_item" );

			OguiButtonStyle* newStyle = loadStyle( "gui_log_new_selected_item" );
			OguiButtonStyle* newUnStyle = loadStyle( "gui_log_new_unselected_item" );

			OguiButtonStyle* scrollUpStyle = loadStyle( "gui_log_arrow_down" );
			OguiButtonStyle* scrollDownStyle = loadStyle( "gui_log_arrow_up" );

			int num_of_elements = getLocaleGuiInt( "gui_log_number_of_items", 0 );
			selectListStyle = new OguiSelectListStyle( unselStyle, selStyle, newStyle, newUnStyle, scrollUpStyle, scrollDownStyle, unselStyle->sizeX, unselStyle->sizeY * num_of_elements, scrollUpStyle->sizeX, scrollUpStyle->sizeY );

		}
		
		{
			int x = getLocaleGuiInt( "gui_log_text_area_x", 0 );
			int y = getLocaleGuiInt( "gui_log_text_area_y", 0 );
			int w = getLocaleGuiInt( "gui_log_text_area_w", 0 );
			int h = getLocaleGuiInt( "gui_log_text_area_h", 0 );
			formattedText = new OguiFormattedText( window, &ogui, x, y, w, h, 1 );

			/*formattedText->setFont( ogui.LoadFont( "data/Fonts/mainmenu_little.ogf" ) );
			formattedText->registerFont( "b", ogui.LoadFont( "data/Fonts/mainmenu_little_bold.ogf" ) );
			formattedText->registerFont( "h1", ogui.LoadFont(  "data/Fonts/mainmenu_little_italic.ogf" ) );
			*/

			const std::string name ="gui_log_text_area";

			const std::string name_font_default = name + "_font_default";
			const std::string name_font_bold = name + "_font_bold";
			const std::string name_font_italic = name + "_font_italic";
			const std::string name_font_underline = name + "_font_underline";
			const std::string name_font_h1 = name + "_font_h1";
			const std::string name_font_page = name + "_font_page";

			fonts.push_back( ogui.LoadFont( getLocaleGuiString( name_font_default.c_str()		) ) );
			fonts.push_back( ogui.LoadFont( getLocaleGuiString( name_font_bold.c_str()		) ) );
			fonts.push_back( ogui.LoadFont( getLocaleGuiString( name_font_italic.c_str()		) ) );
			fonts.push_back( ogui.LoadFont( getLocaleGuiString( name_font_underline.c_str()	) ) );
			fonts.push_back( ogui.LoadFont( getLocaleGuiString( name_font_h1.c_str()			) ) );
			fonts.push_back( ogui.LoadFont( getLocaleGuiString( name_font_page.c_str()		) ) );

			formattedText->setFont( fonts[ 0 ] );
			formattedText->registerFont( "b",   fonts[ 1 ] );
			formattedText->registerFont( "i",   fonts[ 2 ] );
			formattedText->registerFont( "u",	fonts[ 3 ] );
			formattedText->registerFont( "h1",	fonts[ 4 ] );
		}



		loadTypes( currentLogType );
		selectItem( currentSelection );
	}

	void release()
	{
		delete formattedText;
		formattedText = NULL;

		{
			int i = 0;
			for( i = 0; i < (int)fonts.size(); i++ )
			{
				delete fonts[ i ];
				fonts[ i ] = NULL;
			}
		}

		delete textAreaBackground; 
		textAreaBackground = NULL;
		
		delete selectListBackground;
		selectListBackground = NULL;

		delete exitButton;
		exitButton = NULL;

		std::list< OguiButtonStyle* >::iterator i;
		for( i = styles.begin(); i != styles.end(); ++i )
		{
			delete (*i)->image;
			delete (*i)->imageDown;
			delete (*i)->imageDisabled;
			delete (*i)->imageHighlighted;
			delete (*i)->textFont;

			delete (*i); // BUG!?
		}

		styles.erase( styles.begin(), styles.end() );

		delete selectListStyle;
		selectListStyle = NULL;

		delete selectList;
		selectList = NULL;
	}

	OguiButtonStyle* loadStyle( const std::string& button_name )
	{
		IOguiImage* norm = ogui.LoadOguiImage( getLocaleGuiString( ( button_name + "_norm" ).c_str() ) );
		IOguiImage* high = ogui.LoadOguiImage( getLocaleGuiString( ( button_name + "_high" ).c_str() ) );
		IOguiImage* down = ogui.LoadOguiImage( getLocaleGuiString( ( button_name + "_down" ).c_str() ) );
		IOguiImage* disa = ogui.LoadOguiImage( getLocaleGuiString( ( button_name + "_disa" ).c_str() ) );
		IOguiFont*  font = ogui.LoadFont( getLocaleGuiString( ( button_name + "_font" ).c_str() ) );

		int w = getLocaleGuiInt( ( button_name + "_w" ).c_str(), 0 );
		int h = getLocaleGuiInt( ( button_name + "_h" ).c_str(), 0 );

		OguiButtonStyle* result = new OguiButtonStyle( norm, down, disa, high, font, w, h );

		styles.push_back( result );
		return result;

	}

	bool isVisible() const
	{
		return visible;
	}

	void setText( const std::string& text )
	{
		std::string txt( text );

		int min_pos = txt.find( LOG_HEADER_END, 0 );

		while( txt.find( LOG_HEADER_BEGIN, min_pos + 1 ) != txt.npos )
		{
			int min = txt.find( LOG_HEADER_BEGIN, min_pos + 1 );
			int max = txt.find( LOG_HEADER_END, min_pos + 1 ) + LOG_HEADER_END.size();
			txt.replace( min, max - min, "" );
		}

		formattedText->setText( txt );
	}

	OguiWindow*				window;
	OguiSelectList*			selectList;
	OguiSelectListStyle*	selectListStyle;
	OguiFormattedText*		formattedText;
	GUIEffectWindow*		effectWindow;
	OguiButton*				textAreaBackground;
	OguiButton*				selectListBackground;
	OguiButton*				exitButton;

	std::list< OguiButtonStyle* >	styles;
	std::vector< LogEntry >			entries;
	
	bool							shown;
	bool							visible;

	Game&		game;
	Ogui&		ogui;
	LogManager& manager;

	std::vector< IOguiFont* >	fonts;

	static int				currentLogType;
	static int				currentSelection;
};

int LogWindowImpl::currentLogType = 0;
int LogWindowImpl::currentSelection = -1;

///////////////////////////////////////////////////////////////////////////////

LogWindow::LogWindow( Game& game, Ogui& ogui, LogManager& manager ) :
	impl( new LogWindowImpl( game, ogui, manager ) )
{
	impl->fadeIn();
}

//=============================================================================

LogWindow::~LogWindow()
{
	delete impl;
	impl = NULL;
}

///////////////////////////////////////////////////////////////////////////////

void LogWindow::show()
{
	FB_ASSERT( impl != NULL );
	impl->fadeIn();
}

//=============================================================================

void LogWindow::hide()
{
	FB_ASSERT( impl != NULL );
	impl->fadeOut();
}

//=============================================================================

void LogWindow::update( int msecsDelta )
{
	FB_ASSERT( impl != NULL );
	impl->update( msecsDelta );
}

///////////////////////////////////////////////////////////////////////////////

bool LogWindow::isVisible() const
{
	FB_ASSERT( impl != NULL );
	return impl->isVisible();
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui


#include "precompiled.h"

#include "ItemWindow.h"
#include "../util/assert.h"
#include "../util/StringUtil.h"
#include "../game/Game.h"
#include "../game/DHLocaleManager.h"
#include "CombatSubWindowFactory.h"

using namespace game;

namespace ui {
///////////////////////////////////////////////////////////////////////////////
// REGISTER_COMBATSUBWINDOW( ItemWindow );
// has a special updator
///////////////////////////////////////////////////////////////////////////////

ItemWindow::ItemWindow( Ogui* ogui, Game* game, int player ) :
    ogui( ogui ),
	game( game )
{
	FB_ASSERT( ogui );
    
	{
		windowX = getLocaleGuiInt( "gui_item_window_x", 0 );
		windowY = getLocaleGuiInt( "gui_item_window_y", 0 );
		int w = getLocaleGuiInt( "gui_item_window_w", 0 );
		int h = getLocaleGuiInt( "gui_item_window_h", 0 );

		window = ogui->CreateSimpleWindow( windowX, windowY, w, h, NULL );
		window->SetUnmovable();
	}

    lastUpdateValue = -1;
	window->SetEffectListener(this);

	{
		std::string foo = getLocaleGuiString( "gui_item_windows" );
		std::vector< std::string > item_window_names = util::StringSplit( ",", foo );

		for( int i = 0; i < (int)item_window_names.size(); i++ )
		{
			addItemWindow( item_window_names[ i ] );
		}
	}

	// addItem( "gui_keycard_window", std::string( "gui_item_keycard" ) );
}

//.............................................................................

ItemWindow::~ItemWindow()
{
	std::map< std::string, ItemList* >::iterator i;

	for( i = itemWindows.begin(); i != itemWindows.end(); ++i )
	{
		delete i->second;
		i->second;
	}
	
	delete window;
}

///////////////////////////////////////////////////////////////////////////////

void ItemWindow::addItemWindow( const std::string& item_window_name )
{
	int x		= getLocaleGuiInt( ( item_window_name + "_x" ).c_str(), 0  );
	int y		= getLocaleGuiInt( ( item_window_name + "_y" ).c_str(), 0  );
	int add_x	= getLocaleGuiInt( ( item_window_name + "_add_x" ).c_str(), 0  );
	int add_y	= getLocaleGuiInt( ( item_window_name + "_add_y" ).c_str(), 0  );
	
	x -= windowX;
	y -= windowY;

	ItemList* tmp = new ItemList( x, y, add_x, add_y );

	FB_ASSERT( itemWindows.find( item_window_name ) == itemWindows.end() );
	itemWindows.insert( std::pair< std::string, ItemList* >( item_window_name, tmp ) ); 
}

///////////////////////////////////////////////////////////////////////////////

void ItemWindow::addItem( const std::string& target, const Item& item )
{
	FB_ASSERT( itemWindows.find( target ) != itemWindows.end() );
	
	Item i( item );
	if( i.image == NULL )
	{
		FB_ASSERT( ogui );
		FB_ASSERT( window );
		
		i.image = ogui->CreateSimpleImageButton( window, 0, 0, item.w, item.h, NULL, NULL, NULL, item.imageFile.c_str(), 0 );
		i.image->SetDisabled( true );
	}

	if( i.image && itemWindows.find( target ) != itemWindows.end() )
		itemWindows[ target ]->addItem( i );
}

//.............................................................................

void ItemWindow::removeItem( const std::string& target, const Item& item )
{
	FB_ASSERT( itemWindows.find( target ) != itemWindows.end() );

	if( itemWindows.find( target ) != itemWindows.end() )
		itemWindows[ target ]->removeItem( item );
}

///////////////////////////////////////////////////////////////////////////////

ItemWindow::ItemList::ItemList( int x, int y, int addX, int addY ) : 
	startX( x ),
	startY( y ),
	curX( x ),
	curY( y ),
	addX( addX ),
	addY( addY )
{
}

//.............................................................................

ItemWindow::ItemList::~ItemList()
{
	while( !items.empty() )
	{
		delete items.begin()->image;
		items.pop_front();
	}
}

///////////////////////////////////////////////////////////////////////////////

void ItemWindow::ItemList::addItem( const ItemWindow::Item& item )
{
	items.push_back( item );
	
	FB_ASSERT( item.image );
	
	if( item.image != NULL )
		item.image->Move( curX, curY );

	curX += addX;
	curY += addY;
}

//.............................................................................

void ItemWindow::ItemList::removeItem( const ItemWindow::Item& item )
{
	std::list< ItemWindow::Item >::iterator i;
	
	for( i = items.begin(); i != items.end(); ++i )
	{
		if( *i == item )
		{
			std::list< ItemWindow::Item >::iterator remove = i;
			--i;
			delete remove->image;
			items.erase( remove );

			curX -= addX;
			curY -= addY;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void ItemWindow::hide( int fadeTime )
{
	FB_ASSERT( window );

	if(fadeTime)
		window->StartEffect( OGUI_WINDOW_EFFECT_FADEOUT, fadeTime );
	else
		window->Hide();
}

//.............................................................................

void ItemWindow::show( int fadeTime )
{
	FB_ASSERT( window );

	if(fadeTime)
		window->StartEffect( OGUI_WINDOW_EFFECT_FADEIN, fadeTime );

	window->Show();
}

//.............................................................................


void ItemWindow::update()
{
}

//.............................................................................

void ItemWindow::EffectEvent( OguiEffectEvent *e )
{
	FB_ASSERT( e );

	if( e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT )
		window->Hide();
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

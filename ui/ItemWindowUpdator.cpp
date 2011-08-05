
#include "precompiled.h"

#include "ItemWindowUpdator.h"
#include "ItemWindow.h"
#include "../game/Game.h"
#include "../game/scripting/GameScripting.h"
#include "../../editor/parser.h"
#include "../../filesystem/input_file_stream.h"
#include "CombatSubWindowFactory.h"

using namespace game;
using namespace frozenbyte;
using namespace frozenbyte::editor;

namespace ui {

#ifdef LEGACY_FILES
std::string fileName = "Data/Items/item_visualization.txt";
#else
std::string fileName = "data/item/item_visualization.txt";
#endif

///////////////////////////////////////////////////////////////////////////////
namespace {
class ItemWindowUpdatorRegisterer : public CombatSubWindowFactory::ICombatSubWindowConstructor
{
public:
	ItemWindowUpdatorRegisterer( const std::string& s )
	{
		CombatSubWindowFactory::GetSingleton()->RegisterSubWindow( s, this );
	}

	ICombatSubWindow* CreateNewWindow( Ogui* ogui, game::Game* game, int player )
	{
		return new ItemWindowUpdator( game, new ItemWindow( ogui, game, player ) );
	}
};

ItemWindowUpdatorRegisterer* __attribute__((used)) IWURtemp_static_haxoring_constructing_thing = new ItemWindowUpdatorRegisterer( "ItemWindow" );


}
///////////////////////////////////////////////////////////////////////////////

ItemWindowUpdator::ItemWindowUpdator( Game* game, ItemWindow* item ) :
	game( game ),
	updateInFrames( 30 ),
	currentFrame( 0 ),
	itemWindow( item )
{ 

	EditorParser parser;

	filesystem::InputStream fileStream = filesystem::createInputFileStream( fileName );
	fileStream >> parser;

	int i;
	for ( i = 0; i < parser.getGlobals().getSubGroupAmount(); i++ )
	{
		ItemVisual v;
		v.image				= parser.getGlobals().getSubGroup( i ).getValue( "image" );
		v.location			= parser.getGlobals().getSubGroup( i ).getValue( "location" );
		v.watch_variable	= parser.getGlobals().getSubGroup( i ).getValue( "watch_variable" );
		v.shown				= 0;

		items.push_back( v );
	}

}

//.............................................................................

ItemWindowUpdator::~ItemWindowUpdator()
{
	delete itemWindow;
	itemWindow = NULL;
}

//.............................................................................

void ItemWindowUpdator::update()
{
	doUpdate( itemWindow );
	/*currentFrame++;

	if( currentFrame >= updateInFrames )
	{
		currentFrame = 0; 
		// check for the stuff in scripting which stuff is in use and what arent
		doUpdate( item );

	}*/
}

void ItemWindowUpdator::hide( int time )
{
	itemWindow->hide( time );
}

void ItemWindowUpdator::show( int time )
{
	itemWindow->show( time );
}

///////////////////////////////////////////////////////////////////////////////

void ItemWindowUpdator::doUpdate( ItemWindow* itemwindow )
{
	// Logger::getInstance()->error( "DEBUG - DEBUG - DEBUG\n" );
	std::list< ItemVisual >::iterator i;

	for( i = items.begin(); i != items.end(); ++i )
	{
		if( i->shown != game->gameScripting->getGlobalIntVariableValue( i->watch_variable.c_str() )	 )
		{
			i->shown = game->gameScripting->getGlobalIntVariableValue( i->watch_variable.c_str() );
	
			if( i->shown )
			{
				itemwindow->addItem( i->location, i->image );
			}
			else 
			{
				itemwindow->removeItem( i->location, i->image );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

}; // end of namespace ui

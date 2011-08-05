
#include "precompiled.h"

#include "TargetDisplayWindow.h"
#include "TargetDisplayButtonManager.h"
#include "TargetDisplayWindowButton.h"

#include "../ogui/Ogui.h"
#include "../util/assert.h"

#include <sstream>

#include "../../editor/parser.h"
#include "../../filesystem/input_file_stream.h"
#include "../../filesystem/file_package_manager.h"
#include "CombatSubWindowFactory.h"

using namespace frozenbyte;
using namespace frozenbyte::editor;
using namespace game;

namespace ui {

#ifdef LEGACY_FILES
static std::string unitpointersFileName = "Data/GUI/Ingame/Unitpointers/gui_unitpointers.txt";
#else
static std::string unitpointersFileName = "data/GUI/hud/unitpointer/gui_unitpointers.txt";
#endif


///////////////////////////////////////////////////////////////////////////////

TargetDisplayWindow::TargetDisplayWindow( Ogui* ogui, Game* game, int player ) :
	buttons(),
	window( NULL ),
	ogui( ogui ),
	currentTicks( 0 )
{
	FB_ASSERT( ogui != NULL );
	
	window = ogui->CreateSimpleWindow( 0, 0, 1024, 768, NULL, 0 );

	// this is required or the window will catch all the cursor clicks --jpk
	window->SetReactMask(0);
		
	manager = new TargetDisplayButtonManager( ogui, window );

	// FOR SOME REASON, THIS PARTICULAR PARSER INSTANCE TOTALLY DOES NOT GET THE REFERENCED FLAGS CORRECTLY SET
	// WHY??? (THUS, NEED TO DISABLE REFERENCED ERROR LOGGING - THE NON EXISTING LOGGING DOES NOT WORK EITHER EVEN THOUGH ENABLED)
	EditorParser parser(false, true);

	filesystem::InputStream fileStream = filesystem::FilePackageManager::getInstance().getFile( unitpointersFileName );
	fileStream >> parser;

	int i;
	for ( i = 0; i < parser.getGlobals().getSubGroupAmount(); i++ )
	{
		TargetDisplayButtonManager::ButtonCreator basic;
		basic.buttonImages.resize( 4 );

		ParserGroup group = parser.getGlobals().getSubGroup( i );

		int id = 0;
		std::stringstream( group.getValue( "id" ) ) >> id;

		basic.buttonImages[ 0 ] = group.getValue( "topleft" );
		basic.buttonImages[ 1 ] = group.getValue( "topright" );
		basic.buttonImages[ 2 ] = group.getValue( "bottomright" );
		basic.buttonImages[ 3 ] = group.getValue( "bottomleft" );

		basic.font = ogui->LoadFont( group.getValue( "font" ).c_str() );
		
		std::stringstream( group.getValue( "width" ) ) >> basic.width;
		std::stringstream( group.getValue( "height" ) ) >> basic.height;
		
		std::stringstream( group.getValue( "initanimsize" ) ) >> basic.beginAnimPos;

		std::stringstream( group.getValue( "text_offset_x", "0" ) ) >> basic.textOffsetX;
		std::stringstream( group.getValue( "text_offset_y", "0" ) ) >> basic.textOffsetY;

		std::stringstream( group.getValue( "text_center", "1" ) ) >> basic.textCenter;

		// slider
		std::stringstream( group.getValue( "has_slider", "0" ) ) >> basic.hasSlider;
		basic.sliderBackground = group.getValue( "slider_background" );
		basic.sliderForeground = group.getValue( "slider_foreground" );
		basic.sliderBackgroundLow = group.getValue( "slider_background_low" );
		basic.sliderForegroundLow = group.getValue( "slider_foreground_low" );
		basic.sliderBackgroundHigh = group.getValue( "slider_background_high" );
		basic.sliderForegroundHigh = group.getValue( "slider_foreground_high" );
		std::stringstream( group.getValue( "slider_low_limit", "30" ) ) >> basic.sliderLowLimit;
		std::stringstream( group.getValue( "slider_high_limit", "70" ) ) >> basic.sliderHighLimit;
		std::stringstream( group.getValue( "slider_width", "0" ) ) >> basic.sliderWidth;
		std::stringstream( group.getValue( "slider_height", "0" ) ) >> basic.sliderHeight;

		std::stringstream( group.getValue( "rect_style", "0" ) ) >> basic.rect_style;

		manager->registerButtonStyle( id, basic );

	}

	/*TargetDisplayButtonManager::ButtonCreator basic;
	basic.buttonImages.resize( 4 );
	basic.buttonImages[ 0 ] = "Data/GUI/Ingame/Unitpointers/Indicator01_topleft.tga";
	basic.buttonImages[ 1 ] = "Data/GUI/Ingame/Unitpointers/Indicator01_topright.tga";
	basic.buttonImages[ 2 ] = "Data/GUI/Ingame/Unitpointers/Indicator01_bottomright.tga";
	basic.buttonImages[ 3 ] = "Data/GUI/Ingame/Unitpointers/Indicator01_bottomleft.tga";
	basic.font = ogui->LoadFont( "Data/Fonts/mainmenu_little.ogf" );
	basic.width = 16;
	basic.height = 16;
	basic.beginAnimPos = 200;

	// temp test hack fix me bug bug
	manager->registerButtonStyle( 1, basic );
	*/
	  /*
	basic.buttonImages[ 0 ] = "Data/GUI/Ingame/Menuicons/action_stop.tga";
	basic.buttonImages[ 1 ] = "Data/GUI/Ingame/Menuicons/action_stop.tga";
	basic.buttonImages[ 2 ] = "Data/GUI/Ingame/Menuicons/action_stop.tga";
	basic.buttonImages[ 3 ] = "Data/GUI/Ingame/Menuicons/action_stop.tga";
	basic.width = 16;
	basic.height = 16;*/
	// basic.font = NULL;
	// temp test hack fix me bug bug
	// manager->registerButtonStyle( 0, basic );
	// imageWidth = 24;
	// imageHeight = 16;

	TargetDisplayWindowButton::setManager( manager );
	
}

//=============================================================================

TargetDisplayWindow::~TargetDisplayWindow()
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;
	if( buttons.empty() == false )
	{
		for( i = buttons.begin(); i != buttons.end(); ++i )
		{
			i->second->release();
			delete i->second;
			// std::map< void*, TargetDisplayWindowButton* >::iterator remove = i;
			// --i;
			// buttons.erase( remove );
		}

		buttons.clear();
	}

	delete manager;
	TargetDisplayWindowButton::setManager( NULL );

	delete window;
}

///////////////////////////////////////////////////////////////////////////////

bool TargetDisplayWindow::setRisingText( void* p, int x, int y, int w, int h, float distance, int style )
{
	bool result = false;
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	i = buttons.find( p );

	if( i != buttons.end() && style != i->second->getStyle() )
	{
		i->second->release();
		delete i->second;
		buttons.erase( i );
		i = buttons.end();
	}

	if( !TargetDisplayWindowButton::isRegistered( style ) )
		return false;

	if( i == buttons.end() )
	{
		TargetDisplayRisingScoreButton* temp = new TargetDisplayRisingScoreButton( style );
		// temp->setText( "" );
		buttons.insert( std::pair< void*, TargetDisplayWindowButton* >( p, temp ) );
		i = buttons.find( p );
		result = true;
	}

	FB_ASSERT( i != buttons.end() );
	FB_ASSERT( i->first == p );

	i->second->setRect( x, y, w, h, distance );

	i->second->updatedInTick = currentTicks;

	if( i->second->hasSlider() )
	{
		// result = true;
	}

	return result;
}

//=============================================================================

bool TargetDisplayWindow::hasEnded( void* p )
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	i = buttons.find( p );

	if( i != buttons.end() )
	{
		return i->second->hasEnded();
	}
	else
	{
		return true;
	}
}

//=============================================================================

bool TargetDisplayWindow::isAniOver( void* p )
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	i = buttons.find( p );

	if( i != buttons.end() )
	{
		return i->second->isAniOver();
	}
	else
	{
		return true;
	}
}


//=============================================================================

int TargetDisplayWindow::timeActive( void* p )
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	i = buttons.find( p );

	if( i != buttons.end() )
	{
		return i->second->timeActive();
	}
	else
	{
		return 0;
	}
}

//=============================================================================

void TargetDisplayWindow::clearRect( void *p )
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	i = buttons.find( p );

	if( i != buttons.end() )
	{
		i->second->release();
		delete i->second;
		buttons.erase( i );
	}
}

//=============================================================================

bool TargetDisplayWindow::setRect( void* p, int x, int y, int w, int h, float distance, int style )
{
	// temp test hax debug thingie
	// return setRisingText( p, x, y, w, h, distance, style );

	bool result = false;
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	i = buttons.find( p );

	if( i != buttons.end() && style != i->second->getStyle() )
	{
		i->second->release();
		delete i->second;
		buttons.erase( i );
		i = buttons.end();
	}

	if( !TargetDisplayWindowButton::isRegistered( style ) )
		return false;

	if( i == buttons.end() )
	{
		buttons.insert( std::pair< void*, TargetDisplayWindowButton* >( p, new TargetDisplayWindowButton( style ) ) );
		i = buttons.find( p );
		result = true;
	}

	FB_ASSERT( i != buttons.end() );
	FB_ASSERT( i->first == p );

	i->second->setRect( x, y, w, h, distance );

	i->second->updatedInTick = currentTicks;

	if( i->second->hasSlider() )
	{
		// result = true;
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindow::setText( void* p, const std::string& text )
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	i = buttons.find( p );
	if( i != buttons.end() )
	{
		i->second->setText( text );
	}
}

//=============================================================================

void TargetDisplayWindow::updateRect( void *p )
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	i = buttons.find( p );
	if( i != buttons.end() )
	{
		i->second->updateRect();
		i->second->updatedInTick = currentTicks;
	}
}

//=============================================================================

void TargetDisplayWindow::setSliderValue( void* p, float v, float scale )
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	i = buttons.find( p );
	if( i != buttons.end() )
	{
		i->second->setSliderValue( v, scale );
	}
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindow::hideRest()
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	for( i = buttons.begin(); i != buttons.end(); ++i )
	{
		if( i->second->updatedInTick != currentTicks )
		{
			i->second->hide();
			i->second->resetAni();
		}
	}

	currentTicks++;
}

//=============================================================================

void TargetDisplayWindow::removeRest()
{
	std::map< void*, TargetDisplayWindowButton* >::iterator i;

	// breaks here (fixed)
	for( i = buttons.begin(); i != buttons.end(); )
	{
		if( i->second->updatedInTick != currentTicks )
		{
			std::map< void*, TargetDisplayWindowButton* >::iterator remove = i;
			++i;
			
			remove->second->release();
			delete remove->second;
			buttons.erase( remove );
		}
		else
		{
			++i;
		}
	}

	currentTicks++;
	
	
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindow::hide( int fadeTime )
{
	FB_ASSERT( window );

	if(fadeTime)
		window->StartEffect( OGUI_WINDOW_EFFECT_FADEOUT, fadeTime );
	else
		window->Hide();
}

//=============================================================================

void TargetDisplayWindow::show( int fadeTime )
{
	FB_ASSERT( window );

	if(fadeTime)
		window->StartEffect( OGUI_WINDOW_EFFECT_FADEIN, fadeTime );

	window->Show();
}

//=============================================================================

void TargetDisplayWindow::EffectEvent( OguiEffectEvent *e )
{
	FB_ASSERT( e );

	if( e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT )
		window->Hide();
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui

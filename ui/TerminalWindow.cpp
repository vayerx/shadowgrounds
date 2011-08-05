
#include "precompiled.h"

#include "TerminalWindow.h"

#include "../util/StringUtil.h"
#include "../game/DHLocaleManager.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../ui/uidefaults.h"
#include "../system/Timer.h"

#include <sstream>

using namespace game;
using namespace util;

namespace ui {

const static std::string nextpage_separator = "<nextpage>";
const static std::string style_prefix = "gui_screen_style_";

///////////////////////////////////////////////////////////////////////////////

TerminalWindow::TerminalWindow( Ogui* ogui, Game* game, const std::string& style_name ) :
	game( game ),
	ogui( ogui ),

	window( NULL ),
	textarea( NULL ),
	next( NULL ),
	prev( NULL ),
	close( NULL ),
	pageNumButton( NULL ),
	pageNum( -1 ),
	pageMax( 0 ),
	visible( true ),
	
	fonts(),
	text()
{
	// the window
	{
		const std::string name = style_prefix + style_name + "_background";

		const std::string name_x = name + "_x";
		const std::string name_y = name + "_y";
		const std::string name_w = name + "_w";
		const std::string name_h = name + "_h";
		const std::string name_norm = name + "_norm";

		int x = getLocaleGuiInt( name_x.c_str(), 0 );
		int y = getLocaleGuiInt( name_y.c_str(), 0 );
		int w = getLocaleGuiInt( name_w.c_str(), 0 );
		int h = getLocaleGuiInt( name_h.c_str(), 0 );
		std::string background = getLocaleGuiString( name_norm.c_str() );
		
		window = ogui->CreateSimpleWindow( x, y, w, h, background.c_str(), 0 );
	}

	// the text area
	{
		const std::string name = style_prefix + style_name + "_textarea";
		const std::string name_x = name + "_x";
		const std::string name_y = name + "_y";
		const std::string name_w = name + "_w";
		const std::string name_h = name + "_h";

		int x = getLocaleGuiInt( name_x.c_str(), 0 );
		int y = getLocaleGuiInt( name_y.c_str(), 0 );
		int w = getLocaleGuiInt( name_w.c_str(), 0 );
		int h = getLocaleGuiInt( name_h.c_str(), 0 );

		textarea = new OguiFormattedText( window, ogui, x, y, w, h );

		// FIXME: lots of temporary.c_str() crap to convert...
#ifdef _MSC_VER
//#pragma message("FIXME: lots of temporary.c_str() crap to convert...")
#endif

		fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_default" ).c_str()		) ) );
		fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_bold" ).c_str()		) ) );
		fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_italic" ).c_str()		) ) );
		fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_underline" ).c_str()	) ) );
		fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_h1" ).c_str()			) ) );
		fonts.push_back( ogui->LoadFont( getLocaleGuiString( ( name + "_font_page" ).c_str()		) ) );

		textarea->setFont( fonts[ 0 ] );
		textarea->registerFont( "b",    fonts[ 1 ] );
		textarea->registerFont( "i",    fonts[ 2 ] );
		textarea->registerFont( "u",	fonts[ 3 ] );
		textarea->registerFont( "h1",   fonts[ 4 ] );
	}

	// pageNumButton
	{
		const std::string name = style_prefix + style_name + "_page_num";
		int x = getLocaleGuiInt( ( name + "_x" ).c_str(), 0 );
		int y = getLocaleGuiInt( ( name + "_y" ).c_str(), 0 );
		int w = getLocaleGuiInt( ( name + "_w" ).c_str(), 0 );
		int h = getLocaleGuiInt( ( name + "_h" ).c_str(), 0 );
	
		pageNumButton = ogui->CreateSimpleTextButton( window, x, y, w, h, NULL, NULL, NULL, "", 0 );
		pageNumButton->SetFont( fonts[ 5 ]	 );
	}

	// the buttons
	next = loadButton( style_prefix + style_name + "_next" );
	prev = loadButton( style_prefix + style_name + "_prev" );
	close = loadButton( style_prefix + style_name + "_close" );

#ifndef PROJECT_SHADOWGROUNDS
	buttonTexts.push_back( loadText( style_prefix + style_name + "_close" ) );
#endif

	// The vsyncs
	{
		const std::string name = style_prefix + style_name + "_vsync";

		const char* image = getLocaleGuiString( ( name + "_image" ).c_str() );
		
		vsyncX =		getLocaleGuiInt( ( name + "_x" ).c_str(),		0 );
		vsyncBeginPos = getLocaleGuiInt( ( name + "_begin" ).c_str(),	0 );
		vsyncDownPos =	getLocaleGuiInt( ( name + "_end" ).c_str(),		0 );
		vsyncY1 =		(float)getLocaleGuiInt( ( name + "_y1" ).c_str(),		0 );
		vsyncY2 =		(float)getLocaleGuiInt( ( name + "_y2" ).c_str(),		0 );
		int w =			getLocaleGuiInt( ( name + "_w" ).c_str(),		0 );
		int h =			getLocaleGuiInt( ( name + "_h" ).c_str(),		0 );
		vsyncSpeed1	=	(float)getLocaleGuiInt( ( name +  "_speed1" ).c_str(), 10 );
		vsyncSpeed2	=	(float)getLocaleGuiInt( ( name +  "_speed2" ).c_str(), 10 );

		vsyncImage1 = ogui->CreateSimpleImageButton( window, vsyncX, (int)vsyncY1, w, h, image, image, image, image );
		vsyncImage2 = ogui->CreateSimpleImageButton( window, vsyncX, (int)vsyncY2, w, h, image, image, image, image );
		vsyncImage1->SetDisabled( true );
		vsyncImage2->SetDisabled( true );
	}

}

//=============================================================================

TerminalWindow::~TerminalWindow()
{
	int i = 0;
	for( i = 0; i < (int)fonts.size(); i++ )
	{
		delete fonts[ i ];
		fonts[ i ] = NULL;
	}

	for(i = 0; i < (int)buttonTexts.size(); i++)
	{
		delete buttonTexts[i];
	}

	delete textarea;	
	textarea = NULL;

	delete pageNumButton;
	pageNumButton = NULL;
	
	delete next;		
	next = NULL;
	
	delete prev;		
	prev = NULL;
	
	delete close;		
	close = NULL;
	
	delete window;		
	window = NULL;
}

///////////////////////////////////////////////////////////////////////////////

void TerminalWindow::CursorEvent( OguiButtonEvent* eve )
{
	if( eve->triggerButton == next )
	{
		nextPage();
	} 
	else if ( eve->triggerButton == prev )
	{
		prevPage();
	}
	else if ( eve->triggerButton == close )
	{
		// visible = false;
		game->gameUI->closeTerminalWindow();
	}

}

///////////////////////////////////////////////////////////////////////////////

void TerminalWindow::setText( const std::string& txt )
{
	text = StringSplit( nextpage_separator, txt );

	int i = 0;
	for( i = 0; i < (int)text.size(); i++ )
	{
		text[ i ] = StringRemoveWhitespace( text[ i ] );
		if( text[ i ].empty() == false && ( text[ i ][ 0 ] == '\n' || text[ i ][ 0 ] == '\r' ) )
			text[ i ][ 0 ] = ' ';
	}

	pageMax = text.size();

	setPage( 0 );
}

///////////////////////////////////////////////////////////////////////////////

void TerminalWindow::setPage( int page )
{
	if( pageNum == page ) 
		return;

	if( page < 0 )
	{
		page = 0;
	}

	if( page >= (int)text.size() )
	{
		page = text.size() - 1;
	}

	pageNum = page;

	if( pageNum <= 0 )
	{
		prev->SetDisabled( true );
	}
	else
	{
		prev->SetDisabled( false );
	}

	if( pageNum >= (int)text.size() - 1 )
	{
		next->SetDisabled( true );
	}
	else
	{
		next->SetDisabled( false );
	}

	if( pageNum >= 0 && pageNum < (int)text.size() )
	{
		textarea->setText( text[ pageNum ] );
	}

	std::stringstream ss;
	ss << ( pageNum + 1 ) << " / " << pageMax; 
	pageNumButton->SetText( ss.str().c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void TerminalWindow::nextPage()
{
	setPage( pageNum + 1 );
}

//=============================================================================

void TerminalWindow::prevPage()
{
	setPage( pageNum - 1 );
}

///////////////////////////////////////////////////////////////////////////////

bool TerminalWindow::isVisible() const
{
	return visible;
}

///////////////////////////////////////////////////////////////////////////////

void TerminalWindow::update()
{
	static int lastUpdate = Timer::getTime();
	vsyncY1 += ( (float)( Timer::getTime() - lastUpdate ) / 1000.0f ) * vsyncSpeed1;
	vsyncY2 += ( (float)( Timer::getTime() - lastUpdate ) / 1000.0f ) * vsyncSpeed2;

	lastUpdate = Timer::getTime();

	vsyncImage1->Move( vsyncX, (int)( vsyncY1 + 0.5f ));
	vsyncImage2->Move( vsyncX, (int)( vsyncY2 + 0.5f ));

	if( vsyncY1 >= vsyncDownPos ) vsyncY1 = (float)vsyncBeginPos;
	if( vsyncY2 >= vsyncDownPos ) vsyncY2 = (float)vsyncBeginPos;
}

///////////////////////////////////////////////////////////////////////////////

OguiButton* TerminalWindow::loadButton( const std::string& name, int id )
{
	int x = getLocaleGuiInt( ( name + "_x" ).c_str(), 0 );
	int y = getLocaleGuiInt( ( name + "_y" ).c_str(), 0 );
	int w = getLocaleGuiInt( ( name + "_w" ).c_str(), 0 );
	int h = getLocaleGuiInt( ( name + "_h" ).c_str(), 0 );
 
	std::string norm = getLocaleGuiString( ( name + "_norm" ).c_str() );
	std::string disp = getLocaleGuiString( ( name + "_disp" ).c_str() );
	std::string high = getLocaleGuiString( ( name + "_high" ).c_str() );
	std::string down = getLocaleGuiString( ( name + "_down" ).c_str() );

	OguiButton* result = ogui->CreateSimpleImageButton( window, x, y, w, h, norm.c_str(), down.c_str(), high.c_str(), disp.c_str(), id );
	result->SetListener( this );

	return result;
}

///////////////////////////////////////////////////////////////////////////////

OguiTextLabel* TerminalWindow::loadText( const std::string& name )
{
	int x = getLocaleGuiInt( ( name + "_text_x" ).c_str(), 0 );
	int y = getLocaleGuiInt( ( name + "_text_y" ).c_str(), 0 );
	int w = getLocaleGuiInt( ( name + "_text_w" ).c_str(), 0 );
	int h = getLocaleGuiInt( ( name + "_text_h" ).c_str(), 0 );
 
	std::string text = getLocaleGuiString( ( name + "_text" ).c_str() );
	std::string fontname = getLocaleGuiString( ( name + "_text_font" ).c_str() );

	IOguiFont *font = ogui->LoadFont(fontname.c_str());
	OguiTextLabel* result = ogui->CreateTextLabel( window, x, y, w, h, text.c_str());
	result->SetFont(font);
	result->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);

	fonts.push_back(font);
	return result;
}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

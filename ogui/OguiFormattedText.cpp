
#include "precompiled.h"

#include "OguiFormattedText.h"
#include "IOguiFormattedCommand.h"
#include "Ogui.h"
#include <assert.h>
#include <stack>


#include "../util/StringUtil.h"
#include "../util/assert.h"
#include "OguiFormattedCommandImg.h"

#include "../util/Debug_MemoryManager.h"

using namespace util;
///////////////////////////////////////////////////////////////////////////////

struct OguiFormattedText::Rect
{
	Rect() : x( 0 ), y( 0 ), w( 0 ), h( 0 ) { }
	Rect( int x, int y, int w, int h ) :
		x( x ), y( y ), w( w ), h( h ) { }

	int x;
	int y;
	int w;
	int h;
};

///////////////////////////////////////////////////////////////////////////////

OguiFormattedText::OguiFormattedText( OguiWindow* win, Ogui* ogui, int x, int y, int w, int h, int id  ) :
  buttons(),
  commands(),
  fonts(),
  images(),
  tagstack(),
  currentFont( NULL ),
  hAlign( OguiButton::TEXT_H_ALIGN_LEFT ),
  vAlign( OguiButton::TEXT_V_ALIGN_TOP ),
  
  window( win ),
  ogui( ogui ),
  font( NULL ),
  text(),
  lineHeight( 1.0f ),

  position( new Rect( x, y, w, h ) ),
  clip( new Rect( x, y, w, h ) ),
#ifdef PROJECT_SHADOWGROUNDS
  halfClip( false ),
#else
  halfClip( true ),
#endif
  clipPositionTop( 0 ),
  clipPositionBottom( 0 ),
  id( id ),

  releaseMe( NULL )
{
	releaseMe = new OguiFormattedCommandImg;
	registerCommand( "img", releaseMe );
	  
}

//.............................................................................

OguiFormattedText::~OguiFormattedText()
{
	releaseAllButtons();
	delete position;
	delete clip;
	delete releaseMe;
}

///////////////////////////////////////////////////////////////////////////////

int OguiFormattedText::getX() const
{
	return position->x;
}

int OguiFormattedText::getY() const
{
	return position->y;
}

int OguiFormattedText::getW() const
{
	return position->w;
}

int OguiFormattedText::getH() const
{
	return position->h;
}

///////////////////////////////////////////////////////////////////////////////

void OguiFormattedText::registerFont( const std::string& tag_name, IOguiFont*font )
{
	// check that we are not trying to re-register a font
	FB_ASSERT( fonts.find( tag_name ) == fonts.end() );
	
	fonts.insert( std::pair< std::string, IOguiFont* >( tag_name, font ) );
	
}

void OguiFormattedText::deleteRegisteredFonts()
{
	// really delete fonts
	std::map<std::string, IOguiFont*>::iterator it;
	for(it = fonts.begin(); it != fonts.end(); it++)
	{
		delete it->second;
	}
	fonts.clear();
}

void OguiFormattedText::registerCommand( const std::string& command_name, IOguiFormattedCommand* command )
{
	FB_ASSERT( commands.find( command_name ) == commands.end() );

	commands.insert( std::pair< std::string, IOguiFormattedCommand* >( command_name, command ) );
}

//.............................................................................

void OguiFormattedText::setFont( IOguiFont* font )
{
	this->font = font;
}

IOguiFont* OguiFormattedText::getFont()
{
	return this->font;
}
//.............................................................................

void OguiFormattedText::setText( const std::string& text )
{
	if( this->text != text )
	{
		this->text = StringReplace( "<br>", "\n", text );

		releaseAllButtons();
		
		parseTextToButtons();
	}
	else
	{
	}
}

///////////////////////////////////////////////////////////////////////////////

int OguiFormattedText::getId() const
{
	return this->id;
}

Ogui* OguiFormattedText::getOgui() const
{
	return ogui;

}
//.............................................................................

void OguiFormattedText::setTextHAlign( OguiButton::TEXT_H_ALIGN hAlign )
{
	if( this->hAlign != hAlign )
	{
		this->hAlign = hAlign;
		parseTextToButtons();
	}
}

//.............................................................................
// sets vertical alignment for the button caption text

void OguiFormattedText::setTextVAlign( OguiButton::TEXT_V_ALIGN vAlign )
{
	if( this->vAlign != vAlign )
	{
		this->vAlign = vAlign;
		parseTextToButtons();
	}
}

//.............................................................................

void OguiFormattedText::setLineHeight( float f )
{
	lineHeight = f;
}

///////////////////////////////////////////////////////////////////////////////

void OguiFormattedText::parseTextToButtons()
{
	if( font == NULL ) 
		return;

	ParseData data( position->x, position->y, position->w, position->h );
	
	data.next_linebrk = 0;
	data.next_hardbreak = 0;
	data.cur_pos = 0;

	data.currentFont = font;

	data.mx = data.x;
	data.my = position->y;
	data.cur_w = data.w;
	data.add_x = 0;

	linePositionYs.clear();
	linePositionYs.push_back(data.my);
	
	if( hAlign != OguiButton::TEXT_H_ALIGN_LEFT )
	{
		int lw = getLineWidth( data.cur_pos, tagstack, &data );
		if( hAlign == OguiButton::TEXT_H_ALIGN_CENTER )
		{
			data.mx = data.x + ( ( data.w - lw ) / 2);
		} 
		else if ( hAlign == OguiButton::TEXT_H_ALIGN_RIGHT )
		{
			data.mx = data.x + ( ( data.w - lw ) );
		}
	}

	while( data.cur_pos < (int)text.size() )
	{

		// find the place where the first tag starts
		{
			data.next_tag = text.find_first_of( "<", data.cur_pos );
			data.next_hardbreak = text.find_first_of( "\n\r", data.cur_pos );

			if( data.next_hardbreak == text.npos ) 
				data.next_hardbreak = text.size();
		}

		// findLineWidthBreak( pos, width, data.currentFont );
		// find the first line break
		{
			std::pair< int, int > tmp = findLineWidthBreak( data.cur_pos, data.w - data.add_x, data.currentFont );
			data.next_linebrk = tmp.first;
			// add_x += tmp.second;
			data.cur_w = tmp.second;	
		}
		
		
		// create buttons as desired
		// we check which is the first to come, linebrk, next_tag or next_hardbreak
		if( ( data.next_linebrk < data.next_tag || 
			  data.next_hardbreak < data.next_tag ) || 
			  ( data.next_tag == text.npos ) )
		{
			if( data.next_linebrk == text.npos ) data.next_linebrk = data.next_hardbreak;
			if( data.next_hardbreak == text.npos ) data.next_hardbreak = data.next_linebrk;

			
			int break_here = data.next_linebrk<data.next_hardbreak?data.next_linebrk:data.next_hardbreak;
			
			if( data.next_linebrk == data.next_hardbreak )
			{
				// int i = int i = data.currentFont->getStringWidth( text.substr( cur_pos, next_hardbreak - cur_pos ).c_str() );
			} 
			
			int i =  data.currentFont ? data.currentFont->getStringWidth( text.substr( data.cur_pos, break_here - data.cur_pos ).c_str() ) : 0;

			FB_ASSERT( data.currentFont );
			// FB_ASSERT( i != 0 );
			// FB_ASSERT( data.mx == data.x + data.add_x );

			// this happens when the line is complite without a font change
			if( i > 0 )
				createTextButton( data.mx, data.my, i, data.currentFont->getHeight(), text.substr( data.cur_pos, break_here - data.cur_pos ), data.currentFont );

			// might bug in some cases
			// if( cur_pos != break_here )
			data.cur_pos = break_here + 1;
			
			

			if( hAlign != OguiButton::TEXT_H_ALIGN_LEFT && data.cur_pos < (int)text.size() )
			{
				int lw = getLineWidth( data.cur_pos, tagstack, &data );
				if( hAlign == OguiButton::TEXT_H_ALIGN_CENTER )
				{
					data.mx = data.x + ( ( data.w - lw ) / 2);
				} 
				else if ( hAlign == OguiButton::TEXT_H_ALIGN_RIGHT )
				{
					data.mx = data.x + ( ( data.w - lw ) );
				}
			} 
			else 
			{
				data.mx = data.x;
			}

			
			data.my += (int)( data.currentFont->getHeight() * lineHeight );
			linePositionYs.push_back( data.my );
			data.add_x = 0;

			if( ( data.my - data.y ) > data.after_y )
			{
				data.after_y = data.h;
				data.x = data.after_y_x_value;
				data.w = data.after_y_w_value;
			}
		} 
		else 
		{
			// draw the smaller button
			data.cur_w = data.currentFont->getStringWidth(  text.substr( data.cur_pos, data.next_tag - data.cur_pos ).c_str() );

			createTextButton( data.mx, data.my, data.cur_w, data.currentFont->getHeight(), text.substr( data.cur_pos, data.next_tag - data.cur_pos ), data.currentFont );
			
			// find out the place where the next string starts
			data.cur_pos = data.next_tag + 1;
			data.mx += data.cur_w;
			data.add_x += data.cur_w;
			
			// Change the font and try again
			// data.currentFont = myFonts[ tag_name ];
			// find out the new font
			{
				std::string::size_type cnt = text.find_first_of( ">", data.cur_pos );
				if( cnt != text.npos ) 
				{
					std::string tag_name = text.substr( data.cur_pos, cnt - data.cur_pos );
					data.cur_pos = cnt + 1;
					// closing tag
					// ParseFormattedCommand( tag_name );
					parseFormattedCommand( tag_name, &data );
				}
			}

		} //

	}

}

//.............................................................................

void OguiFormattedText::parseFormattedCommand( const std::string& tag_name, ParseData* data )
{
	// check if we have a font named the command
	if( !tag_name.empty() && tag_name[ 0 ] == '/' )
	{
		// FB_ASSERT( tag_name.empty()?true:tagstack.top() == tag_name.substr( 1 ) );
		if( !tagstack.empty() ) 
			tagstack.pop();
		
		if( tagstack.empty() )
		{
			data->currentFont = font;
		} else {
			std::map< std::string, IOguiFont* >::iterator i;
			i = fonts.find( tag_name );
			if( i != fonts.end() )
			{
				data->currentFont = i->second;
			}
		}

	} else {

		// check if a font by that name exists
		{ 
			std::map< std::string, IOguiFont* >::iterator i;

			i = fonts.find( tag_name );
			if( i != fonts.end() )
			{
				data->currentFont = i->second;
				tagstack.push( tag_name );
				
				return;
			}
		}

		// if we get this far it means that a font by the given name doesn't exists
		// so we look for a command by the given name and execute that
		{
			std::map< std::string, IOguiFormattedCommand* >::iterator i;

			std::string command_name = tag_name;
			std::string attributes;
			
			// parse the attributes of the command name
			{
				std::string::size_type i;
				i = command_name.find_first_of( " " );
				if( i != command_name.npos )
				{
					attributes = command_name.substr( i );
					command_name = command_name.substr( 0, i );
				}
			}

			i = commands.find( command_name );
			if( i != commands.end() )
			{	
				// execute the command
				i->second->execute( this, attributes, data );
				
			}
		}

	}

}

//.............................................................................

// code reusabilty through copy-paste
int OguiFormattedText::getLineWidth( int curpos, const std::stack< std::string >& stack, ParseData* data )
{
	std::string::size_type next_tag = 0;
	std::string::size_type next_linebrk = 0;
	std::string::size_type next_hardbreak = 0;
	int cur_pos = curpos;
	IOguiFont* cur_font = font;

	int mx = data->x;
	int my = data->y;
	int cur_w = data->w;
	int add_x = 0;
	
	std::stack< std::string > tempstack = stack;

	if( !stack.empty() )
	{
		cur_font = fonts[ stack.top() ];
	}

	while( cur_pos < (int)text.size() )
	{

		// find the place where the first tag starts
		{
			next_tag = text.find_first_of( "<",cur_pos );
			next_hardbreak = text.find_first_of( "\n\r", cur_pos );

			if( next_hardbreak == text.npos ) 
				next_hardbreak = text.size();
		}

		// findLineWidthBreak( pos, width, cur_font );
		// find the first line break
		{
			std::pair< int, int > tmp = findLineWidthBreak( cur_pos, data->w - add_x, cur_font );
			next_linebrk = tmp.first;
			// add_x += tmp.second;
			cur_w = tmp.second;
		}
		
		
		// create buttons as desired
		// we check which is the first to come, linebrk, next_tag or next_hardbreak
		if( ( next_linebrk < next_tag || next_hardbreak < next_tag ) || ( next_tag == text.npos ) )
		{
			if( next_linebrk == text.npos ) next_linebrk = next_hardbreak;
			if( next_hardbreak == text.npos ) next_hardbreak = next_linebrk;

			std::string::size_type break_here = next_linebrk<next_hardbreak?next_linebrk:next_hardbreak;
			// this happens when the line is complite without a font change

			if ( break_here == next_hardbreak  && next_linebrk != next_hardbreak ) 
			{
				int i = cur_font->getStringWidth( text.substr( cur_pos, next_hardbreak - cur_pos ).c_str() );

			    return add_x + i;
			} else {
				return add_x + cur_w;
			}

			// if( cur_pos != break_here )
				cur_pos = break_here + 1;
			
			mx = data->x;
			my += (int)( cur_font->getHeight() * lineHeight );
			add_x = 0;
		} else {
			// draw the smaller button
			cur_w = cur_font->getStringWidth(  text.substr( cur_pos, next_tag - cur_pos ).c_str() );

			// createTextButton( mx, my, cur_w, cur_font->getHeight(), text.substr( cur_pos, next_tag - cur_pos ), cur_font );
			
			// find out the place where the next string starts
			cur_pos = next_tag + 1;
			mx += cur_w;
			add_x += cur_w;
			
			// Change the font and try again
			// cur_font = myFonts[ tag_name ];
			// find out the new font
			{
				std::string::size_type cnt = text.find_first_of( ">", cur_pos );
				if( cnt != text.npos ) 
				{
					std::string tag_name = text.substr( cur_pos, cnt - cur_pos );
					cur_pos = cnt + 1;
					// closing tag
					if( !tag_name.empty() && tag_name[ 0 ] == '/' )
					{
						// FB_ASSERT( tempstack.top() == tag_name.substr( 1 ) );
						if( !tempstack.empty() ) 
							tempstack.pop();
						
						if( tempstack.empty() )
						{
							cur_font = font;
						} else {
							std::map< std::string, IOguiFont* >::iterator i;
							i = fonts.find( tag_name );
							if( i != fonts.end() )
							{
								cur_font = i->second;
							}
						}

					} else {
						std::map< std::string, IOguiFont* >::iterator i;
						i = fonts.find( tag_name );
						if( i != fonts.end() )
						{
							cur_font = i->second;
							tempstack.push( tag_name );
						}
					}
				}
			}

		} //

	}

	return add_x + cur_w;
}

//.............................................................................

std::pair< int, int > OguiFormattedText::findLineWidthBreak( std::string::size_type pos, int width, IOguiFont* cur_font )
{
	// next_linebrk stores the place where we break the line with the cur_font
#ifndef PROJECT_SHADOWGROUNDS
	const int start_pos = pos;
#endif
	int next_linebrk = pos; 
	int cur_pos = pos;
	int mw = 0;
	while( true )
	{
		int temp_w;
		pos = text.find_first_of( " \t\n\r", pos );
		if( pos == text.npos )
		{
			pos = text.size();
		}
		
		FB_ASSERT( cur_font != NULL );

		if( cur_font == NULL || ( temp_w = cur_font->getStringWidth( text.substr( cur_pos, pos - cur_pos ).c_str() ) ) > width )
		{
			break;
		}
		else
		{
			next_linebrk = pos;
			mw = temp_w;
		}

		if( pos == text.size() ) break;

		pos++;
	}

#ifndef PROJECT_SHADOWGROUNDS
	// fixing a bug with too long line
	if( next_linebrk == start_pos )
	{

		int temp_w = 0;
		cur_pos = start_pos;
		pos = start_pos;
		pos = text.find_first_of( " \t\n\r", start_pos );
		if( pos == text.npos )
			pos = text.size();
		
		FB_ASSERT( cur_font != NULL );

		if( cur_font != NULL )
			temp_w = cur_font->getStringWidth( text.substr( cur_pos, pos - cur_pos ).c_str() );
		
		return std::pair< int, int >( pos, temp_w );
	}
#endif

	return std::pair< int, int >( next_linebrk, mw );
}

//.............................................................................

void OguiFormattedText::releaseAllButtons()
{
	while( !buttons.empty() )
	{
		delete buttons.front();
		buttons.pop_front();
	}

	while( !images.empty() )
	{
		delete images.front();
		images.pop_front();
	}

	while( !tagstack.empty() )
	{
		tagstack.pop();
	}
}

//.............................................................................

void OguiFormattedText::createTextButton( int button_x, int button_y, int button_w, int button_h, const std::string& button_text, IOguiFont* button_font )
{
	FB_ASSERT( ogui );
	FB_ASSERT( window );
	
	bool cl = false;
	float cx = 0.0f;
	float cy = 0.0f;
	float cw = 1.0f;
	float ch = 1.0f;

	if( clip )
	{
		// y < y
		if( button_y < clip->y )
		{
			if( button_y + button_h > clip->y )
			{
				if( halfClip )
				{
					cl = true;
					cy = float( clip->y - button_y ) / float( button_h );
				}
				else
				{
					if( button_y < clipPositionTop )
					{
						clipPositionTop = button_y;
					}
					return;
				}
			}
			else
			{
				return;
			}
		}

		// y > y
		if( button_y + button_h > clip->y + clip->h )
		{
			if( halfClip && button_y < clip->y + clip->h )
			{
				cl = true;
				ch = float( ( clip->y + clip->h ) - button_y )  / float( button_h );
			}
			else
			{
				if( button_y < clipPositionBottom )
				{
					clipPositionBottom = button_y;
				}
				return;
			}
		}

		// x < x
		if( button_x < clip->x )
		{
			if( halfClip && button_x + button_w > clip->x )
			{
				cl = true;
				cx = float( clip->x - button_x ) / float( button_w );
			}
			else
			{

				return;
			}
		}

		// x > x
		if( button_x + button_w > clip->x + clip->w )
		{
			if( halfClip && button_x < clip->x + clip->w )
			{
				cl = true;
				cw = float( ( clip->x + clip->w ) - button_x ) / float( button_w );
			}
			else
			{
				return;
			}
		}
	}

	

	OguiButton* b;
	b = ogui->CreateSimpleTextButton( window, button_x, button_y, button_w, button_h, 
		NULL, NULL, NULL, button_text.c_str(), 0, NULL, false );
	if ( button_font ) b->SetFont( button_font );
	
	// b->SetDisabledImage( ogui->LoadOguiImage( "Data/GUI/debug.tga" ) );

	b->SetTextHAlign( hAlign );
	b->SetDisabled( true );
	b->SetClipToWindow( false );

	if( cl )
	{
		cx *= 100.0f;
		cy *= 100.0f;
		cw *= 100.0f;
		ch *= 100.0f;
		b->SetClip( cx, cy, cw, ch );
	}

	buttons.push_back( b );
}

//.............................................................................

void OguiFormattedText::createImageButton( int button_x, int button_y, int button_w, int button_h, const std::string& image )
{
	FB_ASSERT( ogui );
	FB_ASSERT( window );

	OguiButton* b;
	b = ogui->CreateSimpleImageButton( window, button_x, button_y, button_w, button_h, 
		image.c_str(), image.c_str(), image.c_str(), image.c_str(), 0, NULL, false );

	if( b )
	{
		b->SetDisabled( true );
		b->SetClipToWindow( false );
	}
	

	buttons.push_back( b );
}

//.............................................................................

void OguiFormattedText::createImageButton( int button_x, int button_y, int button_w, int button_h, IOguiImage* image )
{
	FB_ASSERT( ogui );
	FB_ASSERT( window );

	OguiButton* b;
	b = ogui->CreateSimpleImageButton( window, button_x, button_y, button_w, button_h, NULL, NULL, NULL, NULL, 0, NULL, false );
	
	if( b ) 
	{
		b->SetDisabled( true );
		b->SetDisabledImage( image );
		b->SetClipToWindow( false );
	}

	buttons.push_back( b );
	images.push_back( image );
}

///////////////////////////////////////////////////////////////////////////////

void OguiFormattedText::setClip( int x, int y, int w, int h, bool half_clip )
{
	clip->x = x;
	clip->y = y;
	clip->w = w;
	clip->h = h;

	halfClip = half_clip;
}

//.............................................................................

void OguiFormattedText::setY( int y )
{
	if( position->y != y )
	{
		position->y = y;
		releaseAllButtons();
		parseTextToButtons();
	}
}

//.............................................................................

void OguiFormattedText::moveBy( int x, int y, bool clear_top, bool clear_bottom )
{
	std::list< OguiButton* >::iterator i;

	for( i = buttons.begin(); i != buttons.end(); )
	{
		(*i)->MoveBy( x, y );

		// apply clipping
		// FIXME: this looks like a bug...
		// int x = (*i)->GetX();
		// int w = (*i)->GetSizeX();
		int y = (*i)->GetY();
		int h = (*i)->GetSizeY();
		
		if(  (clear_top    &&  y < clip->y)
			|| (clear_bottom &&  y + h > clip->y + clip->h))
		{
			delete (*i);
			i = buttons.erase(i);
		}
		else
		{
			++i;
		}
	}
}

//.............................................................................

void OguiFormattedText::move( int x, int y )
{
	if( position->x != x || position->y != y )
	{
		int diffX = x - position->x;
		int diffY = y - position->y;

		// move
		position->x = x;
		position->y = y;
		
		// change clip as well
		clip->x += diffX;
		clip->y += diffY;

		releaseAllButtons();
		parseTextToButtons();
	}
}

//.............................................................................

int OguiFormattedText::getClipPositionTop() const
{
	return clipPositionTop;
}

//.............................................................................

int OguiFormattedText::getClipPositionBottom() const
{
	return clipPositionBottom;
}

//=============================================================================

void OguiFormattedText::setTransparency( int transparency )
{
	// Logger::getInstance()->warning( std::string( ( "FormattedText" ) + boost::lexical_cast< std::string >( transparency ) ).c_str() );

	std::list< OguiButton* >::iterator i;
	for( i = buttons.begin(); i != buttons.end(); ++i )
	{
		(*i)->SetTransparency( transparency );
	}
}

///////////////////////////////////////////////////////////////////////////////

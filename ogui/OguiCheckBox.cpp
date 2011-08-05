
#include "precompiled.h"

#include "OguiCheckBox.h"

#include "Ogui.h"

#include <assert.h>

///////////////////////////////////////////////////////////////////////////////

OguiCheckBox::OguiCheckBox(  OguiWindow* win, Ogui* ogui, int x, int y, int w, int h, 
		const std::string& checkbox_false_norm, std::string checkbox_false_down, std::string checkbox_false_high, 
		const std::string& checkbox_true_norm, const std::string& checkbox_true_down, const std::string& checkbox_true_high, int id , bool value, bool changeOnClick ) :
	buttonId( 0 ),
	textId( -1 ),
	ogui( ogui ),
	win( win ),
	
	listener( NULL ),

	textButton( NULL ),
	textW( 0 ),
	textH( 0 ),
	textAlign( TEXT_ALIGN_LEFT ),

	value( value ),
	id( id ),
	changeOnClick( changeOnClick ),

	button( NULL ),
	buttonX( x ),
	buttonY( y ),
	buttonW( w ),
	buttonH( h ),

	button_false_norm( NULL ),
	button_false_down( NULL ),
	button_false_high( NULL ),
	button_true_norm( NULL ),
	button_true_down( NULL ),
	button_true_high( NULL )
{
	assert( win );
	assert( ogui );
	
	button = ogui->CreateSimpleImageButton( win, x, y, w, h, NULL, NULL, NULL, buttonId );
	assert( button );
	
	/////////////////////////////////////////////////////////////////////////////

	if( !checkbox_false_norm.empty() ) 
		button_false_norm = ogui->LoadOguiImage( checkbox_false_norm.c_str() );

	if( checkbox_false_down.empty() ) 
		checkbox_false_down = checkbox_false_norm;

	button_false_down = ogui->LoadOguiImage( checkbox_false_down.c_str() );

	if( checkbox_false_high.empty() ) 
		checkbox_false_high = checkbox_false_down;

	button_false_high = ogui->LoadOguiImage( checkbox_false_high.c_str() );

	if( !checkbox_true_norm.empty() )
		button_true_norm = ogui->LoadOguiImage( checkbox_true_norm.c_str() );

	if( !checkbox_true_down.empty() )
		button_true_down = ogui->LoadOguiImage( checkbox_true_down.c_str() );
	else 
		button_true_down = ogui->LoadOguiImage( checkbox_true_norm.c_str() );

	if( !checkbox_true_high.empty() )
		button_true_high = ogui->LoadOguiImage( checkbox_true_high.c_str() );
	else 
		button_true_high = ogui->LoadOguiImage( checkbox_true_norm.c_str() );

	/////////////////////////////////////////////////////////////////////////////

	if( value )
	{
		button->SetImage( button_true_norm );
		button->SetDownImage( button_true_down );
		button->SetHighlightedImage( button_true_high );
	}
	else
	{
		button->SetImage( button_false_norm );
		button->SetDownImage( button_false_down );
		button->SetHighlightedImage( button_false_high );
	}

	/////////////////////////////////////////////////////////////////////////////

	button->SetListener( this );
	button->SetEventMask( OguiButtonEvent::EVENT_TYPE_PRESS );

}

//.............................................................................

OguiCheckBox::~OguiCheckBox()
{
	delete button;

	if( textButton )
		textButton->SetText( NULL );

	delete textButton;

	delete button_false_norm;
	delete button_false_down;
	delete button_false_high;

	delete button_true_norm;
	delete button_true_down;
	delete button_true_high;
}

///////////////////////////////////////////////////////////////////////////////

bool OguiCheckBox::getValue() const
{
	return value;
}

//.............................................................................

int OguiCheckBox::getHeight() const
{
	return textH;
}

//.............................................................................

void OguiCheckBox::setValue( bool v )
{
	if( v != value )
	{
		value = v;
		if( value )
		{
			button->SetImage( button_true_norm );
			button->SetDownImage( button_true_down );
			button->SetHighlightedImage( button_true_high );
		}
		else
		{
			button->SetImage( button_false_norm );
			button->SetDownImage( button_false_down );
			button->SetHighlightedImage( button_false_high );
		}

		// listener launch event
		if( listener )
		{
			OguiCheckBoxEvent eve;
			eve.checkBox = this;
			eve.value = value;
			listener->checkBoxEvent( &eve );
		}
	}
}

//.............................................................................

int OguiCheckBox::getId() const
{
	return id;
}

///////////////////////////////////////////////////////////////////////////////	

void OguiCheckBox::setText( const std::string& text, TEXT_ALIGN align, int w, IOguiFont* font, OguiButton::TEXT_V_ALIGN valign )
{
	delete textButton;
	textButton = NULL;

	textW = w;
	textAlign = align;
	if( font != NULL )
	{
		float foo = ( (float)font->getStringWidth( text.c_str() ) / (float)w );
		foo++;
		textH = (int)foo * buttonH;
	}
	else
	{
		textH = textW;
	}

	{
		int buttonSpace = 0;
		int x = 0, y, w, h;
		
		std::string foo_text;
		if( font != NULL )
			foo_text = breakText( text, font );

		if( textAlign == TEXT_ALIGN_LEFT )
			x = buttonX - textW - buttonSpace;
		else if (textAlign == TEXT_ALIGN_RIGHT )
			x = buttonX + buttonW + buttonSpace;

		y = buttonY;
		w = textW;
		h = textH;
		
		textButton = ogui->CreateSimpleTextButton( win, x, y, w, h, NULL, NULL, NULL, NULL, textId );
		textButton->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
		textButton->SetTextVAlign( valign );
		textButton->SetLineBreaks( true );
		textButton->SetListener( this );
		textButton->SetEventMask( OguiButtonEvent::EVENT_TYPE_PRESS );

		if( font ) 
			textButton->SetFont( font );
		textButton->SetText( foo_text.c_str() );

		
	}

}

std::string OguiCheckBox::breakText( const std::string& text, IOguiFont* font )
{
	// text area behaviour

	// wrap the text into lines, attempt to keep it in width boundary
	int linecnt = 0;
	int lastbr = 0;
	int lastspace = -1;
	int textlen = strlen(text.c_str() );
	char *brtext = new char[textlen + 1];
	strcpy( brtext, text.c_str() );
	for (int i = 0; i < textlen + 1; i++)
	{
		if (brtext[i] == ' ' || brtext[i] == '\n' || brtext[i] == '\0' || brtext[i] == '\t' )
		{
			int sizex = textW;
			char tmpchar = brtext[i];
			brtext[i] = '\0';
			if (font->getStringWidth(&brtext[lastbr]) >= sizex)
			{
				if (lastspace != -1)
				{
					brtext[lastspace] = '\n';
					linecnt++;
					lastbr = lastspace + 1;
					lastspace = i;
				} else {
					if (tmpchar == ' ') 
					{
						tmpchar = '\n';
						linecnt++;
						lastbr = i + 1;
						lastspace = -1;
					} else {
						lastbr = i + 1;
						linecnt++;
					}
				}
			} else {
				if (tmpchar == ' ') lastspace = i;
				if (tmpchar == '\n') { lastbr = i + 1; linecnt++; }
			}
			brtext[i] = tmpchar;
		}
	}

	std::string result = brtext;
	delete [] brtext;

	linecnt++;

	//if( linecnt != 0 )
		textH = font->getHeight() * linecnt;
	//else textH = font->getHeight();

	return result;
}

///////////////////////////////////////////////////////////////////////////////

void OguiCheckBox::setListener( IOguiCheckBoxListener* l )
{
	listener = l;
}

//.............................................................................

void OguiCheckBox::CursorEvent( OguiButtonEvent *eve )
{
	if( eve->eventType == OguiButtonEvent::EVENT_TYPE_PRESS && changeOnClick )
	{
		setValue( !value );
	}
}

///////////////////////////////////////////////////////////////////////////////

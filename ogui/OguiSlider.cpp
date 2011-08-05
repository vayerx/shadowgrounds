
#include "precompiled.h"

#include "OguiSlider.h"
#include "../ogui/Ogui.h"
#include "../util/assert.h"

// #include <assert.h>

OguiSlider*		OguiSlider::updateThis = NULL;
OguiSlider*		OguiSlider::updateThisIfHold = NULL;

OguiSlider::OguiSlider( OguiWindow* win, Ogui* ogui, int x, int y, int w, int h, 
		const std::string& background_norm, const std::string& background_down, const std::string& background_high,
		const std::string& foreground_norm, const std::string& foreground_down, const std::string& foreground_high, int id, float value ) :
  backgroundId( 1 ),
  foregroundId( 2 ),
  ogui( ogui ),
  win( win ),
  x( x ),
  y( y ),
  w( w ),
  h( h ),
  id( id ),
  value( value ),
  offset_x( 0 ),
  offset_y( 0 ),
  horizontal( true ),
  listener( NULL ),
  disabledBackground( NULL ),
  disabledForeground( NULL )
{
	assert( win );
	assert( ogui );
	
	background = ogui->CreateSimpleImageButton( win, x, y, w, h, 
		background_norm.empty()?NULL:background_norm.c_str(), 
		background_down.empty()?NULL:background_down.c_str(),
		background_high.empty()?NULL:background_high.c_str(), backgroundId );
	
	foreground = ogui->CreateSimpleImageButton( win, x, y, w, h, 
		foreground_norm.empty()?NULL:foreground_norm.c_str(), 
		foreground_down.empty()?NULL:foreground_down.c_str(),
		foreground_high.empty()?NULL:foreground_high.c_str(), 
		foreground_norm.empty()?NULL:foreground_norm.c_str(), foregroundId, NULL );

 
	FB_ASSERT( foreground );
	
	if( value > 1.0f ) value = 1.0f;
	if( value < 0.0f ) value = 0.0f;
		
	// foreground->SetDisabledImage( foreground_norm.c_str() );
	if(horizontal)
		foreground->SetClip( 0.0f, 0.0f, value * 100.0f, 100.0f );
	else
		foreground->SetClip( 0.0f, 100.0f - value * 100.0f, 100.0f, 100.0f );
	foreground->SetDisabled( true );
	foreground->SetListener( this );
	foreground->SetEventMask( OGUI_EMASK_PRESS | OGUI_EMASK_CLICK | OGUI_EMASK_HOLD | OGUI_EMASK_OUT );

	background->SetListener( this );
	background->SetEventMask( OGUI_EMASK_PRESS | OGUI_EMASK_CLICK | OGUI_EMASK_HOLD | OGUI_EMASK_OUT );
	
}

OguiSlider::~OguiSlider()
{
	delete disabledBackground;
	delete disabledForeground;

	delete background;
	delete foreground;
}

void OguiSlider::setBarPosition( int x, int y, int w, int h )
{
	this->x = x;
	this->w = w;
	this->h = h;
	
	offset_x = x - this->x;
	offset_y = y - this->y;

	foreground->Resize( w, h );
	foreground->Move( x, y );
}

void OguiSlider::Move( int x, int y )
{
	background->Move( x, y );
	foreground->Move( x + offset_x, y + offset_y );
}

void OguiSlider::setTransparency( int transparency )
{
	background->SetTransparency( transparency );
	foreground->SetTransparency( transparency );
}

float OguiSlider::getValue() const
{
	return value;
}

void OguiSlider::setValue( float value )
{
	if( this->value != value )
	{
		if( value > 1.0f ) value = 1.0f;
		if( value < 0.0f ) value = 0.0f;
		
		this->value = value;

		if( listener ) 
		{
			OguiSliderEvent* eve = new OguiSliderEvent( this, value, OguiSliderEvent::EVENT_TYPE_MOUSEDRAGGED );
			listener->sliderEvent( eve );
			delete eve;
		}

		if(horizontal)
			foreground->SetClip( 0.0f, 0.0f, value * 100.0f, 100.0f );
		else
			foreground->SetClip( 0.0f, 100.0f - value * 100.0f, 100.0f, 100.0f );
	}
}
void OguiSlider::setForegroundImages( IOguiImage *norm, IOguiImage *down, IOguiImage *high)
{
	foreground->SetImage( norm );
	foreground->SetDownImage( down );
	foreground->SetHighlightedImage( high );
}
void OguiSlider::setBackgroundImages( IOguiImage *norm, IOguiImage *down, IOguiImage *high)
{
	background->SetImage( norm );
	background->SetDownImage( down );
	background->SetHighlightedImage( high );
}
void OguiSlider::setDisabledImages( const std::string& background_disabled, const std::string& foreground_disabled )
{
	if( background )
	{
		if( disabledBackground )
			delete disabledBackground;

		disabledBackground = ogui->LoadOguiImage( background_disabled.c_str() );
		background->SetDisabledImage( disabledBackground );
	}

	if( foreground )
	{
		if( disabledForeground )
			delete disabledForeground;

		disabledForeground = ogui->LoadOguiImage( foreground_disabled.c_str() );
		foreground->SetDisabledImage( disabledForeground );
	}
}
	
void OguiSlider::setDisabled( bool disabled )
{
	background->SetDisabled( disabled );
	// foreground->SetDisabled( disabled );
}

void OguiSlider::setListener( IOguiSliderListener* listen )
{
	listener = listen;
}

int OguiSlider::getId() const
{
	return id;
}

void OguiSlider::CursorEvent( OguiButtonEvent *eve )
{
	bool wheelup = (eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_UP_MASK) || (eve->cursorButtonMask & OGUI_BUTTON_WHEEL_UP_MASK);
	bool wheeldown = (eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_DOWN_MASK) || (eve->cursorButtonMask & OGUI_BUTTON_WHEEL_DOWN_MASK);
	bool wheel = wheelup || wheeldown;
	if(wheel)
	{
		if(wheelup)
		{
			setValue( getValue() + 0.1f );
		}
		if(wheeldown)
		{
			setValue( getValue() - 0.1f );
		}
		return;
	}

	if( eve->eventType == OGUI_EMASK_PRESS && updateThis == NULL )
	{
		updateThis = this;
		updateThisIfHold = this;

		if( listener ) 
		{
			float x;
			if(horizontal)
				x = (float)( eve->cursorScreenX - this->x ) / (float)w;
			else
				x = (float)( eve->cursorScreenY - this->y ) / (float)h;
			if( x > 1.0f ) x = 1.0f;
			if( x < 0.0f ) x = 0.0f;
			
			OguiSliderEvent* eve = new OguiSliderEvent( this, x, OguiSliderEvent::EVENT_TYPE_MOUSEDOWN );
			listener->sliderEvent( eve );
			delete eve;
		}
	}

	if( eve->eventType == OGUI_EMASK_OUT && updateThis == this )
	{
		updateThis = NULL;

		if( listener ) 
		{
			OguiSliderEvent* eve = new OguiSliderEvent( this, value, OguiSliderEvent::EVENT_TYPE_RELEASE );
			listener->sliderEvent( eve );
			delete eve;
		}
	}

	if( eve->eventType == OGUI_EMASK_CLICK && updateThis == this )
	{
		updateThis = NULL;
		updateThisIfHold = NULL;
		
		if( listener ) 
		{
			OguiSliderEvent* eve = new OguiSliderEvent( this, value, OguiSliderEvent::EVENT_TYPE_RELEASE );
			listener->sliderEvent( eve );
			delete eve;
		}
	}

	if( eve->eventType == OGUI_EMASK_HOLD && updateThisIfHold == this )
	{
		float x;
		if(horizontal)
			x = (float)( eve->cursorScreenX - this->x ) / (float)w;
		else
			x = (float)( eve->cursorScreenY - this->y ) / (float)h;
		if( x > 1.0f ) x = 1.0f;
		if( x < 0.0f ) x = 0.0f;
		setValue( x );
	}


}

void OguiSlider::setSliderDirection( bool horizontal_ )
{
	horizontal = horizontal_;
}

int OguiSlider::getWidth() const
{
	return w;
}

int OguiSlider::getHeight() const
{
	return h;
}

void OguiSlider::resize(int _w, int _h)
{
	w = _w;
	h = _h;
	background->Resize( w, h );
	foreground->Resize( w, h );
}

OguiGfxSlider::OguiGfxSlider( OguiWindow* win, Ogui* ogui, int x, int y, int w, int h, 
		const std::string& background_norm, const std::string& background_down, const std::string& background_high,
		const std::string& foreground_norm, const std::string& foreground_down, const std::string& foreground_high, int id, float value ) :
	OguiSlider( win, ogui, x, y, w, h, background_norm, background_down, background_high, foreground_norm, foreground_down, foreground_high, id, value )
{
	Initialize();
}

///////////////////////////////////////////////////////////////////////////////

OguiGfxSlider::~OguiGfxSlider()
{

}

void OguiGfxSlider::setDisabled( bool disabled )
{
	background->SetDisabled( disabled );
	foreground->SetDisabled( disabled );
}

void OguiGfxSlider::Initialize()
{
	background->SetListener( NULL );
	background->SetDisabled( false );
	background->SetDisabledImage( NULL );

	foreground->SetListener( NULL );
	foreground->SetDisabled( false );	
	foreground->SetDisabledImage( NULL );
}

///////////////////////////////////////////////////////////////////////////////

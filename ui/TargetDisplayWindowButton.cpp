
#include "precompiled.h"

#include "TargetDisplayButtonManager.h"
#include "TargetDisplayWindowButton.h"

#include "../ogui/Ogui.h"
#include "../ogui/OguiSlider.h"
#include "../util/assert.h"
#include "../system/Timer.h"

#include <math.h>

namespace ui {

TargetDisplayButtonManager* TargetDisplayWindowButton::buttonManager = NULL;

///////////////////////////////////////////////////////////////////////////////

TargetDisplayWindowButton::TargetDisplayWindowButton()
{
	updatedInTick = 0;
	oldW = 0;
	oldH = 0;
	// temp hack fix me 
	// imageWidth = 16;
	// imageHeight = 16;
}

//=============================================================================

TargetDisplayWindowButton::TargetDisplayWindowButton( int s )
{
	updatedInTick = 0;
	oldW = 0;
	oldH = 0;
	FB_ASSERT( buttonManager != NULL );

	b = buttonManager->createButton( s );
	beginAnimInitialPosition = b.beginAnimPos;
	beginAnimPos = b.beginAnimPos;
	imageWidth = b.width;
	imageHeight = b.height;

	style = s;

	startTicks = Timer::getTime();
	createTime = Timer::getTime();
}

//=============================================================================

TargetDisplayWindowButton::TargetDisplayWindowButton( const TargetDisplayWindowButton& button )
{
	updatedInTick = 0;
	oldW = 0;
	oldH = 0;
	b = button.b;
	imageWidth = button.imageWidth;
	imageHeight = button.imageHeight;
	beginAnimPos = button.beginAnimPos;
	beginAnimInitialPosition = button.beginAnimInitialPosition;
	style = button.style;
}

//=============================================================================

TargetDisplayWindowButton::~TargetDisplayWindowButton()
{
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindowButton::operator=( const TargetDisplayWindowButton& button )
{
	b = button.b;
	imageWidth = button.imageWidth;
	imageHeight = button.imageHeight;
	beginAnimPos = button.beginAnimPos;
	beginAnimInitialPosition = button.beginAnimInitialPosition;
	style = button.style;
	createTime = button.createTime;
}

//=============================================================================

float TargetDisplayWindowButton::getAnimPos( int x )
{
	float f = (float)x / 30.0f;

	beginAnimPos -= ( beginAnimPos / 2 ) * f;

	return beginAnimPos;
}

//=============================================================================

int calculateTransparency( float distance )
{
	if( distance < 0.9f )
	{
		return (int)( distance * 55.0f );
	}
	else
	{
		distance -= 0.9f;
		distance *= 10.0f;
		return 50 + (int)( (float)distance * 40.0f );
	}
}

//=============================================================================

void TargetDisplayWindowButton::setRect( int x, int y, int w, int h, float distance )
{
	if( isEmpty() ) return;

	/*if( b.rect_interpolation_factor > 0 )
	{
		if(updatedInTick > 0)
		{
			float fact1 = (1 - b.rect_interpolation_factor);
			float fact2 = b.rect_interpolation_factor;

			// cancel out current size in coordinates
			x += w/2;
			y += h/2;

			// interpolate size
			w = (int)( w * fact1 + oldW * fact2 );
			h = (int)( h * fact1 + oldH * fact2 );

			// add new size to coordinates
			x -= w/2;
			y -= h/2;
		}

		oldW = w;
		oldH = h;
	}*/

	const int topx = x - (int)beginAnimPos - imageWidth / 2;
	const int topy = y - (int)beginAnimPos - imageHeight / 2;
	const int bottomx = x + w + (int)beginAnimPos - imageWidth / 2;
	const int bottomy = y + h + (int)beginAnimPos - imageHeight / 2;


	int transparency = calculateTransparency( distance );

	if( b.hasText )
	{
		if( beginAnimPos < 1.0f ) 
			setButtonTextPosition( topx, bottomy, bottomx + imageWidth, transparency );
		else
			hideButton( buttontext );	
	}
	
	if( b.healthSlider )
	{
		if( beginAnimPos < 1.0f )
		{
			setButtonSliderPosition( topx, topy, bottomx + imageWidth, transparency );
		}
		else
		{
			// FIXME
			hideSlider();
		}
	}

	if( beginAnimPos > 1.0f ) 
		if( transparency < 50 ) 
			getAnimPos( Timer::getTime() - startTicks );
		else transparency = 100;
	else beginAnimPos = 0.0f;

	startTicks = Timer::getTime();

	setButtonPosition( topleft, topx, topy, transparency  );
	setButtonPosition( topright, bottomx, topy, transparency );
	setButtonPosition( bottomright, bottomx, bottomy, transparency );
    setButtonPosition( bottomleft, topx, bottomy, transparency );
	

}

int TargetDisplayWindowButton::timeActive() const
{
	return Timer::getTime() - createTime;
}

//=============================================================================

void TargetDisplayWindowButton::setText( const std::string& text )
{
	buttonManager->setText( b, text );
}

//=============================================================================

void TargetDisplayWindowButton::setSliderValue( float v, float scale )
{
	if( b.healthSlider )
	{
		if( v < b.sliderLowLimit )
		{
			if(b.sliderCurrentImages != 1 && b.healthSliderImages[3] && b.healthSliderImages[4])
			{
				// low images
				b.healthSlider->setBackgroundImages(b.healthSliderImages[2], b.healthSliderImages[2], b.healthSliderImages[2]);
				b.healthSlider->setForegroundImages(b.healthSliderImages[3], b.healthSliderImages[3], b.healthSliderImages[3]);
				b.sliderCurrentImages = 1;
			}
		}
		else if( v > b.sliderHighLimit )
		{
			if(b.sliderCurrentImages != 2 && b.healthSliderImages[3] && b.healthSliderImages[4])
			{
				// high images
				b.healthSlider->setBackgroundImages(b.healthSliderImages[4], b.healthSliderImages[4], b.healthSliderImages[4]);
				b.healthSlider->setForegroundImages(b.healthSliderImages[5], b.healthSliderImages[5], b.healthSliderImages[5]);
				b.sliderCurrentImages = 2;
			}
		}
		else if(b.sliderCurrentImages != 0)
		{
			// normal images
			b.healthSlider->setBackgroundImages(b.healthSliderImages[0], b.healthSliderImages[0], b.healthSliderImages[0]);
			b.healthSlider->setForegroundImages(b.healthSliderImages[1], b.healthSliderImages[1], b.healthSliderImages[1]);
			b.sliderCurrentImages = 0;
		}

		b.healthSlider->resize( (int)( b.sliderWidth * scale ), b.sliderHeight );
		b.healthSlider->setValue( v );
	}
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindowButton::resetAni()
{
	beginAnimPos = beginAnimInitialPosition;
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindowButton::hide()
{
	if( isEmpty() ) return;

	for( int i = 0; i < (int)b.buttons.size(); i++ )
	{
		hideButton( i );
	}

	hideSlider();
}

//=============================================================================

void TargetDisplayWindowButton::show()
{
	if( isEmpty() ) return;

	for( int i = 0; i < (int)b.buttons.size(); i++ )
	{
		b[ i ]->SetDisabled( false );
	}

	showSlider();

	createTime = Timer::getTime();
}

//=============================================================================

void TargetDisplayWindowButton::hideButton( int i )
{
	FB_ASSERT( i >= 0 && i < (int)b.buttons.size() );
	b[ i ]->SetDisabled( true );

	if( b.hasText && i == 4 )
	{
		b[ 4 ]->SetText( NULL );
	}
}

//=============================================================================

void TargetDisplayWindowButton::showButton( int i )
{
	FB_ASSERT( i >= 0 && i < (int)b.buttons.size() );
	b[ i ]->SetDisabled( false );

	if( b.hasText && i == 4 )
	{
		b[ 4 ]->SetText( b.theText.c_str() );
	}
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindowButton::showSlider()
{
	if( b.healthSlider )
	{
		b.healthSlider->setDisabled( false );
		// b.healthSlider->setTransparency( 0 );
		// b.healthSlider->setDisabledImages( );
	}
}

//=============================================================================

void TargetDisplayWindowButton::hideSlider()
{
	if( b.healthSlider )
	{
		b.healthSlider->setDisabled( true );
		// b.healthSlider->Move( 1200, 1000 );
	}
	// b.healthSlider->setTransparency( 100 );
}

bool TargetDisplayWindowButton::hasSlider() const
{
	return ( b.healthSlider != NULL );
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindowButton::release()
{
	hide();
	resetAni();
	buttonManager->releaseButton( b );

}

///////////////////////////////////////////////////////////////////////////////

bool TargetDisplayWindowButton::isEmpty() const
{
	return b.buttons.empty();
}

///////////////////////////////////////////////////////////////////////////////

int TargetDisplayWindowButton::getStyle() const
{
	return style;
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayWindowButton::setButtonPosition( int i, int x, int y, int transparency )
{
	FB_ASSERT( i >= 0 && i < (int)b.buttons.size() );

	if( i >= 0 && i < (int)b.buttons.size() &&
		x - imageWidth >= 0 && x < 1024 && 
		y - imageHeight >= 0 && y < 768 )
	{
		b[ i ]->Move( x, y );
		b[ i ]->SetTransparency( transparency );
		showButton( i );
	}
	else
	{
		hideButton( i );
	}
}

//=============================================================================

void TargetDisplayWindowButton::setButtonSliderPosition( int x, int y, int bottomx, int transparency )
{
	if( isEmpty() ) return;

	if( b.healthSlider )
	{
		if( x - imageWidth >= 0 && x < 1024 && 
			y - imageHeight >= 0 && y < 768 )
		{
			int x_pos = x;
			int y_pos = y;

			if( true )
			{
				x_pos += ( bottomx - x ) / 2;
				x_pos -= b.sliderWidth / 2;
				y_pos -= b.sliderHeight;
			}

			b.healthSlider->Move( x_pos, y_pos );
			b.healthSlider->setTransparency( transparency );
			// FIXME
			showSlider();
		}
		else
		{
			// FIXME
			hideSlider();
		}
	}
}

void TargetDisplayWindowButton::setButtonTextPosition( int x, int y, int bottomx, int transparency )
{
	if( isEmpty() ) return;

	if( b.hasText )
	{
		if( x - imageWidth >= 0 && x < 1024 && 
		y - imageHeight >= 0 && y < 768 )
		{
			int x_pos = x;
			int y_pos = y;

			if( b.textCenter )
			{
				x_pos += ( bottomx - x_pos ) / 2;
				x_pos -= b.textWidth / 2;
				y_pos += b.offsetY;

			}
			else
			{
				x_pos += b.offsetX;
				y_pos += b.offsetY;
			}
		
			FB_ASSERT( b.buttons.size() >= 5 );
			b[ buttontext ]->Move( x_pos, y_pos );
			b[ buttontext ]->SetTransparency( transparency );
			if( transparency > 50 )
			{
				b[ buttontext ]->Move( x_pos, y_pos );
			}
			showButton( buttontext );
		}
		else
		{
			hideButton( buttontext );
		}
	}
}

void TargetDisplayWindowButton::updateRect(void)
{
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayRisingScoreButton::setRect( int x, int y, int w, int h, float distance )
{
	oldx = x;
	oldy = y;
	oldw = w;
	oldh = h;
	float how_long = 2000;
	int rising_length = 150;

	if( isEmpty() ) return;

	const int topx = x - imageWidth / 2;
	const int topy = y - imageHeight / 2;
	const int bottomx = x + w - imageWidth / 2;
	const int bottomy = y + h - imageHeight / 2;

	float t =  ( Timer::getTime() - startTicks ) / how_long; 
	int transparency = (int)( t * 255.0f + 0.5f);

	const int text_pos_y = (int)( ( bottomy - ( bottomy - topy ) / 2.0f ) - (t * rising_length) + 0.5f );

	setButtonTextPosition( topx, text_pos_y, bottomx + imageWidth, transparency );

	if( t > 1.0f )
	{
		amIDead = true;
	}
}

void TargetDisplayRisingScoreButton::updateRect(void)
{
	setRect(oldx, oldy, oldw, oldh);
}

} // end of namespace ui

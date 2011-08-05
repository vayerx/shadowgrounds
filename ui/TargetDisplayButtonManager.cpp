
#include "precompiled.h"

#include "TargetDisplayButtonManager.h"
#include "../util/assert.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiSlider.h"

#include <sstream>
#include <boost/lexical_cast.hpp>

#include "../system/Logger.h"
#include "../game/DHLocaleManager.h"

using namespace game;

namespace ui {

///////////////////////////////////////////////////////////////////////////////

TargetDisplayButtonManager::TargetDisplayButtonManager( Ogui* ogui, OguiWindow* window ) :
	ogui( ogui ),
	window( window )
{
}

//=============================================================================

TargetDisplayButtonManager::~TargetDisplayButtonManager()
{
	std::map< int, std::stack< button > >::iterator i;
	
	for( i = buttonStack.begin(); i != buttonStack.end(); ++i )
	{
		while( !i->second.empty() )
		{
			for( int j = 0; j < (int)i->second.top().buttons.size(); j++ )
			{	
				delete i->second.top()[ j ];
			}

			delete i->second.top().healthSliderImages[0];
			delete i->second.top().healthSliderImages[1];
			delete i->second.top().healthSliderImages[2];
			delete i->second.top().healthSliderImages[3];
			delete i->second.top().healthSliderImages[4];
			delete i->second.top().healthSliderImages[5];
			delete i->second.top().healthSlider;
			i->second.pop();
		}
	}
	
	std::map< int, ButtonCreator >::iterator j;
	for( j = buttonOriginals.begin(); j != buttonOriginals.end(); ++j )
	{
		delete j->second.font;
	}


}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayButtonManager::registerButtonStyle( int style, const ButtonCreator& butt )
{
	std::map< int, ButtonCreator >::iterator i;
	
	i = buttonOriginals.find( style );
	
	FB_ASSERT( i == buttonOriginals.end() );

	buttonOriginals.insert( std::pair< int, ButtonCreator >( style, butt ) );

}

///////////////////////////////////////////////////////////////////////////////

TargetDisplayButtonManager::button TargetDisplayButtonManager::createButton( int style )
{
	std::map< int, std::stack< button > >::iterator i;
	i = buttonStack.find( style );

	if( i != buttonStack.end() )
	{
		if( i->second.empty() )
		{
			return newButton( style );
		}
		else
		{
			button result = i->second.top();
			i->second.pop();

			return result;
		}
	}
	else
	{
		std::stack< button > tmp;
		buttonStack.insert( std::pair< int, std::stack< button > >( style, tmp ) );

		return newButton( style );
	}
}

//=============================================================================

void TargetDisplayButtonManager::releaseButton( button b )
{
	
	std::map< int, std::stack< button > >::iterator i;
	i = buttonStack.find( b.style );

	if( b.hasText )
	{
		b[ 4 ]->SetText( NULL );
		// delete b[ 4 ];
		// b[ 4 ] = NULL;
		// b.buttons.resize( 4 );
		// b.hasText = false;
	}

	/*if( b.healthSlider )
	{
		delete b.healthSlider;
		b.healthSlider = NULL;
	}*/

	FB_ASSERT( i != buttonStack.end() );
	
	if( i == buttonStack.end() ) 
		return;
	
	i->second.push( b );

	/*if( (signed)i->second.size() > stackMaxCount ) 
		stackMaxCount = i->second.size();*/
}

///////////////////////////////////////////////////////////////////////////////

void TargetDisplayButtonManager::setText( button& b, const std::string& txt )
{
	int textW = 10;
	int textH = 20;

	const std::string& text = getLocaleGuiString( txt.c_str() );

	if( b.hasText ) 
	{
		// b.buttons[ 4 ]->SetDisabled( false );
		textW = b.font?b.font->getStringWidth( text.c_str() ):0;
		
		delete b.buttons[ 4 ];
		b.buttons[ 4 ] = ogui->CreateSimpleTextButton( window, 0, 0, textW, textH, NULL, NULL, NULL, text.c_str() );
		b.buttons[ 4 ]->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
		b.buttons[ 4 ]->SetFont( b.font );
		b.buttons[ 4 ]->SetText( NULL );
		b.theText = text;
		b.textWidth = textW;
	}
	else
	{
		textW = b.font?b.font->getStringWidth( text.c_str() ):0;

		b.buttons.resize( 5 );
		b.buttons[ 4 ] = ogui->CreateSimpleTextButton( window, 0, 0, textW, textH, NULL, NULL, NULL, text.c_str() );
		b.buttons[ 4 ]->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
		b.buttons[ 4 ]->SetFont( b.font );
		b.buttons[ 4 ]->SetText( NULL );
		b.hasText = true;
		b.theText = text;
		b.textWidth = b.font?b.font->getStringWidth( text.c_str() ):0;
		// b.theText = boost::lexical_cast< std::string >( b.textWidth );
	}

	/*
	std::stringstream ss;
	ss << "Temp hax: " << b.hasText << "(true), " << b.theText << "(Localization missing), " << b.textWidth << std::endl;
	Logger::getInstance()->error( ss.str().c_str() );
	*/
}

///////////////////////////////////////////////////////////////////////////////

TargetDisplayButtonManager::button TargetDisplayButtonManager::newButton( int style )
{
	button result;

	std::map< int, ButtonCreator >::iterator i;
	i = buttonOriginals.find( style );

	FB_ASSERT( i != buttonOriginals.end() );
	
	if( i == buttonOriginals.end() ) 
		return result;
	
	result.buttons.resize( i->second.buttonImages.size() );

	int j = 0;
	for( j = 0; j < (int)i->second.buttonImages.size(); j++ )
	{
		result[ j ] = ogui->CreateSimpleImageButton( window, 0, 0, i->second.width, i->second.height, 
			i->second.buttonImages[ j ].c_str(), NULL, NULL, 0 );

		result[ j ]->SetReactMask( 0 );
		result[ j ]->SetEventMask( 0 );
		result[ j ]->SetDisabled( true );
	}

	result.style = style;
	result.font  = i->second.font;
	result.width = i->second.width;
	result.height = i->second.height;
	result.beginAnimPos = i->second.beginAnimPos;
	result.offsetX	= i->second.textOffsetX;
	result.offsetY	= i->second.textOffsetY;
	result.textCenter = i->second.textCenter;
	
	if( i->second.hasSlider )
	{
		// result.hasHealthSlider = true;
		result.healthSlider = new OguiGfxSlider( window, ogui, 0, 0, i->second.sliderWidth, i->second.sliderHeight, 
								i->second.sliderBackground, i->second.sliderBackground, i->second.sliderBackground,
								i->second.sliderForeground, i->second.sliderForeground, i->second.sliderForeground );

		result.healthSliderImages[0] = ogui->LoadOguiImage(i->second.sliderBackground.c_str());
		result.healthSliderImages[1] = ogui->LoadOguiImage(i->second.sliderForeground.c_str());
		// low images
		if(!i->second.sliderBackgroundLow.empty())
			result.healthSliderImages[2] = ogui->LoadOguiImage(i->second.sliderBackgroundLow.c_str());
		if(!i->second.sliderForegroundLow.empty())
			result.healthSliderImages[3] = ogui->LoadOguiImage(i->second.sliderForegroundLow.c_str());
		// high images
		if(!i->second.sliderBackgroundHigh.empty())
			result.healthSliderImages[4] = ogui->LoadOguiImage(i->second.sliderBackgroundHigh.c_str());
		if(!i->second.sliderForegroundHigh.empty())
			result.healthSliderImages[5] = ogui->LoadOguiImage(i->second.sliderForegroundHigh.c_str());

		// result.healthSlider->setDisabledImages( i->second.sliderBackground, i->second.sliderForeground );
		result.healthSlider->setDisabled( true );
		result.sliderWidth = i->second.sliderWidth;
		result.sliderHeight = i->second.sliderHeight;
		result.sliderLowLimit = 0.01f * i->second.sliderLowLimit;
		result.sliderHighLimit = 0.01f * i->second.sliderHighLimit;
	}
	else
	{
		result.healthSlider = NULL;
	}
	
	return result;
}

///////////////////////////////////////////////////////////////////////////////

bool TargetDisplayButtonManager::isRegistered( int style ) const
{
	std::map< int, ButtonCreator >::const_iterator i;
	i = buttonOriginals.find( style );

	return ( i != buttonOriginals.end() );
}


const TargetDisplayButtonManager::ButtonCreator &TargetDisplayButtonManager::getButtonStyle( int style ) const
{
	std::map< int, ButtonCreator >::const_iterator i;
	i = buttonOriginals.find( style );

	// returns dummy style!!
	if( i == buttonOriginals.end())
	{
		assert(!buttonOriginals.empty());
		return buttonOriginals.begin()->second;
	}

	return i->second;

}

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

#include "precompiled.h"

#include "OguiLocaleWrapper.h"
#include "Ogui.h"
#include "OguiFormattedText.h"
#include "OguiWindow.h"
#include "OguiButtonStyle.h"
#include "OguiSelectListStyle.h"
#include "OguiSelectList.h"
#include "OguiSlider.h"
#include "../util/fb_assert.h"
#include "../game/DHLocaleManager.h"

#include <map>
#include <list>
#include <string>

#include <fstream>
#include <boost/lexical_cast.hpp>

using namespace game;

///////////////////////////////////////////////////////////////////////////////

namespace {
	bool reportMissingLocales = false;
}

///////////////////////////////////////////////////////////////////////////////

class OguiLocaleWrapper::OguiLocaleWrapperImpl
{
public:
	struct SimpleRect
	{
		int x;
		int y;
		int w;
		int h;
	};

	//=========================================================================

	OguiLocaleWrapperImpl( Ogui* ogui ) : 
		ogui( ogui ),
		windowName(),
		logErrorMessages( false ),
		errorMessagesLogFile(),
		oguiFonts(),
		oguiStyles(),
		oguiSelectListStyles(),
		oguiImages()
	{ 
	}

	~OguiLocaleWrapperImpl() 
	{ 
		{
			std::list< OguiButtonStyle* >::iterator i;
			for( i = oguiStyles.begin(); i != oguiStyles.end(); ++i )
			{
				delete (*i)->image;
				delete (*i)->imageDown;
				delete (*i)->imageDisabled;
				delete (*i)->imageHighlighted;

				delete (*i); // BUG!?
			}
		}

		{
			std::list< OguiSelectListStyle* >::iterator i;
			for( i = oguiSelectListStyles.begin(); i != oguiSelectListStyles.end(); ++i )
			{
				delete (*i);
			}
		}
		
		{
			std::map< std::string, IOguiFont* >::iterator i = oguiFonts.begin();
			for ( i = oguiFonts.begin(); i != oguiFonts.end(); ++i )
			{
				delete i->second;
				i->second = NULL;
			}
		}

		{
			std::list< IOguiImage* >::iterator i = oguiImages.begin();
			for( i  = oguiImages.begin(); i != oguiImages.end(); ++i )
			{
				delete (*i);
			}
		}
	}

	//=========================================================================

	void SetLogging( bool log, const std::string& file )
	{
		errorMessagesLogFile = file;

		if( logErrorMessages != log )
		{
			logErrorMessages = log;
			if( logErrorMessages )
			{
				std::fstream out( errorMessagesLogFile.c_str(), std::ios::out );
				out << "// Missing locales" << std::endl;
			}
		}
	}

	//=========================================================================

	void SetWindowName( const std::string& name )
	{
		windowName = name;
	}

	//=========================================================================

	const std::string &GetWindowName(void)
	{
		return windowName;
	}

	//=========================================================================

	OguiWindow* LoadWindow( const std::string& name )
	{
		SetWindowName( name );
		std::string bgpic = getLocaleString( std::string( "gui_" ) + windowName + "_background_image" );
		
		SimpleRect rect = LoadRect( std::string( "gui_" ) + windowName );

		return ogui->CreateSimpleWindow( rect.x, rect.y, rect.w, rect.h, bgpic.c_str() );
	}

	//=========================================================================

	OguiButton* LoadButton( const std::string& button_name, OguiWindow* window, int id )
	{
		std::string window_name = windowName;
		// std::string temp;
		
		// rect
		SimpleRect rect = LoadRect( std::string( "gui_" ) + window_name + "_" + button_name );

		// images
		std::string img_normal = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_img_normal" );
		std::string img_down = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_img_down" );
		std::string img_high = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_img_high" );
		std::string img_disa = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_img_disabled" );

		std::string img_selected = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_img_selected" );
		std::string img_selected_high = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_img_selected_high" );

		// fonts
		std::string font_normal = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_font_normal" );
		std::string font_down = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_font_down" );
		std::string font_high = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_font_high" );
		std::string font_disa = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_font_disabled" );

		// alignment
		std::string align_h = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_align_h" );
		std::string align_v = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_align_v" );

		// text
		std::string text = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_text" );

		OguiButton* return_value = ogui->CreateSimpleTextButton( window, rect.x, rect.y, rect.w, rect.h, 
			img_normal.empty()?NULL:img_normal.c_str(), 
			img_down.empty()?NULL:img_down.c_str(),
			img_high.empty()?NULL:img_high.c_str(), 
			"",
			id, NULL, true );

		if( img_disa.empty() == false )
			return_value->SetDisabledImage( LoadImage( img_disa ) );

		if( font_normal.empty() == false )
			return_value->SetFont( LoadFont( font_normal ) );
		
		if( font_down.empty() == false )
			return_value->SetDownFont( LoadFont( font_down ) );
		
		if( font_high.empty() == false )
			return_value->SetHighlightedFont( LoadFont( font_high ) );
		
		if( font_disa.empty() == false )
			return_value->SetDisabledFont( LoadFont( font_disa ) );

		if(align_h == "left")
			return_value->SetTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
		else if(align_h == "center")
			return_value->SetTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
		else if(align_h == "right")
			return_value->SetTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);

		if(align_v == "top")
			return_value->SetTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
		else if(align_v == "middle")
			return_value->SetTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);
		else if(align_v == "bottom")
			return_value->SetTextVAlign(OguiButton::TEXT_V_ALIGN_BOTTOM);

		if( text.empty() == false )
			return_value->SetText( text.c_str() );

		if( img_selected.empty() == false )
		{
			IOguiImage* selected_norm = NULL;
			IOguiImage* selected_high = NULL;
			
			if( img_selected.empty() == false )
				selected_norm = ogui->LoadOguiImage( img_selected.c_str() );

			if( img_selected_high.empty() == false )
				selected_high = ogui->LoadOguiImage( img_selected_high.c_str() );

			return_value->SetSelectedImages( selected_norm, selected_high );
		}		

		return return_value;
	}

	//=========================================================================

	OguiFormattedText* LoadFormattedText( const std::string& name, OguiWindow* window, int id )
	{
		std::string window_name = windowName;
		// std::string temp;
		
		// rect
		SimpleRect rect = LoadRect( std::string( "gui_" ) + window_name + "_" + name );
	
		// fonts
		const std::string font_normal = getLocaleString( std::string( "gui_" ) + window_name + "_" + name + "_font_normal" );
		const std::string font_b = getLocaleString( std::string( "gui_" ) + window_name + "_" + name + "_font_bold" );
		const std::string font_i = getLocaleString( std::string( "gui_" ) + window_name + "_" + name + "_font_italic" );
		const std::string font_h1 = getLocaleString( std::string( "gui_" ) + window_name + "_" + name + "_font_h1" );

		// alignment
		std::string align_h = getLocaleString( std::string( "gui_" ) + window_name + "_" + name + "_align_h" );
		std::string align_v = getLocaleString( std::string( "gui_" ) + window_name + "_" + name + "_align_v" );

		// text
		std::string text = getLocaleString( std::string( "gui_" ) + window_name + "_" + name + "_text" );

		OguiFormattedText* return_value = new OguiFormattedText( window, ogui, rect.x, rect.y, rect.w, rect.h, id );

		if( font_normal.empty() == false )
			return_value->setFont( LoadFont( font_normal ) );
		
		if( font_b.empty() == false )
			return_value->registerFont( "b", LoadFont( font_b ) );
		
		if( font_i.empty() == false )
			return_value->registerFont( "i", LoadFont( font_i ) );
		
		if( font_h1.empty() == false )
			return_value->registerFont( "h1", LoadFont( font_h1 ) );

		if(align_h == "left")
			return_value->setTextHAlign(OguiButton::TEXT_H_ALIGN_LEFT);
		else if(align_h == "center")
			return_value->setTextHAlign(OguiButton::TEXT_H_ALIGN_CENTER);
		else if(align_h == "right")
			return_value->setTextHAlign(OguiButton::TEXT_H_ALIGN_RIGHT);

		if(align_v == "top")
			return_value->setTextVAlign(OguiButton::TEXT_V_ALIGN_TOP);
		else if(align_v == "middle")
			return_value->setTextVAlign(OguiButton::TEXT_V_ALIGN_MIDDLE);
		else if(align_v == "bottom")
			return_value->setTextVAlign(OguiButton::TEXT_V_ALIGN_BOTTOM);

		if( text.empty() == false )
			return_value->setText( text.c_str() );

		return return_value;
	}

	//=========================================================================

	OguiSelectList* LoadSelectList( const std::string& name, OguiWindow* window, int id )
	{
		std::string window_name = windowName;

		OguiSelectList* return_value = NULL;
		OguiSelectListStyle* selectListStyle = NULL;
		{
			OguiButtonStyle* unselStyle = LoadStyle( std::string( "gui_" ) + window_name + "_" + name + "_unselected_item" );
			OguiButtonStyle* selStyle = LoadStyle( std::string( "gui_" ) + window_name + "_" + name + "_selected_item" );

			OguiButtonStyle* newStyle = LoadStyle( std::string( "gui_" ) + window_name + "_" + name + "_new_selected_item" );
			OguiButtonStyle* newUnStyle = LoadStyle( std::string( "gui_" ) + window_name + "_" + name + "_new_unselected_item" );

			OguiButtonStyle* scrollUpStyle = LoadStyle( std::string( "gui_" ) + window_name + "_" + name + "_arrow_down" );
			OguiButtonStyle* scrollDownStyle = LoadStyle( std::string( "gui_" ) + window_name + "_" + name + "_arrow_up" );

			int num_of_elements = getLocaleInt( std::string( "gui_" ) + window_name + "_" + name + "_number_of_items", 0 );
			selectListStyle = new OguiSelectListStyle( unselStyle, selStyle, newStyle, newUnStyle, scrollUpStyle, scrollDownStyle, unselStyle->sizeX, unselStyle->sizeY * num_of_elements, scrollUpStyle->sizeX, scrollUpStyle->sizeY );
			oguiSelectListStyles.push_back( selectListStyle );
			// BUG BUG, leaks memory
		}

		int x = getLocaleInt( std::string( "gui_" ) + window_name + "_" + name + "_x", 0 );
		int y = getLocaleInt( std::string( "gui_" ) + window_name + "_" + name + "_y", 0 );
		FB_ASSERT( selectListStyle != NULL );
		return_value = ogui->CreateSelectList( window, x, y, selectListStyle, 0, NULL, NULL, false, 0, id );


		return return_value;
	}

	//////////////////////////////////////////////////////////////////////////////

	OguiSlider* LoadSlider( const std::string& button_name, OguiWindow* window, int id )
	{
		OguiSlider* return_value = NULL;

		std::string window_name = windowName;
		
		// rect
		SimpleRect rect = LoadRect( std::string( "gui_" ) + window_name + "_" + button_name );

		// images
		std::string back_normal = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_back_normal" );
		std::string back_down = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_back_down" );
		std::string back_high = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_back_high" );
		std::string back_disa = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_back_disabled" );

		std::string fore_normal = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_fore_normal" );
		std::string fore_down = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_fore_down" );
		std::string fore_high = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_fore_high" );
		std::string fore_disa = getLocaleString( std::string( "gui_" ) + window_name + "_" + button_name + "_fore_disabled" );

		return_value = new OguiSlider( window, ogui, rect.x, rect.y, rect.w, rect.h, 
			back_normal, back_down, back_high,
			fore_normal, fore_down, fore_high, id, 1.0f );
		
		return_value->setDisabledImages( back_disa, fore_disa );


		return return_value;
	}

	//////////////////////////////////////////////////////////////////////////////

	IOguiFont* LoadFont( const std::string& name )
	{
		fb_assert( name.empty() == false );

		std::map< std::string, IOguiFont* >::iterator i;
		i = oguiFonts.find( name );
		if( i != oguiFonts.end() )
			return i->second;

		IOguiFont* font = ogui->LoadFont( name.c_str() );
		if( font == NULL )
			return NULL;

		oguiFonts.insert( std::pair< std::string, IOguiFont* >( name, font ) );

		return font;
	}

	//=========================================================================

private:
	SimpleRect LoadRect( const std::string& name )
	{
		SimpleRect return_value;

		return_value.x = getLocaleInt( name + "_x", 0 );
		return_value.y = getLocaleInt( name + "_y", 0 );
		return_value.w = getLocaleInt( name + "_w", 0 );
		return_value.h = getLocaleInt( name + "_h", 0 );

		return return_value;
	}

	//=========================================================================

	OguiButtonStyle* LoadStyle( const std::string& button_name )
	{
		IOguiImage* norm = ogui->LoadOguiImage( getLocaleString( button_name + "_norm"  ).c_str() );
		IOguiImage* high = ogui->LoadOguiImage( getLocaleString( button_name + "_high"  ).c_str() );
		IOguiImage* down = ogui->LoadOguiImage( getLocaleString( button_name + "_down"  ).c_str() );
		IOguiImage* disa = ogui->LoadOguiImage( getLocaleString( button_name + "_disa"  ).c_str() );
		IOguiFont*  font = LoadFont( getLocaleString( button_name + "_font"  ) );

		int w = getLocaleInt( button_name + "_w", 0 );
		int h = getLocaleInt( button_name + "_h", 0 );

		OguiButtonStyle* result = new OguiButtonStyle( norm, down, disa, high, font, w, h );

		oguiStyles.push_back( result );
		return result;

	}

	//=========================================================================

	IOguiImage* LoadImage( const std::string& file_name )
	{
		IOguiImage* result = NULL;

		result = ogui->LoadOguiImage( file_name.c_str() );
		if( result ) 
			oguiImages.push_back( result );

		return result;
	}

	//=========================================================================

	std::string getLocaleString( const std::string& name )
	{
		if( name.empty() )
			return "";

		if( DHLocaleManager::getInstance()->hasString( DHLocaleManager::BANK_GUI, name.c_str() ) == false )
		{
			LogMissingLocale( name );
			return "";
		}

		return DHLocaleManager::getInstance()->getString( DHLocaleManager::BANK_GUI, name.c_str() );
	}
	
	//=========================================================================

	int getLocaleInt( const std::string& name, int default_value = 0 )
	{
		std::string s = getLocaleString( name );
		if( s.empty() )
			return default_value;
		
		return boost::lexical_cast< int >( s );
	}

	//=========================================================================

	void LogMissingLocale( const std::string& name )
	{
		if( logErrorMessages )
		{
			std::fstream out( errorMessagesLogFile.c_str(), std::ios::out | std::ios::app );
			out << name << " = " << std::endl;
		}
		
		if( reportMissingLocales )
		{
			std::string temp = "ERROR: Couldn't find locale key: " + name;
			Logger::getInstance()->error( temp.c_str() ); 
		}
	}

	//=========================================================================

	Ogui* ogui;
	std::string windowName;

	bool		logErrorMessages;
	std::string errorMessagesLogFile;
	

	std::map< std::string, IOguiFont* >	oguiFonts;
	std::list< OguiButtonStyle* >		oguiStyles;
	std::list< OguiSelectListStyle* >	oguiSelectListStyles;
	std::list< IOguiImage* >			oguiImages;
};
///////////////////////////////////////////////////////////////////////////////

OguiLocaleWrapper::OguiLocaleWrapper( Ogui* ogui ) :
	impl( NULL )
{
	impl = new OguiLocaleWrapperImpl( ogui );
}

//=============================================================================

OguiLocaleWrapper::~OguiLocaleWrapper()
{
	delete impl;
	impl = NULL;
}

//=============================================================================

void OguiLocaleWrapper::SetLogging( bool log, const std::string& file )
{
	FB_ASSERT( impl );
	impl->SetLogging( log, file );
}

//=============================================================================

void OguiLocaleWrapper::SetWindowName( const std::string& name )
{
	FB_ASSERT( impl );
	impl->SetWindowName( name );
}

const std::string &OguiLocaleWrapper::GetWindowName(void)
{
	FB_ASSERT( impl );
	return impl->GetWindowName();
}

//=============================================================================

OguiWindow*	OguiLocaleWrapper::LoadWindow( const std::string& name )
{
	FB_ASSERT( impl );
	return impl->LoadWindow( name );
}

//=============================================================================

OguiButton*	OguiLocaleWrapper::LoadButton( const std::string& button_name, OguiWindow* window, int id )
{
	FB_ASSERT( impl );
	return impl->LoadButton( button_name, window, id );
}

//=============================================================================

OguiFormattedText* OguiLocaleWrapper::LoadFormattedText( const std::string& name, OguiWindow* window, int id )
{
	FB_ASSERT( impl );
	return impl->LoadFormattedText( name, window, id );
}

//=============================================================================

OguiSelectList*	OguiLocaleWrapper::LoadSelectList( const std::string& name, OguiWindow* window, int id )
{
	FB_ASSERT( impl );
	return impl->LoadSelectList( name, window, id );
}

//=============================================================================

OguiSlider*	OguiLocaleWrapper::LoadSlider( const std::string& name, OguiWindow* window, int id )
{
	FB_ASSERT( impl );
	return impl->LoadSlider( name, window, id );
}

//=============================================================================

IOguiFont*	OguiLocaleWrapper::LoadFont( const std::string& name )
{
	FB_ASSERT( impl );
	return impl->LoadFont( name );
}

///////////////////////////////////////////////////////////////////////////////

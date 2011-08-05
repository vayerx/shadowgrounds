
#include "precompiled.h"

#include "OguiFormattedCommandImg.h"
#include "OguiFormattedText.h"
#include "Ogui.h"

#include <assert.h>
#include <string>



///////////////////////////////////////////////////////////////////////////////

OguiFormattedCommandImg::OguiFormattedCommandImg()
{
}

//.............................................................................

OguiFormattedCommandImg::~OguiFormattedCommandImg()
{
}

///////////////////////////////////////////////////////////////////////////////

void OguiFormattedCommandImg::execute( OguiFormattedText* text, const std::string& parameters, OguiFormattedText::ParseData* data )
{
	parseParameters( parameters );

	std::string src =	castParameter( "src", std::string( "" ) );
	int width =			castParameter( "width", 0 );
	int height =		castParameter( "height", 0 );
	std::string align = castParameter( "align", std::string( "center" ) );
	bool nowarp	=		castParameter( "nowarp", false );
	std::string type =	castParameter( "type", std::string( "image" ) );
	
	if( width <= 0 || height <= 0 || src.empty() )
		return;

	if( data->mx != data->x )
	{
		data->mx = data->x;
		data->my += data->currentFont->getHeight();
	}

	{
		int x = 0;
		int y = data->my;
		int w = width;
		int h = height;

		if( align == "center" )
		{
#ifdef PROJECT_SHADOWGROUNDS
			x = ( data->w - width ) / 2;
#else
			x = data->x + ( data->w - width ) / 2;
#endif
			
			data->mx = x + w;
			data->my = y + h;

		}
		else if( align == "left" )
		{
			x = data->x;
			
			if( nowarp )
			{
				data->mx = data->x;
				data->my = y + h;
			}
			else
			{
				data->after_y = height;
				data->after_y_x_value = data->x;
				data->after_y_w_value = data->w;

				data->w = data->w - width;
				data->mx = data->x + w;
				data->x = data->x + w;
			}
			
		}
		else if( align == "right" )
		{
#ifdef PROJECT_SHADOWGROUNDS
			x = data->w - width;
#else
			x = data->x + data->w - width;
#endif
			if( nowarp )
			{
				data->mx = data->x;
				data->my = y + h;
			}
			else
			{
				data->after_y = height;
				data->after_y_x_value = data->x;
				data->after_y_w_value = data->w;
				
				data->w = data->w - width;
				data->mx = data->x;
				data->x = data->x;
			}
		}

		if( type != "video" )
		{

			text->createImageButton( x, y, w, h, src );
		}
		else
		{
			text->createImageButton( x, y, w, h, text->getOgui()->LoadOguiVideo( src.c_str(), 0 ) );
		}

	}

}

///////////////////////////////////////////////////////////////////////////////


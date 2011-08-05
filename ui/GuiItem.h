#ifndef INC_GUIITEM_H
#define INC_GUIITEM_H

#include <string>
#include "../game/DHLocaleManager.h"

class OguiButton;

namespace ui {
///////////////////////////////////////////////////////////////////////////////

// POD class for the use of ItemWindow
//
// Represents a single item that player has on the screen
// can be generated from a given locale string...
class GuiItem
{
public:
	GuiItem() : 
		imageFile( "" ), 
		w( 0 ), 
		h( 0 ), 
		image( NULL ) 
	{ }
	
	GuiItem( const GuiItem& item ) : 
		imageFile( item.imageFile ), 
		w( item.w ), 
		h( item.h ), 
		image( item.image ) 
	{ }

	GuiItem( const std::string& locale_name ) : 
		imageFile( "" ),
		w( 0 ),
		h( 0 ),
		image( NULL )
	{
		imageFile = game::getLocaleGuiString( locale_name.c_str() );
		w = game::getLocaleGuiInt( ( locale_name + "_w" ).c_str(), 0 );
		h = game::getLocaleGuiInt( ( locale_name + "_h" ).c_str(), 0 );
	}

	~GuiItem()  { 	}


	void operator=( const GuiItem& item ) 
	{ 
		imageFile = item.imageFile; 
		w = item.w; 
		h = item.h; 
		image = item.image;
	}

	bool operator==( const GuiItem& item )
	{
		return( imageFile == item.imageFile && w == item.w && h == item.h );
	}

	bool operator!=( const GuiItem& item )
	{
		return !operator==( item );
	}

	std::string imageFile;
	int w;
	int h;
	OguiButton* image;

};
///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

#endif

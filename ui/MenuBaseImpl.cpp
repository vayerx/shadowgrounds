
#include "precompiled.h"

#include "MenuBaseImpl.h"
#include <assert.h>
#include <sstream>
#include "../ogui/Ogui.h"
#include "../game/DHLocaleManager.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "GameController.h"
#include "../util/assert.h"

using namespace game;

namespace ui {

const static OguiButton::TEXT_H_ALIGN	horizontalAlign = OguiButton::TEXT_H_ALIGN_CENTER;
const static OguiButton::TEXT_V_ALIGN	verticalAlign = OguiButton::TEXT_V_ALIGN_MIDDLE;



MenuBaseImpl::MenuBaseImpl( OguiWindow* window ) :
  win( window ),
  ogui( NULL ),

  buttonX( 0 ),
  buttonW( 0 ),
  buttonH( 0 ),
  buttonY( 0 ),
  buttonAddX( 0 ),
  buttonAddY( 0 ),
  separatorH( 0 ),
  separatorW( 0 ),

  buttonNormal(),
  buttonHigh(),
  buttonDown(),
  //buttonPaddingString( "|" ),
  buttonPaddingString( " " ),

  smallButtonNormal(),
  smallButtonHigh(),
  smallButtonDown(),
	smallButtonDisabled(),
	smallButtonDisabledImage( NULL ),

  smallButtonX( 0 ),
  smallButtonY( 0 ),
  smallButtonStartAddX( 0 ),
  smallButtonStartAddY( 0 ),
  smallButtonW( 1 ),
  smallButtonH( 1 ),
  smallButtonAddX( 0 ),
  smallButtonAddY( 0 ),

  closeMeButtonX( 0 ),
  closeMeButtonY( 0 ),
  closeMeButtonW( 0 ),
  closeMeButtonH( 0 ),

  closeMeButtonAddX( 0 ),
  closeMeButtonAddY( 0 ),

  closeMeButtonNormal(),
  closeMeButtonHigh(),
  closeMeButtonDown(),

  buttons(),
  selectButtons(),
  numberOfWorkingSelectButtons( 0 ),
  activeSelection( -1 ),

  imageSelectNorm( NULL ),
  imageSelectDown( NULL ),
  
  fontSelectNorm( NULL ),
  fontSelectDown( NULL ),
  fontDescNorm( NULL ),
  fontDescDown( NULL ),

  buttonFontSelectNormal(),
  buttonFontSelectDown(),
  buttonFontDescNormal(),
  buttonFontDescDown(),

  headerText( NULL ),
  headerTextX( 0 ),
  headerTextY( 0 ),
  headerTextW( 1 ),
  headerTextH( 1 ),

  editBuffer(),
  editHandle( 0 ),
  editButtonP( NULL ),
  editCursorDrawn( false ),
  editCursorDrawnTime( 0 ),
  editCursorBlinkTime( 500 ),

  soundClick(),
  soundMouseover(),
  soundDisabled(),

  game( NULL ),
  closeMenuByEsc( true ),
  canWeCloseTheMenuNow( true )

{
	buttonW		= getLocaleGuiInt( "gui_menu_common_button_w", 0 );
	buttonH		= getLocaleGuiInt( "gui_menu_common_button_h", 0 );
	
	buttonAddX	= getLocaleGuiInt( "gui_menu_common_button_add_x", 0 );
	buttonAddY	= getLocaleGuiInt( "gui_menu_common_button_add_y", 28 );
	separatorH		= getLocaleGuiInt( "gui_menu_common_separator_h", 35 );
	separatorW		= getLocaleGuiInt( "gui_menu_common_separator_w", 0 );
	
	buttonNormal	= getLocaleGuiString( "gui_menu_common_button_img_normal" );
	buttonHigh		= getLocaleGuiString( "gui_menu_common_button_img_high" );
	buttonDown		= getLocaleGuiString( "gui_menu_common_button_img_down" );

	smallButtonNormal = getLocaleGuiString( "gui_menu_common_smallbutton_img_normal" );
	smallButtonHigh   = getLocaleGuiString( "gui_menu_common_smallbutton_img_high" );
	smallButtonDown	  = getLocaleGuiString( "gui_menu_common_smallbutton_img_down" );
	smallButtonDisabled = getLocaleGuiString( "gui_menu_common_smallbutton_img_disabled" );

	smallButtonStartAddX	= getLocaleGuiInt( "gui_menu_common_smallbutton_startadd_x", 0 );
	smallButtonStartAddY	= getLocaleGuiInt( "gui_menu_common_smallbutton_startadd_y", 0 );
	smallButtonW			= getLocaleGuiInt( "gui_menu_common_smallbutton_w", 0 );
	smallButtonH			= getLocaleGuiInt( "gui_menu_common_smallbutton_h", 0 );
	smallButtonAddX			= getLocaleGuiInt( "gui_menu_common_smallbutton_add_x", 0 );
	smallButtonAddY			= getLocaleGuiInt( "gui_menu_common_smallbutton_add_y", 0 );
	
	closeMeButtonX = getLocaleGuiInt( "gui_menu_common_closebutton_x", 0 );
	closeMeButtonY = getLocaleGuiInt( "gui_menu_common_closebutton_y", 0 );
	closeMeButtonW = getLocaleGuiInt( "gui_menu_common_closebutton_w", 0 );
	closeMeButtonH = getLocaleGuiInt( "gui_menu_common_closebutton_h", 0 );

	closeMeButtonAddX	= getLocaleGuiInt( "gui_menu_common_closebutton_add_x", 0 );
	closeMeButtonAddY	= getLocaleGuiInt( "gui_menu_common_closebutton_add_y", 0 );

	closeMeButtonNormal	= getLocaleGuiString( "gui_menu_common_closebutton_img_normal" );
	closeMeButtonHigh	= getLocaleGuiString( "gui_menu_common_closebutton_img_high" );
	closeMeButtonDown	= getLocaleGuiString( "gui_menu_common_closebutton_img_down" );

	buttonFontSelectNormal	= getLocaleGuiString( "gui_menu_common_button_font_normal" );
	buttonFontSelectDown	= getLocaleGuiString( "gui_menu_common_button_font_down" );
	buttonFontDescNormal	= getLocaleGuiString( "gui_menu_common_desc_font_normal" );
	buttonFontDescDown		= getLocaleGuiString( "gui_menu_common_desc_font_down" );

	headerTextX		= getLocaleGuiInt( "gui_menu_common_header_x", 0 );
	headerTextY		= getLocaleGuiInt( "gui_menu_common_header_y", 0 );
	headerTextW		= getLocaleGuiInt( "gui_menu_common_header_w", 0 );
	headerTextH		= getLocaleGuiInt( "gui_menu_common_header_h", 0 );

	soundClick		= getLocaleGuiString( "gui_menu_sound_click" );
	soundMouseover  = getLocaleGuiString( "gui_menu_sound_mouseover" );
	soundDisabled	= getLocaleGuiString( "gui_menu_sound_disabled" );

}

MenuBaseImpl::~MenuBaseImpl()
{
	delete smallButtonDisabledImage;
	smallButtonDisabledImage = NULL;

/*	assert( game  );
	assert( game->gameUI ); 
	assert( game->gameUI->getController(0) );
	// if( editButtonP )
	if( closeMenuByEsc )
		game->gameUI->getController(0)->removeKeyreader( editHandle ); */
}

void MenuBaseImpl::debugKeyreader( int keyreader, bool release, std::string who )
{
	std::stringstream ss;
	ss << who << " ";
	if( release ) ss << "released keyreader " << keyreader;
	else ss << "created keyreader " << keyreader;

	Logger::getInstance()->debug( ss.str().c_str() );
}
//.............................................................................

void MenuBaseImpl::hide()
{
	assert( win );
	win->Hide();
}

void MenuBaseImpl::show()
{
	assert( win );
	win->Show();
}

void MenuBaseImpl::raise()
{
	assert( win );
	win->Raise();
}

bool MenuBaseImpl::isVisible() const
{
	assert( win );
	return win->IsVisible();
}

bool MenuBaseImpl::wasQuitPressed() const
{
	return false;
} 

//.............................................................................

void MenuBaseImpl::selectButton( int i )
{
	if ( !selectButtons.empty() && i >= 0 && i <= numberOfWorkingSelectButtons )
	{
		if ( selectButtons.find( i ) != selectButtons.end() )
		{
			std::map< int, OguiButton* >::iterator it;

			if ( activeSelection != -1 )
			{
				downlightSelectButton( activeSelection );
			}

			it = selectButtons.find( i );

			if ( it != selectButtons.end() )
			{
				highlightSelectButton( i );
				activeSelection = i;
			}
		}
	} else if ( i == -1 ) {
		if ( activeSelection != -1 )
		{
			downlightSelectButton( activeSelection );
		}
	} 
}

//=============================================================================

void MenuBaseImpl::highlightSelectButton( int i )
{
	if ( !selectButtons.empty() && i >= 0 && i <= numberOfWorkingSelectButtons )
	{
		std::map< int, OguiButton* >::iterator it = selectButtons.find( i );
		if ( it	!= selectButtons.end() )
		{
			// FB_ASSERT( imageSelectDown );
			FB_ASSERT( it->second );

			if( imageSelectDown )
				it->second->SetImage( imageSelectDown );
			
			SelectionButtonDescs* descs = NULL;

			if( it->second->GetArgument() != NULL )
			{
				descs = (SelectionButtonDescs*)it->second->GetArgument();
			}

			if( fontSelectDown )
				it->second->SetFont( fontSelectDown );

			if( descs && descs->first && fontDescDown )
				descs->first->SetFont( fontDescDown );

			if( descs && descs->second && fontDescDown )
				descs->second->SetFont( fontDescDown );
		}
	}
}

//=============================================================================

void MenuBaseImpl::downlightSelectButton( int i )
{
	if ( !selectButtons.empty() && i >= 0 && i <= numberOfWorkingSelectButtons )
	{
		std::map< int, OguiButton* >::iterator it = selectButtons.find( i );
		if ( it	!= selectButtons.end() )
		{
			// FB_ASSERT( imageSelectNorm );
			FB_ASSERT( it->second );

			if( imageSelectNorm )
				it->second->SetImage( imageSelectNorm );	
		
			SelectionButtonDescs* descs = NULL;

			if( it->second->GetArgument() != NULL )
			{
				descs = (SelectionButtonDescs*)it->second->GetArgument();
			}

			if( fontSelectNorm )
				it->second->SetFont( fontSelectNorm );

			if( descs && descs->first && fontDescNorm )
				descs->first->SetFont( fontDescNorm );

			if( descs && descs->second && fontDescNorm )
				descs->second->SetFont( fontDescNorm );

		}
	}

}

//=============================================================================

void MenuBaseImpl::CursorEvent( OguiButtonEvent* eve )
{
	if( eve->eventType == OGUI_EMASK_CLICK &&
		!(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_UP_MASK) &&
		!(eve->cursorOldButtonMask & OGUI_BUTTON_WHEEL_DOWN_MASK))
	{
		selectButton( eve->triggerButton->GetId() );

		int i = eve->triggerButton->GetId();
		std::map< int, OguiButton* >::iterator it;
		it = selectButtons.find( i );
		
		if ( i > numberOfWorkingSelectButtons && i < (int)selectButtons.size() && it != selectButtons.end() )
		{
			if( game )
				game->gameUI->playGUISound( soundDisabled.c_str() );
		} else {
			if( game )
				game->gameUI->playGUISound( soundClick.c_str() );
		}

	} 
	else if( eve->eventType == OGUI_EMASK_OVER )
	{
		if( game )
			game->gameUI->playGUISound(	soundMouseover.c_str() );

		highlightSelectButton( eve->triggerButton->GetId() );
	}
	else if( eve->eventType == OGUI_EMASK_LEAVE )
	{
		if( activeSelection != eve->triggerButton->GetId() )
			downlightSelectButton( eve->triggerButton->GetId() );
	}
}

void MenuBaseImpl::escPressed()
{
	if( !closeMenuByEsc )
	{
		handleEsc();
	}
}

//.............................................................................

void MenuBaseImpl::update()
{
	if( editButtonP )
	{
		if( Timer::getTime() - editCursorDrawnTime > editCursorBlinkTime )
		{
			editCursorDrawnTime = Timer::getTime();
			editCursorDrawn = !editCursorDrawn;

			if( editCursorDrawn ) editBufferAfter = "_";
			else editBufferAfter = "";
			
			editButtonP->SetText( ( editBufferBefore + editBuffer + editBufferAfter ).c_str() );
		}
	}
}

//.............................................................................


void MenuBaseImpl::readKey( char ascii, int keycode, const char *keycodeName )
{
	

	if( editButtonP )
	{

		switch( keycode )
		{
		case 1: // esc
			editButtonEnter("");
			break;

		case 14: // backspace
			if( !editBuffer.empty() )
				editBuffer.erase( editBuffer.size() - 1 );
			break;

		case 28: // enter
			editButtonEnter( editBuffer );
			break;

		default:
			if ( ascii != '\0' )
				editBuffer += ascii;
			break;
		}

		if( editButtonP )
			editButtonP->SetText( ( editBufferBefore + editBuffer + editBufferAfter ).c_str() );
	} 
	else if( closeMenuByEsc )
	{
		switch( keycode )
		{
		case 1:
			handleEsc();
			break;

		default:
			break;
		}
	}

	
}

void MenuBaseImpl::handleEsc()
{
	if( canWeCloseTheMenuNow && editButtonP == NULL )
		closeMenu();
}

void MenuBaseImpl::editButton( OguiButton* button, const std::string& defaultstring, const std::string& before )
{
	if( editButtonP ) 
		editButtonEnter( editBuffer );

	editBufferBefore = before;

	editButtonP = button;
	editBuffer = defaultstring;
	// editBuffer.clear();

	editButtonP->SetText( editBufferBefore.c_str() );

	// editHandle = game->gameUI->getController(0)->addKeyreader( this );

}

void MenuBaseImpl::editButtonEnter( const std::string& text )
{
	if( editButtonP )
	{
		editButtonP->SetText( ( editBufferBefore + text ).c_str() );
		//game->gameUI->getController(0)->removeKeyreader( editHandle );
		editButtonP = NULL;
	}
}

//.............................................................................

OguiButton* MenuBaseImpl::addButton( const std::string& text, int command, IOguiFont* font, IOguiFont* high, IOguiFont* down, IOguiFont* disa, OguiButton::TEXT_H_ALIGN halign )
{
	assert( ogui );
	assert( win );

	OguiButton* b;
	b = ogui->CreateSimpleTextButton( win, buttonX, buttonY, buttonW, buttonH, 
		buttonNormal.c_str(), buttonDown.c_str(), buttonHigh.c_str(), 
		( buttonPaddingString + text ).c_str(), command );
	b->SetListener( this );
	if ( font ) b->SetFont( font );
	if ( high ) b->SetHighlightedFont( high );
	if ( down ) b->SetDownFont( down );
	if ( disa ) b->SetDisabledFont( disa );
	b->SetTextHAlign( halign );
	b->SetTextVAlign( verticalAlign );

	b->SetEventMask( OGUI_EMASK_CLICK |  OGUI_EMASK_OVER );

	buttonX += buttonAddX;
	buttonY += buttonAddY;

	buttons.push_back( b );

	return b;
}

OguiButton* MenuBaseImpl::addDescription( const std::string& text, int x_add, int y_add, IOguiFont* font )
{
	int x = buttonX + x_add;
	int y = buttonY + y_add;

	assert( ogui );
	assert( win );

	OguiButton* b = ogui->CreateSimpleTextButton( win, x, y, buttonDescriptionW, buttonDescriptionH,
		NULL, NULL, NULL, text.c_str(), 3 );
	b->SetListener( this );
	b->SetReactMask( 0 );
	b->SetDisabled( true );
	if( font ) 
		b->SetFont( font );
	b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );

	buttons.push_back( b );

	return b;
}


OguiButton*	MenuBaseImpl::addSmallButton( const std::string& text, int command, IOguiFont* font, IOguiFont* high, IOguiFont* down, IOguiFont* disa  )
{
	assert( ogui );
	assert( win );


	if( smallButtonX < buttonX || smallButtonY < buttonY )
	{
		smallButtonX = buttonX + smallButtonStartAddX;
		smallButtonY = buttonY + smallButtonStartAddY;
	}

	OguiButton* b;
	b = ogui->CreateSimpleTextButton( win, smallButtonX, smallButtonY, smallButtonW, smallButtonH, 
		smallButtonNormal.c_str(), smallButtonDown.c_str(), smallButtonHigh.c_str(), 
		text.c_str(), command );
	b->SetListener( this );

	if( font ) b->SetFont( font );
	if( high ) b->SetHighlightedFont( high );
	if( down ) b->SetDownFont( down );
	if( disa ) b->SetDisabledFont( disa );

	b->SetEventMask( OGUI_EMASK_CLICK |  OGUI_EMASK_OVER );
	b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	
	if( smallButtonDisabledImage == NULL && !smallButtonDisabled.empty() )
	{
		smallButtonDisabledImage = ogui->LoadOguiImage( smallButtonDisabled.c_str() );
	}
	
	if( smallButtonDisabledImage )
	  b->SetDisabledImage( smallButtonDisabledImage );

	smallButtonX += smallButtonAddX;
	smallButtonY += smallButtonAddY;

	buttons.push_back( b );
	return b;
}

void MenuBaseImpl::addCloseButton( const std::string& text, int command, IOguiFont* font )
{
	assert( ogui );
	assert( win );

	OguiButton* b;
	b = ogui->CreateSimpleTextButton( win, closeMeButtonX, closeMeButtonY, 
		closeMeButtonW, closeMeButtonH, closeMeButtonNormal.c_str(), closeMeButtonDown.c_str(),
		closeMeButtonHigh.c_str(), text.c_str(), command );

	b->SetListener( this );


	b->SetEventMask( OGUI_EMASK_CLICK |  OGUI_EMASK_OVER );
	
	
	closeMeButtonX += closeMeButtonAddX;
	closeMeButtonY += closeMeButtonAddY;

	buttons.push_back( b );
}

void MenuBaseImpl::addSelectionButton( const std::string& text, int command, IOguiFont* font, void* param )
{
	assert( ogui );
	assert( win );
	assert( command >= 0 );

	if( command > numberOfWorkingSelectButtons ) numberOfWorkingSelectButtons = command;

	OguiButton* b;
	b = ogui->CreateSimpleTextButton( win, buttonX, buttonY, buttonW, buttonH, 
		buttonNormal.c_str(), buttonDown.c_str(), buttonHigh.c_str(), 
		( buttonPaddingString + text ).c_str(), command, param );
	b->SetListener( this );
	if ( font ) b->SetFont( font );
	b->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	b->SetEventMask( OGUI_EMASK_CLICK | OGUI_EMASK_OVER | OGUI_EMASK_LEAVE );

	// std::stringstream ss;
	// ss << buttonAddY << std::endl;
	// Logger::getInstance()->debug( ss.str().c_str() );
	buttonX += buttonAddX;
	buttonY += buttonAddY;

	selectButtons.insert( std::pair< int, OguiButton* >( command, b ) );
}

OguiButton* MenuBaseImpl::addImageButtton( const std::string& image_norm, const std::string& image_down, const std::string& image_high, const std::string& image_disa, int command, int x, int y, int w, int h )
{
	assert( ogui );
	assert( win );

	OguiButton* b;
	b = ogui->CreateSimpleImageButton( win, x, y, w, h, image_norm.c_str(), image_down.c_str(), image_high.c_str(), image_disa.c_str(), command );
	b->SetListener( this );

	b->SetEventMask( OGUI_EMASK_CLICK |  OGUI_EMASK_OVER );

	buttons.push_back( b );

	return b;
}

void MenuBaseImpl::addHeaderText( const std::string& text, IOguiFont* font )
{
	assert( ogui );
	assert( win );
	
	headerText = ogui->CreateTextLabel( win, headerTextX, headerTextY, headerTextW, headerTextH, text.c_str() );
	headerText->SetTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
	
	if ( font ) 
		headerText->SetFont( font );

}


void MenuBaseImpl::addSeparator()
{
	buttonX += separatorW;
	buttonY += separatorH;
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui

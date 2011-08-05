
#include "precompiled.h"

#include "CreditsMenu.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiFormattedText.h"
#include "../game/DHLocaleManager.h"
#include "../game/SimpleOptions.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../ui/GameController.h"

#include "../util/Debug_MemoryManager.h"

using namespace game;

namespace ui
{

#ifdef PROJECT_SHADOWGROUNDS

#if defined CRYPT_GERMANY_DEMO || CRYPT_GERMANY

		/*
    "<h1>Frozenbyte development team</h1>\n"
        "\n"
        " \n"
		"Project management\n"
		"<b>Lauri Hyvärinen</b>\n"
		" \n" 
		" \n"
		"Game design\n"
		"<b>Frozenbyte team</b>\n"
		" \n"
		" \n"
		"Story design\n"
		"<b>Joel Kinnunen</b>\n"
		"<b>Ari Pulkkinen</b>\n"
		" \n"
		" \n"
		"Senior programmers\n"
		"<b>Juha Hiekkamäki</b>\n"
		"<b>Jukka Kokkonen</b>\n"
		"<b>Ilkka Kuusela</b>\n"
		" \n"
		" \n"
		"Programmer\n"
		"<b>Petri Purho</b>\n"
		" \n"
		" \n"
		"Lead Artist\n"
		"<b>Timo Maaranen</b>\n"
		"\n"
		"\n"
		"Senior Artists\n"
		"<b>Arttu Autio</b>\n"
		"<b>Vesa Lepola</b>\n"
		"<b>Tero Rickström</b>\n"
		"<b>Samuli Snellman</b>\n"
		" \n"
		" \n"
		"Artists\n"
		"<b>Santtu Huotilainen</b>\n"
		"<b>Matti Hämäläinen</b>\n"
		"<b>Kristian Vuorinen</b>\n"
		" \n"
		" \n"
		"Sound and music development\n"
		"<b>Ari Pulkkinen</b>\n"
		" \n"
		" \n"
		" \n"
		" \n"
		" \n"
		"<h1>Frozenbyte management team</h1>\n"
		"\n"
		"<b>Lauri Hyvärinen</b>\n"
		"<b>Joel Kinnunen</b>\n"
		"<b>Petter Kinnunen</b>\n"
		" \n"
		" \n"
		" \n"
		" \n"
		" \n"
		"<h1>DTP ENTERTAINMENT AG:</h1>\n"
		"\n"
		"Product Manager:\n"
		"<b>Egbert Latza</b>\n"
		"\n"
		"\n"
		"Marketing & PR:\n"
		"<b>Carsten Fichtelmann</b>\n"
		"<b>Christopher Kellner</b>\n"
		"<b>Marina Selikowitsch</b>\n"
		"\n"
		"\n"
		"Box Artwork:\n"
		"<b>Randel KG</b>\n"
		"\n"
		"\n"
		"Testing:\n"
		"<b>Timo Gerken</b>\n"
		"<b>Mathias Reichert</b>\n"
		"\n"
		"\n"
		"Translation & Voice recordings:\n"
		"<b>Studio Ulrich Mühl</b>\n"
		"\n"
		"\n"
		"Localisation Manager :\n"
		"<b>Egbert Latza</b>\n"
		"\n"
		"\n"
		"German voices:\n"
		"<b>Marion v. Stengel</b>\n"
		"<b>Erik Schäffler</b>\n"
		"<b>Rüdiger Schulzki</b>\n"
		"<b>Eberhard Haar</b>\n"
		"<b>Gerhart Hinze</b>\n"
		"<b>Christos Topoulos</b>\n"
		"<b>Guido Zimmermann</b>\n"
		"<b>Frank Felicetti</b>\n"
*/


std::string G_UiCredits =

"\n\n\n\n\n\n\n\nGame Design\n\n<b>Frozenbyte team</b>\n"

"\n\nProject lead\n\n<b>Lauri Hyvärinen</b>\n"

"\n\nBusiness & development manager\n\n<b>Joel Kinnunen</b>\n"

"\n\nProgramming\n\n<b>Juha Hiekkamäki</b>\n<b>Jukka Kokkonen</b>\n<b>Ilkka Kuusela</b>\n<b>Petri Purho</b>\n"

"\n\nArt\n\n<b>Timo Maaranen</b>\n<b>Vesa Lepola</b>\n<b>Tero Rickström</b>\n<b>Samuli Snellman</b>\n"

"<b>Santtu Huotilainen</b>\n<b>Matti Hämäläinen</b>\n<b>Kristian Vuorinen</b>\n"

"\n\nMusic & Audio\n\n<b>Ari Pulkkinen</b>\n"

"\n\nMarketing director\n\n<b>Petter Kinnunen</b>\n"

"\n\n\n\n"

"<b>Additional Frozenbyte credits</b>\n"
"\n\n"
"Story & Script\n\n<b>Joel Kinnunen</b>\n<b>Ari Pulkkinen</b>\n<b>Ilkka Kuusela</b>\n"
"\n\n"
"Additional Contributors\n"
"<b>Niklas Collin</b>\n"
"<b>Risto Hyvärinen</b>\n"
"<b>Juha Ikonen</b>\n"
"<b>Jussi Järvinen</b>\n"
"<b>Anssi Penttilä</b>\n"
"\n\n"
"The angry guitar wizard\n"
"<b>Jussi \"Amen\" Sydänmaa</b>\n"
"\n\n"
"Music co-mixing\n"
"<b>Antti \"Wilhelm\" Paajanen</b>\n"

"\n\n\n\n\n"

"<b>DTP ENTERTAINMENT AG</b>\n"
"\n\n"

"Product Manager\n"
"<b>Egbert Latza</b>\n"

"\n\n"
"Marketing & PR\n"
"<b>Carsten Fichtelmann</b>\n"
"<b>Christopher Kellner</b>\n"
"<b>Marina Selikowitsch</b>\n"

"\n\n"
"Box Artwork\n"
"<b>Randel KG</b>\n"

"\n\n"
"Testing\n"
"<b>Timo Gerken</b>\n"
"<b>Mathias Reichert</b>\n"

"\n\n"
"Translation & Voice recordings\n"
"<b>Studio Ulrich Mühl</b>\n"

"\n\n"
"Localization Manager\n"
"<b>Egbert Latza</b>\n"

"\n\n"
"German voices\n"
"<b>Marion v. Stengel</b>\n"
"<b>Erik Schäffler</b>\n"
"<b>Rüdiger Schulzki</b>\n"
"<b>Eberhard Haar</b>\n"
"<b>Gerhart Hinze</b>\n"
"<b>Christos Topoulos</b>\n"
"<b>Guido Zimmermann</b>\n"
"<b>Frank Felicetti</b>\n"



"\n\n"
"\n\n\n"
"Frozenbyte thanks\n"
"\n"
"<b>Sebastian Aaltonen</b>\n"
"<b>Tero Antinkaapo</b>\n"
"<b>Kai-Peter Bäckman</b>\n"
"<b>Petteri Henttu</b>\n"
"<b>Maritta Hyvärinen</b>\n"
"<b>Jarno Kantelinen</b>\n"
"<b>Mikko Lehtonen</b>\n"
"<b>Tatu Petersen-Jessen</b>\n"
"<b>Esko Piirainen</b>\n"
"<b>Santeri Pilli</b>\n"
"\n\n"
"<b>AudioGodz Inc / Lani Minella</b>\n"
"<b>Lordi</b>\n"
"<b>Tekes / Keith Bonnici</b>\n"
"<b>TE-keskus / Veli-Matti Virrankari</b>\n"
"<b>NBC Staff</b>\n"
"<b>Neogames</b>\n"
"\n\n"
"<b>Adage</b>\n"
"<b>Bugbear</b>\n"
"<b>Housemarque</b>\n"
"<b>Remedy</b>\n"
"\n\n"
"\n\n"
"<b>Thanks to all our families and friends!</b>\n"
"\n\n"
"\n\n"
"\n\n"
"Shadowgrounds, Copyright (c) 2005 Frozenbyte, Inc.\n"
"\n\n"
"FMOD Sound System, copyright (c) Firelight Technologies Pty, Ltd., 1994-2005.\n"
"Portions utilize Microsoft Windows Media Technologies. Copyright (c) 1999-2002 Microsoft Corporation. All Rights Reserved.\n"
"\n\n\n"
		;

#else


std::string G_UiCredits = 


"\n\n\n\n\n\n\n\nGame Design\n\n<b>Frozenbyte team</b>\n"

"\n\nProject lead\n\n<b>Lauri Hyvärinen</b>\n"

"\n\nBusiness & development manager\n\n<b>Joel Kinnunen</b>\n"

"\n\nProgramming\n\n<b>Juha Hiekkamäki</b>\n<b>Jukka Kokkonen</b>\n<b>Ilkka Kuusela</b>\n<b>Petri Purho</b>\n"

"\n\nArt\n\n<b>Timo Maaranen</b>\n<b>Vesa Lepola</b>\n<b>Tero Rickström</b>\n<b>Samuli Snellman</b>\n"

"<b>Santtu Huotilainen</b>\n<b>Matti Hämäläinen</b>\n<b>Kristian Vuorinen</b>\n"

"\n\nMusic & Audio\n\n<b>Ari Pulkkinen</b>\n"

"\n\nMarketing director\n\n<b>Petter Kinnunen</b>\n"

"\n\n"

"<b>Additional Frozenbyte credits</b>\n"
"\n\n"
"Story & Script\n\n<b>Joel Kinnunen</b>\n<b>Ari Pulkkinen</b>\n<b>Ilkka Kuusela</b>\n"
"\n\n"
"Additional Contributors\n"
"<b>Niklas Collin</b>\n"
"<b>Risto Hyvärinen</b>\n"
"<b>Juha Ikonen</b>\n"
"<b>Jussi Järvinen</b>\n"
"<b>Anssi Penttilä</b>\n"
"\n\n"
"\n\n"
"Voice-over management\n"
"<b>Jarno \"Stakula\" Sarkula / Stakula Oy	</b>\n"
"\n\n"
"Casting/Directing\n"
"<b>AudioGodz Inc / Lani Minella</b>\n"
"\n\n"
"Actors & actress\n"
"<b>Marc Biagi</b>\n"
"<b>Dan Castle</b>\n"
"<b>Brook Chalmers</b>\n"
"<b>Max McGill</b>\n"
"<b>Lani Minella</b>\n"
"<b>Chris Wilcox</b>\n"
"\n\n"
"The angry guitar wizard\n"
"<b>Jussi \"Amen\" Sydänmaa</b>\n"
"\n\n"
"Music co-mixing\n"
"<b>Antti \"Wilhelm\" Paajanen</b>\n"
"\n\n"
"<b>SHADOWGROUNDS 1.5</b>\n\nby Alternative Games\n"
"\nProgramming\n\n<b>Tuomas Närväinen</b>\n<b>Turo Lamminen</b>\n"
"\n\n"
"Frozenbyte thanks\n"
"\n"
"<b>Sebastian Aaltonen</b>\n"
"<b>Tero Antinkaapo</b>\n"
"<b>Kai-Peter Bäckman</b>\n"
"<b>Petteri Henttu</b>\n"
"<b>Maritta Hyvärinen</b>\n"
"<b>Jarno Kantelinen</b>\n"
"<b>Mikko Lehtonen</b>\n"
"<b>Tatu Petersen-Jessen</b>\n"
"<b>Esko Piirainen</b>\n"
"<b>Santeri Pilli</b>\n"
"\n\n"
"<b>Lordi</b>\n"
"<b>Tekes / Keith Bonnici</b>\n"
"<b>TE-keskus / Veli-Matti Virrankari</b>\n"
"<b>NBC Staff</b>\n"
"\n\n"
"<b>Adage</b>\n"
"<b>Bugbear</b>\n"
"<b>Housemarque</b>\n"
"<b>Remedy</b>\n"
"\n\n"
"\n\n"
"<b>Thanks to all our families and friends!</b>\n"
"\n\n"
"\n\n"
"\n\n"
"Shadowgrounds, Copyright (c) 2005 Frozenbyte, Inc.\n"
"\n\n"
"Shadowgrounds 1.5, Copyright (c) 2011 Alternative Games Ltd.\n"
"\n\n"
"FMOD Sound System, copyright (c) Firelight Technologies Pty, Ltd., 1994-2005.\n"
"Portions utilize Microsoft Windows Media Technologies. Copyright (c) 1999-2002 Microsoft Corporation. All Rights Reserved.\n"
"\n\n"
"PhysX™ technology provided under license from AGEIA Technologies, Inc."
"Copyright (c) 2002, 2003, 2004, 2005, 2006 AGEIA Technologies, Inc., USA. \n"
"All rights reserved. http://www.ageia.com.\n"
"\n\n\n"
		;

#endif

#else

extern const char *ui_credits_text;
std::string G_UiCredits = std::string(ui_credits_text);

#endif

///////////////////////////////////////////////////////////////////////////////

CreditsMenu::CreditsMenu( MenuCollection* menu, MenuCollection::Fonts* fonts, Ogui* o_gui, Game* g ) :
  MenuBaseImpl( NULL ),
  menuCollection( menu ),
  fonts( fonts ),
  textWindow( NULL ),
  theText( NULL ),
  yPosition( 768.0f ),
  lastYPosition( 768 ),
  lastUpdate( 0 ),
  speed( 0.0f )
{
  	assert( o_gui );
	assert( menu );
	assert( fonts );
	game = g;

	ogui = o_gui;

	win = ogui->CreateSimpleWindow( getLocaleGuiInt( "gui_creditsmenu_window_x", 0 ), getLocaleGuiInt( "gui_creditsmenu_window_y", 0 ), 
									getLocaleGuiInt( "gui_creditsmenu_window_w", 1024 ), getLocaleGuiInt( "gui_creditsmenu_window_h", 768 ), NULL );
	
	// win = ogui->CreateSimpleWindow( 0, 0, 1024, 768, NULL );
	win->Hide();
	win->SetUnmovable();


	menu->setBackgroundImage( getLocaleGuiString( "gui_credits_background_image" ) );

	/*addCloseButton( getLocaleGuiString( "gui_om_closeme" ), COMMANDS_CLOSEME, fonts->medium.normal );
	
	textWindow = ogui->CreateSimpleWindow( 0, 768, 1024, 1024, NULL );
	textWindow->SetMoveBoundaryType( OguiWindow::MOVE_BOUND_NO_PART_IN_SCREEN );
	textWindow->SetReactMask( 0 );
	textWindow->Hide();
	*/
	

	const std::string& credits = G_UiCredits;

	theText = new OguiFormattedText( win, ogui, 162, 400, 700, 4300 );
	theText->setTextHAlign( OguiButton::TEXT_H_ALIGN_CENTER );

	credits_fonts.resize(3);
	credits_fonts[0] = ogui->LoadFont(getLocaleGuiString( "gui_creditsmenu_font_normal" ));
	credits_fonts[1] = ogui->LoadFont(getLocaleGuiString( "gui_creditsmenu_font_bold" ));
	credits_fonts[2] = ogui->LoadFont(getLocaleGuiString( "gui_creditsmenu_font_h1" ));
	theText->setFont( credits_fonts[0] );
	theText->registerFont( "b", credits_fonts[1] );
	theText->registerFont( "h1", credits_fonts[2] );
	theText->setLineHeight( (float)atof(getLocaleGuiString("gui_creditsmenu_line_height")) );

	theText->setText( credits );
	
#ifdef LEGACY_FILES
	maskWindow = ogui->CreateSimpleWindow( 0, 0, 1024, 768, "data/GUI/Menus/credits-hack.tga" );
#else
	maskWindow = ogui->CreateSimpleWindow( 0, 0, 1024, 768, "data/gui/menu/credits/window/credits_hack.tga" );
#endif
	maskWindow->SetUnmovable();


	/*
	theText = ogui->CreateTextLabel( textWindow, 0, 0, 1024, 25, credits.c_str() );
	theText->SetFont( fonts->medium );
	theText->SetLinebreaks( true );
	*/


	speed = 0.03f;
	lastUpdate = Timer::getTime();

	update();

	// textWindow->Show();

	///////////////////////////////////////////////////////////////////////////
	assert( game  );
	assert( game->gameUI ); 
	assert( game->gameUI->getController(0) );

	if( game->inCombat )
	{
		closeMenuByEsc = false;
	} 
	else
	{
		
		closeMenuByEsc = true;
		editHandle = game->gameUI->getController(0)->addKeyreader( this );
		// debugKeyreader( editHandle, false, "CreditsMenu::CreditsMenu()" );
	}
		
}

//.............................................................................

CreditsMenu::~CreditsMenu()
{
	for(unsigned int i = 0; i < credits_fonts.size(); i++)
	{
		delete credits_fonts[i];
	}

	if( closeMenuByEsc )
	{
		game->gameUI->getController(0)->removeKeyreader( editHandle );
		// debugKeyreader( editHandle, true, "CreditsMenu::~CreditsMenu()" );
	}

	delete theText;
	delete textWindow;

	while( !buttons.empty() )
	{
		delete *(buttons.begin());
		buttons.pop_front();
	}

	delete win;
	delete maskWindow;
}

///////////////////////////////////////////////////////////////////////////////

int CreditsMenu::getType() const
{
	return MenuCollection::MENU_TYPE_CREDITSMENU;
}

//.........................................................................

void CreditsMenu::closeMenu()
{
	assert( menuCollection );
	menuCollection->closeMenu();
}

//.............................................................................

void CreditsMenu::openMenu( int m )
{
	assert( menuCollection );
	menuCollection->openMenu( m );
}

//.........................................................................

void CreditsMenu::applyChanges()
{
}

//.........................................................................

void CreditsMenu::CursorEvent( OguiButtonEvent* eve )
{
	MenuBaseImpl::CursorEvent( eve );
	
	if( eve->eventType == OGUI_EMASK_CLICK )
	{
		switch( eve->triggerButton->GetId() )
		{
		case COMMANDS_CLOSEME:
			closeMenu();
			break;
		};
	}
	
}

//.............................................................................

void CreditsMenu::update()
{
	int hack = (int)lastYPosition;
	// if( yPosition >= 768 ) textWindow->Raise();
	if( yPosition < -10000 ) { yPosition = 2000; hack = 2000; /*speed += speed;*/ }

	// Timer::getTime();

	yPosition -= ( Timer::getTime() - lastUpdate ) * speed;
	lastUpdate = Timer::getTime();
	
	
	theText->moveBy( 0, (int)yPosition - hack );
	lastYPosition = (int)yPosition;
	// textWindow->MoveTo( 0, 0 ); // (int)yPosition );
	// theText->Move( 0, yPosition );
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace

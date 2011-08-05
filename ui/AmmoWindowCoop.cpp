
#include "precompiled.h"

#include "AmmoWindowCoop.h"
#include "CombatSubWindowFactory.h"

namespace ui {
///////////////////////////////////////////////////////////////////////////////
REGISTER_COMBATSUBWINDOW( AmmoWindowCoop );
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

AmmoWindowCoop::AmmoWindowCoop( Ogui *ogui, game::Game *game, int numOfPlayers  ) :
	AmmoWindow(),
	windows( numOfPlayers )
{
	int i = 0;
	for( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ] = new AmmoWindow( ogui, game, i, true );
	}
}

//=============================================================================

AmmoWindowCoop::~AmmoWindowCoop()
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		delete windows[ i ];
	}
}

///////////////////////////////////////////////////////////////////////////////

void AmmoWindowCoop::hide(int fadeTime)
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->hide( fadeTime );
	}
}

//=============================================================================

void AmmoWindowCoop::show(int fadeTime)
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->show( fadeTime );
	}
}


//=============================================================================

void AmmoWindowCoop::update()
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->update();
	}
}


//=============================================================================

void AmmoWindowCoop::EffectEvent(OguiEffectEvent *e)
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->EffectEvent( e );
	}
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui


#include "precompiled.h"

#include "WeaponWindowCoop.h"
#include "CombatSubWindowFactory.h"

namespace ui {
///////////////////////////////////////////////////////////////////////////////
REGISTER_COMBATSUBWINDOW( WeaponWindowCoop );
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

WeaponWindowCoop::WeaponWindowCoop( Ogui *ogui, game::Game *game, int numOfPlayers  ) :
	WeaponWindow(),
	windows( numOfPlayers )
{
	int i = 0;
	for( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ] = new WeaponWindow( ogui, game, i, true );
	}
}

//=============================================================================

WeaponWindowCoop::~WeaponWindowCoop()
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		delete windows[ i ];
	}
}

///////////////////////////////////////////////////////////////////////////////

void WeaponWindowCoop::hide(int fadeTime)
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->hide( fadeTime );
	}
}

//=============================================================================

void WeaponWindowCoop::show(int fadeTime)
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->show( fadeTime );
	}
}


//=============================================================================

void WeaponWindowCoop::update()
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->update();
	}
}


//=============================================================================

void WeaponWindowCoop::EffectEvent(OguiEffectEvent *e)
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->EffectEvent( e );
	}
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui

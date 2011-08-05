
#include "precompiled.h"

#include "HealthWindowCoop.h"
#include "CombatSubWindowFactory.h"

namespace ui {
///////////////////////////////////////////////////////////////////////////////
namespace {
REGISTER_COMBATSUBWINDOW( HealthWindowCoop );
}
///////////////////////////////////////////////////////////////////////////////

HealthWindowCoop::HealthWindowCoop( Ogui* ogui, game::Game* game, int numOfPlayers ) :
	HealthWindow(),
	windows( numOfPlayers )
{
	int i = 0;
	for( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ] = new HealthWindow( ogui, game, i, true );
	}
}

//=============================================================================

HealthWindowCoop::~HealthWindowCoop()
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		delete windows[ i ];
	}
}

///////////////////////////////////////////////////////////////////////////////
	
void HealthWindowCoop::hide(int fadeTime )
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->hide( fadeTime );
	}
}

//=============================================================================

void HealthWindowCoop::show(int fadeTime)
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->show( fadeTime );
	}
}

//=============================================================================

void HealthWindowCoop::update()
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->update();
	}
}

//=============================================================================

void HealthWindowCoop::updateCurve()
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->updateCurve();
	}
}

//=============================================================================

void HealthWindowCoop::EffectEvent(OguiEffectEvent *e)
{
	int i = 0;
	for ( i = 0; i < (int)windows.size(); i++ )
	{
		windows[ i ]->EffectEvent( e );
	}
}

///////////////////////////////////////////////////////////////////////////////
/*
void HealthWindowCoop::setHealthTextMultiplier( int player_num, float multiplier )
{
	if( player_num < 0 || player_num >= (signed)windows.size() )
	{
		Logger::getInstance()->error( "HealthWindowCoop::setHealthTextMultiplier() - tried to setTextMultiplier to player that doesn't exist" );
		return;
	}
	
	windows[ player_num ]->setHealthTextMultiplier( player_num, multiplier );
}
*/
///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui

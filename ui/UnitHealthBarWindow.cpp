
#include "precompiled.h"

#include "UnitHealthBarWindow.h"

#include "../game/Game.h"
#include "../game/Unit.h"
#include "../ogui/OguiSlider.h"

#include "../game/DHLocaleManager.h"
#include "CombatSubWindowFactory.h"
#include "GenericBarWindow.h"

using namespace game;

namespace ui
{
///////////////////////////////////////////////////////////////////////////////
REGISTER_COMBATSUBWINDOW( UnitHealthBarWindow );
///////////////////////////////////////////////////////////////////////////////

class UnitHealthBarWindowUpdator : public IGenericBarWindowUpdator
{
public:
	UnitHealthBarWindowUpdator( game::Unit* unit ) : unit( unit ) { }

	float update()
	{
		if( unit )
			return (float)((float)unit->getHP() / (float)unit->getMaxHP());
		else
			return 0.0f;
	}

	game::Unit* unit;
};

///////////////////////////////////////////////////////////////////////////////

UnitHealthBarWindow::UnitHealthBarWindow( Ogui* ogui, game::Game* game, int p ) :
	game(game),
	barWindow( new GenericBarWindow( ogui, game, p ) ),
	flashingWindow( new GenericBarWindow(ogui, game, p) ),
	flashingAmount(0.0f),
	flashing(false),
	lastFlash(0),
	unit(NULL)
{
	barWindow->loadDataFromLocales( "unit_health_bar" );
	flashingWindow->loadDataFromLocales( "unit_health_bar_flash" );
	flashingWindow->setValue(0);
	flashingWindow->hide(0);
}

//=============================================================================

UnitHealthBarWindow::~UnitHealthBarWindow()
{
	delete flashingWindow;
	delete barWindow;
}

///////////////////////////////////////////////////////////////////////////////

void UnitHealthBarWindow::hide( int fadeTime )
{
	barWindow->hide( fadeTime );
	flashingWindow->hide( fadeTime );
}

//=============================================================================

void UnitHealthBarWindow::show( int fadeTime )
{
	barWindow->show( fadeTime );
	flashingWindow->show( fadeTime );
}

///////////////////////////////////////////////////////////////////////////////

void UnitHealthBarWindow::update()
{
	barWindow->update();
	flashingWindow->update();
	if(flashing && !barWindow->isHidden() && !game->isPaused())
	{
		int delta_time = Timer::getTime() - lastFlash;

		// has been faded long enough
		if(delta_time > 2 * flashingWindow->getFadeOffTime() + flashingWindow->getFadeOutTime())
		{
			flashingWindow->show(flashingWindow->getFadeInTime());
			lastFlash = Timer::getTime();
		}
		// has been visible long enough
		else if(delta_time > flashingWindow->getFadeOffTime() + flashingWindow->getFadeOutTime())
		{
			flashingWindow->hide(flashingWindow->getFadeOutTime());
		}

		float flashingStart = (float)((float)unit->getHP() / (float)unit->getMaxHP());

		// move
		int x = 0, y = 0, w, h = 0;
		flashingWindow->getWindowRect(x, y, w, h);
		int x2 = 0, y2, w2 = 0, h2;
		barWindow->getWindowRect(x2, y2, w2, h2);
		flashingWindow->move(x2 + (int)(flashingStart * w2 + 0.5f), y);
		flashingWindow->resize((int)((flashingAmount - flashingStart) * w2 + 0.5f), h);

		// reset value
		flashingWindow->setValue(0.0f);
		flashingWindow->setValue(1.0f);
	}
}

//=============================================================================

void UnitHealthBarWindow::setUnit( Unit* unit )
{
	this->unit = unit;
	barWindow->setUpdator( new UnitHealthBarWindowUpdator( unit ) );
}

//=============================================================================

void UnitHealthBarWindow::EffectEvent( OguiEffectEvent *e )
{
	barWindow->EffectEvent( e );
}

void UnitHealthBarWindow::setFlashing(int amount)
{
	if(amount > 0)
	{
		flashing = true;
		flashingAmount = (float)((float)amount / (float)unit->getMaxHP());
	}
	else
	{
		flashing = false;
	}
}

///////////////////////////////////////////////////////////////////////////////
}

#include "precompiled.h"

#include <map>
#include <list>
#ifdef _WIN32
#include <malloc.h>
#endif
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "VehicleWindow.h"

#include "../game/Unit.h"
#include "../game/UnitType.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/UpgradeManager.h"
#include "../game/UpgradeType.h"
#include "../game/PlayerWeaponry.h"
#include "../game/Weapon.h"
#include "../game/scripting/GameScripting.h"
#include "../game/UnitInventory.h"
#include "../game/GameStats.h"

#include "../ogui/Ogui.h"
#include "../ogui/OguiWindow.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiLocaleWrapper.h"
#include "../ogui/OguiSlider.h"

#include "CombatWindow.h"
#include "ICombatSubWindow.h"

#include "../util/fb_assert.h"

#include "../game/DHLocaleManager.h"

using namespace game;

namespace ui {

///////////////////////////////////////////////////////////////////////////////

class VehicleWindow::VehicleWindowImpl
{
private:
	Ogui* ogui;
	game::Game* game;
	game::Unit* unit;

	OguiLocaleWrapper	oguiLoader;

	OguiWindow *window;

	OguiSlider*	healthbarSlider;
	std::string healthbarLow;
	std::string healthbarNormal;
	std::string healthbarBg;

	OguiButton *ammoBackground[2];
	OguiButton *ammoText[2];
	std::string window_name;

public:


	//-------------------------------------------------------------------------
	VehicleWindowImpl( Ogui *ogui, game::Game *game, game::Unit *unit, const char *params ) :
		ogui( ogui ),
		game( game ),
		unit( unit ),
		oguiLoader( ogui )
	{
		window_name = params;

		window = oguiLoader.LoadWindow( (window_name + "window").c_str() );
		healthbarSlider = oguiLoader.LoadSlider( "health_bar", window, 0 );
		healthbarSlider->setDisabled( true );
		healthbarLow = getLocaleGuiString(("gui_" + window_name + "window_health_bar_fore_low").c_str());
		healthbarNormal = getLocaleGuiString(("gui_" + window_name + "window_health_bar_fore_disabled").c_str());
		healthbarBg = getLocaleGuiString(("gui_" + window_name + "window_health_bar_back_disabled").c_str());

		ammoBackground[0] = oguiLoader.LoadButton( "ammo_primary", window, 0 ); ammoBackground[0]->SetDisabled(true);
		ammoBackground[1] = oguiLoader.LoadButton( "ammo_secondary", window, 0 ); ammoBackground[1]->SetDisabled(true);
		ammoText[0] = oguiLoader.LoadButton( "ammo_primary_text", window, 0 ); ammoText[0]->SetDisabled(true);
		ammoText[1] = oguiLoader.LoadButton( "ammo_secondary_text", window, 0 ); ammoText[1]->SetDisabled(true);

		setCombatWindowVisibility();
	}

	~VehicleWindowImpl()
	{
		CombatWindow *combatwin = game->gameUI->getCombatWindow(0);
		if(combatwin)
		{
			combatwin->setSubWindowsVisible(true, true);
		}
		delete healthbarSlider;
		delete ammoBackground[0]; delete ammoBackground[1];
		delete ammoText[0]; delete ammoText[1];
		delete window;
	}


	void update()
	{
		unit = game->gameUI->getFirstPerson(0);
		if(unit == NULL) return;

		int ammoAmount[2] = {0,0};
		int maxAmmo[2] = {0,0};

		int selWeap[2] = { unit->getSelectedWeapon(), unit->getAttachedWeapon(unit->getSelectedWeapon()) };
		for(int i = 0; i < 2; i++)
		{
			if (selWeap[i] != -1 && unit->getWeaponType(selWeap[i]) != NULL)
			{
				ammoAmount[i] = unit->getWeaponAmmoAmount(selWeap[i]);
				maxAmmo[i] = unit->getWeaponMaxAmmoAmount(selWeap[i]);
			}

			if(ammoAmount[i] == 0 && maxAmmo[i] == 0)
			{
				ammoText[i]->SetText( "" );
			}
			else
			{
				ammoText[i]->SetText( int2str(ammoAmount[i]) );
			}
		}


		float hpPercentage = unit->getHP() / (float) unit->getMaxHP();
		if(hpPercentage > 1) hpPercentage = 1;
		if(hpPercentage < 0) hpPercentage = 0;

		if(hpPercentage > 0.3f)
			healthbarSlider->setDisabledImages( healthbarBg, healthbarNormal);
		else
			healthbarSlider->setDisabledImages( healthbarBg, healthbarLow);

		healthbarSlider->setValue( hpPercentage );
	}

	bool setCombatSubWindowVisible(std::string name, bool visible)
	{
		CombatWindow *combatwin = game->gameUI->getCombatWindow(0);
		if(combatwin == NULL)
		{
			return false;
		}

		ICombatSubWindow* win = combatwin->getSubWindow(name);
		if(win)
		{
			if(!visible) win->hide();
			else win->show();
			return true;
		}

		return false;
	}

	void setCombatWindowVisibility()
	{
		CombatWindow *combatwin = game->gameUI->getCombatWindow(0);

		const char *const_combatwindows_array = getLocaleGuiString(("gui_" + window_name + "window_show_combat_windows").c_str());

		if(combatwin)
		{
			bool show_radar = strstr(const_combatwindows_array, "CombatRadar") != NULL;
			combatwin->setSubWindowsVisible(false, show_radar);
		}

		int length = strlen(const_combatwindows_array) + 1;
		char *combatwindows_array = (char *)alloca(length);
		memcpy(combatwindows_array, const_combatwindows_array, length);
		int i = 0;
		int start = 0;
		while(true)
		{
			if(combatwindows_array[i] == 0 || combatwindows_array[i] == ',')
			{
				bool was_end = combatwindows_array[i] == 0;
				combatwindows_array[i] = 0;
				
				const char *subwin = combatwindows_array + start;
				setCombatSubWindowVisible(subwin, true);

				if(was_end) break;
				start = i + 1;
			}
			i++;
		}
	}

	void hide()
	{
		window->Hide();
	}

	void show()
	{
		window->Hide();
	}

};
///////////////////////////////////////////////////////////////////////////////

VehicleWindow::VehicleWindow( Ogui *ogui, game::Game *game, game::Unit *unit, const char *params ) :
	impl( NULL )
{
	impl = new VehicleWindowImpl( ogui, game, unit, params );
}

//.............................................................................

VehicleWindow::~VehicleWindow()
{
	delete impl;
	impl = NULL;
}

void VehicleWindow::update()
{
	impl->update();
}

void VehicleWindow::hide()
{
	impl->hide();
}

void VehicleWindow::show()
{
	impl->show();
}
void VehicleWindow::setCombatWindowVisibility()
{
	impl->setCombatWindowVisibility();
}
///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

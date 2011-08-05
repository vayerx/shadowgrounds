
#include "precompiled.h"

#include "UpgradeAvailableWindow.h"

#include <assert.h>
#include "Visual2D.h"
#include "uidefaults.h"
#include "../game/Unit.h"
#include "../game/UnitInventory.h"
#include "../convert/str2int.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/UpgradeManager.h"
#include "../game/DHLocaleManager.h"
#include "../system/Timer.h"
#include "CombatSubWindowFactory.h"
#include "../ogui/OguiAligner.h"

#include "../util/Debug_MemoryManager.h"


using namespace game;

namespace ui
{
///////////////////////////////////////////////////////////////////////////////
REGISTER_COMBATSUBWINDOW( UpgradeAvailableWindow );
///////////////////////////////////////////////////////////////////////////////

  UpgradeAvailableWindow::UpgradeAvailableWindow(Ogui *ogui, game::Game *game, int player)
  {
    this->ogui = ogui;
    this->game = game;
    this->player = player;

    int xPosition = getLocaleGuiInt("gui_upgrade_available_position_x", 0);
    int yPosition = getLocaleGuiInt("gui_upgrade_available_position_y", 0);

    this->win = ogui->CreateSimpleWindow(xPosition, yPosition, getLocaleGuiInt("gui_upgrade_available_size_x", 0), getLocaleGuiInt("gui_upgrade_available_size_y", 0), NULL);
		this->win->SetUnmovable();

    this->upgradeAvailableImage = ogui->LoadOguiImage(getLocaleGuiString("gui_upgrade_available_image"));

    upgradeAvailableButton = ogui->CreateSimpleImageButton(win, 0, 0, getLocaleGuiInt("gui_upgrade_available_size_x", 0), getLocaleGuiInt("gui_upgrade_available_size_y", 0), NULL, NULL, NULL);
		upgradeAvailableButton->SetDisabled(true);
		upgradeAvailableButton->SetDisabledImage(upgradeAvailableImage);

		lastUpdateValue = -1;
		lastUpdateWasAvailable = false;

		win->SetEffectListener(this);

#ifdef PROJECT_SURVIVOR
		OguiAligner::align(win, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
		OguiAligner::align(upgradeAvailableButton, OguiAligner::WIDESCREEN_FIX_RIGHT, ogui);
#endif
  }


  UpgradeAvailableWindow::~UpgradeAvailableWindow()
  {
    if (upgradeAvailableButton != NULL)
    {
      delete upgradeAvailableButton;
      upgradeAvailableButton = NULL;
    }
    if (win != NULL)
    {
			delete win;
			win = NULL;
		}

		delete upgradeAvailableImage;
  }


	void UpgradeAvailableWindow::hide(int fadeTime)
	{
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
		else
			win->Hide();
	}


	void UpgradeAvailableWindow::show(int fadeTime)
	{
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);

		win->Show();
	}



  void UpgradeAvailableWindow::update()
  {
		if (game->gameUI->getFirstPerson(0) == NULL)
			return;

		int upgAvailable = game::UnitInventory::getUnitItemCount(game, game->gameUI->getFirstPerson(0), "upgradepart");

		if (upgAvailable != lastUpdateValue)
		{			
			bool forcedUpdate = false;
			if (lastUpdateValue == -1)
				forcedUpdate = true;

			lastUpdateValue = upgAvailable;

			bool isAvail = game->upgradeManager->isAvailableUpgrades(game->gameUI->getFirstPerson(0));

			if (isAvail != lastUpdateWasAvailable
				|| forcedUpdate)
			{
				lastUpdateWasAvailable = isAvail;

				if (isAvail)
				{
					upgradeAvailableButton->SetDisabledImage(upgradeAvailableImage);
				} else {
					upgradeAvailableButton->SetDisabledImage(NULL);
				}
			}
		}

		// NEW: fade-in/fade-out
		if (lastUpdateWasAvailable)
		{
			float curTimeValue = (float)((Timer::getTime() / 10) % 200) / 200.0f;
			upgradeAvailableButton->SetTransparency(35 + int(35.0f * sinf(3.1415f * 2.0f * curTimeValue)));
		}
  }

	void UpgradeAvailableWindow::EffectEvent(OguiEffectEvent *e)
	{
		if(e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
			win->Hide();
	}

}

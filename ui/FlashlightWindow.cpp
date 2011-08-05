
#include "precompiled.h"

#include "FlashlightWindow.h"
#include "CombatSubWindowFactory.h"

#include <assert.h>
#include "Visual2D.h"
#include "uidefaults.h"
#include "../game/Unit.h"
#include "../game/Flashlight.h"
#include "../convert/str2int.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/DHLocaleManager.h"
#include "../ogui/OguiAligner.h"

#include "../util/Debug_MemoryManager.h"

using namespace game;

namespace ui
{

///////////////////////////////////////////////////////////////////////////////

REGISTER_COMBATSUBWINDOW( FlashlightWindow );

///////////////////////////////////////////////////////////////////////////////


  FlashlightWindow::FlashlightWindow(Ogui *ogui, game::Game *game, int player)
  {
    this->ogui = ogui;
    this->game = game;
    this->player = player;

    int xPosition = getLocaleGuiInt("gui_flashlight_position_x", 0);
    int yPosition = getLocaleGuiInt("gui_flashlight_position_y", 0);

    this->win = ogui->CreateSimpleWindow(xPosition, yPosition, getLocaleGuiInt("gui_flashlight_size_x", 0), getLocaleGuiInt("gui_flashlight_size_y", 0), NULL);
		this->win->SetUnmovable();

    this->flashlightBackgroundOnImage = ogui->LoadOguiImage(getLocaleGuiString("gui_flashlight_on_image"));
    this->flashlightBackgroundLowImage = ogui->LoadOguiImage(getLocaleGuiString("gui_flashlight_low_image"));
    this->flashlightBackgroundOffImage = ogui->LoadOguiImage(getLocaleGuiString("gui_flashlight_off_image")); 

    this->fillupImage = ogui->LoadOguiImage(getLocaleGuiString("gui_flashlight_fillup_image"));
    this->fillupLowImage = ogui->LoadOguiImage(getLocaleGuiString("gui_flashlight_fillup_low_image"));

    flashlightBackgroundButton = ogui->CreateSimpleImageButton(win, 0, 0, getLocaleGuiInt("gui_flashlight_size_x", 0), getLocaleGuiInt("gui_flashlight_size_y", 0), NULL, NULL, NULL);
		flashlightBackgroundButton->SetDisabled(true);
		flashlightBackgroundButton->SetDisabledImage(flashlightBackgroundOffImage);

    fillupButton = ogui->CreateSimpleImageButton(win, getLocaleGuiInt("gui_flashlight_fillup_offset_x", 0), getLocaleGuiInt("gui_flashlight_fillup_offset_y", 0), getLocaleGuiInt("gui_flashlight_fillup_size_x", 0), getLocaleGuiInt("gui_flashlight_fillup_size_y", 0), NULL, NULL, NULL);
		fillupButton->SetDisabled(true);
		fillupButton->SetDisabledImage(fillupImage);
 
		win->SetEffectListener(this);

#ifdef PROJECT_SURVIVOR
		OguiAligner::align(win, OguiAligner::WIDESCREEN_FIX_LEFT, ogui);
		OguiAligner::align(flashlightBackgroundButton, OguiAligner::WIDESCREEN_FIX_LEFT, ogui);
		OguiAligner::align(fillupButton, OguiAligner::WIDESCREEN_FIX_LEFT, ogui);
#endif
  }


  FlashlightWindow::~FlashlightWindow()
  {
    if (flashlightBackgroundButton != NULL)
    {
      delete flashlightBackgroundButton;
      flashlightBackgroundButton = NULL;
    }
    if (fillupButton != NULL)
    {
      delete fillupButton;
      fillupButton = NULL;
    }
    if (win != NULL)
    {
			delete win;
			win = NULL;
		}

		delete flashlightBackgroundOnImage;
		delete flashlightBackgroundOffImage;
		delete flashlightBackgroundLowImage;

		delete fillupImage;
		delete fillupLowImage;
  }


	void FlashlightWindow::hide(int fadeTime)
	{
    /*
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
		else
			win->Hide();
    */
    win->Hide();
	}


	void FlashlightWindow::show(int fadeTime)
	{
    /*
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);

		win->Show();
    */
    win->Show();
	}



  void FlashlightWindow::update()
  {
		if (game->gameUI->getFirstPerson(0) == NULL)
			return;

		game::Flashlight *fl = game->gameUI->getFirstPerson(0)->getFlashlight();

		int energyPercentage = 0;
		int recharging = 0;
		bool needsRecharge = false;
		bool flashlightOn = false;
		if (fl != NULL)
		{
			energyPercentage = fl->getFlashlightEnergy();
			recharging = fl->getRechargingAmount();
			needsRecharge = fl->doesNeedRecharge();
			flashlightOn = fl->isFlashlightOn();
		}

		float fillTo = (float)energyPercentage;

		if (fillTo < 0) fillTo = 0;
		if (fillTo > 100) fillTo = 100;
		
		fillupButton->SetClip(0, 0, fillTo, 100);

		if (flashlightOn)
		{
			flashlightBackgroundButton->SetDisabledImage(flashlightBackgroundOnImage);
			fillupButton->SetDisabledImage(fillupImage);
		} else {
			if (needsRecharge)
			{
				flashlightBackgroundButton->SetDisabledImage(flashlightBackgroundLowImage);
				fillupButton->SetDisabledImage(fillupLowImage);
			} else {
				flashlightBackgroundButton->SetDisabledImage(flashlightBackgroundOffImage);
				fillupButton->SetDisabledImage(fillupImage);
			}
		}
  }

	void FlashlightWindow::EffectEvent(OguiEffectEvent *e)
	{
		if(e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
			win->Hide();
	}

}


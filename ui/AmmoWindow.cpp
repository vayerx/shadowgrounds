
#include "precompiled.h"

#include "AmmoWindow.h"

#include <assert.h>
#include "Visual2D.h"
#include "uidefaults.h"
#include "../game/Unit.h"
#include "../convert/str2int.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/DHLocaleManager.h"
#include "../game/Weapon.h"
#include "CombatSubWindowFactory.h"
#include "../util/Debug_MemoryManager.h"
#include "../ogui/OguiAligner.h"

#include <boost/lexical_cast.hpp>

using namespace game;

namespace ui
{

///////////////////////////////////////////////////////////////////////////////
REGISTER_COMBATSUBWINDOW( AmmoWindow );
///////////////////////////////////////////////////////////////////////////////

	AmmoWindow::AmmoWindow() :
		ogui( NULL ),
		game( NULL ),
		clientNum( 0 ),
		win( NULL ),

		fontClip( NULL ),
		fontTotal( NULL ),

		fillupImage( NULL ),
		fillupButton( NULL ),

		ammoBackgroundImage( NULL ),
		ammoBackgroundButton( NULL ),

		ammoTotalBackgroundImage( NULL ),
		ammoTotalBackgroundButton( NULL ),

		ammoAmountText( NULL ),
		ammoTotalAmountText( NULL ),

		lastUpdateValue( 0 ),
		lastUpdateWeapon( 0 ),
		lastUpdateTotalValue( 0 )
	{
	}

	AmmoWindow::AmmoWindow(Ogui *ogui, game::Game *game, int clientNum, bool coop )
	{
		this->ogui = ogui;
		this->game = game;
		this->clientNum = clientNum;

		
		std::string prefix = "gui_ammo_";

		if( coop ) 
		{
			prefix = "gui_ammo_coop" + boost::lexical_cast< std::string >( clientNum ) + "_";
		}

		int xPosition = getLocaleGuiInt( ( prefix + "position_x" ).c_str(), 0);
		int yPosition = getLocaleGuiInt( ( prefix + "position_y" ).c_str(), 0);

		this->win = ogui->CreateSimpleWindow(xPosition, yPosition, getLocaleGuiInt( ( prefix + "size_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "size_y" ).c_str(), 0), NULL);
		this->win->SetUnmovable();

		this->ammoBackgroundImage = ogui->LoadOguiImage(getLocaleGuiString( ( prefix + "clip_image").c_str()));
		this->ammoTotalBackgroundImage = ogui->LoadOguiImage(getLocaleGuiString( ( prefix + "total_image").c_str()));

		this->fillupImage = ogui->LoadOguiImage(getLocaleGuiString( ( prefix + "clip_fillup_image").c_str()));

		ammoBackgroundButton = ogui->CreateSimpleImageButton(win, getLocaleGuiInt( ( prefix + "clip_offset_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "clip_offset_y" ).c_str(), 0), getLocaleGuiInt( ( prefix + "clip_size_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "clip_size_y" ).c_str(), 0), NULL, NULL, NULL);
		ammoBackgroundButton->SetDisabled(true);
		ammoBackgroundButton->SetDisabledImage(ammoBackgroundImage);

		ammoTotalBackgroundButton = ogui->CreateSimpleImageButton(win, getLocaleGuiInt( ( prefix + "total_offset_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "total_offset_y" ).c_str(), 0), getLocaleGuiInt( ( prefix + "total_size_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "total_size_y" ).c_str(), 0), NULL, NULL, NULL);
		ammoTotalBackgroundButton->SetDisabled(true);
		ammoTotalBackgroundButton->SetDisabledImage(ammoTotalBackgroundImage);

		fillupButton = ogui->CreateSimpleImageButton(win, getLocaleGuiInt( ( prefix + "clip_fillup_offset_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "clip_fillup_offset_y" ).c_str(), 0), getLocaleGuiInt( ( prefix + "clip_fillup_size_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "clip_fillup_size_y" ).c_str(), 0), NULL, NULL, NULL);
		fillupButton->SetDisabled(true);
		fillupButton->SetDisabledImage(fillupImage);

		fontClip = ogui->LoadFont( getLocaleGuiString( (prefix + "font_clip" ).c_str() ) );

		ammoAmountText = ogui->CreateTextLabel(win, getLocaleGuiInt( ( prefix + "clip_offset_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "clip_offset_y" ).c_str(), 0), getLocaleGuiInt( ( prefix + "clip_size_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "clip_size_y" ).c_str(), 0), "");
		ammoAmountText->SetFont( fontClip ? fontClip : ui::defaultIngameNumbersBoldFont );

		fontTotal = ogui->LoadFont( getLocaleGuiString( (prefix + "font_total" ).c_str() ) );

		ammoTotalAmountText = ogui->CreateTextLabel(win, getLocaleGuiInt( ( prefix + "total_offset_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "total_offset_y" ).c_str(), 0), getLocaleGuiInt( ( prefix + "total_size_x" ).c_str(), 0), getLocaleGuiInt( ( prefix + "total_size_y" ).c_str(), 0), "");
		ammoTotalAmountText->SetFont( fontTotal ? fontTotal : ui::defaultIngameNumbersBoldSmallFont );

		lastUpdateValue = -1;
		lastUpdateWeapon = -1;
		lastUpdateTotalValue = -1;
		win->SetEffectListener(this);

		int alignment = OguiAligner::WIDESCREEN_FIX_RIGHT;
		if(xPosition < 512)
			alignment = OguiAligner::WIDESCREEN_FIX_LEFT;

#ifdef PROJECT_SURVIVOR
		OguiAligner::align(win, alignment, ogui);
		OguiAligner::align(ammoBackgroundButton, alignment, ogui);
		OguiAligner::align(ammoTotalBackgroundButton, alignment, ogui);
		OguiAligner::align(fillupButton, alignment, ogui);
		OguiAligner::align(ammoAmountText, alignment, ogui);
		OguiAligner::align(ammoTotalAmountText, alignment, ogui);
#endif
	}


	AmmoWindow::~AmmoWindow()
	{
		if (ammoAmountText != NULL)
		{
			delete ammoAmountText;
			ammoAmountText = NULL;
		}
		if (ammoTotalAmountText != NULL)
		{
			delete ammoTotalAmountText;
			ammoTotalAmountText = NULL;
		}
		if (ammoBackgroundButton != NULL)
		{
			delete ammoBackgroundButton;
			ammoBackgroundButton = NULL;
		}
		if (ammoTotalBackgroundButton != NULL)
		{
			delete ammoTotalBackgroundButton;
			ammoTotalBackgroundButton = NULL;
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

		delete ammoBackgroundImage;

		delete fillupImage;

		delete fontClip;
		delete fontTotal;
	}


	void AmmoWindow::hide(int fadeTime)
	{
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEOUT, fadeTime);
		else
			win->Hide();
	}


	void AmmoWindow::show(int fadeTime)
	{
		if(fadeTime)
			win->StartEffect(OGUI_WINDOW_EFFECT_FADEIN, fadeTime);

		win->Show();
	}



	void AmmoWindow::update()
	{
		if (game->gameUI->getFirstPerson( clientNum ) == NULL)
			return;

		Unit *u = game->gameUI->getFirstPerson( clientNum );

		int selWeap = u->getSelectedWeapon();
		int ammoAmount = 0;
		int ammoTotalAmount = 0;
		int maxAmmo = 1;
		if (selWeap != -1
			&& u->getWeaponType(selWeap) != NULL)
		{
			ammoAmount = u->getWeaponAmmoInClip(selWeap);
			maxAmmo = u->getWeaponClipSize(selWeap);
			ammoTotalAmount = u->getWeaponAmmoAmount(selWeap);
			
			// NEW: ... (temp for demo)
			if (ammoAmount > ammoTotalAmount)
			{
				ammoAmount = ammoTotalAmount;
			}
			ammoTotalAmount -= ammoAmount;
			if (ammoTotalAmount < 0)
				ammoTotalAmount = 0;

			// HACK: if shotgun
			if (u->getFireReloadDelay(selWeap) > 0 && u->isClipReloading())
			{
				if (u->getWeaponType(selWeap)->isSingleReloading())
				{
					ammoAmount = ammoAmount - 1;
					if (ammoAmount < 0) ammoAmount = 0;
				//} else {
				//	ammoAmount = 0;
				//  // todo, show "reloading" text.
				}
			}
		}

		float fillTo = 0;
		if (maxAmmo > 0)
		{
			fillTo = 100.0f * ammoAmount / maxAmmo;
		}

		if (ammoAmount != lastUpdateValue || selWeap != lastUpdateWeapon
			|| ammoTotalAmount != lastUpdateTotalValue)
		{
			lastUpdateValue = ammoAmount;
			lastUpdateWeapon = selWeap;
			lastUpdateTotalValue = ammoTotalAmount;

			if (selWeap == -1)
			{
				fillupButton->SetClip(100, 0, 100, 100);
				ammoAmountText->SetText("");
				ammoTotalAmountText->SetText("");
			} else {
				if (fillTo < 0) fillTo = 0;
				if (fillTo > 100) fillTo = 100;

				fillupButton->SetClip(100 - fillTo, 0, 100, 100);

				if (maxAmmo == 0)
				{
					ammoTotalAmountText->SetText("");
					if (ammoTotalAmount > 9999)
					{
						ammoAmountText->SetText("");
					} else {
						ammoAmountText->SetText(int2str(ammoTotalAmount));
					}
				} else {
					ammoAmountText->SetText(int2str(ammoAmount));
					if (ammoTotalAmount > 9999)
					{
						ammoTotalAmountText->SetText("");
					} else {
						ammoTotalAmountText->SetText(int2str(ammoTotalAmount));
					}
				}
			}
		}
	}

	void AmmoWindow::EffectEvent(OguiEffectEvent *e)
	{
		if(e->eventType == OguiEffectEvent::EVENT_TYPE_FADEDOUT)
			win->Hide();
	}

}

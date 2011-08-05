
#ifndef AMMOWINDOW_H
#define AMMOWINDOW_H

#include "../ogui/Ogui.h"
#include "ICombatSubWindow.h"

namespace game
{
	class Game;
}

namespace ui
{

	/**
	 *
	 * Player ammo on-screen status window.
	 *
	 * @version 1.0, 23.12.2004
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see CombatWindow
	 *
	 */

	class AmmoWindow : public ICombatSubWindow, private IOguiEffectListener
	{
	public:

		AmmoWindow();
		AmmoWindow(Ogui *ogui, game::Game *game, int clientNum, bool coop = false );
		virtual ~AmmoWindow();

		//void moveTo(int x, int y);

		virtual void hide(int fadeTime = 0);
		virtual void show(int fadeTime = 0);
		virtual void update();
		virtual void EffectEvent(OguiEffectEvent *e);

	private:
		Ogui *ogui;
		game::Game *game;
		int clientNum;
		OguiWindow *win;

		IOguiFont* fontClip;
		IOguiFont* fontTotal;

		IOguiImage *fillupImage;
		OguiButton *fillupButton;

		IOguiImage *ammoBackgroundImage;
		OguiButton *ammoBackgroundButton;

		IOguiImage *ammoTotalBackgroundImage;
		OguiButton *ammoTotalBackgroundButton;

		OguiTextLabel *ammoAmountText;
		OguiTextLabel *ammoTotalAmountText;

		int lastUpdateValue;
		int lastUpdateWeapon;
		int lastUpdateTotalValue;
	};

}


#endif

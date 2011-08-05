
#ifndef WEAPONWINDOW_H
#define WEAPONWINDOW_H


#include "../ogui/Ogui.h"
#include "ICombatSubWindow.h"

#include <string>
#include <vector>

#ifdef PROJECT_SURVIVOR
	#define WEAPONWINDOW_MAX_WEAPONS 5
#else
	#define WEAPONWINDOW_MAX_WEAPONS 10
#endif

namespace game
{
	class Game;
}

namespace ui
{

	/**
	 *
	 * Player weapon on-screen status window.
	 *
	 * @version 1.0, 23.12.2004
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see CombatWindow
	 *
	 */

	class WeaponWindow : public ICombatSubWindow, private IOguiEffectListener
	{
	public:
		WeaponWindow();
		WeaponWindow(Ogui *ogui, game::Game *game, int clientNum, bool cooperative = false, const std::string& profile = "" );
		virtual ~WeaponWindow();

		//void moveTo(int x, int y);

		virtual void hide(int fadeTime = 0);
		virtual void show(int fadeTime = 0);
		virtual void update();
		virtual void EffectEvent(OguiEffectEvent *e);

		void forceUpdate();

	private:
		Ogui *ogui;
		game::Game *game;
		int clientNum;
		OguiWindow *win;

		IOguiImage *weaponImages[WEAPONWINDOW_MAX_WEAPONS];
		OguiButton *weaponIconButton;

		OguiWindow *listWin;

		IOguiImage *weaponListImage;
		IOguiImage *weaponListSelectedImage;
		IOguiImage *weaponListDisabledImage;
		OguiButton *weaponListButtons[WEAPONWINDOW_MAX_WEAPONS];

		std::vector< std::string >	weaponNames;

		IOguiFont*		weaponSelectionFont;
		IOguiFont*		playerNameFont;
		OguiTextLabel*	weaponSelectionText;
		//int selectionTextTimeLeft;
		OguiButton*		labelButton; // used for labels like Player #1

		int				hideWeaponSelection;
		int				hideWeaponSelectionFadeLength;
		int				lastUpdateWeaponText;

		std::string profile;
		bool coop;


		int lastUpdateValue;
		int lastUpdateWeaponAmount;
	};

}


#endif

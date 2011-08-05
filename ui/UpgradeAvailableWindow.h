
#ifndef UPGRADEAVAILABLEWINDOW_H
#define UPGRADEAVAILABLEWINDOW_H

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
   * Player upgrade available on-screen status window.
   *
   * @version 1.0, 28.12.2004
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see CombatWindow
   *
   */

  class UpgradeAvailableWindow : public ICombatSubWindow, private IOguiEffectListener
  {
  public:

    UpgradeAvailableWindow(Ogui *ogui, game::Game *game, int player);
    ~UpgradeAvailableWindow();

		void hide(int fadeTime = 0);
		void show(int fadeTime = 0);
		void update();
		void EffectEvent(OguiEffectEvent *e);

  private:
    Ogui *ogui;
    game::Game *game;
    int player;

    OguiWindow *win;

    IOguiImage *upgradeAvailableImage;
		OguiButton *upgradeAvailableButton;

		int lastUpdateValue;
		bool lastUpdateWasAvailable;
  };

}


#endif

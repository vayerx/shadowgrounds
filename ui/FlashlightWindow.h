
#ifndef FLASHLIGHTWINDOW_H
#define FLASHLIGHTWINDOW_H

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
   * Flashlight on-screen status window.
   *
   * @version 1.0, 23.12.2004
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see CombatWindow
   *
   */

	class FlashlightWindow: public ICombatSubWindow, private IOguiEffectListener
  {
  public:

    FlashlightWindow(Ogui *ogui, game::Game *game, int player);
    ~FlashlightWindow();

    //void moveTo(int x, int y);

		void hide(int fadeTime = 0);
		void show(int fadeTime = 0);
		void update();
		void EffectEvent(OguiEffectEvent *e);

  private:
    Ogui *ogui;
    game::Game *game;
    int player;
    OguiWindow *win;

    IOguiImage *fillupImage;
    IOguiImage *fillupLowImage;
    OguiButton *fillupButton;

    IOguiImage *flashlightBackgroundOnImage;
    IOguiImage *flashlightBackgroundLowImage;
    IOguiImage *flashlightBackgroundOffImage;   
		OguiButton *flashlightBackgroundButton;
  };

}


#endif

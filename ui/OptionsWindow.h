
#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include "../game/gamedefs.h"
#include "../ogui/IOguiButtonListener.h"

namespace game
{
  class Game;
}

class Ogui;
class OguiWindow;
class OguiButton;
class OguiTextLabel;

namespace ui
{
  class OptionsWindow : public IOguiButtonListener
	{
		public:
			OptionsWindow(game::Game *game, Ogui *ogui, int player);

			~OptionsWindow();
			
	    virtual void CursorEvent(OguiButtonEvent *eve);

		private:
			Ogui *ogui;
			game::Game *game;
			int player;
			OguiWindow *win;

			OguiTextLabel *playerTextLabel[MAX_PLAYERS_PER_CLIENT];
			OguiButton *playerEnabledButton[MAX_PLAYERS_PER_CLIENT];
			OguiButton *playerControllerButton[MAX_PLAYERS_PER_CLIENT];
			OguiTextLabel *autoadjustTextLabel;
			OguiTextLabel *difficultyTextLabel;
			OguiButton *difficultyButton;
			OguiButton *autoadjustButton;

			void refresh();

	};

}

#endif


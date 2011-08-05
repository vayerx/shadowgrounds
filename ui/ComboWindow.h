#ifndef INC_COMBOWINDOW_H
#define INC_COMBOWINDOW_H

#include "ICombatSubWindow.h"

namespace game {
	class Game;
}

class Ogui;

namespace ui { 

class ComboWindow : public ICombatSubWindow
{
public:
	ComboWindow( Ogui* ogui, game::Game* game, int player_num );
	~ComboWindow();

	void hide( int time = 0 );
	void show( int time = 0 );
	void update();

private:
	class ComboWindowImpl;
	ComboWindowImpl* impl;

};

}

#endif

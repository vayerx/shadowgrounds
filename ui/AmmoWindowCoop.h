#ifndef INC_AMMOWINDOWCOOP_H
#define INC_AMMOWINDOWCOOP_H

#include <vector>
#include "AmmoWindow.h"

class Ogui;
namespace game
{
  class Game;
}

namespace ui {

class AmmoWindowCoop : public AmmoWindow
{
public:
	AmmoWindowCoop( Ogui *ogui, game::Game *game, int numOfPlayers  );
	~AmmoWindowCoop();

	void hide(int fadeTime = 0);
	void show(int fadeTime = 0);
	void update();
	void EffectEvent(OguiEffectEvent *e);

private:
	std::vector< AmmoWindow* > windows;
};

} // end of namespace ui

#endif

#ifndef INC_WEAPONWINDOWCOOP_H
#define INC_WEAPONWINDOWCOOP_H

#include <vector>
#include "WeaponWindow.h"

class Ogui;
namespace game
{
  class Game;
}


namespace ui {

class WeaponWindowCoop : public WeaponWindow
{
public:
	WeaponWindowCoop( Ogui* ogui, game::Game* game, int numOfPlayers );
	~WeaponWindowCoop();

	void hide(int fadeTime = 0);
	void show(int fadeTime = 0);
	void update();
	void EffectEvent(OguiEffectEvent *e);

private:
	std::vector< WeaponWindow* > windows;
};

} // end of namespace ui


#endif

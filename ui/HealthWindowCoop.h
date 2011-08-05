#ifndef INC_HEALTHWINDOWCOOP_H
#define INC_HEALTHWINDOWCOOP_H

#include <vector>
#include "HealthWindow.h"

class Ogui;
namespace game
{
  class Game;
}


namespace ui {

class HealthWindowCoop : public HealthWindow
{
public:
	HealthWindowCoop( Ogui* ogui, game::Game* game, int numOfPlayers );
	~HealthWindowCoop();

	void hide(int fadeTime = 0);
	void show(int fadeTime = 0);
	void update();
	void updateCurve();
	void EffectEvent(OguiEffectEvent *e);

	// void setHealthTextMultiplier( int player_num, float m );
private:
	std::vector< HealthWindow* > windows;
};

} // end of namespace ui

#endif

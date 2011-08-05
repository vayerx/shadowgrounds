#ifndef INC_VEHICLEWINDOW_H
#define INC_VEHICLEWINDOW_H

class Ogui;
namespace game {
	class Game;
	class Unit;
}

namespace ui {

class VehicleWindow 
{
public:
	VehicleWindow( Ogui *ogui, game::Game *game, game::Unit *unit, const char *params );
	~VehicleWindow();

	void update();
	void setCombatWindowVisibility();

	void hide();
	void show();

private:
	class VehicleWindowImpl;
	VehicleWindowImpl* impl;
}; 

} // end of namespace ui

#endif

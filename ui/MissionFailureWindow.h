#ifndef INC_MISSIONFAILUREWINDOW_H
#define INC_MISSIONFAILUREWINDOW_H

namespace game {
	class Game;
}

class Ogui;

namespace ui {

class MissionFailureWindow
{
public:
	MissionFailureWindow(Ogui *ogui, game::Game *game);
	~MissionFailureWindow();
	
	bool closeMePlease() const; 
	bool shouldRestart() const;

private:
	class MissionFailureWindowImpl;
	MissionFailureWindowImpl *impl;
};

}

#endif

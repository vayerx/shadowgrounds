#ifndef INC_MISSIONSELECTIONWINDOW_H
#define INC_MISSIONSELECTIONWINDOW_H

namespace game {
	class Game;
}

class Ogui;

namespace ui {

class MissionSelectionWindow
{
public:
	MissionSelectionWindow( Ogui *ogui, game::Game *game );
	~MissionSelectionWindow();
	
	// bool AllowLoading() const;
	// bool CloseMePlease() const; 
	
	void AddMissionButton( const std::string& id );
	void Update( int update_l );
	

private:
	class MissionSelectionWindowImpl;
	MissionSelectionWindowImpl* impl;
};

} // end of namespace ui

#endif

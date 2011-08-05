#ifndef INC_SCOREWINDOW_H
#define INC_SCOREWINDOW_H

namespace game {
	class Game;
}

class Ogui;

namespace ui {

class ScoreWindow
{
public:
	ScoreWindow( Ogui *ogui, game::Game *game, int player );
	~ScoreWindow();
	
	bool AllowLoading() const;
	bool CloseMePlease() const; 
	bool shouldRestart() const;

	void PleaseClose();

	void Update( int update_l );
	
	int getPlayer();
	void setPlayer(int player);

private:
	class ScoreWindowImpl;
	ScoreWindowImpl* impl;
};

} // end of namespace ui

#endif

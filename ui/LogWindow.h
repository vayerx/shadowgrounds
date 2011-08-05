#ifndef INC_LOGWINDOW_H
#define INC_LOGWINDOW_H

class Ogui;

namespace game
{
	class Game;
}

namespace ui {

class LogManager;
class LogWindowImpl;

class LogWindow
{
public:
	LogWindow( game::Game &game, Ogui &ogui, LogManager& manager );
	~LogWindow();

	void show();
	void hide();

	bool isVisible() const;

	// updates effects
	void update( int msecsDelta );

private:
	LogWindowImpl* impl;
};

} // end of namespace ui

#endif

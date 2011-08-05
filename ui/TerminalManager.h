#ifndef INC_TERMINALMANAGER_H
#define INC_TERMINALMANAGER_H

#include <string>

class Ogui;

namespace game
{
	class Game;
}

namespace ui
{

class TerminalManagerImpl;

class TerminalManager
{
public:
	TerminalManager( Ogui* ogui, game::Game* game );
	~TerminalManager();

	void openTerminalWindow( const std::string& name );
	void closeTerminalWindow();

	void update();

	bool isWindowOpen() const;

private:

	TerminalManagerImpl* impl;
};

} // end of namespace ui

#endif

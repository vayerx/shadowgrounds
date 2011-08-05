#ifndef INC_ITEMWINDOWUPDATOR_H
#define INC_ITEMWINDOWUPDATOR_H

#include <string>
#include <list>

#include "ICombatSubWindow.h"

namespace game 
{
	class Game;
}

namespace ui {

///////////////////////////////////////////////////////////////////////////////

class ItemWindow;

class ItemWindowUpdator : public ICombatSubWindow
{
public:
	ItemWindowUpdator( game::Game* game, ItemWindow* itemWindow );
	~ItemWindowUpdator();

	virtual void hide( int time = 0 );
	virtual void show( int time = 0 );
	virtual void update();
	

private:

	void doUpdate( ItemWindow* itemwindow );
	
	struct ItemVisual
	{
		std::string watch_variable;
		std::string image;
		std::string location;
		int			shown;
	};
	//.........................................................................

	game::Game* game;
	
	int updateInFrames;
	int currentFrame;
	
	ItemWindow* itemWindow;
	//.........................................................................

	std::list< ItemVisual > items;
};

///////////////////////////////////////////////////////////////////////////////

} // end of namespace ui

#endif

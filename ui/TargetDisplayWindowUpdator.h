#ifndef INC_TARGETDISPLAYWINDOWUPDATOR_H
#define INC_TARGETDISPLAYWINDOWUPDATOR_H

#include <list>
#include "../game/IItemListener.h"
#include "ICombatSubWindow.h"

namespace game
{
	class Game;
	class Item;
	class Unit;
}

namespace ui {

const int risingMessageStyle = 0;

class TargetDisplayWindow;

class TargetDisplayWindowUpdator : public ICombatSubWindow, public game::IItemListener
{
public:
	TargetDisplayWindowUpdator( game::Game* game, TargetDisplayWindow* window );
	~TargetDisplayWindowUpdator();

	virtual void hide( int time = 0 );
	virtual void show( int time = 0 );
	virtual void update();
	
	void risingMessage( game::Unit* unit, const std::string& text, int style = risingMessageStyle );
	void onDestruction( game::Item* item );

private:
	void updateUpdatables();

	game::Game* game;
	
	int			currentFrame;
	const int	updateTargets;
	const int	removeUnnessary;
	TargetDisplayWindow* window;

	std::list< game::Item* > itemsToBeUpdated;
	std::list< game::Unit* > unitsToBeUpdated;
	std::list< game::Unit* > risingMessages;
};

} // end of namespace ui
#endif

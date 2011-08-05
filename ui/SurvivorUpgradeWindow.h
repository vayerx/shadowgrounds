#ifndef INC_SURVIVORUPGRADEWINDOW_H
#define INC_SURVIVORUPGRADEWINDOW_H

class Ogui;
namespace game {
	class Game;
	class GameUI;
	class Unit;
}

namespace ui {

class SurvivorUpgradeWindow 
{
public:
	SurvivorUpgradeWindow( Ogui *ogui, game::Game *game, game::Unit *unit );
	~SurvivorUpgradeWindow();

	//=========================================================================
	// interface me?

	void applyUpgrades();
	void undoUpgrades();

	void effectUpdate(int msecTimeDelta);
	void raise();

	void fadeOut();
	int getFadeInTime() const;
	int getFadeOutTime() const;
	int isVisible() const;

	static void preloadTextures(game::GameUI *gameUI);

	//=========================================================================

private:
	class SurvivorUpgradeWindowImpl;
	SurvivorUpgradeWindowImpl* impl;
}; 

} // end of namespace ui

#endif

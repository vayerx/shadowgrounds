#ifndef INC_INGAMEGUITABS_H
#define INC_INGAMEGUITABS_H

#include "../game/GameUI.h"

class Ogui;

namespace game
{
	class Game;
}

namespace ui {

class IngameGuiTabsImpl;

class IngameGuiTabs 
{
public:
	/*
	enum IngameWindows
	{
		map = 0,
		upgrade = 1,
		log = 2
	};*/
	typedef game::GameUI::WINDOW_TYPE IngameWindows;

	IngameGuiTabs( Ogui *ogui, game::Game *game );
	~IngameGuiTabs();

	void show();
	void hide();
	
	void raise();
	void update(int ms);

	bool isVisible() const;

	void setActive( IngameWindows current_active );

private:
	
	IngameGuiTabsImpl* impl;
};

} // end of namespace ui

#endif

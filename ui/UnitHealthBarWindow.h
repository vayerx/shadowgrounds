#ifndef INC_UNITHEALTHBARWINDOW_H
#define INC_UNITHEALTHBARWINDOW_H

#include "ICombatSubWindow.h"
#include "../ogui/Ogui.h"

class OguiSlider;

namespace game
{
	class Game;
	class Unit;
}

namespace ui {

class GenericBarWindow;

class UnitHealthBarWindow : public ICombatSubWindow,  private IOguiEffectListener
{
public:
	UnitHealthBarWindow( Ogui* ogui, game::Game* game, int p );
	~UnitHealthBarWindow();

	void hide(int fadeTime = 0);
	void show(int fadeTime = 0);
	void update();
	void EffectEvent(OguiEffectEvent *e);

	void setUnit( game::Unit* unit );

	void setFlashing(int amount);
private:
	/*Ogui*		ogui;
	game::Game* game;
	OguiWindow* win;

	OguiSlider* slider;

	game::Unit*	unit;
	*/
	game::Game* game;
	GenericBarWindow* barWindow;
	GenericBarWindow* flashingWindow;

	float flashingAmount;
	bool flashing;
	int lastFlash;
	game::Unit *unit;
};
	
} // end of namespace ui

#endif

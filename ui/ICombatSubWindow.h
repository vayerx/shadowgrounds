#ifndef INC_ICOMBATSUBWINDOW_H
#define INC_ICOMBATSUBWINDOW_H

namespace ui {

class ICombatSubWindow
{
public:
	ICombatSubWindow() { }
	virtual ~ICombatSubWindow() { }

	virtual void hide( int time = 0 ) = 0;
	virtual void show( int time = 0 ) = 0;
	virtual void update() = 0;

	virtual void updateAnimation() { }
};
	
} // end of namespace ui
#endif

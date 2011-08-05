#ifndef INC_TARGETDISPLAYWINDOW_H
#define INC_TARGETDISPLAYWINDOW_H

#include <map>
#include <string>

#include "TargetDisplayWindowButton.h"

class Ogui;
class OguiButton;
class OguiWindow;
class OguiEffectEvent;

namespace game
{
	class Game;
}

namespace ui {

///////////////////////////////////////////////////////////////////////////////

class TargetDisplayButtonManager;

///////////////////////////////////////////////////////////////////////////////

// Displays the square on top of a game target
class TargetDisplayWindow
{
public:
	TargetDisplayWindow( Ogui *ogui, game::Game *game, int player = 0 );
	~TargetDisplayWindow();

	bool setRisingText( void* p, int x, int y, int w, int h, float distance = 0.0f, int style = 0 );

	bool setRect( void* p, int x, int y, int w, int h, float distance = 0.0f, int style = 0 );
	void updateRect( void *p );
	void clearRect( void *p );
	void setText( void* p, const std::string& text );
	bool hasEnded( void* p );
	bool isAniOver( void* p );
	int timeActive( void* p );
	void setSliderValue( void *p, float v, float scale );
	void hideRest();
	void removeRest();

	inline TargetDisplayButtonManager *getManager(void) { return manager; }

	void hide( int fadeTime = 0 );
	void show( int fadeTime = 0 );
		
	void EffectEvent( OguiEffectEvent *e );

private:
	int maxAmountOnScreen;

	std::map< void*, TargetDisplayWindowButton* > buttons;

	OguiWindow* window;
	Ogui*		ogui;
	
	TargetDisplayButtonManager* manager;

	unsigned int	currentTicks;
};


} // end of namespace ui

#endif

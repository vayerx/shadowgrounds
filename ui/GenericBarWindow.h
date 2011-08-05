#ifndef INC_GENERICBARWINDOW_H
#define INC_GENERICBARWINDOW_H

#include "../ogui/Ogui.h"
#include "ICombatSubWindow.h"
#include <string>

class OguiSlider;
class IOguiFont;

namespace game
{
	class Game;
	class Unit;
}

namespace ui {

///////////////////////////////////////////////////////////////////////////////

class IGenericBarWindowUpdator
{
public:
	virtual ~IGenericBarWindowUpdator() { }

	// returns a slider value between 1 and 0
	virtual float update() = 0;
};

///////////////////////////////////////////////////////////////////////////////

class GenericBarWindow : public ICombatSubWindow, private IOguiEffectListener
{
public:
	GenericBarWindow( Ogui* ogui, game::Game* game, int player );
	~GenericBarWindow();

	void loadDataFromLocales( const std::string& locale_name );

	
	void hide( int fadeTime );
	void show( int fadeTime );
	void update();
	
	// NOTE! deletes the given updator, when it's done with it
	void setUpdator( IGenericBarWindowUpdator* up );

	void EffectEvent( OguiEffectEvent *e );

	void moveBy(int x, int y);
	void move(int x, int y);
	void resize(int x, int y);
	void getWindowRect(int &x, int &y, int &w, int &h);

	inline bool isHidden() const { return reallyHidden; }

	int getFadeOffTime() const { return fadeOffTime; }
	int getFadeOutTime() const { return fadeOutTime; }
	int getFadeInTime()  const { return fadeInTime; }

	void setValue( float value );
	void raise();

private:

	void fadeHide( int fadeTime );
	void fadeShow( int fadeTime );

	void Release();

	int getTime() const;

	Ogui*		ogui;
	game::Game* game;
	int player;

	OguiWindow* win;
	OguiWindow* win_hidden;
	OguiSlider* slider;
	IGenericBarWindowUpdator* updator;

	std::vector<OguiButton*> decorations;
	std::vector<IOguiFont*> decorationfonts;
	
	bool isOfFadingType;
	float sliderValue;
	int	lastTime;
	bool hidden;
	bool reallyHidden;

	std::string messageOnHide;
	std::string messageOnHideStyle;
	std::string messageOnShow;
	std::string messageOnShowStyle;


	int fadeOffTime;
	int fadeOutTime;
	int fadeInTime;

	float valueStart;
	float valueScale;

	int showTextOffsetX;
	int showTextOffsetY;
	int hideTextOffsetX;
	int hideTextOffsetY;
};

///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui
#endif

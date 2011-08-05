
#include "precompiled.h"

#include <stdlib.h> // for NULL

#include "MissionFailureWindow.h"

#include "../ogui/Ogui.h"
#include "../ogui/OguiWindow.h"
#include "../ogui/OguiFormattedText.h"
#include "../ogui/OguiLocaleWrapper.h"
#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/GameProfiles.h"
#include "../game/GameStats.h"
#include "../game/scripting/GameScripting.h"
#include "../game/DHLocaleManager.h"
#include "../util/StringUtil.h"
#include "../game/options/options_gui.h"
using namespace game;

namespace ui {

class MissionFailureWindow::MissionFailureWindowImpl : private IOguiButtonListener
{
public:
	bool close;
	bool restart;

	Ogui*		ogui;
	game::Game*	game;

	OguiLocaleWrapper	oguiLoader;
	OguiWindow*	win;	
	OguiButton* closeButton;
	OguiButton* restartButton;
	OguiFormattedText* textArea;

public:
	MissionFailureWindowImpl(Ogui *ogui, game::Game *game) : ogui(ogui), game(game), oguiLoader(ogui)
	{
		close = false;
		restart = false;

		win = oguiLoader.LoadWindow( "missionfailurewindow" );
		closeButton = oguiLoader.LoadButton( "closebutton", win, 0 );
		closeButton->SetListener(this);
		restartButton = oguiLoader.LoadButton( "restartbutton", win, 0 );
		restartButton->SetListener(this);
		textArea = oguiLoader.LoadFormattedText( "textarea", win, 0 );
	}

	~MissionFailureWindowImpl()
	{
		delete closeButton;
		delete restartButton;
		delete textArea;
		delete win;
	}

	virtual void CursorEvent( OguiButtonEvent *eve )
	{
		if (eve->eventType == OguiButtonEvent::EVENT_TYPE_CLICK)
		{
			if(eve->triggerButton == closeButton)
			{
				close = true;
				restart = false;
			}
			else if(eve->triggerButton == restartButton)
			{
				close = true;
				restart = true;
			}
		}
	}

	bool closeMePlease() const
	{
		return close;
	}

	bool shouldRestart() const
	{
		return restart;
	}
};

MissionFailureWindow::MissionFailureWindow(Ogui *ogui, game::Game *game)
{
	impl = new MissionFailureWindowImpl(ogui, game);
}

MissionFailureWindow::~MissionFailureWindow()
{
	delete impl;
}

bool MissionFailureWindow::closeMePlease() const
{
	return impl->closeMePlease();
}

bool MissionFailureWindow::shouldRestart() const
{
	return impl->shouldRestart();
}
}
